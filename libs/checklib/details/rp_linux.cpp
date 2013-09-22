#include "rp_linux.h"

#include "checklib_exception.h"

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
#include <boost/lambda/lambda.hpp>

// TODO: перенести это в отдельный header и в реализации под windows тоже использовать его
class ServiceInstance
{
public:
	ServiceInstance()
		: mWork(mService)
	{
		mThread = boost::thread(boost::bind(&boost::asio::io_service::run, boost::ref(mService)));
	}
	~ServiceInstance()
	{
		mService.stop();
		mThread.join();
	}

	boost::asio::io_service &io_service()
	{
		return mService;
	}

private:
	boost::asio::io_service mService;
	boost::asio::io_service::work mWork;
	boost::thread mThread;
};

ServiceInstance instance;

checklib::details::RestrictedProcessImpl::RestrictedProcessImpl()
	: mTimer(instance.io_service())
{
	mIsRunning.store(false);
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

bool checklib::details::RestrictedProcessImpl::isRunning() const
{
	return mIsRunning.load();
}

// Запуск процесса
void checklib::details::RestrictedProcessImpl::start()
{
	if(isRunning()) return;
	mChildPid = fork();
	if(mChildPid == -1)
	{
		return;
	}
	if(mChildPid == 0)
	{
		// Дочерний процесс. Перенаправляем потоки, задаем лимиты и запускаем

		if(mStandardInput != "stdin")
		{
			int d = open(mStandardInput.toLocal8Bit().data(), O_RDONLY);
			dup2(d, 0);
			close(d);
		}
		if(mStandardOutput != "stdout")
		{
			int d = open(mStandardOutput.toLocal8Bit().data(), O_TRUNC | O_CREAT | O_WRONLY,
						 S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
			if(d == -1)
			{
				qDebug() << "File not opened";
			}
			dup2(d, 1);
			close(d);
		}
		if(mStandardError != "stderr")
		{
			int d = open(mStandardError.toLocal8Bit().data(), O_TRUNC | O_CREAT | O_WRONLY,
						 S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
			dup2(d, 2);
			close(d);
		}

		if(mLimits.useMemoryLimit)
		{
			rlimit limit;
			limit.rlim_max = limit.rlim_cur = mLimits.memoryLimit + 1024 * 1024;

			setrlimit(RLIMIT_AS, &limit);
		}
		if(mLimits.useTimeLimit)
		{
			rlimit limit;
			limit.rlim_cur = limit.rlim_max = mLimits.timeLimit / 1000 + 1;
			setrlimit(RLIMIT_CPU, &limit);
		}

		char **args = new char*[mParams.size() + 2];
		args[0] = new char[mProgram.size() + 1];
		strcpy(args[0], mProgram.toLocal8Bit().data());

		for(int i = 0; i < mParams.size(); i++)
		{
			args[i + 1] = new char[mParams[i].length() + 1];
			strcpy(args[i + 1], mParams[i].toLocal8Bit().data());
		}
		args[mParams.size() + 1] = 0;
		execv(mProgram.toLocal8Bit().data(), args);

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

void checklib::details::RestrictedProcessImpl::reset()
{
	mStandardInput = "stdin";
	mStandardOutput = "stdout";
	mStandardError = "stderr";

}

void checklib::details::RestrictedProcessImpl::sendBufferToStandardInput(const QByteArray &data)
{

}

void checklib::details::RestrictedProcessImpl::timerHandler(const boost::system::error_code &err)
{
	if(err) return;
//	qDebug() << "Timer";
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
		mProcessStatus.store(psTimeLimit);
		kill(mChildPid, SIGUSR1);
		waitpid(mChildPid, NULL, WCONTINUED);
		mIsRunning.store(false);
		return;
	}

	int fullTime = mStartTime.msecsTo(QDateTime::currentDateTime());
	if(fullTime > 2000 && currentCPUTime * 8 < fullTime)
	{
		mProcessStatus.store(psIdlenessLimit);
		kill(mChildPid, SIGUSR1);
		waitpid(mChildPid, NULL, WCONTINUED);
		mIsRunning.store(false);
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
		is >> cur;
		int tmp = mPeakMemoryUsage.load();
		mPeakMemoryUsage.store(max(static_cast<int>(cur), tmp));
	}
	if(mLimits.useMemoryLimit && mPeakMemoryUsage.load() > mLimits.memoryLimit)
	{
		mProcessStatus.store(psMemoryLimit);
		kill(mChildPid, SIGUSR1);
	}

//	qDebug() << "Before checking exit status";

	int status;
	int r = waitpid(mChildPid, &status, WNOHANG);
	if(r < 0)
	{
//		qDebug() << "waitpid error";
	}
	else if(r == 0)
	{
//		qDebug() << "timer.Still_running";
	}
	else
	{
		if(WIFEXITED(status))
		{
//			qDebug() << "Timer.WIFEXITED";
			mProcessStatus.store(psExited);
			mIsRunning.store(false);
			return;
		}
		if(WIFSIGNALED(status))
		{
//			qDebug() << "Timer.WIFSIGNALED";
			mProcessStatus.store(psRuntimeError);
			mIsRunning.store(false);
			return;
		}
	}
	mTimer.expires_from_now(boost::posix_time::millisec(100));
	mTimer.async_wait(boost::bind(&checklib::details::RestrictedProcessImpl::timerHandler,
	                              boost::ref(*this), boost::lambda::_1));
}
