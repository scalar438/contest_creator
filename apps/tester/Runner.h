#pragma once

#include "checklib/rp.h"

#include <QObject>
#include <QString>

// Запускает программу и выдает ее вердикты
class Runner : public QObject
{
	Q_OBJECT
public:
	Runner(const QString &programName, checklib::Limits limits);

	int getTime() const;

	int getMemoryUsage() const;

	checklib::ProcessStatus getProcessStatus() const;
public slots:
	void startTest(QString inputFileName, QString outputFileName);

signals:

	void finished(int exitCode);

	void error(QString errorDescription);

private:

	checklib::RestrictedProcess *mProcess;

	checklib::Limits mLimits;
};
