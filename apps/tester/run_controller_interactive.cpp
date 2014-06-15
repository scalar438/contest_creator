#include "run_controller_interactive.h"
#include "console_utils.h"
#include <functional>

// Поток, читающий выходные данные одной программы и подающий считанные данные другой
class PipeThread
{
public:
	PipeThread(checklib::RestrictedProcess &readProgram, checklib::RestrictedProcess &writeProgram)
		: mReadProgram(readProgram)
		, mWriteProgram(writeProgram)
	{

	}

	void operator()()
	{
		std::string str;
		while(mReadProgram.getDataFromStandardOutput(str))
		{
			std::cerr << "Program: " << mReadProgram.program() << ", data: " << str << std::endl;
			mWriteProgram.sendDataToStandardInput(str, true);
		}
	}
private:

	checklib::RestrictedProcess &mReadProgram, &mWriteProgram;
};

RunControllerInteractive::RunControllerInteractive(boost::asio::io_service &io, ParamsReader &reader)
	: mIo(io)
	, mTimer(io)
	, mReader(reader)
{
	mProgram.setProgram(mReader.programName);
	mProgram.setLimits(mReader.limits);

	mInteractor.setProgram(mReader.interactorName);
	std::cerr << "InteractorName: " << mReader.interactorName << std::endl;
	checklib::Limits limits;
	limits.useTimeLimit = true;
	limits.useMemoryLimit = true;
	limits.timeLimit = 30 * 1000;
	limits.memoryLimit = 1024 * 1024 * 1024;
	mInteractor.setLimits(limits);
}

RunControllerInteractive::~RunControllerInteractive()
{
	mProgramReaderThread.join();
	mProgramWriterThread.join();
}

void RunControllerInteractive::startTesting()
{
	mTimer.expires_from_now(boost::posix_time::milliseconds(500));
	mTimer.async_wait(std::bind(&RunControllerInteractive::printUsageTimerHandler, this, std::placeholders::_1));
	mCurrentTest = 0;

	mIo.post(std::bind(&RunControllerInteractive::startCurrentTest, this));
}

void RunControllerInteractive::printUsageTimerHandler(boost::system::error_code err)
{
	if(err) return;
	std::cout << cu::cursorPosition(0);
	printUsage(false);
	std::cout.flush();

	mTimer.expires_from_now(boost::posix_time::milliseconds(500));
	mTimer.async_wait(std::bind(&RunControllerInteractive::printUsageTimerHandler, this, std::placeholders::_1));
}

void RunControllerInteractive::printUsage(bool final)
{
	using namespace cu;
	ColorSaver saver;

	if(final)
	{
		std::cout << textColor(yellow) << cursorPosition(0) << "Test " << mCurrentTest + 1 << ": ";
		std::cout << textColor(white) << mProgram.CPUTime() << textColor(lightGray) << " ms ";
		std::cout << textColor(white) << mProgram.peakMemoryUsage() / 1024 << textColor(lightGray) << " kB ";
	}
	else
	{
		std::cout << textColor(olive) << cursorPosition(0) << "Test " << mCurrentTest + 1 << ": ";
		std::cout << textColor(lightGray) << mProgram.CPUTime() << " ms ";
		std::cout << mProgram.peakMemoryUsage() / 1024 << " kB ";
	}
	std::cout << std::flush;
}

void RunControllerInteractive::startCurrentTest()
{
	if(mCurrentTest < 0 || mCurrentTest >= int(mReader.tests.size()))
	{
		throw std::logic_error("Attempting to start non-exit test");
	}

	mProgram.reset();
	mInteractor.reset();

	mProgram.setStandardInput(checklib::ss::Interactive);
	mProgram.setStandardOutput(checklib::ss::Interactive);

	mInteractor.setStandardInput(checklib::ss::Interactive);
	mInteractor.setStandardOutput(checklib::ss::Interactive);
	std::vector<std::string> vs = {
		mReader.tests[mCurrentTest].inputFile,
		mReader.outputFile
	};
	mInteractor.setParams(vs);

	mProgram.start();
	mInteractor.start();
	mProgramReaderThread.swap(std::thread(PipeThread(mProgram, mInteractor)));
	mProgramWriterThread.swap(std::thread(PipeThread(mInteractor, mProgram)));
}
