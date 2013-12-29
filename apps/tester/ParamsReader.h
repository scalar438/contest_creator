#pragma once
#include "checklib/rp_types.h"

#include <QString>
#include <QSettings>
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
	// 0 - do not generate, 1 - generate, 2 - generate missing
	int genAnswers;
private:

	QSettings mSettings;
	// Читает и парсит названия входных и выходных файлов
	void readTests();
};
