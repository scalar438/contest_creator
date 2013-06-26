#pragma once

#include <windows.h>
#include <psapi.h>

#include <QObject>
#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QDebug>

#include "restricted_process_types.h"
#include "checklib_exception.h"

namespace checklib
{
namespace details
{

class RestrictedProcessImpl : public QObject
{
	Q_OBJECT
public:

	RestrictedProcessImpl(QObject *parent = nullptr)
		: QObject(parent)
	{
		mStandardInput = "stdin";
		mStandardOutput = "stdout";
		mStandardError = "stderr";
	}

	~RestrictedProcessImpl()
	{

	}

	QString getProgram() const
	{
		return mProgram;
	}

	void setProgram(const QString &program)
	{
		mProgram = program;
	}

	QStringList getParams() const
	{
		return mParams;
	}

	void setParams(const QStringList &params)
	{
		mParams = params;
	}

	bool isRunning() const
	{
		return false;
	}

	void start()
	{
		if(isRunning()) return;

		STARTUPINFOA si;
		memset(&si, 0, sizeof si);
		si.cb = sizeof si;
		si.dwFlags = STARTF_USESTDHANDLES;
		if(mStandardInput == "stdin") si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		else
		{
			qDebug() << mStandardInput.toAscii().data();
			HANDLE f = CreateFileA(mStandardInput.toAscii().data(),
			                       GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if(f == INVALID_HANDLE_VALUE)
			{
				throw
			}
			si.hStdInput = f;
		}

		if(mStandardOutput == "stdout") si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		else
		{
			HANDLE f = CreateFileA(mStandardOutput.toAscii().data(),
			                       GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if(f == INVALID_HANDLE_VALUE)
			{
				qDebug() << "Error";
			}
			si.hStdOutput = f;
		}

		if(mStandardError == "stderr") si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
		else
		{
			HANDLE f = CreateFileA(mStandardError.toAscii().data(),
			                       GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if(f == INVALID_HANDLE_VALUE)
			{

			}
			si.hStdError = f;
		}

		PROCESS_INFORMATION pi;
		if(!CreateProcessA(NULL, "szCommandLine", NULL, NULL, FALSE, CREATE_SUSPENDED | CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
		{
			qDebug() << "Error calling process";
		}
	}

	void terminate()
	{
	}

	void wait()
	{

	}

	bool wait(int milliseconds)
	{
		return false;
	}

	/// Код возврата.
	int exitCode() const
	{
		return 1;
	}

	/// Тип завершения программы
	ProcessStatus exitType() const
	{
		return mProcessStatus;
	}

	/// Пиковое значение потребляемой памяти
	int peakMemoryUsage() const
	{
		return 42;
	}

	/// Сколько процессорного времени израсходовал процесс
	int CPUTime() const
	{
		return 10;
	}

	Restrictions getRestrictions() const
	{
		return mRestrictions;
	}

	void setRestrictions(const Restrictions &restrictions)
	{
		mRestrictions = restrictions;
	}

	void redirectStandardInput(const QString &fileName)
	{
		mStandardInput = fileName;
	}

	void redirectStandardOutput(const QString &fileName)
	{
		mStandardOutput = fileName;
	}

	void redirectStandardError(const QString &fileName)
	{
		mStandardError = fileName;
	}

	void redirectStandardStream(StandardStream stream, const QString &fileName)
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

	void sendBufferToStandardStream(StandardStream stream, const QByteArray &data)
	{

	}

protected:
	/*	void run() override
		{
			FILETIME creationTime, exitTime, kernelTime, userTime;

			while(WaitForSingleObject(mProcess, 100) == WAIT_TIMEOUT)
			{
				GetProcessTimes(mProcess, &creationTime, &exitTime, &kernelTime, &userTime);

				kernelTime.dwHighDateTime += userTime.dwHighDateTime;
				kernelTime.dwLowDateTime += userTime.dwLowDateTime;

				if(QDateTime)

					if(mRestrictions.useTimeLimit &&
					        ((kernelTime.dwHighDateTime << 32) + kernelTime.dwLowDateTime) / 10000000 > mRestrictions.timeLimit)
					{
						TerminateProcess(mProcess, 0);
						mProcessStatus = etTimeLimit;
					}

				PROCESS_MEMORY_COUNTERS counters;
				GetProcessMemoryInfo(mPlatformData->process, &counters, sizeof counters);
				if(mRestrictions.useMemoryLimit && counters.PeakWorkingSetSize > mRestrictions.memoryLimit)
				{
					TerminateProcess(mProcess, 0);
					mProcessStatus = etMemoryLimit;
				}
			}
		}
	*/
private:
	HANDLE mProcess;

	QString mProgram;
	QStringList mParams;
	QDateTime mStartTime, mEndTime;

	QString mStandardInput, mStandardOutput, mStandardError;

	ProcessStatus mProcessStatus;

	Restrictions mRestrictions;
};

}
}

/*
checklib::RestrictedProcess::RestrictedProcess(QObject *parent)
	: QObject(parent), mPlatformData(new checklib::details::platform_data),
	  mExitCode(0),
	  mProcessStatus(etNormal),
	  mStandardInput("stdin"),
	  mStandardOutput("stdout"),
	  mStandardError("stderr")
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
	return 0;
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

QString checklib::RestrictedProcess::getProgram() const
{
	return mProgram;
}

void checklib::RestrictedProcess::setProgram(const QString &program)
{
	mProgram = program;
}

QStringList checklib::RestrictedProcess::params() const
{
	return mParams;
}

void checklib::RestrictedProcess::setParams(const QStringList &params)
{
	mParams = params;
}

void checklib::RestrictedProcess::sendBufferToStandardStream(checklib::StandardStream stream, const QByteArray &data)
{
}
*/
