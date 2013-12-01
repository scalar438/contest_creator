#include "test.h"

#include <iostream>

#include <QCoreApplication>
#include <QThread>
#include <QDebug>
#include <QFile>

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	QString settingsFileName = "settings.ini";

	if(app.arguments().size() > 1)
	{
		if(app.arguments()[1] == "-ini")
		{
			if(app.arguments().size() > 2)
			{
				settingsFileName = app.arguments()[2];
			}
			else
			{
				std::cout << "Wrong format arguments. Usage: -ini <settings-file>" << std::endl;
				return 0;
			}
		}
		else
		{
			std::cout << "Unknown argument: " << app.arguments()[1].toStdString() << std::endl;
			return 0;
		}
	}
	if(!QFile(settingsFileName).exists())
	{
		std::cout << "Settings file is not exists" << std::endl;
		return 0;
	}

	ParamsReader reader(settingsFileName);

	Tester tester;
	Runner runner(reader.programName(), reader.limits());
	QThread thrd;

	QObject::connect(&runner, &Runner::finished, &tester, &Tester::onTestFinished);
	QObject::connect(&tester, &Tester::nextTest, &runner, &Runner::startTest);
	QObject::connect(&tester, &Tester::testCompleted, &thrd, &QThread::quit);

	return app.exec();
}
