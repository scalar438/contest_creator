#pragma once
#include "checklib/rp.h"

#include <QObject>
#include <QString>
#include <QSettings>
#include <QFile>
#include <vector>
#include <memory>

struct OneTest
{
	QString inputFile;
	QString answerFile;
};

class ParamsReader;
class Runner;
class ResourceManager;

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

	void onTestFinished(int);

protected:

	void timerEvent(QTimerEvent *arg) override;

private:

	const ParamsReader *mReader;

	Runner *mRunner;

	int mCurrentTest;

	int mCheckTimer;

	bool mIsRunning;

	int mNumberOfDigits;

	std::shared_ptr<ResourceManager> mResourceManager;

	void printUsage();

	void beginTest();

	std::string toString(int n, int digits = 1);
};

// Управляет входными и выходными данными тестируемой программы и автоматически
// их удаляет
class ResourceManager
{
public:
	ResourceManager(const QString &inputFile, const QString &outputFile, const QString &testFile);
	~ResourceManager();

private:
	QString mInputFile, mOutputFile, mTestFile;
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

	QString programName;
	QString checker;
	QString inputFile;
	QString outputFile;
	checklib::Limits limits;
	std::vector<OneTest> tests;
	bool interrupt;
	// 0 - do not generate, 1 - generate, 2 - generate missing
	int genAnswers;
private:

	QSettings mSettings;
	// Читает и парсит названия входных и выходных файлов
	void readTests();
};
