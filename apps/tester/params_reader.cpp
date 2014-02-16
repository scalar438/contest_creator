#include "params_reader.h"
#include "TesterExceptions.h"
#include <QFile>

ParamsReader::ParamsReader(const QString &settingsFileName)
	: mSettings(settingsFileName.toStdString())
{
	if(!mSettings.contains("RunProgram")) throw TesterException("Program must be set");
	programName = QString::fromStdString(mSettings.readString("RunProgram"));

	QString tmpString = QString::fromStdString(mSettings.readString("TimeLimit", "1"));

	limits.useTimeLimit = !(tmpString == "no" || tmpString == "0");
	if(limits.useTimeLimit)
	{
		bool flag;
		limits.timeLimit = static_cast<int>(tmpString.toDouble(&flag) * 1000);
		if(!flag) throw TesterException("Time limit is invalid");
	}

	tmpString = QString::fromStdString(mSettings.readString("MemoryLimit", "64")).toLower();
	limits.useMemoryLimit = !(tmpString == "no" || tmpString == "0");
	if(limits.useMemoryLimit)
	{
		bool flag;
		limits.memoryLimit = tmpString.toInt(&flag) * 1024 * 1024;
		if(!flag) throw TesterException("Memory limit is invalid");
	}

	interrupt = QString::fromStdString(mSettings.readString("Interrupt", "YES")).toLower() == "yes" ||
				mSettings.readString("Interrupt", "YES") == "1";

	if(!mSettings.contains("Checker")) throw TesterException("Checker must be set");
	checker = QString::fromStdString(mSettings.readString("Checker"));

	{
		QString tmp = QString::fromStdString(mSettings.readString("GenAnswers", "Auto")).toLower();
		if(tmp == "yes" || tmp == "1") genAnswers = GenerateAlways;
		else if(tmp == "no" || tmp == "0") genAnswers = NotGenerate;
		else genAnswers = GenerateMissing;
	}

	readTests();

	inputFile = QString::fromStdString(mSettings.readString("InputFile", "#STDIN"));
	outputFile = QString::fromStdString(mSettings.readString("OutputFile", "#STDOUT"));
}

void ParamsReader::readTests()
{
	auto testNumberS = QString::fromStdString(mSettings.readString("TestNumber", "auto")).toLower();
	int testNumber;
	bool autoTestNumber = (testNumberS == "auto");
	if(!autoTestNumber)
	{
		bool flag;
		testNumber = testNumberS.toInt(&flag);
		if(!flag) testNumber = 1;
	}
	else testNumber = 1000000000;

	if(!mSettings.contains("TestInput")) throw TesterException("Input files in not exists");
	QString testInput = QString::fromStdString(mSettings.readString("TestInput"));
	int zStart, zEnd;

	// Вспомогательные функции
	auto getZerosPos = [&zStart, &zEnd](const QString & str)
	{
		zStart = str.indexOf('0');
		zEnd = zStart;
		if(zStart == -1) return;
		while(zEnd < str.length() && str[zEnd] == '0') ++zEnd;
	};

	auto getFileName = [&zStart, &zEnd](const QString &str, int testNumber) -> QString
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
			if(!autoTestNumber) throw TesterException("input file(s) was not found");
			break;
		}
		tests.push_back(tmp);
	}
	if(tests.empty()) throw TesterException("input file(s) was not found");
	testNumber = int(tests.size());
	QString testOutput = QString::fromStdString(mSettings.readString("TestAnswer", "00.a"));
	getZerosPos(testOutput);

	for(int i = 0; i < testNumber; ++i)
	{
		tests[i].answerFile = getFileName(testOutput, i + 1);
	}
}
