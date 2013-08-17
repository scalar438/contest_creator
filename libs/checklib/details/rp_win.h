#pragma once

#include "rp_types.h"

#include <windows.h>
#include <psapi.h>

#include <QObject>
#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QThread>
#include <QTimer>

//#include <boost/asio.hpp>

namespace checklib
{
namespace details
{

class ReadThread : public QThread
{

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

	bool isRunning() const;

	void start();
	void terminate();
	void wait();
	bool wait(int milliseconds);

	/// Код возврата.
	int exitCode() const;

	/// Тип завершения программы
	ProcessStatus processStatus() const;

	/// Пиковое значение потребляемой памяти
	int peakMemoryUsage() const;

	/// Сколько процессорного времени израсходовал процесс
	int CPUTime() const;

	Limits getLimits() const;
	void setLimits(const Limits &restrictions);

	void redirectStandardInput(const QString &fileName);
	void redirectStandardOutput(const QString &fileName);
	void redirectStandardError(const QString &fileName);

	void sendBufferToStandardStream(StandardStream stream, const QByteArray &data);
public:

	void run();

private:
	HANDLE mProcess;

	QString mProgram;
	QStringList mParams;
	QDateTime mStartTime, mEndTime;

	QString mStandardInput, mStandardOutput, mStandardError;

	ProcessStatus mProcessStatus;
	int mExitCode;

	Limits mLimits;

	PROCESS_INFORMATION mCurrentInformation;

	QTimer mCheckTimer;

	void doCheck();

private slots:

	void checkTimerTimeout();

};

}
}
