#include "params_reader.h"
#include "tester_exceptions.h"
#include "io_consts.h"

#include <fstream>
#include <boost/algorithm/string.hpp>

ParamsReader::ParamsReader(const std::string &settingsFileName)
	: mSettings(settingsFileName)
{
	if(!mSettings.contains("RunProgram")) throw TesterException("Program must be set");
	programName = mSettings.readString("RunProgram");

	std::string tmpString = mSettings.readString("TimeLimit", "1");
	boost::to_lower(tmpString);

	limits.useTimeLimit = !(tmpString == "no" || tmpString == "0");
	if(limits.useTimeLimit)
	{
		bool flag;
		limits.timeLimit = mSettings.readDouble("TimeLimit", flag);
		if(!flag) throw TesterException("Time limit is invalid");
	}

	tmpString = mSettings.readString("MemoryLimit", "64");
	boost::to_lower(tmpString);
	limits.useMemoryLimit = !(tmpString == "no" || tmpString == "0");
	if(limits.useMemoryLimit)
	{
		bool flag;
		limits.memoryLimit = mSettings.readDouble("MemoryLimit", flag) * 1024 * 1024;
		if(!flag) throw TesterException("Memory limit is invalid");
	}

	tmpString = mSettings.readString("Interrupt", "YES");
	boost::to_lower(tmpString);
	interrupt = tmpString == "yes" ||
				mSettings.readString("Interrupt", "YES") == "1";

	if(!mSettings.contains("Checker")) throw TesterException("Checker must be set");
	checker = mSettings.readString("Checker");

	tmpString = mSettings.readString("GenAnswers", "Auto");
	boost::to_lower(tmpString);
	if(tmpString == "yes" || tmpString == "1") genAnswers = GenerateAlways;
	else if(tmpString == "no" || tmpString == "0") genAnswers = NotGenerate;
	else genAnswers = GenerateMissing;

	readTests();

	inputFile = mSettings.readString("InputFile", Stdin);
	outputFile = mSettings.readString("OutputFile", Stdout);
}

void ParamsReader::readTests()
{
	auto testNumberS = mSettings.readString("TestNumber", "auto");
	boost::to_lower(testNumberS);
	int testNumber;
	bool autoTestNumber = (testNumberS == "auto");
	if(!autoTestNumber)
	{
		bool flag;
		testNumber = mSettings.readInt("TestNumber", flag);
		if(!flag) testNumber = 1;
	}
	else testNumber = 1000000000;

	if(!mSettings.contains("TestInput")) throw TesterException("Input files in not exists");
	std::string testInput = mSettings.readString("TestInput");
	int zStart, zEnd;

	// Вспомогательные функции
	auto getZerosPos = [&zStart, &zEnd](const std::string & str)
	{
		zStart = 0;
		while(zStart != (int)str.length() && str[zStart] != '0') ++zStart;
		zEnd = zStart;
		if(zStart == (int)str.length()) return;
		while(zEnd != (int)str.length() && str[zEnd] == '0') ++zEnd;
	};

	auto getFileName = [&zStart, &zEnd](const std::string &str, int testNumber) -> std::string
	{
		std::string tmp;
		while(testNumber)
		{
			tmp += (testNumber % 10 + '0');
			testNumber /= 10;
		}
		std::reverse(tmp.begin(), tmp.end());
		while(int(tmp.length()) < zEnd - zStart) tmp = "0" + tmp;

		return str.substr(0, zStart) + tmp + str.substr(zEnd);
	};

	auto fileExists = [](const std::string &str) -> bool
	{
		std::ifstream is(str);
		return is.good();
	};

	getZerosPos(testInput);
	for(int i = 0; i < testNumber; ++i)
	{
		OneTest tmp;

		tmp.inputFile = getFileName(testInput, i + 1);
		if(!fileExists(tmp.inputFile))
		{
			if(!autoTestNumber) throw TesterException("input file(s) was not found");
			break;
		}
		tests.push_back(tmp);
	}
	if(tests.empty()) throw TesterException("input file(s) was not found");
	testNumber = int(tests.size());
	std::string testOutput = mSettings.readString("TestAnswer", "00.a");
	getZerosPos(testOutput);

	for(int i = 0; i < testNumber; ++i)
	{
		tests[i].answerFile = getFileName(testOutput, i + 1);
	}
}
