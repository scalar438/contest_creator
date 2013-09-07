#pragma once

#include "../rp_types.h"

#include <memory>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/atomic.hpp>

#include <windows.h>
#include <psapi.h>

#include <QObject>
#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QRunnable>

namespace checklib
{
namespace details
{

class RestrictedProcessImpl : public QObject, public std::enable_shared_from_this<RestrictedProcessImpl>
{
public:
	RestrictedProcessImpl(QObject *parent = nullptr);
	~RestrictedProcessImpl();

	QString getProgram() const;
	void setProgram(const QString &program);

	QStringList getParams() const;
	void setParams(const QStringList &params);

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
	int peakMemoryUsage() const;

	// Сколько процессорного времени израсходовал процесс
	int CPUTime() const;

	void reset();

	Limits getLimits() const;
	void setLimits(const Limits &restrictions);

	void redirectStandardInput(const QString &fileName);
	void redirectStandardOutput(const QString &fileName);
	void redirectStandardError(const QString &fileName);

	void sendBufferToStandardInput(const QByteArray &data);

private:
	QString mProgram;
	QStringList mParams;
	QDateTime mStartTime, mEndTime;

	QString mStandardInput, mStandardOutput, mStandardError;

	boost::atomic<ProcessStatus> mProcessStatus;
	boost::atomic<int> mExitCode;

	Limits mLimits;

	PROCESS_INFORMATION mCurrentInformation;

	boost::mutex mTimerMutex;
	boost::mutex mFinalizeMutex;
	boost::asio::deadline_timer mTimer;

	mutable boost::atomic<int> mOldCPUTime, mOldPeakMemoryUsage;
	boost::atomic<bool> mIsRunnig;

	void doCheck();

	void doFinalize();

	void timerHandler(const boost::system::error_code &err, std::shared_ptr<RestrictedProcessImpl> ptr);
};

}
}
