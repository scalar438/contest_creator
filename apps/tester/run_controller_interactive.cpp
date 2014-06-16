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
	mProgram.finished.connect(std::bind(&RunControllerInteractive::onProgramFinished, this, std::placeholders::_1));

	mInteractor.setProgram(mReader.interactorName);
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
	if(err || mCurrentTest >= mReader.tests.size()) return;
	std::cout << cu::cursorPosition(0);
	printUsage(false);
	std::cout.flush();

	mTimer.expires_from_now(boost::posix_time::milliseconds(500));
	mTimer.async_wait(std::bind(&RunControllerInteractive::printUsageTimerHandler, this, std::placeholders::_1));
}

void RunControllerInteractive::printUsage(bool final)
{
	std::cout << std::endl << std::this_thread::get_id().hash() << std::endl;
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
	mInteractor.setParams({mReader.tests[mCurrentTest].inputFile,
	                       mReader.outputFile
	                      });

	mProgram.start();
	mInteractor.start();
#ifdef _DEBUG
	// Для проверки на зависание потока
	if(mProgramReaderThread.joinable()) mProgramReaderThread.join();
	if(mProgramWriterThread.joinable()) mProgramWriterThread.join();
#else
	if(mProgramReaderThread.joinable()) mProgramReaderThread.detach();
	if(mProgramWriterThread.joinable()) mProgramWriterThread.detach();
#endif
	mProgramReaderThread = std::thread(PipeThread(mProgram, mInteractor));
	mProgramWriterThread = std::thread(PipeThread(mInteractor, mProgram));
}

void RunControllerInteractive::programFinished()
{
	using namespace checklib;
	using namespace cu;

	printUsage(true);
	bool failed = true;
	if(!mInteractor.isRunning())
	{
		{
			ColorSaver saver;
			std::cout << textColor(red) << "Wrong answer";
		}
		std::cout << ": interactor was ended before program" << std::endl;
	}
	else
	{
		switch(mProgram.processStatus())
		{
			case psExited:
			{
				checklib::RestrictedProcess rp;
				rp.setProgram(mReader.checker);
				rp.setParams({mReader.tests[mCurrentTest].inputFile,
							  mReader.outputFile,
							  mReader.tests[mCurrentTest].answerFile
							 });
				rp.start();
				rp.wait();

				if(rp.exitCode() == 0)
				{
					failed = false;
				}
			}
			break;

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
		}
	}

	++mCurrentTest;
	if(mCurrentTest != mReader.tests.size() &&
			(mReader.interrupt || !failed))
	{
		mIo.post(std::bind(&RunControllerInteractive::startCurrentTest, this));
	}
	else
	{
		endTesting();
	}
}

void RunControllerInteractive::onProgramFinished(int exitCode)
{
	mIo.post(std::bind(&RunControllerInteractive::programFinished, this));
}

void RunControllerInteractive::endTesting()
{
	mTimer.cancel();
}
