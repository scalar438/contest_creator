#include "rp_win.h"
#include "../checklib_exception.h"
#include "../rp_consts.h"
#include "../timer_service.h"

#include <boost/filesystem.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/thread.hpp>

#include <deque>

#include <Windows.h>
#include <iostream>
#include <string>
#include <strsafe.h>

checklib::details::RestrictedProcessImpl::RestrictedProcessImpl()
    : mTimer(TimerService::instance()->io_service())
{
	mStandardInput  = ss::Stdin;
	mStandardOutput = ss::Stdout;
	mStandardError  = ss::Stderr;
	reset();
}

checklib::details::RestrictedProcessImpl::~RestrictedProcessImpl()
{
	doFinalize();
	while (mIsRunning.load())
		Sleep(0);
}

std::string checklib::details::RestrictedProcessImpl::getProgram() const
{
	return mProgram;
}

void checklib::details::RestrictedProcessImpl::setProgram(std::string program)
{
	mProgram = std::move(program);
}

std::vector<std::string> checklib::details::RestrictedProcessImpl::getParams() const
{
	return mParams;
}

void checklib::details::RestrictedProcessImpl::setParams(std::vector<std::string> params)
{
	mParams = std::move(params);
}

std::string checklib::details::RestrictedProcessImpl::currentDirectory() const
{
	return mCurrentDirectory;
}

void checklib::details::RestrictedProcessImpl::setCurrentDirectory(std::string directory)
{
	mCurrentDirectory = std::move(directory);
}

bool checklib::details::RestrictedProcessImpl::isRunning() const
{
	return mIsRunning.load();
}

void checklib::details::RestrictedProcessImpl::start()
{
	if (isRunning()) return;

	STARTUPINFOA si;
	memset(&si, 0, sizeof si);
	si.cb      = sizeof si;
	si.dwFlags = STARTF_USESTDHANDLES;

	SECURITY_ATTRIBUTES sa;
	sa.bInheritHandle       = TRUE;
	sa.lpSecurityDescriptor = NULL;
	sa.nLength              = sizeof sa;
	std::vector<Handle> handlesForAutoClose; // Хендлы, требующие закрытия на выходе из функции
	std::deque<Handle> tmpHandles; // Хендлы, требующие закрытия после окончания работы программы

	if (mStandardInput == ss::Stdin)
		si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	else
	{
		HANDLE f;

		if (mStandardInput == ss::Interactive)
		{
			HANDLE readPipe, writePipe;
			CreatePipe(&readPipe, &writePipe, &sa, 0);
			SetHandleInformation(writePipe, HANDLE_FLAG_INHERIT, 0);
			f = readPipe;
			tmpHandles.push_back(Handle(writePipe));
		}
		else
		{
			f = CreateFileA(mStandardInput.c_str(), GENERIC_READ, FILE_SHARE_READ, &sa,
			                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (f == INVALID_HANDLE_VALUE) throw CannotOpenFile(mStandardInput);
		}
		si.hStdInput = f;
		handlesForAutoClose.push_back(Handle(f));
	}

	if (mStandardOutput == ss::Stdout)
		si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	else
	{
		HANDLE f;

		if (mStandardOutput == ss::Interactive)
		{
			HANDLE readPipe, writePipe;
			CreatePipe(&readPipe, &writePipe, &sa, 0);
			SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);
			f = writePipe;
			tmpHandles.push_back(Handle(readPipe));
		}
		else
		{
			f = CreateFileA(mStandardOutput.c_str(), GENERIC_WRITE, 0, &sa, CREATE_ALWAYS,
			                FILE_ATTRIBUTE_NORMAL, NULL);
			if (f == INVALID_HANDLE_VALUE) throw CannotOpenFile(mStandardOutput);
		}
		si.hStdOutput = f;
		handlesForAutoClose.push_back(Handle(f));
	}

	if (mStandardError == ss::Stderr)
		si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	else
	{
		HANDLE f;

		if (mStandardError == ss::Interactive)
		{
			HANDLE readPipe, writePipe;
			CreatePipe(&readPipe, &writePipe, &sa, 0);
			SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);
			f = writePipe;
			tmpHandles.push_back(Handle(readPipe));
		}
		else
		{
			f = CreateFileA(mStandardError.c_str(), GENERIC_WRITE, 0, &sa, CREATE_ALWAYS,
			                FILE_ATTRIBUTE_NORMAL, NULL);
			if (f == INVALID_HANDLE_VALUE) throw CannotOpenFile(mStandardError);
		}
		si.hStdError = f;
		handlesForAutoClose.push_back(Handle(f));
	}
	PROCESS_INFORMATION pi;

	boost::filesystem::path programPath(mProgram);
	std::string cmdLine;
	{
		auto tmp = programPath.native();
		cmdLine  = std::string(tmp.begin(), tmp.end());
	}
	for (size_t i = 0; i < mParams.size(); i++)
	{
		cmdLine += " ";
		if (std::find(mParams[i].begin(), mParams[i].end(), ' ') != mParams[i].end())
		{
			cmdLine += '\"';
			cmdLine += mParams[i];
			cmdLine += '\"';
		}
		else
			cmdLine += mParams[i];
	}
	std::vector<char> cmdLineVector(cmdLine.length() + 1);
	std::copy(cmdLine.begin(), cmdLine.end(), cmdLineVector.data());
	cmdLineVector[cmdLine.length()] = 0;

	std::vector<char> curDir;

	if (!mCurrentDirectory.empty())
	{
		curDir.resize(mCurrentDirectory.size() + 1);
		strcpy_s(curDir.data(), mCurrentDirectory.size() + 1, mCurrentDirectory.c_str());
	}
	else
	{
		// TODO: возможно, имеет смысл брать текущую рабочую директорию вместо пути к запускаемой
		// программе
		auto tmpCurrentDir = programPath.branch_path().native();
		std::string currentDir(tmpCurrentDir.begin(), tmpCurrentDir.end());
		curDir.resize(currentDir.size() + 1);
		strcpy_s(curDir.data(), currentDir.size() + 1, currentDir.c_str());
	}

	if (!CreateProcessA(NULL, &cmdLineVector[0], &sa, NULL, TRUE, CREATE_SUSPENDED, NULL,
	                    curDir.data(), &si, &pi))
		throw CannotStartProcess(mProgram);

	auto pop = [&tmpHandles]() -> Handle {
		auto res = tmpHandles[0];
		tmpHandles.pop_front();
		return res;
	};
	if (mStandardInput == ss::Interactive) mInputHandle = pop();
	if (mStandardOutput == ss::Interactive) mOutputHandle = pop();
	if (mStandardError == ss::Interactive) mErrorHandle = pop();

	ResumeThread(pi.hThread);
	mProcessStatus.store(psRunning);
	mCurrentInformation = pi;
	auto err            = WaitForSingleObject(pi.hProcess, 0);
	mutex_locker guard(mTimerMutex);
	mTimer.expires_from_now(boost::posix_time::milliseconds(100));
	mTimer.async_wait(boost::bind(&checklib::details::RestrictedProcessImpl::timerHandler,
	                              boost::ref(*this), boost::lambda::_1));
	mNotChangedTimeCount = 0;
	mIsRunning.store(true);
}

