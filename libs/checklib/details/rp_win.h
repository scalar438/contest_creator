#pragma once

#include "../rp_types.h"
#include "i_process.hpp"

#include <memory>
#include <string>
#include <vector>

#include <atomic>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/signals2.hpp>
#include <mutex>
#include <thread>

#include <Psapi.h>
#include <Windows.h>

namespace checklib::details
{

// RAII handle wrapper with reference counter
class Handle
{
public:
	explicit Handle(HANDLE h = INVALID_HANDLE_VALUE)
	    : ptr(std::shared_ptr<HANDLE>(new HANDLE(h), HandleCloser()))
	{}

	void set_handle(HANDLE h) { ptr = std::shared_ptr<HANDLE>(new HANDLE(h), HandleCloser()); }

	HANDLE handle() const
	{
		if (ptr)
			return *ptr;
		else
			return INVALID_HANDLE_VALUE;
	}

	void reset() { set_handle(INVALID_HANDLE_VALUE); }

private:
	struct HandleCloser
	{
		void operator()(HANDLE *h)
		{
			if (*h != INVALID_HANDLE_VALUE)
			{
				CloseHandle(h);
			}
		}
	};

	std::shared_ptr<HANDLE> ptr;
};

class RestrictedProcessImpl : public checklib::details::IProcess
{
public:
	RestrictedProcessImpl();
	~RestrictedProcessImpl();

	std::string getProgram() const;
	void setProgram(std::string program);

	std::vector<std::string> getParams() const;
	void setParams(std::vector<std::string> params);

	std::string currentDirectory() const;
	void setCurrentDirectory(std::string directory);

	void start();
	void terminate();
	void wait();

	void start(const ProcessExecuteParameters &) override;

	// From IProcess
	bool wait(int milliseconds) override;

	// Код возврата.
	std::optional<int> exit_code() const override;

	void kill() override;

	// Тип завершения программы
	ProcessStatus processStatus() const;

	// Пиковое значение потребляемой памяти
	int peak_memory_usage() override;

	// Сколько процессорного времени израсходовал процесс
	int cpu_time() override;

	void reset();

	Limits getLimits() const;
	void setLimits(const Limits &limits);

	void redirectStandardInput(const std::string &fileName);
	void redirectStandardOutput(const std::string &fileName);
	void redirectStandardError(const std::string &fileName);

	bool sendDataToStandardInput(const std::string &data, bool newLine);
	bool getDataFromStandardOutput(std::string &data);

	bool closeStandardInput();

private:
	std::string mProgram;
	std::vector<std::string> mParams;
	std::string mCurrentDirectory;

	std::string mStandardInput, mStandardOutput, mStandardError;

	// Сколько раз подряд при измерении процессорного времени оно не менялось.
	// Если больше определенного лимита - программа не выполняется, проставляем
	// IDLENESS_LIMIT_EXCEEDED
	int mNotChangedTimeCount;

	std::atomic<ProcessStatus> mProcessStatus;
	std::atomic<int> mExitCode;

	Limits mLimits;

	PROCESS_INFORMATION mCurrentInformation;

	typedef std::lock_guard<std::mutex> mutex_locker;

	std::mutex mTimerMutex;
	std::mutex mHandlesMutex;
	boost::asio::deadline_timer mTimer;

	mutable std::atomic<int> mCPUTime, mPeakMemoryUsage;
	std::atomic<bool> mIsRunning;

	Handle mInputHandle, mOutputHandle, mErrorHandle;

	void doCheck();

	void doFinalize();

	void destroyHandles();

	void timerHandler(const boost::system::error_code &err);

	int peakMemoryUsageS() const;

	int CPUTimeS() const;
};

} // namespace checklib::details
