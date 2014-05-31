#include "run_controller.h"
#include "console_utils.h"
#include "io_consts.h"
#include <boost/filesystem.hpp>
#include <functional>
using namespace cu;
using namespace checklib;

RunController::RunController(boost::asio::io_service &io, ParamsReader &reader)
	: mIo(io)
	, mTimer(mIo)
	, mReader(reader)
{
	mProcess.setProgram(reader.programName);
	mProcess.setLimits(mReader.limits);

	mProcess.finished.connect(std::bind(&RunController::onProgramFinished, this, std::placeholders::_1));
}

void RunController::startTesting()
{
	mCurrentTest = -1;
	mIo.post(std::bind(&RunController::nextTest, this));
	mTimer.expires_from_now(boost::posix_time::milliseconds(500));
	mTimer.async_wait(std::bind(&RunController::printUsageTimerHandler, this, std::placeholders::_1));

	// На случай, если этот файл уже существует
	boost::filesystem::remove(mReader.inputFile);
}

void RunController::nextTest()
{
	mCurrentTest++;
	if(mCurrentTest == int(mReader.tests.size()))
	{
		endTesting();
		return;
	}

	mProcess.reset();

	boost::filesystem::copy(mReader.tests[mCurrentTest].inputFile, mReader.inputFile);
	if(mReader.inputFile == Stdin) mProcess.setStandardInput(mReader.inputFile);
	if(mReader.outputFile == Stdout) mProcess.setStandardOutput(mReader.outputFile);

	mProcess.start();
}

void RunController::endCurrrentTest()
{
	ColorSaver saver;
	printUsage(true);

	bool failed = true;

	switch(mProcess.processStatus())
	{
	case psTimeLimitExceeded:
		{
			std::cout << textColor(red) << "Time limit exceeded" << std::endl;
		}
		break;

	case psMemoryLimitExceeded:
		{
			std::cout << textColor(red) << "Memory limit exceeded" << std::endl;
		}
		break;

	case psIdlenessLimitExceeded:
		{
			std::cout << textColor(cyan) << "Idleness limit exceeded" << std::endl;
		}
		break;

	case psRuntimeError:
		{
			std::cout << textColor(red) << "Runtime error" << std::endl;
		}
		break;

	case psExited:
		{
			if(mReader.genAnswers == ParamsReader::GenerateAlways ||
					mReader.genAnswers == ParamsReader::GenerateMissing &&
					!boost::filesystem::exists(mReader.tests[mCurrentTest].answerFile))
			{
				boost::filesystem::copy(mReader.outputFile, mReader.tests[mCurrentTest].answerFile);
			}

			checklib::RestrictedProcess checker;
			checker.setProgram(mReader.checker);
			checker.setParams(
				{mReader.tests[mCurrentTest].inputFile, mReader.outputFile, mReader.tests[mCurrentTest].answerFile});
			checker.start();
			checker.wait();
			failed = checker.exitCode() != 0;
		}
		break;

	default:
		throw std::logic_error("Unexpected exit type of the process");
	}

	boost::filesystem::remove(mReader.inputFile);
	boost::filesystem::remove(mReader.outputFile);

	if(!failed || !mReader.interrupt)
	{
		mIo.post(std::bind(&RunController::nextTest, this));
	}
	else
	{
		endTesting();
	}
}

void RunController::printUsageTimerHandler(boost::system::error_code err)
{
	if(err) return;
	std::cout << cursorPosition(0);
	printUsage(false);
	std::cout.flush();

	mTimer.expires_from_now(boost::posix_time::milliseconds(500));
	mTimer.async_wait(std::bind(&RunController::printUsageTimerHandler, this, std::placeholders::_1));
}

void RunController::onProgramFinished(int)
{
	mIo.post(std::bind(&RunController::endCurrrentTest, this));
}

void RunController::printUsage(bool final)
{
	using namespace cu;
	ColorSaver saver;

	if(final)
	{
		std::cout << textColor(yellow) << cursorPosition(0) << "Test " << mCurrentTest + 1 << ": ";
		std::cout << textColor(white) << mProcess.CPUTime() << textColor(lightGray) << " ms ";
		std::cout << textColor(white) << mProcess.peakMemoryUsage() / 1024 << textColor(lightGray) << " kB ";
	}
	else
	{
		std::cout << textColor(olive) << cursorPosition(0) << "Test " << mCurrentTest + 1 << ": ";
		std::cout << textColor(lightGray) << mProcess.CPUTime() << " ms ";
		std::cout << mProcess.peakMemoryUsage() / 1024 << " kB ";
	}
}

void RunController::endTesting()
{
	mTimer.cancel();
}
