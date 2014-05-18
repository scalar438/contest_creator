#include "run_controller.h"
#include "console_utils.h"
#include <functional>
using namespace cu;
using namespace checklib;

RunController::RunController(boost::asio::io_service &io, ParamsReader &reader)
	: mIo(io)
	, mReader(reader)
	, mTimer(mIo)
{
	mProcess.setProgram(reader.programName);
	mProcess.setLimits(mReader.limits);

	mProcess.finished.connect(std::bind(&RunController::onProgramFinished, this, std::placeholders::_1));
}

void RunController::startTesting()
{
	mCurrentTest = 0;
	mIo.post(std::bind(&RunController::nextTest, this));
	mTimer.expires_from_now(boost::posix_time::milliseconds(500));
	mTimer.async_wait(std::bind(&RunController::printUsageTimerHandler, this, std::placeholders::_1));
}

void RunController::nextTest()
{
	mCurrentTest++;
	if(mCurrentTest > int(mReader.tests.size())) endTesting();

	mProcess.reset();
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
			std::cout << textColor(red) << "Time limit exceeded";
		}
		break;

	case psMemoryLimitExceeded:
		{
			std::cout << textColor(red) << "Memory limit exceeded";
		}
		break;

	case psIdlenessLimitExceeded:
		{
			std::cout << textColor(red) << "Idleness limit exceeded";
		}
		break;

	case psRuntimeError:
		{
			std::cout << textColor(red) << "Runtime error";
		}
		break;

	case psExited:
		{
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

	std::cout << std::endl;
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
	std::cout << cursorPosition(0) << "Test " << mCurrentTest << ": ";
	std::cout << mProcess.CPUTime() << " ms ";
	std::cout << mProcess.peakMemoryUsage() / 1024 << " kB ";
}

void RunController::endTesting()
{
	mTimer.cancel();
}
