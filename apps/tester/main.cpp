#include "RunController.h"
#include "ParamsReader.h"
#include "consoleUtils.h"
#include "Runner.h"
#include "checklib/checklib_exception.h"

#include <iostream>

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
					throw std::exception("Wrong argument format. Usage: -ini <settings-file>");
				}
			}
			else
			{
				throw std::exception(("Unknown argument: " + app.arguments()[1].toStdString()).c_str());
			}
		}
		if(!QFile(settingsFileName).exists()) throw std::exception("Settings file is not exists");

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
		std::cout << cu::textColor(cu::red) << "Testing error: " << cu::textColor(cu::lightGray) << e.what();
		return -1;
	}
	catch(std::logic_error &e)
	{
		std::cout << cu::textColor(cu::red) << "Internal error: " << cu::textColor(cu::lightGray) << e.what();
		return -1;
	}
	catch(std::exception &e)
	{
		std::cout << cu::textColor(cu::red) << "Error: " << cu::textColor(cu::lightGray) << e.what();
		return -1;
	}
}
