#pragma once

#include "../rp_types.h"

#include <memory>
#include <atomic>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#include <QDateTime>
#include <QString>
#include <QStringList>

namespace checklib
{
namespace details
{

class RestrictedProcessImpl : public QObject
{
	Q_OBJECT
public:
	RestrictedProcessImpl(QObject *parent);
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

	// Отправить буфер в указанный стандартный поток.
	// Если этот поток направлен в файл, или программа не запущена, то ничего не произойдет
	bool sendDataToStandardInput(const QString &data, bool newLine);

	// Получить буфер из стандартного потока вывода
	bool getDataFromStandardOutput(QString &data);

signals:

	void finished();

private:
	QString mProgram;
	QStringList mParams;
	QDateTime mStartTime, mEndTime;

	QString mStandardInput, mStandardOutput, mStandardError;

	QString mCurrentDirectory;

	std::atomic<ProcessStatus> mProcessStatus;
	std::atomic<int> mExitCode;

	Limits mLimits;

	pid_t mChildPid;

	typedef boost::lock_guard<boost::mutex> mutex_locker;

	boost::mutex mTimerMutex;
	boost::mutex mHandlesMutex;
	boost::asio::deadline_timer mTimer;

	mutable std::atomic<int> mCPUTime, mPeakMemoryUsage;
	std::atomic<bool> mIsRunning;

	int mInputPipe[2], mOutputPipe[2], mErrorPipe[2];

	// Количество тиков на секунду.
	float mTicks;

	void doCheck();

	void doFinalize();

	void timerHandler(const boost::system::error_code &err);

	int peakMemoryUsageS() const;

	int CPUTimeS() const;
};

}
}