void checklib::details::RestrictedProcessImpl::terminate()
{
	if (isRunning())
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
	if (!isRunning()) return false;
	auto res = WaitForSingleObject(mCurrentInformation.hProcess, milliseconds);
	if (res == WAIT_TIMEOUT)
	{
		return false;
	}
	if (res == WAIT_OBJECT_0)
	{
		if (mProcessStatus.load() == psRunning) mProcessStatus.store(psExited);
		doFinalize();
		mIsRunning.store(false);
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
	if (isRunning()) return peakMemoryUsageS();
	return mPeakMemoryUsage.load();
}

// Сколько процессорного времени израсходовал процесс
int checklib::details::RestrictedProcessImpl::CPUTime()
{
	if (isRunning()) return CPUTimeS();
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

	mInputHandle.reset();
	mOutputHandle.reset();
	mErrorHandle.reset();

	mCPUTime.store(0);
	mPeakMemoryUsage.store(0);
	mProcessStatus.store(psNotRunning);
	mIsRunning.store(false);
}

checklib::Limits checklib::details::RestrictedProcessImpl::getLimits() const
{
	return mLimits;
}

void checklib::details::RestrictedProcessImpl::setLimits(const Limits &limits)
{
	if (isRunning()) return;
	mLimits = limits;
}

void checklib::details::RestrictedProcessImpl::redirectStandardInput(const std::string &fileName)
{
	mStandardInput = fileName;
}

void checklib::details::RestrictedProcessImpl::redirectStandardOutput(const std::string &fileName)
{
	mStandardOutput = fileName;
}

void checklib::details::RestrictedProcessImpl::redirectStandardError(const std::string &fileName)
{
	mStandardError = fileName;
}

bool checklib::details::RestrictedProcessImpl::sendDataToStandardInput(const std::string &data,
                                                                       bool newLine)
{
	mutex_locker lock(mHandlesMutex);
	if (!isRunning()) return false;
	DWORD count;
	if (!WriteFile(mInputHandle.handle(), data.c_str(), data.length(), &count, NULL))
	{
		return false;
	}
	if (newLine)
	{
		char c = '\n';
		if (!WriteFile(mInputHandle.handle(), &c, 1, &count, NULL))
		{
			return false;
		}
	}
	return true;
}

bool checklib::details::RestrictedProcessImpl::getDataFromStandardOutput(std::string &data)
{
	if (!isRunning()) return false;
	if (mStandardOutput != ss::Interactive) return false;

	const int MAX = 100;
	char buf[MAX];
	data = "";
	while (true)
	{
		DWORD count = 0;
		if (!ReadFile(mOutputHandle.handle(), buf, MAX - 1, &count, NULL))
		{
			return false;
		}
		buf[count] = 0;
		data += buf;
		if (data.size() >= 2 && data.substr(data.size() - 2, 2) == "\r\n")
		{
			data.resize(data.size() - 2);
			break;
		}
	}

	return true;
}

bool checklib::details::RestrictedProcessImpl::closeStandardInput()
{
	if (mStandardInput == ss::Interactive && mInputHandle.handle() != INVALID_HANDLE_VALUE)
	{
		bool res = CloseHandle(mInputHandle.handle());
		mInputHandle.reset();
		return res;
	}
	return false;
}

void checklib::details::RestrictedProcessImpl::doCheck()
{
	int oldCPUTime = mCPUTime.load();
	int time       = CPUTime();
	if (mLimits.useTimeLimit)
	{
		if (time > mLimits.timeLimit)
		{
			mProcessStatus.store(psTimeLimitExceeded);
			doFinalize();
			return;
		}
	}
	if (time == oldCPUTime)
	{
		if (mNotChangedTimeCount++ > 20)
		{
			mProcessStatus.store(psIdlenessLimitExceeded);
			doFinalize();
			return;
		}
	}
	else
	{
		mNotChangedTimeCount = 1;
	}
	if (mLimits.useMemoryLimit)
	{
		if (peakMemoryUsage() > mLimits.memoryLimit)
		{
			mProcessStatus.store(psMemoryLimitExceeded);
			doFinalize();
		}
	}
}

void checklib::details::RestrictedProcessImpl::doFinalize()
{
	mutex_locker lock1(mHandlesMutex);
	if (!isRunning()) return;
	// Сохранить параметры перед закрытием
	CPUTimeS();
	peakMemoryUsageS();

	if (mLimits.useTimeLimit && mCPUTime.load() > mLimits.timeLimit)
	{
		mProcessStatus.store(psTimeLimitExceeded);
	}
	if (mLimits.useMemoryLimit && mPeakMemoryUsage.load() > mLimits.memoryLimit)
	{
		mProcessStatus.store(psMemoryLimitExceeded);
	}

	if (WaitForSingleObject(mCurrentInformation.hProcess, 0) == WAIT_TIMEOUT)
	{
		if (mProcessStatus.load() == psRunning)
		{
			throw std::logic_error("Process status is invalid");
		}

		if (!TerminateProcess(mCurrentInformation.hProcess, -1))
		{
			throw Exception("Cannot terminate process");
		}
		else
		{
			WaitForSingleObject(mCurrentInformation.hProcess, INFINITE);
		}
	}

	DWORD tmpExitCode;
	if (!GetExitCodeProcess(mCurrentInformation.hProcess, &tmpExitCode))
	{
		throw std::logic_error("Cannot get exit code");
	}
	// TODO: Сделать через JOB_OBJECT_MSG_ABNORMAL_EXIT_PROCESS - надежнее
	switch (tmpExitCode)
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
	case EXCEPTION_INVALID_HANDLE: mProcessStatus.store(psRuntimeError);
	}

	mExitCode.store(tmpExitCode);

	mInputHandle.reset();
	mOutputHandle.reset();
	mErrorHandle.reset();
}

