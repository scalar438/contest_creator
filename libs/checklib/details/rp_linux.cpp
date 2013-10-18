#include "rp_linux.h"
#include "rp_consts.h"
#include "checklib_exception.h"
#include <timer_service.h>

#include <exception>
#include <sstream>
#include <fstream>
#include <string>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <QStringList>
#include <QDebug>
#include <QFileInfo>
#include <boost/lambda/lambda.hpp>

checklib::details::RestrictedProcessImpl::RestrictedProcessImpl()
	: mTimer(TimerService::instance()->io_service())
{
	mTicks = static_cast<float>(sysconf(_SC_CLK_TCK));

	reset();
}

checklib::details::RestrictedProcessImpl::~RestrictedProcessImpl()
{
}

QString checklib::details::RestrictedProcessImpl::getProgram() const
{
	return mProgram;
}

void checklib::details::RestrictedProcessImpl::setProgram(const QString &program)
{
	mProgram = program;
}

QStringList checklib::details::RestrictedProcessImpl::getParams() const
{
	return mParams;
}

void checklib::details::RestrictedProcessImpl::setParams(const QStringList &params)
{
	mParams = params;
}

QString checklib::details::RestrictedProcessImpl::currentDirectory() const
{
	return mCurrentDirectory;
}

void checklib::details::RestrictedProcessImpl::setCurrentDirectory(const QString &directory)
{
	mCurrentDirectory = directory;
}

bool checklib::details::RestrictedProcessImpl::isRunning() const
{
	return mIsRunning.load();
}

