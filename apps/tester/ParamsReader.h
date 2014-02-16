#pragma once
#include "checklib/rp_types.h"
#include "settings.h"

#include <QString>
#include <vector>

struct OneTest
{
	QString inputFile;
	QString answerFile;
};

class ParamsReader
{
public:
	ParamsReader(const QString &settingsFileName);

	QString programName;
	QString checker;
	QString inputFile;
	QString outputFile;
	checklib::Limits limits;
	std::vector<OneTest> tests;
	bool interrupt;

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