void checklib::details::RestrictedProcessImpl::timerHandler(const boost::system::error_code &err)
{
	if (err) return;
	doCheck();

	switch (WaitForSingleObject(mCurrentInformation.hProcess, 0))
	{
	case WAIT_OBJECT_0:
		if (mProcessStatus.load() == psRunning) mProcessStatus.store(psExited);
		doFinalize();
		destroyHandles();
		break;
	case WAIT_TIMEOUT: {
		mutex_locker lock(mTimerMutex);
		mTimer.expires_from_now(boost::posix_time::milliseconds(100));
		mTimer.async_wait(boost::bind(&checklib::details::RestrictedProcessImpl::timerHandler,
		                              boost::ref(*this), boost::lambda::_1));
	}
	break;
	case WAIT_FAILED: {
		void *cstr;
		FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
		               GetLastError(),
		               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		               (LPSTR)&cstr, 0, NULL);
		LocalFree(cstr);
	}
	break;
	}
}

void checklib::details::RestrictedProcessImpl::destroyHandles()
{
	{
		mutex_locker lock(mHandlesMutex);
		if (!isRunning()) return;
		CloseHandle(mCurrentInformation.hProcess);
		CloseHandle(mCurrentInformation.hThread);

		mIsRunning.store(false);
	}
}
