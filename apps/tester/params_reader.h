#pragma once
#include "checklib/rp_types.h"
#include "settings.h"

#include <vector>

struct OneTest
{
	std::string inputFile;
	std::string answerFile;
};

class ParamsReader
{
public:
	ParamsReader(const std::string &settingsFileName);

	std::string programName;
	std::string checker;
	std::string inputFile;
	std::string outputFile;
	checklib::Limits limits;
	std::vector<OneTest> tests;
	bool interrupt;
	bool isInteractive;
	std::string interactorName;

	enum GenAnswersMode
	{
		NotGenerate, GenerateMissing, GenerateAlways
	};

	GenAnswersMode genAnswers;
private:

	Settings mSettings;
	// Читает и парсит названия входных и выходных файлов
	void readTests();
};