// Запуск процесса
void checklib::details::RestrictedProcessImpl::start()
{
	if(isRunning()) return;

	if(mStandardInput == ss::Interactive)
	{
		pipe(mInputPipe);
	}
	if(mStandardOutput == ss::Interactive)
	{
		pipe(mOutputPipe);
	}
	if(mStandardError == ss::Interactive)
	{
		pipe(mErrorPipe);
	}

	mChildPid = fork();
	if(mChildPid == -1)
	{
		return;
	}
	if(mChildPid == 0)
	{
		// Дочерний процесс. Перенаправляем потоки, задаем лимиты и запускаем

		if(mStandardInput != ss::Stdin)
		{
			int d;

			if(mStandardInput == ss::Interactive)
			{
				d = mInputPipe[0];
				close(mInputPipe[1]);
			}
			else
			{
				d = open(mStandardInput.toLocal8Bit().data(), O_RDONLY);
				if(d == -1) qDebug() << "File not opened";
			}
			dup2(d, 0);
			close(d);
		}
		if(mStandardOutput != ss::Stdout)
		{
			int d;

			if(mStandardOutput == ss::Interactive)
			{
				d = mOutputPipe[1];
				close(mOutputPipe[0]);
			}
			else
			{
				d = open(mStandardOutput.toLocal8Bit().data(), O_TRUNC | O_CREAT | O_WRONLY,
				         S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
				if(d == -1) qDebug() << "File not opened";
			}
			dup2(d, 1);
			close(d);
		}
		if(mStandardError != ss::Stderr)
		{
			int d;

			if(mStandardError == ss::Interactive)
			{
				d = mErrorPipe[1];
				close(mErrorPipe[0]);
			}
			else
			{
				d = open(mStandardError.toLocal8Bit().data(), O_TRUNC | O_CREAT | O_WRONLY,
				         S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
				if(d == -1) qDebug() << "File not opened";
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

		QString programPath = QFileInfo(mProgram).absoluteFilePath();
		if(!mCurrentDirectory.isEmpty())
		{
			chdir(mCurrentDirectory.toLocal8Bit().data());
		}
		else
		{
			chdir(QFileInfo(programPath).absolutePath().toLocal8Bit().data());
		}

		char **args = new char*[mParams.size() + 2];
		args[0] = new char[programPath.size() + 1];
		strcpy(args[0], programPath.toLocal8Bit().data());

		for(int i = 0; i < mParams.size(); i++)
		{
			args[i + 1] = new char[mParams[i].length() + 1];
			strcpy(args[i + 1], mParams[i].toLocal8Bit().data());
		}
		args[mParams.size() + 1] = 0;
		execv(programPath.toLocal8Bit().data(), args);

		exit(-1);
	}
	else
	{
		// Родительский процесс, задаем параметры, необходимые для слежения за дочерним
		mProcessStatus.store(psRunning);
		mTimer.expires_from_now(boost::posix_time::milliseconds(100));
		mTimer.async_wait(boost::bind(&checklib::details::RestrictedProcessImpl::timerHandler,
		                              boost::ref(*this), boost::lambda::_1));
		mIsRunning.store(true);
		mStartTime = QDateTime::currentDateTime();

		if(mStandardInput == ss::Interactive) close(mInputPipe[0]);
		if(mStandardOutput == ss::Interactive) close(mOutputPipe[1]);
		if(mStandardError == ss::Interactive) close(mErrorPipe[1]);
	}
}

// Завершает процесс вручную. Тип завершения становится etTerminated
void checklib::details::RestrictedProcessImpl::terminate()
{
	kill(mChildPid, SIGTERM);
}

// Ждать завершения процесса
void checklib::details::RestrictedProcessImpl::wait()
{
	wait(INT_MAX);
}

// Ждать завершения процесса не более чем @param миллисекунд.
// @return true если программа завершилась (сама или от превышения лимитов), false - если таймаут ожидания
bool checklib::details::RestrictedProcessImpl::wait(int milliseconds)
{
	const int resolution = 10;
	int cur = 0;
	while(cur < milliseconds)
	{
		int status;
		boost::this_thread::sleep(boost::posix_time::milliseconds(resolution));
		if(!isRunning()) return true;
		cur += resolution;
	}
	return false;
}

// Код возврата.
int checklib::details::RestrictedProcessImpl::exitCode() const
{
	return mExitCode;
}

// Тип завершения программы
checklib::ProcessStatus checklib::details::RestrictedProcessImpl::processStatus() const
{
	return mProcessStatus.load();
}

// Пиковое значение потребляемой памяти
int checklib::details::RestrictedProcessImpl::peakMemoryUsage()
{
	return mPeakMemoryUsage.load();
}

// Сколько процессорного времени израсходовал процесс
int checklib::details::RestrictedProcessImpl::CPUTime()
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
void checklib::details::RestrictedProcessImpl::redirectStandardInput(const QString &fileName)
{
	mStandardInput = fileName;
}

// Перенаправить стандартный поток вывода в указанный файл.
// Если stdout, то перенаправления не происходит.
void checklib::details::RestrictedProcessImpl::redirectStandardOutput(const QString &fileName)
{
	mStandardOutput = fileName;
}

// Перенаправить стандартный поток ошибок в указанный файл.
// Если stderr, то перенаправления не происходит.
void checklib::details::RestrictedProcessImpl::redirectStandardError(const QString &fileName)
{
	mStandardError = fileName;
}

bool checklib::details::RestrictedProcessImpl::sendDataToStandardInput(const QString &data, bool newLine)
{
	if(!isRunning() || mStandardInput != ss::Interactive) return false;

	auto count = write(mInputPipe[1], data.toLocal8Bit().data(), data.length());
	if(count == -1) return false;
	if(newLine)
	{
		const char c = '\n';
		write(mInputPipe[1], &c, 1);
	}
	return true;
}

bool checklib::details::RestrictedProcessImpl::getDataFromStandardOutput(QString &data)
{
	if(!isRunning() || mStandardOutput != ss::Interactive) return false;

	data = "";
	while(true)
	{
		const int MAX = 100;
		char buf[MAX];

		auto count = read(mOutputPipe[0], buf, MAX - 1);
		if(count == -1) return false;
		buf[count] = 0;
		data += buf;

		if(data.endsWith("\n"))
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

	mStandardInput = ss::Stdin;
	mStandardOutput = ss::Stdout;
	mStandardError = ss::Stderr;

	mPeakMemoryUsage.store(0);
	mCPUTime.store(0);
	mIsRunning.store(false);
}

void checklib::details::RestrictedProcessImpl::doFinalize()
{
	mutex_locker locker(mHandlesMutex);

	if(!mIsRunning.load()) return;

	kill(mChildPid, SIGUSR1);
	waitpid(mChildPid, NULL, WCONTINUED);

	mIsRunning.store(false);
}

void checklib::details::RestrictedProcessImpl::timerHandler(const boost::system::error_code &err)
{
	if(err) return;
	if(!isRunning()) return;

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
		return;
	}

	int fullTime = mStartTime.msecsTo(QDateTime::currentDateTime());
	if(fullTime > 2000 && currentCPUTime * 8 < fullTime)
	{
		mProcessStatus.store(psIdlenessLimitExceeded);
		doFinalize();
		return;
	}

	is.close();

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
		return;
	}

	int status;
	int r = waitpid(mChildPid, &status, WNOHANG);
	if(r < 0)
	{
		qDebug() << "Error";
	}
	else if(r == 0)
	{
		mTimer.expires_from_now(boost::posix_time::millisec(100));
		mTimer.async_wait(boost::bind(&checklib::details::RestrictedProcessImpl::timerHandler,
		                              boost::ref(*this), boost::lambda::_1));
	}
	else
	{
		if(WIFEXITED(status)) mProcessStatus.store(psExited);
		if(WIFSIGNALED(status)) mProcessStatus.store(psRuntimeError);

		mIsRunning.store(false);
	}
}
