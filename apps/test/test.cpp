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
	emit testCompleted();
}

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
	mProcess->setStandardInput(inputFileName);
	mProcess->setStandardOutput(outputFileName);
	mProcess->start();
	mProcess->wait();
}

ParamsReader::ParamsReader(const QString &settingsFileName)
	: mSettings(settingsFileName, QSettings::IniFormat)
{
	mProgramName = mSettings.value("RunProgram").toString();

	mLimits.useTimeLimit = mSettings.value("TimeLimit", 1).toString().toLower() != "no";
	mLimits.timeLimit = mSettings.value("TimeLimit", 1).toInt() * 1000;

	mLimits.useMemoryLimit = mSettings.value("MemoryLimit", 64).toString().toLower() != "no";
	mLimits.useMemoryLimit = mSettings.value("MemoryLimit", 64).toInt() * 1024 * 1024;

	readLimits();
}

QString ParamsReader::programName() const
{
	return mProgramName;
}

checklib::Limits ParamsReader::limits() const
{
	return mLimits;
}

std::vector<OneTest> ParamsReader::tests() const
{
	return mTests;
}

void ParamsReader::readLimits()
{
	auto testNumberS = mSettings.value("TestNumber", "auto").toString().toLower();
	int testNumber;
	bool autoTestNumber = (testNumberS == "auto");
	if(autoTestNumber)
	{
		bool flag;
		testNumber = testNumberS.toInt(&flag);
		if(!flag) testNumber = 1;
	}

	QString testInput = mSettings.value("TestInput", "00").toString();
	int zStart, zEnd;

	auto getZerosPos = [&zStart, &zEnd](const QString &str)
	{
		zStart = str.indexOf('0');
		zEnd = zStart;
		if(zStart == -1) return;
		while(zEnd < str.length() && str[zEnd] == '0') ++zEnd;
	};

	auto getFileName = [zStart, zEnd](const QString &str, int testNumber) -> QString
	{
		QString tmp = QString::number(testNumber);
		while(tmp.length() < zEnd - zStart) tmp = "0" + tmp;
		return str.left(zStart) +
				tmp +
				str.right(str.length() - zEnd);
	};

	getZerosPos(testInput);

	for(int i = 0; i < testNumber; ++i)
	{
		OneTest tmp;

		tmp.inputFile = getFileName(testInput, i + 1);

		if(!QFile(tmp.inputFile).exists())
		{
			if(!autoTestNumber) throw std::exception("input file(s) was not found");
			break;
		}
		mTests.push_back(tmp);
	}
	testNumber = int(mTests.size());
	QString testOutput = mSettings.value("TestOutput", "00.a").toString();
	getZerosPos(testOutput);

	for(auto i = 0; i < testNumber; ++i)
	{
		mTests[i].outputFile = getFileName(testOutput, i + 1);
	}
}
