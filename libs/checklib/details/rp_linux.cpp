#include "rp_linux.h"
#include "../rp_consts.h"
#include "../checklib_exception.h"
#include "../timer_service.h"

#include <exception>
#include <sstream>
#include <fstream>
#include <string>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <boost/lambda/lambda.hpp>
#include <boost/chrono.hpp>
#include <boost/filesystem.hpp>

#include <errno.h>

const int checklib::details::RestrictedProcessImpl::sTimerDuration = 100;

checklib::details::RestrictedProcessImpl::RestrictedProcessImpl()
	: mTimer(TimerService::instance()->io_service())
{
	mTicks = static_cast<float>(sysconf(_SC_CLK_TCK));

	mStandardInput = ss::Stdin;
	mStandardOutput = ss::Stdout;
	mStandardError = ss::Stderr;

	reset();
}

checklib::details::RestrictedProcessImpl::~RestrictedProcessImpl()
{
	doFinalize();
}

std::string checklib::details::RestrictedProcessImpl::getProgram() const
{
	return mProgram;
}

void checklib::details::RestrictedProcessImpl::setProgram(const std::string &program)
{
	mProgram = program;
}

const std::vector<std::string> &checklib::details::RestrictedProcessImpl::getParams() const
{
	return mParams;
}

void checklib::details::RestrictedProcessImpl::setParams(const std::vector<std::string> &params)
{
	mParams = params;
}

std::string checklib::details::RestrictedProcessImpl::currentDirectory() const
{
	return mCurrentDirectory;
}

void checklib::details::RestrictedProcessImpl::setCurrentDirectory(const std::string &directory)
{
	mCurrentDirectory = directory;
}

bool checklib::details::RestrictedProcessImpl::is_running() const
{
	return mIsRunning.load();
}

