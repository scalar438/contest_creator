#pragma once

#include "../rp_types.h"

#include <memory>
#include <vector>
#include <string>

#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/signals2.hpp>
#include <boost/filesystem.hpp>

#include <windows.h>
#include <psapi.h>

namespace checklib
{
namespace details
{

class HandleCloser
{
private:
	struct AutoCloser
	{
		AutoCloser(HANDLE h = INVALID_HANDLE_VALUE)
		{
			setHandle(h);
		}

		~AutoCloser()
		{
			if(handle != INVALID_HANDLE_VALUE)
			{
				CloseHandle(handle);
			}
		}

		void setHandle(HANDLE h)
		{
			handle = h;
		}

		HANDLE handle;
	};

public:
	HandleCloser(HANDLE h = INVALID_HANDLE_VALUE)
		: ptr(new AutoCloser(h))
	{
	}

	void setHandle(HANDLE h)
	{
		ptr = std::shared_ptr<AutoCloser>(new AutoCloser(h));
	}

	HANDLE handle() const
	{
		if(ptr) return ptr->handle;
		else return INVALID_HANDLE_VALUE;
	}

	void reset()
	{
		setHandle(INVALID_HANDLE_VALUE);
	}

private:
	std::shared_ptr<AutoCloser> ptr;
};

class RestrictedProcessImpl
{
public:
	RestrictedProcessImpl();
	~RestrictedProcessImpl();

	std::string getProgram() const;
	void setProgram(const std::string &program);

	std::vector<std::string> getParams() const;
	void setParams(const std::vector<std::string> &params);

	std::string currentDirectory() const;
	void setCurrentDirectory(const std::string &directory);


	bool isRunning() const;

	void start();
	void terminate();
	void wait();
	bool wait(int milliseconds);

	// Код возврата.
	int exitCode() const;

	// Тип завершения программы
	ProcessStatus processStatus() const;

	// Пиковое значение потребляемой памяти
	int peakMemoryUsage();

	// Сколько процессорного времени израсходовал процесс
	int CPUTime();

	void reset();

	Limits getLimits() const;
	void setLimits(const Limits &limits);

	void redirectStandardInput(const std::string &fileName);
	void redirectStandardOutput(const std::string &fileName);
	void redirectStandardError(const std::string &fileName);

	bool sendDataToStandardInput(const std::string &data, bool newLine);
	bool getDataFromStandardOutput(std::string &data);

	boost::signals2::signal<void(int)> finished;

private:
	std::string mProgram;
	std::vector<std::string> mParams;
	std::string mCurrentDirectory;

	std::string mStandardInput, mStandardOutput, mStandardError;

	// Сколько раз подряд при измерении процессорного времени оно не менялось.
	// Если больше определенного лимита - программа не выполняется, проставляем IDLENESS_LIMIT_EXCEEDED
	int mNotChangedTimeCount;

	boost::atomic<ProcessStatus> mProcessStatus;
	boost::atomic<int> mExitCode;

	Limits mLimits;

	PROCESS_INFORMATION mCurrentInformation;

	typedef boost::lock_guard<boost::mutex> mutex_locker;

	boost::mutex mTimerMutex;
	boost::mutex mHandlesMutex;
	boost::asio::deadline_timer mTimer;

	mutable boost::atomic<int> mCPUTime, mPeakMemoryUsage;
	boost::atomic<bool> mIsRunning;

	HandleCloser mInputHandle, mOutputHandle, mErrorHandle;

	void doCheck();

	void doFinalize();

	void destroyHandles();

	void timerHandler(const boost::system::error_code &err);

	int peakMemoryUsageS() const;

	int CPUTimeS() const;
};

}
}
