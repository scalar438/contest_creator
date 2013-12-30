#include "Runner.h"
#include "checklib/checklib_exception.h"

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
	mProcess->setLimits(mLimits);
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
