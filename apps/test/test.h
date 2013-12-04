#pragma once
#include "checklib/rp.h"

#include <QObject>
#include <QString>
#include <QSettings>
#include <vector>

struct OneTest
{
	QString inputFile;
	QString outputFile;
};

class ParamsReader
{
public:
	ParamsReader(const QString &settingsFileName);

	QString programName() const;
	checklib::Limits limits() const;
	std::vector<OneTest> tests() const;
private:
	QSettings mSettings;
	QString mProgramName;
	checklib::Limits mLimits;
	std::vector<OneTest> mTests;

	void readLimits();
};

// Класс, обеспечивающий запуск программы
class Tester : public QObject
{
	Q_OBJECT
public:
	Tester();
	~Tester();

public slots:

	void onTestFinished(int exitCode);

	void startTesting();

signals:

	void nextTest(QString inputFileName, QString outputFileName);

	void testCompleted();
};

// Класс, живущий в отдельном потоке, запускающий программу и выдающий ее вердикты
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

private:

	checklib::RestrictedProcess *mProcess;

	checklib::Limits mLimits;
};
