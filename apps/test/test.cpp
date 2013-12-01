#include "test.h"

#include <QSettings>
#include <QFile>

Tester::Tester(const QString &settingsFileName)
{
	QSettings settings(settingsFileName);

}

Tester::~Tester()
{

}

void Tester::onCheckTimerTimeout()
{

}

void Tester::onTestFinished(int exitCode)
{

}


Runner::Runner(const QString &programName, checklib::Limits limits)
{
	mLimits = limits;
	// TODO добавить парсинг параметров
	mProcess.setProgram(programName);

	connect(&mProcess, &checklib::RestrictedProcess::finished, this, &Runner::finished);
}

int Runner::getTime() const
{
	return mProcess.CPUTime();
}

int Runner::getMemoryUsage() const
{
	return mProcess.peakMemoryUsage();
}

checklib::ProcessStatus Runner::getProcessStatus() const
{
	return mProcess.processStatus();
}

void Runner::run(QString inputFileName, QString outputFileName)
{
	mProcess.setStandardInput(inputFileName);
	mProcess.setStandardOutput(outputFileName);
	mProcess.start();
	mProcess.wait();
}