// Запуск процесса
void checklib::details::RestrictedProcessImpl::start()
{
	if(is_running()) return;

	auto createPipe = [](Pipe *p)
	{
		int pp[2];
		if(pipe(pp) == -1) throw Exception("Cannot create pipe");
		p[0] = Pipe(pp[0]);
		p[1] = Pipe(pp[1]);
	};

	Pipe inputPipe[2], outputPipe[2], errorPipe[2];

	if(mStandardInput == ss::Interactive)
	{
		createPipe(inputPipe);
	}
	if(mStandardOutput == ss::Interactive)
	{
		createPipe(outputPipe);
	}
	if(mStandardError == ss::Interactive)
	{
		createPipe(errorPipe);
	}
	// Пайп для проверки ошибки вызова exec
	Pipe checkPipe[2];
	createPipe(checkPipe);

	// Устанавливаем флаг автоматического закрытия pipe при вызове exec
	fcntl(checkPipe[1].pipe(), F_SETFD, fcntl(checkPipe[1].pipe(), F_GETFD) | FD_CLOEXEC);

	mChildPid = fork();
	if(mChildPid == -1) throw CannotStartProcess(mProgram, "Cannot be forked");
	if(mChildPid == 0)
	{
		// Дочерний процесс. Перенаправляем потоки, задаем лимиты и запускаем
		// Деструкторы в этой ветке вызваны не будут. Поэтому вызываем очистку руками

		auto writeMsgAndExit = [checkPipe](const std::string &msg)
		{
			write(checkPipe[1].pipe(), msg.c_str(), msg.length());
			close(checkPipe[1].pipe());
			exit(EXIT_FAILURE);
		};

		close(checkPipe[0].pipe());

		if(mStandardInput != ss::Stdin)
		{
			int d;

			if(mStandardInput == ss::Interactive)
			{
				d = inputPipe[0].pipe();
				close(inputPipe[1].pipe());
			}
			else
			{
				d = open(mStandardInput.c_str(), O_RDONLY);
				if(d == -1) writeMsgAndExit("1" + mStandardInput);
			}
			dup2(d, 0);
			close(d);
		}
		if(mStandardOutput != ss::Stdout)
		{
			int d;

			if(mStandardOutput == ss::Interactive)
			{
				d = outputPipe[1].pipe();
				close(outputPipe[0].pipe());
			}
			else
			{
				d = open(mStandardOutput.c_str(), O_TRUNC | O_CREAT | O_WRONLY,
				         S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
				if(d == -1) writeMsgAndExit("1" + mStandardOutput);
			}
			dup2(d, 1);
			close(d);
		}
		if(mStandardError != ss::Stderr)
		{
			int d;

			if(mStandardError == ss::Interactive)
			{
				d = errorPipe[1].pipe();
				close(errorPipe[0].pipe());
			}
			else
			{
				d = open(mStandardError.c_str(), O_TRUNC | O_CREAT | O_WRONLY,
				         S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
				if(d == -1) writeMsgAndExit("1" + mStandardError);
			}
			dup2(d, 2);
			close(d);
		}

		if(mLimits.useTimeLimit)
		{
			rlimit limit;
			limit.rlim_cur = limit.rlim_max = mLimits.timeLimit / 1000 + 2;
			setrlimit(RLIMIT_CPU, &limit);
		}
		if(mLimits.useMemoryLimit)
		{
			rlimit limit;
			limit.rlim_cur = limit.rlim_max = mLimits.memoryLimit;
			setrlimit(RLIMIT_DATA, &limit);
		}

		boost::filesystem::path programPath(mProgram);
		if(!mCurrentDirectory.empty())
		{
			chdir(mCurrentDirectory.c_str());
		}

		char **args = new char*[mParams.size() + 2];
		args[0] = new char[programPath.native().length() + 1];
		strcpy(args[0], programPath.c_str());

		for(size_t i = 0; i != mParams.size(); i++)
		{
			args[i + 1] = new char[mParams[i].length() + 1];
			strcpy(args[i + 1], mParams[i].c_str());
		}
		args[mParams.size() + 1] = 0;

		execv(programPath.native().c_str(), args);

		writeMsgAndExit("0");
	}
	else
	{
		// Родительский процесс, задаем параметры, необходимые для слежения за дочерним

		close(checkPipe[1].pipe());

		if(mStandardInput == ss::Interactive) mInputPipe = inputPipe[1];
		if(mStandardOutput == ss::Interactive) mOutputPipe = outputPipe[0];
		if(mStandardError == ss::Interactive) mErrorPipe = errorPipe[0];

		std::string msg;
		while(1)
		{
			char msg_c[100];
			int count = read(checkPipe[0].pipe(), msg_c, 99);
			if(count <= 0) break;
			msg_c[count] = 0;
			msg += msg_c;
		}
		if(!msg.empty())
		{
			// WARNING: Без kill почему-то не работает ожидание завершения, несмотря на вызов exit в потомке
			kill(mChildPid, SIGKILL);
			waitpid(mChildPid, nullptr, 0);
			if(msg[0] == '0') throw CannotStartProcess(mProgram);
			if(msg[0] == '1') throw CannotOpenFile(msg.substr(msg.length() - 1));
			throw std::logic_error("Message from child has invalid code");
		}

		mTimer.expires_from_now(boost::posix_time::milliseconds(sTimerDuration));
		mTimer.async_wait(boost::bind(&checklib::details::RestrictedProcessImpl::timerHandler,
									  boost::ref(*this), boost::lambda::_1));

		mOldCPUTime = -1;
		mProcessStatus.store(psRunning);
		mIsRunning.store(true);
	}
}

// Завершает процесс вручную. Тип завершения становится etTerminated
void checklib::details::RestrictedProcessImpl::terminate()
{
	if(is_running())
	{
		kill(mChildPid, SIGKILL);
		mProcessStatus.store(psTerminated);
	}
}

// Ждать завершения процесса
void checklib::details::RestrictedProcessImpl::wait()
{
	wait(INT_MAX);
}

// Ждать завершения процесса не более чем @param миллисекунд.
// true если программа завершилась (сама или от превышения лимитов), false - если таймаут ожидания
bool checklib::details::RestrictedProcessImpl::wait(int milliseconds)
{
	boost::chrono::system_clock::time_point start = boost::chrono::system_clock::now();

	const int resolution = 10;
	while(1)
	{
		if(!is_running()) return true;
		boost::chrono::duration<double> msec = (boost::chrono::system_clock::now() - start) * 1000;
		if(msec.count() > milliseconds) break;
		boost::this_thread::sleep(boost::posix_time::milliseconds(resolution));
	}

	return false;
}

// Код возврата.
int checklib::details::RestrictedProcessImpl::exit_code() const
{
	return mExitCode;
}

// Тип завершения программы
checklib::ProcessStatus checklib::details::RestrictedProcessImpl::processStatus() const
{
	return mProcessStatus.load();
}

// Пиковое значение потребляемой памяти
int checklib::details::RestrictedProcessImpl::peak_memory_usage()
{
	return mPeakMemoryUsage.load();
}

// Сколько процессорного времени израсходовал процесс
int checklib::details::RestrictedProcessImpl::cpu_time()
{
	return mCPUTime.load();
}

checklib::Limits checklib::details::RestrictedProcessImpl::getLimits() const
{
	return mLimits;
}

void checklib::details::RestrictedProcessImpl::setLimits(const Limits &limits)
{
	mLimits = limits;
}

// Перенаправить стандартный поток ввода в указанный файл.
// Если stdin, то перенаправления не происходит.
void checklib::details::RestrictedProcessImpl::redirectStandardInput(const std::string &fileName)
{
	mStandardInput = fileName;
}

// Перенаправить стандартный поток вывода в указанный файл.
// Если stdout, то перенаправления не происходит.
void checklib::details::RestrictedProcessImpl::redirectStandardOutput(const std::string &fileName)
{
	mStandardOutput = fileName;
}

// Перенаправить стандартный поток ошибок в указанный файл.
// Если stderr, то перенаправления не происходит.
void checklib::details::RestrictedProcessImpl::redirectStandardError(const std::string &fileName)
{
	mStandardError = fileName;
}

bool checklib::details::RestrictedProcessImpl::sendDataToStandardInput(const std::string &data, bool newLine)
{
	if(!is_running() || mStandardInput != ss::Interactive) return false;

	auto count = write(mInputPipe.pipe(), data.c_str(), data.length());
	if(count == -1) return false;
	if(newLine)
	{
		const char c = '\n';
		write(mInputPipe.pipe(), &c, 1);
	}
	return true;
}

bool checklib::details::RestrictedProcessImpl::getDataFromStandardOutput(std::string &data)
{
	if(!is_running() || mStandardOutput != ss::Interactive) return false;

	data = "";
	while(true)
	{
		const int MAX = 100;
		char buf[MAX];

		auto count = read(mOutputPipe.pipe(), buf, MAX - 1);
		if(count == -1) return false;
		buf[count] = 0;
		data += buf;

		if(data.back() == '\n')
		{
			data.resize(data.length() - 1);
			break;
		}
	}
	return true;
}

void checklib::details::RestrictedProcessImpl::reset()
{
	doFinalize();

	mInputPipe.reset();
	mOutputPipe.reset();
	mErrorPipe.reset();

	mPeakMemoryUsage.store(0);
	mCPUTime.store(0);
	mIsRunning.store(false);
	mProcessStatus.store(psNotRunning);
}

void checklib::details::RestrictedProcessImpl::doFinalize()
{
	mutex_locker locker(mHandlesMutex);

	if(!mIsRunning.load()) return;

	kill(mChildPid, SIGUSR1);
	int status = 0;
	waitpid(mChildPid, &status, WCONTINUED);

	mIsRunning.store(false);

	mInputPipe.reset();
	mOutputPipe.reset();
	mErrorPipe.reset();
}

void checklib::details::RestrictedProcessImpl::timerHandler(const boost::system::error_code &err)
{
	if(err) return;
	if(!is_running()) return;

	using namespace std;

	ostringstream name;
	name << mChildPid;
	ifstream is("/proc/" + name.str() + "/stat");

	// Получение времени работы процесса. Интересуют нас только последние два числа
	string pid, comm, state, ppid, pgrp, session, tty_nr;
	string tpgid, flags, minflt, cminflt, majflt, cmajflt;
	long long int utime, stime;
	is >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
	   >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
	   >> utime >> stime;

	int currentCPUTime = (utime + stime) / mTicks * 1000;
	mCPUTime.store(currentCPUTime);
	if(mLimits.useTimeLimit && mLimits.timeLimit < currentCPUTime)
	{
		mProcessStatus.store(psTimeLimitExceeded);
		doFinalize();
		finished(-2);
		return;
	}
	is.close();

	if(mOldCPUTime == currentCPUTime)
	{
		mUnchangedTicks++;
		if(mUnchangedTicks * sTimerDuration > 2000)
		{
			mProcessStatus.store(psIdlenessLimitExceeded);
			doFinalize();
			finished(-2);
			return;
		}
	}
	else
	{
		mUnchangedTicks = 0;
		mOldCPUTime = currentCPUTime;
	}

	is.open("/proc/" + name.str() + "/status");
	string str;

	bool found = false;
	while(getline(is, str))
	{
		if(str.substr(0, 7) == std::string("VmPeak:"))
		{
			istringstream iis(str);
			string tmp;
			iis >> tmp;
			long long int rr;
			iis >> rr;
			mPeakMemoryUsage.store(static_cast<int>(rr * 1024));
			found = true;
			break;
		}
	}

	if(!found)
	{
		is.close();
		is.open("/proc/" + name.str() + "/statm");
		long long int cur;
		if(is >> cur)
		{
			int tmp = mPeakMemoryUsage.load();
			mPeakMemoryUsage.store(max(static_cast<int>(cur), tmp));
		}
	}
	if(mLimits.useMemoryLimit && mPeakMemoryUsage.load() > mLimits.memoryLimit)
	{
		mProcessStatus.store(psMemoryLimitExceeded);
		doFinalize();
		finished(-2);
		return;
	}

	int status;
	int r = waitpid(mChildPid, &status, WNOHANG);
	if(r < 0)
	{
		//qWarning() << "Waitpid error";
	}
	else if(r == 0)
	{
		mTimer.expires_from_now(boost::posix_time::millisec(sTimerDuration));
		mTimer.async_wait(boost::bind(&checklib::details::RestrictedProcessImpl::timerHandler,
		                              boost::ref(*this), boost::lambda::_1));
	}
	else
	{
		if(WIFEXITED(status))
		{
			mProcessStatus.store(psExited);
		}
		if(WIFSIGNALED(status))
		{
			// Иначе уже нужный статус был установлен
			if(mProcessStatus.load() == psRunning) mProcessStatus.store(psRuntimeError);
		}

		doFinalize();
		mExitCode.store(WEXITSTATUS(status));

		mIsRunning.store(false);
		finished(mExitCode.load());
	}
}

bool checklib::details::RestrictedProcessImpl::closeStandardInput()
{
	if(mStandardInput == ss::Interactive && mInputPipe.pipe() != -1)
	{
		return close(mInputPipe.pipe()) == 0;
	}
	return false;
}
