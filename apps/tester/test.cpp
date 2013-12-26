#include "test.h"
#include "consoleUtils.h"
#include "checklib/checklib_exception.h"

#include <iostream>

#include <QSettings>
#include <QFile>
#include <QDebug>
#include <QTimerEvent>
#include <QProcess>

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

void Tester::onTestFinished(int)
{
	mIsRunning = false;
	std::cout << cu::textColor(cu::white);
	printUsage();

	bool needContinue = false;

	{
		cu::ColorSaver saver;
		switch(mRunner->getProcessStatus())
		{
		case checklib::psRuntimeError:
			std::cout << cu::textColor(cu::red) << "Runtime error";
			break;
		case checklib::psTimeLimitExceeded:
			std::cout << cu::textColor(cu::red) << "Time limit exceeded";
			break;
		case checklib::psMemoryLimitExceeded:
			std::cout << cu::textColor(cu::red) << "Memory limit exceeded";
			break;
		case checklib::psIdlenessLimitExceeded:
			std::cout << cu::textColor(cu::red) << "Idleness limit exceeded";
			break;
		case checklib::psExited:
			needContinue = true;
			break;
		default:
			throw std::logic_error("Unexpected process status");
		}
	}

	if(needContinue)
	{
		checklib::RestrictedProcess process;
		process.setProgram(mReader->checker);
		process.setParams(QStringList() << mReader->inputFile <<
					 mReader->outputFile << mReader->tests[mCurrentTest].answerFile);
		process.start();
		process.wait();

		mCurrentTest++;

		needContinue = !(mCurrentTest == (int)mReader->tests.size() ||
				process.exitCode() && mReader->interrupt);
	}

	if(needContinue)
	{
		beginTest();
	}
	else
	{
		emit testCompleted();
	}
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
	std::cout << cu::cursorPosition(0) << "Test " << mCurrentTest + 1 << ": "
				 << mRunner->getTime() << " ms " << mRunner->getMemoryUsage() / 1024 << " KBytes ";
}

void Tester::beginTest()
{
	mResourceManager.reset();
	mResourceManager = std::make_shared<ResourceManager>(mReader->inputFile,
	                   mReader->outputFile,
	                   mReader->tests[mCurrentTest].inputFile);

	mIsRunning = true;

	emit nextTest(mReader->inputFile, mReader->outputFile);
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
	mProcess->reset();
	if(inputFileName.toUpper() == "#STDIN") mProcess->setStandardInput(inputFileName);
	if(outputFileName.toUpper() == "#STDOUT") mProcess->setStandardOutput(outputFileName);
	try
	{
		mProcess->start();
	}
	catch(checklib::Exception &)
	{
		emit finished(-1);
		return;
	}
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

	{
		QString tmp = mSettings.value("GenAnswers", "YES").toString().toLower();
		if(tmp == "yes" || tmp == "1") genAnswers = 1;
		else if(tmp == "0" || tmp == "no") genAnswers = 0;
		else genAnswers = 2;
	}

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

	if(!mSettings.contains("TestInput")) throw std::exception("Input files in not exists");
	QString testInput = mSettings.value("TestInput").toString();
	int zStart, zEnd;

	// Вспомогательные функции
	auto getZerosPos = [&zStart, &zEnd](const QString & str)
	{
		zStart = str.indexOf('0');
		zEnd = zStart;
		if(zStart == -1) return;
		while(zEnd < str.length() && str[zEnd] == '0') ++zEnd;
	};

	auto getFileName = [&zStart, &zEnd](const QString & str, int testNumber) -> QString
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
		tests[i].answerFile = getFileName(testOutput, i + 1);
	}
}


ResourceManager::ResourceManager(const QString &inputFile, const QString &outputFile, const QString &testFile)
	: mInputFile(inputFile), mOutputFile(outputFile), mTestFile(testFile)
{
	if(QFile::exists(mInputFile)) QFile::remove(mInputFile);

	QFile::copy(testFile, mInputFile);
	QFile::remove(mOutputFile);
}

ResourceManager::~ResourceManager()
{
	QFile::remove(mInputFile);
	QFile::remove(mOutputFile);
}
