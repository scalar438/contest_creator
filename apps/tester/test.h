﻿#pragma once
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
	QString outputFile;
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

	void onTestFinished(int exitCode);

protected:

	void timerEvent(QTimerEvent *arg) override;

private:

	const ParamsReader *mReader;

	Runner *mRunner;

	int mCurrentTest;

	int mCheckTimer;

	bool mIsRunning;

	std::shared_ptr<ResourceManager> mResourceManager;

	void printUsage();

	void beginTest();
};

// Класс для копирования входных и выходных файлов и их очистки
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
	bool genAnswers;
private:

	QSettings mSettings;
	// Читает и парсит названия входных и выходных файлов
	void readTests();
};
