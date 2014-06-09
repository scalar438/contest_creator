#include "run_controller_simple.h"
#include "console_utils.h"
#include "io_consts.h"
#include <boost/filesystem.hpp>
#include <functional>
#include <iostream>
using namespace cu;
using namespace checklib;

RunControllerSimple::RunControllerSimple(boost::asio::io_service &io, ParamsReader &reader)
	: mIo(io)
	, mTimer(mIo)
	, mReader(reader)
{
	mProcess.setProgram(reader.programName);
	mProcess.setLimits(mReader.limits);

	mProcess.finished.connect(std::bind(&RunControllerSimple::onProgramFinished, this, std::placeholders::_1));
}

void RunControllerSimple::startTesting()
{
	mCurrentTest = 0;
	mIo.post(std::bind(&RunControllerSimple::startCurrentTest, this));
	mTimer.expires_from_now(boost::posix_time::milliseconds(500));
	mTimer.async_wait(std::bind(&RunControllerSimple::printUsageTimerHandler, this, std::placeholders::_1));

	// На случай, если этот файл уже существует
	boost::filesystem::remove(mReader.inputFile);
}

void RunControllerSimple::startCurrentTest()
{
	if(mCurrentTest < 0 || mCurrentTest >= int(mReader.tests.size()))
	{
		throw std::logic_error("Attempting to start non-exit test");
	}

	mProcess.reset();

	boost::filesystem::copy(mReader.tests[mCurrentTest].inputFile, mReader.inputFile);
	if(mReader.inputFile == Stdin) mProcess.setStandardInput(mReader.inputFile);
	if(mReader.outputFile == Stdout) mProcess.setStandardOutput(mReader.outputFile);

	mProcess.start();
}

void RunControllerSimple::endCurrrentTest()
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
			if((mReader.genAnswers == ParamsReader::GenerateMissing &&
			        !boost::filesystem::exists(mReader.tests[mCurrentTest].answerFile)) ||
			        mReader.genAnswers == ParamsReader::GenerateAlways)
			{
				boost::filesystem::remove(mReader.tests[mCurrentTest].answerFile);
				boost::filesystem::copy(mReader.outputFile, mReader.tests[mCurrentTest].answerFile);
				std::cout << "Answer file was created" << std::endl;
			}
			else
			{
				checklib::RestrictedProcess checker;
				checker.setProgram(mReader.checker);
				checker.setParams(
				{mReader.tests[mCurrentTest].inputFile, mReader.outputFile, mReader.tests[mCurrentTest].answerFile});
				checker.start();
				checker.wait();
				failed = checker.exitCode() != 0;
			}
		}
		break;

	default:
		throw std::logic_error("Unexpected exit type of the process");
	}

	boost::filesystem::remove(mReader.inputFile);
	boost::filesystem::remove(mReader.outputFile);

	mCurrentTest++;

	if(mCurrentTest != int(mReader.tests.size()) && (!failed || !mReader.interrupt))
	{
		mIo.post(std::bind(&RunControllerSimple::startCurrentTest, this));
	}
	else
	{
		endTesting();
	}
}

void RunControllerSimple::printUsageTimerHandler(boost::system::error_code err)
{
	if(err || mCurrentTest == mReader.tests.size()) return;
	std::cout << cursorPosition(0);
	printUsage(false);
	std::cout.flush();

	mTimer.expires_from_now(boost::posix_time::milliseconds(500));
	mTimer.async_wait(std::bind(&RunControllerSimple::printUsageTimerHandler, this, std::placeholders::_1));
}

void RunControllerSimple::onProgramFinished(int)
{
	mIo.post(std::bind(&RunControllerSimple::endCurrrentTest, this));
}

void RunControllerSimple::printUsage(bool final)
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
	std::cout << std::flush;
}

void RunControllerSimple::endTesting()
{
	mTimer.cancel();
}
