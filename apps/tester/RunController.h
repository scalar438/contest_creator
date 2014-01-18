#pragma once
#include "checklib/rp.h"

#include <QObject>
#include <QString>
#include <QSettings>
#include <QFile>
#include <vector>
#include <memory>

class ParamsReader;
class Runner;
class ResourceManager;

// Класс, обеспечивающий управление запуском программы
class RunController : public QObject
{
	Q_OBJECT
public:
	RunController(const ParamsReader *reader, Runner *runner);
	~RunController();

public slots:

	void startTesting();

signals:

	void nextTest(QString inputFileName, QString outputFileName);

	void testCompleted();

private slots:

	void onTestFinished(int);

	void onError(QString errorDescription);

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

	void printUsage(bool final);

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
