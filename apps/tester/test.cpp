#include "test.h"
#include "consoleUtils.h"

#include <iostream>

#include <QSettings>
#include <QFile>
#include <QDebug>
#include <QTimerEvent>

//-----------------------------------------------------
// Tester
//-----------------------------------------------------

Tester::Tester(const ParamsReader *reader, Runner *runner)
	: mReader(reader),
	  mRunner(runner)
{
	QObject::connect(runner, &Runner::finished, this, &Tester::onTestFinished, Qt::QueuedConnection);
	QObject::connect(this, &Tester::nextTest, runner, &Runner::startTest, Qt::QueuedConnection);

	mIsRunning = false;
	mCheckTimer = this->startTimer(200);
}

Tester::~Tester()
{

}

void Tester::onTestFinished(int exitCode)
{
	std::cout << cu::textColor(cu::white);
	printUsage();
}

void Tester::timerEvent(QTimerEvent *arg)
{
	if(mIsRunning && arg->timerId() == mCheckTimer)
	{
		// TODO: Сделать нормальную заливку - с определением ширины через GetConsoleBufferInfo
		std::cout << cu::textColor(cu::lightGray) << cu::cursorPosition(0);
		for(int i = 0; i < 70; ++i) std::cout << " ";

		printUsage();
	}
}

void Tester::printUsage()
{
	std::cout << cu::cursorPosition(0) << "Test " << mCurrentTest + 1 << ": ";
}

void Tester::beginTest()
{
	mResourceManager = std::make_shared<ResourceManager>(mReader->tests[mCurrentTest].inputFile,
														 mReader->inputFile, mReader->outputFile);
	mIsRunning = true;

	emit nextTest(mReader->tests[mCurrentTest].inputFile, mReader->outputFile);
}

void Tester::startTesting()
{
	mCurrentTest = 0;

	beginTest();
}

//----------------------------------------------
// Runner
//----------------------------------------------

Runner::Runner(const QString &programName, checklib::Limits limits)
	: mProcess(new checklib::RestrictedProcess(this))
{
	mLimits = limits;
	// TODO добавить парсинг параметров
	mProcess->setProgram(programName);

	connect(mProcess, &checklib::RestrictedProcess::finished, this, &Runner::finished);
}

int Runner::getTime() const
{
	return mProcess->CPUTime();
}

int Runner::getMemoryUsage() const
{
	return mProcess->peakMemoryUsage();
}

checklib::ProcessStatus Runner::getProcessStatus() const
{
	return mProcess->processStatus();
}

void Runner::startTest(QString inputFileName, QString outputFileName)
{
	qDebug() << "testStarted";
	if(inputFileName == "#STDIN") mProcess->setStandardInput(inputFileName);
	if(inputFileName == "#STDOUT") mProcess->setStandardOutput(outputFileName);
	mProcess->start();
	mProcess->wait();
}


//---------------------------------------------------
// ParamsReader
//---------------------------------------------------

ParamsReader::ParamsReader(const QString &settingsFileName)
	: mSettings(settingsFileName, QSettings::IniFormat)
{
	if(!mSettings.contains("RunProgram")) throw std::exception("Program must be set");
	programName = mSettings.value("RunProgram").toString();

	QString tmpString = mSettings.value("TimeLimit", 1).toString().toLower();

	limits.useTimeLimit = !(tmpString == "no" || tmpString == "0");
	if(limits.useTimeLimit)
	{
		bool flag;
		limits.timeLimit = tmpString.toInt(&flag) * 1000;
		if(!flag) throw std::exception("Time limit is invalid");
	}

	tmpString = mSettings.value("MemoryLimit", 64).toString().toLower();
	limits.useMemoryLimit = !(tmpString == "no" || tmpString == "0");
	if(limits.useMemoryLimit)
	{
		bool flag;
		limits.memoryLimit = tmpString.toInt(&flag) * 1024 * 1024;
		if(!flag) throw std::exception("Memory limit is invalid");
	}

	interrupt = mSettings.value("Interrupt", "YES").toString().toLower() == "yes" ||
			mSettings.value("Interrupt", "YES").toString() == "1";

	if(!mSettings.contains("Checker")) throw std::exception("Checker must be set");
	checker = mSettings.value("Checker").toString();

	genAnswers = mSettings.value("GenAnswers", "YES").toString().toLower() == "yes" ||
			mSettings.value("Interrupt", "YES").toString() == "1";

	readTests();

	inputFile = mSettings.value("InputFile", "#STDIN").toString();
	outputFile = mSettings.value("OutputFile", "#STDOUT").toString();
}

void ParamsReader::readTests()
{
	auto testNumberS = mSettings.value("TestNumber", "auto").toString().toLower();
	int testNumber;
	bool autoTestNumber = (testNumberS == "auto");
	if(!autoTestNumber)
	{
		bool flag;
		testNumber = testNumberS.toInt(&flag);
		if(!flag) testNumber = 1;
	}
	else testNumber = 1000000000;

	QString testInput = mSettings.value("TestInput", "00").toString();
	int zStart, zEnd;

	auto getZerosPos = [&zStart, &zEnd](const QString &str)
	{
		zStart = str.indexOf('0');
		zEnd = zStart;
		if(zStart == -1) return;
		while(zEnd < str.length() && str[zEnd] == '0') ++zEnd;
	};

	auto getFileName = [&zStart, &zEnd](const QString &str, int testNumber) -> QString
	{
		QString tmp = QString::number(testNumber);
		while(tmp.length() < zEnd - zStart) tmp = "0" + tmp;
		return str.left(zStart) +
				tmp +
				str.right(str.length() - zEnd);
	};

	getZerosPos(testInput);
	for(int i = 0; i < testNumber; ++i)
	{
		OneTest tmp;

		tmp.inputFile = getFileName(testInput, i + 1);
		if(!QFile(tmp.inputFile).exists())
		{
			if(!autoTestNumber) throw std::exception("input file(s) was not found");
			break;
		}
		tests.push_back(tmp);
	}
	if(tests.empty()) throw std::exception("input file(s) was not found");
	testNumber = int(tests.size());
	QString testOutput = mSettings.value("TestAnswer", "00.a").toString();
	getZerosPos(testOutput);

	for(int i = 0; i < testNumber; ++i)
	{
		tests[i].outputFile = getFileName(testOutput, i + 1);
	}
}


ResourceManager::ResourceManager(const QString &inputFile, const QString &outputFile, const QString &testFile)
	: mInputFile(inputFile), mOutputFile(outputFile), mTestFile(testFile)
{
//	if(QFile::exists(mInputFile)) QFile::remove(mInputFile);

	QFile::copy(testFile, mInputFile);
	QFile::remove(mOutputFile);
}

ResourceManager::~ResourceManager()
{
	QFile::remove(mInputFile);
	QFile::remove(mOutputFile);
}
