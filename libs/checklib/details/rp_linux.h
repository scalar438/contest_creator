#pragma once

#include "../rp_types.h"

#include <memory>
#include <atomic>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/signals2.hpp>

namespace checklib
{
namespace details
{

class Pipe
{
private:
	struct PipeCloser
	{
		PipeCloser(int pp)
			: p(pp)
		{

		}
		~PipeCloser()
		{
			close(p);
		}
		int p;
	};

	std::shared_ptr<PipeCloser> p;
public:
	explicit Pipe(int pp = -1)
		: p(new PipeCloser(pp))
	{
	}

	int pipe() const
	{
		if(p) return p->p;
		return -1;
	}

	void reset()
	{
		p.reset();
	}
};

class RestrictedProcessImpl
{
public:
	RestrictedProcessImpl();
	~RestrictedProcessImpl();

	std::string getProgram() const;
	void setProgram(const std::string &program);

	const std::vector<std::string> &getParams() const;
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

	// Отправить буфер в указанный стандартный поток.
	// Если этот поток направлен в файл, или программа не запущена, то ничего не произойдет
	bool sendDataToStandardInput(const std::string &data, bool newLine);

	// Получить буфер из стандартного потока вывода
	bool getDataFromStandardOutput(std::string &data);

	bool closeStandardInput();

	boost::signals2::signal<void(int)> finished;

private:
	std::string mProgram;
	std::vector<std::string> mParams;
	//QDateTime mStartTime, mEndTime;

	std::string mStandardInput, mStandardOutput, mStandardError;

	std::string mCurrentDirectory;

	std::atomic<ProcessStatus> mProcessStatus;
	std::atomic<int> mExitCode;

	Limits mLimits;

	pid_t mChildPid;

	typedef boost::lock_guard<boost::mutex> mutex_locker;

	boost::mutex mTimerMutex;
	boost::mutex mHandlesMutex;
	boost::asio::deadline_timer mTimer;

	// Сколько тиков таймера прошло без изменения CPU time, нужно для определения IL
	int mUnchangedTicks;
	int mOldCPUTime;

	mutable std::atomic<int> mCPUTime, mPeakMemoryUsage;
	std::atomic<bool> mIsRunning;

	Pipe mInputPipe, mOutputPipe, mErrorPipe;

	// Количество тиков на секунду.
	float mTicks;

	// Интервал таймера проверки в миллисекундах
	const static int sTimerDuration = 100;

	void doCheck();

	void doFinalize();

	void timerHandler(const boost::system::error_code &err);

	int peakMemoryUsageS() const;

	int CPUTimeS() const;
};

}
}
