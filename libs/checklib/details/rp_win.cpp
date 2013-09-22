#include "rp_win.h"

#include <boost/thread.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/shared_array.hpp>

#include <Windows.h>

#include <QDebug>

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

// TODO: Сделать бросание исключения в случае ошибок

checklib::details::RestrictedProcessImpl::RestrictedProcessImpl()
	: mTimer(instance.io_service())
{
	reset();
}

checklib::details::RestrictedProcessImpl::~RestrictedProcessImpl()
{
	doFinalize();
	while(mIsRunning.load()) boost::this_thread::yield();
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
		if(mParams[i].contains(' '))
		{
			cmdLine += '\"';
			cmdLine += mParams[i];
			cmdLine += '\"';
		}
		else cmdLine += mParams[i];
	}

	//LPCSTR curDir;
	boost::shared_array<char> curDir;
	if(!mCurrentDirectory.isEmpty())
	{
		curDir = boost::shared_array<char>(new char[mCurrentDirectory.size() + 1]);
		strcpy(curDir.get(), mCurrentDirectory.toLocal8Bit().data());
	}

	if(!CreateProcessA(NULL, cmdLine.toLocal8Bit().data(), &sa, NULL, TRUE,
					   CREATE_NO_WINDOW | CREATE_SUSPENDED, NULL, curDir.get(), &si, &pi))
	{
		qDebug() << "Cannot create process";
		return;
	}
	handles.clear();
	ResumeThread(pi.hThread);
	mProcessStatus.store(psRunning);
	mCurrentInformation = pi;
	mStartTime = QDateTime::currentDateTime();
	mutex_locker guard(mTimerMutex);
	mTimer.expires_from_now(boost::posix_time::milliseconds(100));
	mTimer.async_wait(boost::bind(&checklib::details::RestrictedProcessImpl::timerHandler, boost::ref(*this),
	                              boost::lambda::_1));
	mIsRunning.store(true);
}

void checklib::details::RestrictedProcessImpl::terminate()
{
	if(isRunning())
	{
		mProcessStatus.store(psTerminated);
		mutex_locker lock(mHandlesMutex);
		if(isRunning()) TerminateProcess(mCurrentInformation.hProcess, -1);
	}
}

void checklib::details::RestrictedProcessImpl::wait()
{
	wait(INT_MAX);
}

bool checklib::details::RestrictedProcessImpl::wait(int milliseconds)
{
	if(!isRunning()) return false;
	auto res = WaitForSingleObject(mCurrentInformation.hProcess, milliseconds);
	if(res == WAIT_TIMEOUT)
	{
		return false;
	}
	if(res == WAIT_OBJECT_0)
	{
		if(mProcessStatus.load() == psRunning) mProcessStatus.store(psExited);
		doFinalize();
		destroyHandles();
		return true;
	}
	// Тут надо бросить исключение
	return false;
}

// Код возврата.
int checklib::details::RestrictedProcessImpl::exitCode() const
{
	return mExitCode.load();
}

// Тип завершения программы
checklib::ProcessStatus checklib::details::RestrictedProcessImpl::processStatus() const
{
	return mProcessStatus.load();
}

// Пиковое значение потребляемой памяти
int checklib::details::RestrictedProcessImpl::peakMemoryUsage()
{
	mutex_locker lock(mHandlesMutex);
	if(isRunning()) return peakMemoryUsageS();
	return mPeakMemoryUsage.load();
}

// Сколько процессорного времени израсходовал процесс
int checklib::details::RestrictedProcessImpl::CPUTime()
{
	mutex_locker lock(mHandlesMutex);
	if(isRunning()) return CPUTimeS();
	return mCPUTime.load();
}

int checklib::details::RestrictedProcessImpl::peakMemoryUsageS() const
{
	PROCESS_MEMORY_COUNTERS data;
	GetProcessMemoryInfo(mCurrentInformation.hProcess, &data, sizeof data);
	mPeakMemoryUsage.store(static_cast<int>(data.PeakWorkingSetSize));
	return static_cast<int>(data.PeakWorkingSetSize);
}

