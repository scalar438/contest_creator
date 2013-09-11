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

boost::asio::io_service io;

checklib::details::RestrictedProcessImpl::RestrictedProcessImpl()
	: mTimer(io)
{
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
		mProcessStatus = psFailed;
		return;
	}
	if(mChildPid == 0)
	{
		// Дочерний процесс. Перенаправляем потоки, задаем лимиты и запускаем

		if(mStandardInput != "stdin")   freopen(mStandardInput.toLocal8Bit().data(), "rt", stdin);
		if(mStandardOutput != "stdout") freopen(mStandardOutput.toLocal8Bit().data(), "wt", stdout);
		if(mStandardError != "stderr")  freopen(mStandardError.toLocal8Bit().data(), "wt", stdout);

		if(mLimits.useMemoryLimit)
		{
			rlimit limit;
			limit.rlim_max = limit.rlim_cur = mLimits.memoryLimit + 1024 * 1024;

			setrlimit(RLIMIT_AS, &limit);
		}
		if(mLimits.useTimeLimit)
		{
			rlimit limit;
			// 100 - c потолка, возможно, что неверно
			limit.rlim_cur = limit.rlim_max = 100 * mLimits.timeLimit;
			setrlimit(RLIMIT_CPU, &limit);
		}

		char **args = new char*[mParams.size()];
		for(int i = 0; i < mParams.size(); i++)
		{
			args[i] = new char[mParams[i].length() + 1];
			strcpy(args[i], mParams[i].toLocal8Bit().data());
		}

		qDebug() << mProgram;

		execl(mProgram.toLocal8Bit().data(), mProgram.toLocal8Bit().data());

		exit(-1);
	}
	else
	{
		// Родительский процесс, задаем параметры, необходимые для слежения за дочерним
		mProcessStatus = psRunning;
		mTimer.expires_from_now(boost::posix_time::milliseconds(100));
		mTimer.async_wait(boost::bind(&checklib::details::RestrictedProcessImpl::timerHandler,
									  boost::ref(*this), boost::lambda::_1));
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
	int status;
	waitpid(mChildPid, &status, WNOHANG);
}

// Код возврата.
int checklib::details::RestrictedProcessImpl::exitCode() const
{
	return mExitCode;
}

// Тип завершения программы
checklib::ProcessStatus checklib::details::RestrictedProcessImpl::processStatus() const
{
	return mProcessStatus;
}

// Пиковое значение потребляемой памяти
int checklib::details::RestrictedProcessImpl::peakMemoryUsage()
{
	return 0;
}

// Сколько процессорного времени израсходовал процесс
int checklib::details::RestrictedProcessImpl::CPUTime()
{
	return 0;
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

}

void checklib::details::RestrictedProcessImpl::sendBufferToStandardInput(const QByteArray &data)
{

}

void checklib::details::RestrictedProcessImpl::timerHandler(const boost::system::error_code &err)
{
	if(isRunning()) return;

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

	if(mLimits.timeLimit < utime + stime)
	{
		mProcessStatus = psTimeLimit;
		kill(mChildPid, SIGUSR1);
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
			mOldPeakMemoryUsage.store(static_cast<int>(rr * 1024));
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
		int tmp = mOldPeakMemoryUsage.load();
		mOldPeakMemoryUsage.store(max(static_cast<int>(cur), tmp));
	}
	if(mOldPeakMemoryUsage.load() > mLimits.memoryLimit)
	{
		mProcessStatus = psMemoryLimit;
		kill(mChildPid, SIGUSR1);
	}

	int status;
	waitpid(mChildPid, &status, WNOHANG);
	if(WIFEXITED(status))
	{
		mProcessStatus.store(psExited);
		return;
	}
	if(WIFSIGNALED(status))
	{
		 mProcessStatus.store(psRuntimeError);
		 return;
	}

}
