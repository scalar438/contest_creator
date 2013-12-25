#include "rp_win.h"
#include "../timer_service.h"
#include "../rp_consts.h"
#include "../checklib_exception.h"

#include <boost/thread.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/shared_array.hpp>

#include <deque>

#include <Windows.h>
#include <strsafe.h>

#include <QDebug>
#include <QFileInfo>

checklib::details::RestrictedProcessImpl::RestrictedProcessImpl(QObject *parent)
	: QObject(parent)
	, mTimer(TimerService::instance()->io_service())
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
	std::vector<HandleCloser> handlesForAutoClose; // Хендлы, требующие закрытия на выходе из функции
	std::deque<HandleCloser> tmpHandles;           // Хендлы, требующие закрытия после окончания работы программы

	if(mStandardInput == ss::Stdin) si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	else
	{
		HANDLE f;

		if(mStandardInput == ss::Interactive)
		{
			HANDLE readPipe, writePipe;
			if(!CreatePipe(&readPipe, &writePipe, &sa, 0))
			{
				qDebug() << "CreatePipe error";
			}
			f = readPipe;
			tmpHandles.push_back(HandleCloser(writePipe));
		}
		else
		{
			std::vector<wchar_t> str(mStandardInput.length() + 1, 0);
			mStandardInput.toWCharArray(&str[0]);

			f = CreateFile(&str[0], GENERIC_READ, FILE_SHARE_READ,
			               &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if(f == INVALID_HANDLE_VALUE) throw FileNotFound(mStandardInput);
		}
		si.hStdInput = f;
		handlesForAutoClose.push_back(HandleCloser(f));
	}

	if(mStandardOutput == ss::Stdout) si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	else
	{
		HANDLE f;

		if(mStandardOutput == ss::Interactive)
		{
			HANDLE readPipe, writePipe;
			CreatePipe(&readPipe, &writePipe, &sa, 0);
			f = writePipe;
			tmpHandles.push_back(HandleCloser(readPipe));
		}
		else
		{
			std::vector<wchar_t> str(mStandardOutput.length() + 1, 0);
			mStandardOutput.toWCharArray(&str[0]);
			f = CreateFile(&str[0], GENERIC_WRITE, 0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if(f == INVALID_HANDLE_VALUE) throw FileNotFound(mStandardOutput);
		}
		si.hStdOutput = f;
		handlesForAutoClose.push_back(HandleCloser(f));
	}

	if(mStandardError == ss::Stderr) si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	else
	{
		HANDLE f;

		if(mStandardError == ss::Interactive)
		{
			HANDLE readPipe, writePipe;
			CreatePipe(&readPipe, &writePipe, &sa, 0);
			f = writePipe;
			tmpHandles.push_back(HandleCloser(readPipe));
		}
		else
		{
			std::vector<wchar_t> str(mStandardError.length() + 1, 0);
			mStandardError.toWCharArray(&str[0]);
			f = CreateFile(&str[0], GENERIC_WRITE, 0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if(f == INVALID_HANDLE_VALUE) throw FileNotFound(mStandardError);
		}
		si.hStdError = f;
		handlesForAutoClose.push_back(HandleCloser(f));
	}

	PROCESS_INFORMATION pi;
	QString programPath = QFileInfo(mProgram).absoluteFilePath();
	QString cmdLine = "\"" + programPath + "\"";
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

	boost::shared_array<char> curDir;

	if(!mCurrentDirectory.isEmpty())
	{
		curDir = boost::shared_array<char>(new char[mCurrentDirectory.size() + 1]);
		strcpy_s(curDir.get(), mCurrentDirectory.size() + 1, mCurrentDirectory.toLocal8Bit().data());
	}
	else
	{
		QString currentDir = QFileInfo(programPath).absolutePath();
		curDir = boost::shared_array<char>(new char[currentDir.size() + 1]);
		strcpy_s(curDir.get(), currentDir.size() + 1, currentDir.toLocal8Bit().data());
	}

	if(!CreateProcessA(NULL, cmdLine.toLocal8Bit().data(), &sa, NULL, TRUE,
					   CREATE_SUSPENDED, NULL, curDir.get(), &si, &pi)) throw CannotStartProcess(mProgram);
	handlesForAutoClose.clear();

	auto pop = [&tmpHandles]() -> HandleCloser { auto res = tmpHandles[0]; tmpHandles.pop_front(); return res;};
	if(mStandardInput == ss::Interactive) mInputHandle = pop();
	if(mStandardOutput == ss::Interactive) mOutputHandle = pop();
	if(mStandardError == ss::Interactive) mErrorHandle = pop();

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
	auto res = WaitForSingleObject(mCurrentInformation.hProcess, milliseconds);
	if(res == WAIT_TIMEOUT)
	{
		return false;
	}
	if(res == WAIT_OBJECT_0)
	{
		if(mProcessStatus.load() == psRunning) mProcessStatus.store(psExited);
		doFinalize();
		while(mIsRunning.load()) Sleep(0);
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
	if(isRunning()) return peakMemoryUsageS();
	return mPeakMemoryUsage.load();
}

// Сколько процессорного времени израсходовал процесс
int checklib::details::RestrictedProcessImpl::CPUTime()
{
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
	terminate();
	wait();

	mStandardInput = ss::Stdin;
	mStandardOutput = ss::Stdout;
	mStandardError = ss::Stderr;
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

bool checklib::details::RestrictedProcessImpl::sendDataToStandardInput(const QString &data, bool newLine)
{
	mutex_locker lock(mHandlesMutex);
	if(!isRunning()) return false;
	DWORD count;
	if(!WriteFile(mInputHandle.handle(), data.toLocal8Bit().data(), data.length(), &count, NULL))
	{
		qDebug() << "WriteFile1 error";
		return false;
	}
	if(newLine)
	{
		char c = '\n';
		if(!WriteFile(mInputHandle.handle(), &c, 1, &count, NULL))
		{
			qDebug() << "WriteFile2 error";
			return false;
		}
	}
	return true;
}

bool checklib::details::RestrictedProcessImpl::getDataFromStandardOutput(QString &data)
{
	if(!isRunning()) return false;
	if(mStandardOutput != ss::Interactive) return false;

	const int MAX = 100;
	char buf[MAX];
	data = "";
	while(true)
	{
		DWORD count = 0;
		if(!ReadFile(mOutputHandle.handle(), buf, MAX - 1, &count, NULL))
		{
			qDebug() << "Readfile error";
			return false;
		}
		buf[count] = 0;
		data += buf;
		if(data.endsWith("\r\n"))
		{
			data.resize(data.size() - 2);
			break;
		}
	}

	return true;
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
			qWarning() << "Process status is invalid";
		}

		if(mOutputHandle.handle() != INVALID_HANDLE_VALUE)
		{
			if(!CancelIoEx(mOutputHandle.handle(), NULL))
			{
				qWarning() << "IO cannot be canceled";
			}
		}

		if(!TerminateProcess(mCurrentInformation.hProcess, -1))
		{
			qWarning() << "TerminateProcess failed";
		}
		else
		{
			WaitForSingleObject(mCurrentInformation.hProcess, INFINITE);
		}
	}

	DWORD tmpExitCode;
	if(!GetExitCodeProcess(mCurrentInformation.hProcess, &tmpExitCode))
	{
		qWarning() << "Cannot get exit code process";
	}
	// Определение исключения делается через код возврата. Через JOB_OBJECT_MSG_ABNORMAL_EXIT_PROCESS будет надежнее
	switch(tmpExitCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:
	case EXCEPTION_DATATYPE_MISALIGNMENT:
	case EXCEPTION_BREAKPOINT:
	case EXCEPTION_SINGLE_STEP:
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
	case EXCEPTION_FLT_DENORMAL_OPERAND:
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
	case EXCEPTION_FLT_INEXACT_RESULT:
	case EXCEPTION_FLT_INVALID_OPERATION:
	case EXCEPTION_FLT_OVERFLOW:
	case EXCEPTION_FLT_STACK_CHECK:
	case EXCEPTION_FLT_UNDERFLOW:
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
	case EXCEPTION_INT_OVERFLOW:
	case EXCEPTION_PRIV_INSTRUCTION:
	case EXCEPTION_IN_PAGE_ERROR:
	case EXCEPTION_ILLEGAL_INSTRUCTION:
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
	case EXCEPTION_STACK_OVERFLOW:
	case EXCEPTION_INVALID_DISPOSITION:
	case EXCEPTION_GUARD_PAGE:
	case EXCEPTION_INVALID_HANDLE:
		mProcessStatus.store(psRuntimeError);
	}

	mExitCode.store(tmpExitCode);

	mInputHandle.reset();
	mOutputHandle.reset();
	mErrorHandle.reset();
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
		{
			void* cstr;
			FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPSTR) &cstr,
				0,
				NULL
			);
			qWarning() << "Wait failed: " << QString::fromLocal8Bit((char*)cstr) <<
						", isRunning =" << isRunning();
			LocalFree(cstr);
		}
		break;
	}
}

void checklib::details::RestrictedProcessImpl::destroyHandles()
{
	{
		mutex_locker lock(mHandlesMutex);
		if(!isRunning()) return;
		CloseHandle(mCurrentInformation.hProcess);
		CloseHandle(mCurrentInformation.hThread);

		mIsRunning.store(false);
	}
	emit finished(mExitCode);
}
