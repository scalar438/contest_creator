#include "test.h"

#include <QSettings>
#include <QFile>
#include <QDebug>

Tester::Tester()
{
}

Tester::~Tester()
{

}

void Tester::onTestFinished(int exitCode)
{

}

void Tester::startTesting()
{
	qDebug() << "start testing";
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

void Runner::startTest(QString inputFileName, QString outputFileName)
{
	mProcess.setStandardInput(inputFileName);
	mProcess.setStandardOutput(outputFileName);
	mProcess.start();
	mProcess.wait();
}


ParamsReader::ParamsReader(const QString &settingsFileName)
{

}

QString ParamsReader::programName() const
{
	return "";
}

checklib::Limits ParamsReader::limits() const
{
	return checklib::Limits();
}
