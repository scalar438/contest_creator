﻿#pragma once

#include "../rp_types.h"

#include <memory>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>

#include <windows.h>
#include <psapi.h>

#include <QDateTime>
#include <QString>
#include <QStringList>

namespace checklib
{
namespace details
{

class RestrictedProcessImpl
{
public:
	RestrictedProcessImpl();
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

	void sendBufferToStandardInput(const QByteArray &data);

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

	void doCheck();

	void doFinalize();

	void destroyHandles();

	void timerHandler(const boost::system::error_code &err);

	int peakMemoryUsageS() const;

	int CPUTimeS() const;
};

}
}
