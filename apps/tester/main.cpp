#include "RunController.h"
#include "ParamsReader.h"
#include "ConsoleUtils.h"
#include "Runner.h"
#include "TesterExceptions.h"
#include "checklib/checklib_exception.h"

#include <iostream>
#include <stdexcept>

#include <QCoreApplication>
#include <QThread>
#include <QDebug>
#include <QFile>
#include <QDir>

int main(int argc, char *argv[])
{
	cu::ColorSaver saver;

	QCoreApplication app(argc, argv);

	QString settingsFileName = "test.ini";

	try
	{
		for(int i = 1; i < app.arguments().size(); ++i)
		{
			if(app.arguments()[i] == "-ini")
			{
				++i;
				if(app.arguments().size() == i) throw TesterException("Wrong argument format. Usage: -ini <settings-file>");
				settingsFileName = app.arguments()[i];
			}
			else if(app.arguments()[i] == "-v" || app.arguments()[i] == "-version")
			{
				std::cout << "Tester version: 1.0.1" << std::endl;
				return 0;
			}
		}
		if(!QFile(settingsFileName).exists()) throw TesterException("Settings file is not exists");

		ParamsReader reader(settingsFileName);
		Runner runner(reader.programName, reader.limits);
		RunController tester(&reader, &runner);

		QThread thrd;
		runner.moveToThread(&thrd);
		thrd.start();

		QObject::connect(&tester, &RunController::testCompleted, &thrd, &QThread::quit);
		QObject::connect(&thrd, &QThread::finished, &QCoreApplication::quit);
		QMetaObject::invokeMethod(&tester, "startTesting", Qt::QueuedConnection);
		return app.exec();
	}
	catch(checklib::Exception &e)
	{
		std::cout << cu::textColor(cu::red) << "Testing error: " << cu::textColor(cu::lightGray) << e.what() << std::endl;
		return -1;
	}
	catch(std::logic_error &e)
	{
		std::cout << cu::textColor(cu::red) << "Internal error: " << cu::textColor(cu::lightGray) << e.what() << std::endl;
		return -1;
	}
	catch(TesterException &e)
	{
		std::cout << cu::textColor(cu::red) << "Error: " << cu::textColor(cu::lightGray) << e.what() << std::endl;
		return -1;
	}
}
