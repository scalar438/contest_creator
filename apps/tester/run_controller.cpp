#include "run_controller.h"
#include <functional>

RunController::RunController(boost::asio::io_service &io, ParamsReader &reader)
	: mIo(io)
	, mReader(reader)
{
	mProcess.setProgram(reader.programName);
}

void RunController::startTesting()
{
	mCurrentTest = 0;
	mIo.post(std::bind(&RunController::nextTest, this));
}

void RunController::nextTest()
{
	mCurrentTest++;
	if(mCurrentTest == int(mReader.tests.size())) return;
}

void RunController::checkResults()
{

}

void RunController::printUsageTimerHandler(boost::system::error_code err)
{
	if(err) return;

}

void RunController::onProgramFinished(int exitCode)
{

}

void RunController::printUsage(bool final)
{

}
