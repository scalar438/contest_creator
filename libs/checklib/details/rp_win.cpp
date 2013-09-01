﻿#include "rp_win.h"

#include <functional>
#include <strsafe.h>
#include <QThreadPool>

#include <boost/thread.hpp>
#include <boost/lambda/lambda.hpp>

#include <Windows.h>

class HandleCloser
{
public:
	HandleCloser(HANDLE h = INVALID_HANDLE_VALUE)
	{
		setHandle(h);
	}

	~HandleCloser()
	{
		if(handle != INVALID_HANDLE_VALUE) CloseHandle(handle);
	}

	void setHandle(HANDLE h)
	{
		handle = h;
	}

private:
	HANDLE handle;
};

typedef std::unique_ptr<HandleCloser> Closer;

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

checklib::details::RestrictedProcessImpl::RestrictedProcessImpl(QObject *parent)
	: QObject(parent),
	  mTimer(instance.io_service())
{
	mStandardInput = "stdin";
	mStandardOutput = "stdout";
	mStandardError = "stderr";

	mOldCPUTime = mOldPeakMemoryUsage = 0;
}

checklib::details::RestrictedProcessImpl::~RestrictedProcessImpl()
{
	if(isRunning()) terminate();
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
	return processStatus() == psRunning;
}

void checklib::details::RestrictedProcessImpl::start()
{
	if(isRunning()) return;

	STARTUPINFOA si;
	memset(&si, 0, sizeof si);
	si.cb = sizeof si;
	si.dwFlags = STARTF_USESTDHANDLES;

	SECURITY_ATTRIBUTES sa;
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	sa.nLength = sizeof sa;
	std::vector<Closer> handles;

	// TODO: Сделать бросание исключения в случае ошибки
	if(mStandardInput == "stdin") si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	else
	{
		std::vector<wchar_t> str(mStandardInput.length() + 1, 0);
		mStandardInput.toWCharArray(&str[0]);
		HANDLE f = CreateFile(&str[0], GENERIC_READ, FILE_SHARE_READ,
		                      &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(f == INVALID_HANDLE_VALUE)
		{
			qDebug() << "Cannot open file" << mStandardInput;
			return;
		}
		si.hStdInput = f;
		handles.push_back(Closer(new HandleCloser(f)));
	}

	if(mStandardOutput == "stdout") si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	else
	{
		std::vector<wchar_t> str(mStandardOutput.length() + 1, 0);
		mStandardOutput.toWCharArray(&str[0]);
		HANDLE f = CreateFile(&str[0], GENERIC_WRITE, 0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(f == INVALID_HANDLE_VALUE)
		{
			qDebug() << "Cannot open file" << mStandardOutput;
			return;
		}
		si.hStdOutput = f;
		handles.push_back(Closer(new HandleCloser(f)));
	}

	if(mStandardError == "stderr") si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	else
	{
		std::vector<wchar_t> str(mStandardError.length() + 1, 0);
		mStandardError.toWCharArray(&str[0]);
		HANDLE f = CreateFile(&str[0], GENERIC_WRITE, 0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(f == INVALID_HANDLE_VALUE)
		{
			qDebug() << "Cannot open file" << mStandardError;
			return;
		}
		si.hStdError = f;
		handles.push_back(Closer(new HandleCloser(f)));
	}

	PROCESS_INFORMATION pi;
	QString cmdLine = "\"" + mProgram + "\"";
	for(int i = 0; i < mParams.size(); i++)
	{
		cmdLine += " ";
		cmdLine += mParams[i];
	}

	if(!CreateProcessA(NULL, cmdLine.toAscii().data(), &sa, NULL, TRUE, CREATE_NO_WINDOW | CREATE_SUSPENDED, NULL, NULL, &si, &pi))
	{
		qDebug() << "Cannot create process";
		return;
	}
	handles.clear();
	ResumeThread(pi.hThread);
	mProcessStatus = psRunning;
	mCurrentInformation = pi;
	mStartTime = QDateTime::currentDateTime();
	mTimer.expires_from_now(boost::posix_time::milliseconds(100));
	mTimer.async_wait(boost::bind(&checklib::details::RestrictedProcessImpl::timerHandler, boost::ref(*this),
	                              boost::lambda::_1));
}

void checklib::details::RestrictedProcessImpl::terminate()
{
	if(isRunning())
	{
		mProcessStatus = psTerminated;
		TerminateProcess(mCurrentInformation.hProcess, -1);
	}
}

void checklib::details::RestrictedProcessImpl::wait()
{
	wait(INT_MAX);
}

bool checklib::details::RestrictedProcessImpl::wait(int milliseconds)
{
	if(!isRunning()) return false;
	return WaitForSingleObject(mCurrentInformation.hProcess, milliseconds) != WAIT_TIMEOUT;
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
int checklib::details::RestrictedProcessImpl::peakMemoryUsage() const
{
	if(!isRunning())
	{
		return mOldPeakMemoryUsage;
	}
	PROCESS_MEMORY_COUNTERS data;
	GetProcessMemoryInfo(mCurrentInformation.hProcess, &data, sizeof data);
	return mOldPeakMemoryUsage = int(data.PeakWorkingSetSize);
}

// Сколько процессорного времени израсходовал процесс
int checklib::details::RestrictedProcessImpl::CPUTime() const
{
	if(!isRunning())
	{
		return mOldCPUTime;
	}
	FILETIME creationTime, exitTime, kernelTime, userTime;
	GetProcessTimes(mCurrentInformation.hProcess, &creationTime, &exitTime, &kernelTime, &userTime);
	auto ticks = ((kernelTime.dwHighDateTime * 1ll) << 32) + kernelTime.dwLowDateTime +
	             ((userTime.dwHighDateTime * 1ll) << 32) + userTime.dwLowDateTime;
	return mOldCPUTime = int(ticks / 1000 / 10);
}

checklib::Limits checklib::details::RestrictedProcessImpl::getLimits() const
{
	return mLimits;
}

void checklib::details::RestrictedProcessImpl::setLimits(const Limits &limits)
{
	mLimits = limits;
}

void checklib::details::RestrictedProcessImpl::redirectStandardInput(const QString &fileName)
{
	mStandardInput = fileName;
}

void checklib::details::RestrictedProcessImpl::redirectStandardOutput(const QString &fileName)
{
	mStandardOutput = fileName;
}

void checklib::details::RestrictedProcessImpl::redirectStandardError(const QString &fileName)
{
	mStandardError = fileName;
}

void checklib::details::RestrictedProcessImpl::sendBufferToStandardStream(StandardStream stream, const QByteArray &data)
{

}

void checklib::details::RestrictedProcessImpl::doCheck()
{
	int time = CPUTime();
	if(mLimits.useTimeLimit)
	{
		if(time > mLimits.timeLimit)
		{
			mProcessStatus = psTimeLimit;
			if(!TerminateProcess(mCurrentInformation.hProcess, -1))
			{
				qDebug() << "TerminateProcess failed";
			}
		}
	}
	int fullTime = mStartTime.msecsTo(QDateTime::currentDateTime());
	if(fullTime > 2000 && time * 8 < fullTime)
	{
		mProcessStatus = psIdlenessLimit;
		if(!TerminateProcess(mCurrentInformation.hProcess, -1))
		{
			qDebug() << "TerminateProcess failed";
		}
	}
	if(mLimits.useMemoryLimit)
	{
		if(peakMemoryUsage() > mLimits.memoryLimit)
		{
			mProcessStatus = psMemoryLimit;
			if(!TerminateProcess(mCurrentInformation.hProcess, -1))
			{
				qDebug() << "TerminateProcess failed";
			}
		}
	}
}

void checklib::details::RestrictedProcessImpl::doFinalize()
{
	if(mProcessStatus == psRunning)
	{
		mProcessStatus = psExited;
		doCheck();
	}
	if(!GetExitCodeProcess(mCurrentInformation.hProcess, (LPDWORD)&mExitCode))
	{
		qDebug() << "Cannot get exit code process";
	}
	CloseHandle(mCurrentInformation.hProcess);
	CloseHandle(mCurrentInformation.hThread);
}

void checklib::details::RestrictedProcessImpl::timerHandler(const boost::system::error_code &err)
{
	doCheck();
	if(WaitForSingleObject(mCurrentInformation.hProcess, 1) != WAIT_TIMEOUT)
	{
		doFinalize();
	}
	else
	{
		mTimer.expires_from_now(boost::posix_time::milliseconds(100));
		mTimer.async_wait(boost::bind(&checklib::details::RestrictedProcessImpl::timerHandler, boost::ref(*this),
		                              boost::lambda::_1));
	}
}