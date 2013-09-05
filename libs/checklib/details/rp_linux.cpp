#include <exception>
#include <sstream>
#include <fstream>
#include <string>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <QStringList>
#include <QDebug>

#include "checklib_exception.h"
#include "restricted_process.h"

// TODO: заменить return в случае неправильного вызова на исключения

struct checklib::details::platform_data
{
	pid_t pid;
};

checklib::RestrictedProcess::RestrictedProcess(const QString &program, const QStringList &params)
	: RestrictedProcess(nullptr, program, params)
{
	// Implementation in other constructor
}

checklib::RestrictedProcess::RestrictedProcess(QObject *parent, const QString &program, const QStringList &params)
	: QObject(parent), mPlatformData(new checklib::details::platform_data),
	  mExitCode(0),
	  mProcessStatus(etNormal),
	  mStandardInput("stdin"),
	  mStandardOutput("stdout"),
	  mStandardError("stderr"),
	  mProgram(program),
	  mParams(params)
{
	mCheckTimer.setInterval(100);
	connect(&mCheckTimer, SIGNAL(timeout()), SLOT(checkOnce()));
}

checklib::RestrictedProcess::~RestrictedProcess()
{

}

bool checklib::RestrictedProcess::isRunning() const
{
	return exitType() == etRunning;
}

/// Запуск процесса
void checklib::RestrictedProcess::start()
{
	if(isRunning()) return;
	mPlatformData->pid = fork();
	if(mPlatformData->pid == -1)
	{
		mProcessStatus = etFailed;
		return;
	}
	if(mPlatformData->pid == 0)
	{
		// Дочерний процесс. Перенаправляем потоки, задаем лимиты и запускаем

		if(mStandardInput != "stdin")   freopen(mStandardInput.toLocal8Bit().data(), "rt", stdin);
		if(mStandardOutput != "stdout") freopen(mStandardOutput.toLocal8Bit().data(), "wt", stdout);
		if(mStandardError != "stderr")  freopen(mStandardError.toLocal8Bit().data(), "wt", stdout);

		if(mRestrictions.useMemoryLimit)
		{
			rlimit limit;
			limit.rlim_max = limit.rlim_cur = mRestrictions.memoryLimit + 1024 * 1024;

			setrlimit(RLIMIT_AS, &limit);
		}
		if(mRestrictions.useTimeLimit)
		{
			rlimit limit;
			// 100 - c потолка, возможно, что неверно
			limit.rlim_cur = limit.rlim_max = 100 * mRestrictions.timeLimit;
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
		mCheckTimer.start();
		mProcessStatus = etRunning;
	}
}

/// Завершает процесс вручную. Тип завершения становится etTerminated
void checklib::RestrictedProcess::terminate()
{

}

/// Ждать завершения процесса
void checklib::RestrictedProcess::wait()
{

}

/// Ждать завершения процесса не более чем @param миллисекунд.
/// @return true если программа завершилась (сама или от превышения лимитов), false - если таймаут ожидания
bool checklib::RestrictedProcess::wait(int milliseconds)
{
	int status;
	waitpid(mPlatformData->pid, &status, WNOHANG);
}

/// Код возврата.
int checklib::RestrictedProcess::exitCode() const
{
	return mExitCode;
}

/// Тип завершения программы
checklib::ProcessStatus checklib::RestrictedProcess::exitType() const
{
	return mProcessStatus;
}

/// Пиковое значение потребляемой памяти
size_t checklib::RestrictedProcess::peakMemoryUsage() const
{
	return 0;
}

/// Сколько процессорного времени израсходовал процесс
int checklib::RestrictedProcess::CPUTime() const
{
	return 0;
}

checklib::Restrictions checklib::RestrictedProcess::getRestrictions() const
{
	return mRestrictions;
}

void checklib::RestrictedProcess::setRestrictions(const Restrictions &restrictions)
{
	mRestrictions = restrictions;
}

/// Перенаправить стандартный поток ввода в указанный файл.
/// Если stdin, то перенаправления не происходит.
/// Если stdout, то перенавравляется на вывод текущего приложения
void checklib::RestrictedProcess::redirectStandardInput(const QString &fileName)
{
	if(isRunning()) return;
	mStandardInput = fileName;
}

/// Перенаправить стандартный поток вывода в указанный файл.
/// Если stdout, то перенаправления не происходит.
void checklib::RestrictedProcess::redirectStandardOutput(const QString &fileName)
{
	if(isRunning()) return;
	mStandardOutput = fileName;
}

/// Перенаправить стандартный поток ошибок в указанный файл.
/// Если stderr, то перенаправления не происходит.
void checklib::RestrictedProcess::redirectStandardError(const QString &fileName)
{
	if(isRunning()) return;
	mStandardError = fileName;
}

void checklib::RestrictedProcess::redirectStandardStream(checklib::StandardStream stream, const QString &fileName)
{
	switch(stream)
	{
	case ssStdin:
		redirectStandardInput(fileName);
		break;
	case ssStdout:
		redirectStandardOutput(fileName);
		break;
	case ssStderr:
		redirectStandardError(fileName);
		break;
	}
}

void checklib::RestrictedProcess::sendBufferToStandardStream(checklib::StandardStream stream, const QByteArray &data)
{
}

void checklib::RestrictedProcess::checkOnce()
{
	if(isRunning()) return;

	using namespace std;

	ostringstream name;
	name << mPlatformData->pid;
	ifstream is("/proc/" + name.str() + "/stat");

	// Получение времени работы процесса. Интересуют нас только последние два числа
	string pid, comm, state, ppid, pgrp, session, tty_nr;
	string tpgid, flags, minflt, cminflt, majflt, cmajflt;
	long long int utime, stime;
	is >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
	   >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
	   >> utime >> stime;

	if(mRestrictions.timeLimit < utime + stime)
	{
		mProcessStatus = etTimeLimit;
		kill(mPlatformData->pid, SIGUSR1);
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
			mPeakMemory = rr;
			found = true;
			mPeakMemory = mPeakMemory * 1024;
			break;
		}
	}

	if(!found)
	{
		is.close();
		is.open("/proc/" + name.str() + "/statm");
		long long int cur;
		is >> cur;
		mPeakMemory = max(cur, mPeakMemory);
	}
	if(mPeakMemory > mRestrictions.memoryLimit)
	{
		mProcessStatus = etMemoryLimit;
		kill(mPlatformData->pid, SIGUSR1);
	}

	int status;
	waitpid(mPlatformData->pid, &status, WNOHANG);
	if(WIFEXITED(status))
	{
		if(mProcessStatus == etRunning) mProcessStatus = etRuntimeError;
		mCheckTimer.stop();
	}
	if(WIFSIGNALED(status))
	{
		if(mProcessStatus == etRunning) mProcessStatus = etRuntimeError;
		mCheckTimer.stop();
	}
}
