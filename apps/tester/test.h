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

class ParamsReader;
class Runner;

// Класс, обеспечивающий запуск программы
class Tester : public QObject
{
	Q_OBJECT
public:
	Tester(const ParamsReader *reader, Runner *runner);
	~Tester();

public slots:

	void startTesting();

signals:

	void nextTest(QString inputFileName, QString outputFileName);

	void testCompleted();

private slots:

	void onTestFinished(int exitCode);

protected:

	void timerEvent(QTimerEvent *arg) override;

private:

	const ParamsReader *mReader;

	Runner *mRunner;

	int mCurrentTest;

	int mCheckTimer;

	std::vector<OneTest> mTests;
	bool mIsRunning;

	void printUsage();
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

class ParamsReader
{
public:
	ParamsReader(const QString &settingsFileName);

	QString programName() const;
	checklib::Limits limits() const;
	std::vector<OneTest> tests() const;
	QString checker() const;
	bool interrupt() const;
	bool genAnswers() const;
private:
	QSettings mSettings;
	QString mProgramName;
	QString mChecker;
	checklib::Limits mLimits;
	std::vector<OneTest> mTests;
	bool mInterrupt;
	bool mGenAnswers;

	// Читает и парсит названия входных и выходных файлов
	void readTests();
};
