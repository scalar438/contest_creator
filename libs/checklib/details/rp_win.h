#pragma once

#include "../rp_types.h"

#include <memory>
// Без этого moc-компилятор падает при попытке распарсить эти заголовочники
#ifndef Q_MOC_RUN
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>
#endif

#include <windows.h>
#include <psapi.h>

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QDebug>

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

class RestrictedProcessImpl : public QObject
{
	Q_OBJECT
public:
	RestrictedProcessImpl(QObject *parent = nullptr);
	~RestrictedProcessImpl();

	QString getProgram() const;
	void setProgram(const QString &program);

	QStringList getParams() const;
	void setParams(const QStringList &params);

	QString currentDirectory() const;
	void setCurrentDirectory(const QString &directory);


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

	void redirectStandardInput(const QString &fileName);
	void redirectStandardOutput(const QString &fileName);
	void redirectStandardError(const QString &fileName);

	bool sendDataToStandardInput(const QString &data, bool newLine);
	bool getDataFromStandardOutput(QString &data);

signals:

	void finished(int exitCode);

private:
	QString mProgram;
	QStringList mParams;
	QString mCurrentDirectory;

	QDateTime mStartTime;

	QString mStandardInput, mStandardOutput, mStandardError;

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