int checklib::details::RestrictedProcessImpl::CPUTimeS() const
{
	FILETIME creationTime, exitTime, kernelTime, userTime;
	GetProcessTimes(mCurrentInformation.hProcess, &creationTime, &exitTime, &kernelTime, &userTime);
	auto ticks = ((kernelTime.dwHighDateTime * 1ll) << 32) + kernelTime.dwLowDateTime +
	             ((userTime.dwHighDateTime * 1ll) << 32) + userTime.dwLowDateTime;
	ticks = ticks / 1000 / 10;
	mCPUTime.store(static_cast<int>(ticks));
	return static_cast<int>(ticks);
}

void checklib::details::RestrictedProcessImpl::reset()
{
	mStandardInput = "stdin";
	mStandardOutput = "stdout";
	mStandardError = "stderr";
	mParams.clear();

	mCPUTime.store(0);
	mPeakMemoryUsage.store(0);
	mProcessStatus.store(psNotRunning);
	mLimits = Limits();
	mIsRunning.store(false);
	mCurrentDirectory = "";
}

checklib::Limits checklib::details::RestrictedProcessImpl::getLimits() const
{
	return mLimits;
}

void checklib::details::RestrictedProcessImpl::setLimits(const Limits &limits)
{
	if(isRunning()) return;
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

void checklib::details::RestrictedProcessImpl::sendBufferToStandardInput(const QByteArray &data)
{
// Пока не реализовано
}

void checklib::details::RestrictedProcessImpl::doCheck()
{
	int time = CPUTime();
	if(mLimits.useTimeLimit)
	{
		if(time > mLimits.timeLimit)
		{
			mProcessStatus.store(psTimeLimitExceeded);
			doFinalize();
		}
	}
	int fullTime = mStartTime.msecsTo(QDateTime::currentDateTime());
	if(fullTime > 2000 && time * 8 < fullTime)
	{
		mProcessStatus.store(psIdlenessLimitExceeded);
		doFinalize();
	}
	if(mLimits.useMemoryLimit)
	{
		if(peakMemoryUsage() > mLimits.memoryLimit)
		{
			mProcessStatus.store(psMemoryLimitExceeded);
			doFinalize();
		}
	}
}

void checklib::details::RestrictedProcessImpl::doFinalize()
{
	mutex_locker lock1(mHandlesMutex);
	if(!isRunning()) return;
	
	// Сохранить параметры перед закрытием 
	CPUTimeS();
	peakMemoryUsageS();

	if(mLimits.useTimeLimit && mCPUTime.load() > mLimits.timeLimit)
	{
		mProcessStatus.store(psTimeLimitExceeded);
	}
	if(mLimits.useMemoryLimit && mPeakMemoryUsage.load() > mLimits.memoryLimit)
	{
		mProcessStatus.store(psMemoryLimitExceeded);
	}

	if(WaitForSingleObject(mCurrentInformation.hProcess, 0) == WAIT_TIMEOUT)
	{
		if(mProcessStatus.load() == psRunning) 
		{
			qDebug() << "Logic error";
		}
		TerminateProcess(mCurrentInformation.hProcess, -1);
	}

	DWORD tmpExitCode;
	if(!GetExitCodeProcess(mCurrentInformation.hProcess, &tmpExitCode))
	{
		qDebug() << "Cannot get exit code process";
	}
	mExitCode.store(tmpExitCode);
}

void checklib::details::RestrictedProcessImpl::timerHandler(const boost::system::error_code &err)
{
	if(err) return;
	doCheck();

	switch(WaitForSingleObject(mCurrentInformation.hProcess, 0))
	{
	case WAIT_OBJECT_0:
		if(mProcessStatus.load() == psRunning) mProcessStatus.store(psExited);
		doFinalize();
		destroyHandles();
		break;
	case WAIT_TIMEOUT:
		{
			mutex_locker lock(mTimerMutex);
			mTimer.expires_from_now(boost::posix_time::milliseconds(100));
			mTimer.async_wait(boost::bind(&checklib::details::RestrictedProcessImpl::timerHandler, boost::ref(*this),
			                              boost::lambda::_1));
		}
		break;
	case WAIT_FAILED:
		qDebug() << "Wait failed";
		break;
	}
}

void checklib::details::RestrictedProcessImpl::destroyHandles()
{
	mutex_locker lock(mHandlesMutex);
	if(!isRunning()) return;
	CloseHandle(mCurrentInformation.hProcess);
	CloseHandle(mCurrentInformation.hThread);
	mIsRunning.store(false);
}
