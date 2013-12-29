#include "ParamsReader.h"
#include <QFile>

ParamsReader::ParamsReader(const QString &settingsFileName)
	: mSettings(settingsFileName, QSettings::IniFormat)
{
	if(!mSettings.contains("RunProgram")) throw std::exception("Program must be set");
	programName = mSettings.value("RunProgram").toString();

	QString tmpString = mSettings.value("TimeLimit", 1).toString().toLower();

	limits.useTimeLimit = !(tmpString == "no" || tmpString == "0");
	if(limits.useTimeLimit)
	{
		bool flag;
		limits.timeLimit = tmpString.toInt(&flag) * 1000;
		if(!flag) throw std::exception("Time limit is invalid");
	}

	tmpString = mSettings.value("MemoryLimit", 64).toString().toLower();
	limits.useMemoryLimit = !(tmpString == "no" || tmpString == "0");
	if(limits.useMemoryLimit)
	{
		bool flag;
		limits.memoryLimit = tmpString.toInt(&flag) * 1024 * 1024;
		if(!flag) throw std::exception("Memory limit is invalid");
	}

	interrupt = mSettings.value("Interrupt", "YES").toString().toLower() == "yes" ||
	            mSettings.value("Interrupt", "YES").toString() == "1";

	if(!mSettings.contains("Checker")) throw std::exception("Checker must be set");
	checker = mSettings.value("Checker").toString();

	{
		QString tmp = mSettings.value("GenAnswers", "YES").toString().toLower();
		if(tmp == "yes" || tmp == "1") genAnswers = 1;
		else if(tmp == "0" || tmp == "no") genAnswers = 0;
		else genAnswers = 2;
	}

	readTests();

	inputFile = mSettings.value("InputFile", "#STDIN").toString();
	outputFile = mSettings.value("OutputFile", "#STDOUT").toString();
}

void ParamsReader::readTests()
{
	auto testNumberS = mSettings.value("TestNumber", "auto").toString().toLower();
	int testNumber;
	bool autoTestNumber = (testNumberS == "auto");
	if(!autoTestNumber)
	{
		bool flag;
		testNumber = testNumberS.toInt(&flag);
		if(!flag) testNumber = 1;
	}
	else testNumber = 1000000000;

	if(!mSettings.contains("TestInput")) throw std::exception("Input files in not exists");
	QString testInput = mSettings.value("TestInput").toString();
	int zStart, zEnd;

	// Вспомогательные функции
	auto getZerosPos = [&zStart, &zEnd](const QString & str)
	{
		zStart = str.indexOf('0');
		zEnd = zStart;
		if(zStart == -1) return;
		while(zEnd < str.length() && str[zEnd] == '0') ++zEnd;
	};

	auto getFileName = [&zStart, &zEnd](const QString & str, int testNumber) -> QString
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
		tests.push_back(tmp);
	}
	if(tests.empty()) throw std::exception("input file(s) was not found");
	testNumber = int(tests.size());
	QString testOutput = mSettings.value("TestAnswer", "00.a").toString();
	getZerosPos(testOutput);

	for(int i = 0; i < testNumber; ++i)
	{
		tests[i].answerFile = getFileName(testOutput, i + 1);
	}
}
