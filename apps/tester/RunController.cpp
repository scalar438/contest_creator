#include "RunController.h"
#include "ParamsReader.h"
#include "Runner.h"
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

RunController::RunController(const ParamsReader *reader, Runner *runner)
	: mReader(reader),
	  mRunner(runner)
{
	QObject::connect(runner, &Runner::finished, this, &RunController::onTestFinished, Qt::QueuedConnection);
	QObject::connect(this, &RunController::nextTest, runner, &Runner::startTest, Qt::QueuedConnection);

	mIsRunning = false;
	mCheckTimer = this->startTimer(400);

	mNumberOfDigits = 0;
	int d = 1;
	while(int(mReader->tests.size()) >= d)
	{
		d *= 10;
		mNumberOfDigits++;
	}
}

RunController::~RunController()
{

}

void RunController::onTestFinished(int)
{
	mIsRunning = false;
	printUsage(true);

	bool normalExit = false;

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
		normalExit = true;
		break;
	default:
		throw std::logic_error("Unexpected process status");
	}

	bool needContinue = !mReader->interrupt;
	if(normalExit)
	{
		if(mReader->genAnswers == ParamsReader::GenerateAlways ||
				mReader->genAnswers == mReader->GenerateMissing && !QFile::exists(mReader->tests[mCurrentTest].answerFile))
		{
			QFile::remove(mReader->tests[mCurrentTest].answerFile);
			QFile::rename(mReader->outputFile, mReader->tests[mCurrentTest].answerFile);
			std::cout << cu::textColor(cu::standard) << "Answer file \"" <<
						 mReader->tests[mCurrentTest].answerFile.toStdString() <<
						 "\" was created\n";
			needContinue = true;
		}
		else
		{
			checklib::RestrictedProcess process;
			process.setProgram(mReader->checker);
			process.setParams(QStringList() << mReader->inputFile <<
							  mReader->outputFile << mReader->tests[mCurrentTest].answerFile);
			process.start();
			process.wait();
			needContinue = needContinue || process.exitCode() == 0;
		}
	}
	else
	{
		std::cout << std::endl;
	}

	mCurrentTest++;
	if(needContinue && (int)mReader->tests.size() != mCurrentTest)
	{
		beginTest();
	}
	else
	{
		emit testCompleted();
	}
}

void RunController::timerEvent(QTimerEvent *arg)
{
	if(mIsRunning && arg->timerId() == mCheckTimer)
	{
		printUsage(false);
	}
}

void RunController::printUsage(bool final)
{
	using namespace cu;
	std::cout << cursorPosition(0);
	if(final)
	{
		std::cout << textColor(lightGray) << "Test " << toString(mCurrentTest + 1, mNumberOfDigits) << ": "
		          << textColor(white) << mRunner->getTime() << textColor(lightGray) << " ms "
		          << textColor(white) << mRunner->getMemoryUsage() / 1024 << textColor(lightGray) << " KB ";
	}
	else
	{
		std::cout << textColor(darkGray) << "Test " << toString(mCurrentTest + 1, mNumberOfDigits) << ": "
		          << mRunner->getTime() << " ms "
		          << mRunner->getMemoryUsage() / 1024 << " KB ";
	}
}

void RunController::beginTest()
{
	mResourceManager.reset();
	mResourceManager = std::make_shared<ResourceManager>(mReader->inputFile,
	                   mReader->outputFile,
	                   mReader->tests[mCurrentTest].inputFile);

	mIsRunning = true;

	emit nextTest(mReader->inputFile, mReader->outputFile);
}

std::string RunController::toString(int n, int digits)
{
	std::string res;
	while(n)
	{
		res += n % 10 + '0';
		n /= 10;
	}
	while((int)res.length() < digits)
	{
		res += '0';
	}
	std::reverse(res.begin(), res.end());
	return res;
}

void RunController::startTesting()
{
	mCurrentTest = 0;

	beginTest();
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
