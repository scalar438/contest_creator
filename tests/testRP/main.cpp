#include <checklib/checklib.h>
#include <iostream>
#include <map>
#include <vector>
using namespace checklib;

bool is_running_checking()
{
	RestrictedProcess runner;

	Limits limits;
	limits.useTimeLimit = true;
	limits.timeLimit    = 1000;
	runner.setLimits(limits);

	runner.setProgram("../examples/TL/pTL");

	runner.start();
	if (!(!runner.wait(500) && runner.isRunning() && runner.processStatus() == checklib::psRunning))
		return false;

	runner.wait();
	return !runner.isRunning();
}

bool test_terminate()
{
	RestrictedProcess runner;

	runner.setProgram("./examples/pTL");
	runner.start();

	if (runner.wait(500)) return false;

	runner.terminate();
	return runner.wait(100) && runner.processStatus() == checklib::psTerminated;
}

bool test_exit_code()
{
	RestrictedProcess runner;
	runner.setProgram("./examples/pArgsExitCode");
	runner.start();

	runner.wait();
	if (runner.exitCode() != 42) return false;

	runner.reset();
	runner.setParams({"123"});
	runner.start();
	runner.wait();
	return runner.exitCode() == 123 && !runner.isRunning();
}

bool testTL()
{
	RestrictedProcess runner;

	Limits limits;
	limits.useTimeLimit = true;
	limits.timeLimit    = 2000;
	runner.setLimits(limits);

	runner.setProgram("./examples/pTL");
	runner.start();

	runner.wait();

	std::cout << "pTL time: " << runner.CPUTime();
	return runner.processStatus() == checklib::psTimeLimitExceeded && !runner.isRunning();
}

bool test_args()
{
	return true;
	/*  RestrictedProcess runner;
	  runner.setProgram("./examples/pArgsOut");
	  QStringList params;
	  params << "param1"
	         << "param with space"
	         << "param3";
	  runner.setParams(toStringList(params));

	  runner.start();
	  runner.wait();

	  QVERIFY(runner.processStatus() == checklib::psExited);
	  QVERIFY(!runner.isRunning());

	  std::ifstream is(boost::filesystem::path(args_output).native());

	  params.prepend("./examples/pArgsOut");
	  int count;
	  is >> count;
	  std::string str;
	  std::getline(is, str);
	  QVERIFY(is.good());
	  QVERIFY(count == params.size());
	  std::getline(is, str);
	  QVERIFY(is.good());
	  // Первый аргумент - путь до исполняемого файла. Сравниваем его именно как путь (из-за того,
	  // что в windows в нем могут быть как прямые, так и обратные слеши)
	  QVERIFY(boost::filesystem::path(params[0].toStdString()) == boost::filesystem::path(str));
	  for (int i = 1; i < count; ++i)
	  {
	      std::getline(is, str);
	      QVERIFY(is.good());
	      QVERIFY(str == params[i].toStdString());
	  }*/
}

bool test_ml()
{
	RestrictedProcess runner;

	Limits limits;
	limits.useMemoryLimit = true;
	limits.memoryLimit    = 64 * 1000 * 1000;
	runner.setLimits(limits);

	runner.setProgram("./examples/pML");
	runner.start();
	runner.wait();

	std::cout << "pML memory: " << runner.peakMemoryUsage();
	return runner.processStatus() == checklib::psMemoryLimitExceeded && !runner.isRunning();
}

bool test_re()
{
	RestrictedProcess runner;
	runner.setProgram("./examples/pRE");
	runner.setParams({"1"});

	runner.start();
	runner.wait();

	if (runner.processStatus() != checklib::psRuntimeError) return false;

	runner.reset();
	runner.setParams({"0"});

	runner.start();
	runner.wait();

	return runner.processStatus() == checklib::psRuntimeError && !runner.isRunning();
}

bool test_standard_streams_redirection()
{
	return true;
	/*    RestrictedProcess runner;

	Limits limits;
	limits.useMemoryLimit = true;
	limits.memoryLimit    = 65536 * 1024;
	limits.useTimeLimit   = true;
	limits.timeLimit      = 2000;
	runner.setLimits(limits);

	runner.setProgram("./examples/pSum");

	const int a = 24;
	const int b = 18;

	std::ofstream os(boost::filesystem::path(sum_input).native());
	os << a << " " << b << std::endl << "0 0";
	os.close();

	// TODO: надо сделать без кучи конвертаций - напрямую через path
	runner.setStandardInput(
	    QFileInfo(QString::fromStdString(sum_input)).absoluteFilePath().toStdString());
	runner.setStandardOutput(
	    QFileInfo(QString::fromStdString(sum_output)).absoluteFilePath().toStdString());

	runner.start();
	runner.wait();
	qDebug() << "sum time and memory" << runner.CPUTime() << runner.peakMemoryUsage();
	QVERIFY(runner.processStatus() == checklib::psExited);

	{
	    std::ifstream is(boost::filesystem::path(sum_output).native());

	    int val = a + b + 1;

	    QVERIFY(is.good());
	    QVERIFY(bool(is >> val));
	    QVERIFY(val == a + b);
	}

	runner.reset();
	runner.setProgram("./examples/pStderr_out");

#ifdef Q_OS_WIN
	runner.setStandardError(
	    QString::fromStdWString(boost::filesystem::path(stderr_out_error).native()).toStdString());
#else
	runner.setStandardError(
	    QString::fromLocal8Bit(boost::filesystem::path(stderr_out_error).native().c_str())
	        .toStdString());
#endif
	runner.start();
	runner.wait();

	QVERIFY(runner.processStatus() == checklib::psExited);

	{
	    std::ifstream is(boost::filesystem::path(stderr_out_error).native());
	    std::string str;
	    QVERIFY(is.good());
	    QVERIFY(bool(std::getline(is, str)));
	    QVERIFY(str == "Test printing to stderr");
	    QVERIFY(bool(std::getline(is, str)));
	    QVERIFY(str == "Line2");
	}*/
}


bool test_il()
{
	RestrictedProcess runner;

	runner.setProgram("./examples/pIL");
	runner.start();
	runner.wait();

	return runner.processStatus() == checklib::psIdlenessLimitExceeded && !runner.isRunning();
}

bool test_interactive()
{
	RestrictedProcess runner;

	Limits limits;
	limits.useMemoryLimit = true;
	limits.memoryLimit    = 65536 * 1024;
	limits.useTimeLimit   = true;
	limits.timeLimit      = 2000;
	runner.setLimits(limits);

	runner.setStandardInput(checklib::ss::Interactive);
	runner.setStandardOutput(checklib::ss::Interactive);

	runner.setProgram("./examples/pSum");

	runner.start();
	runner.sendDataToStandardInput("4 5\n");
	std::string ans;
	if (!runner.getDataFromStandardOutput(ans))
		throw std::exception("getDataFromStandardOutput returned false");
	if (ans != "9") return false;

	runner.sendDataToStandardInput("2 3", true);
	if (!runner.getDataFromStandardOutput(ans))
		throw std::exception("getDataFromStandardOutput returned false");

	if (ans != "5") return false;

	runner.sendDataToStandardInput("0 0", true);
	runner.wait();

	return runner.processStatus() == checklib::psExited;
}

bool test_exception()
{
	RestrictedProcess runner;
	runner.setProgram("./examples/FileDoesNotExists");
	bool exceptionFlag = false;
	try
	{
		runner.start();
		runner.wait();
	}
	catch (Exception &)
	{
		exceptionFlag = true;
	}

	return exceptionFlag;
}

int main(int argc, char *argv[])
{
	try
	{
		// if (argc != 2) return -1;
		std::map<std::string, bool (*)()> funcs = {
		    {"is_running_checking", is_running_checking},
		    {"test_terminate", test_terminate},
		    {"test_exit_code", test_exit_code},
		    {"testTL", testTL},
		    {"test_args", test_args},
		    {"test_ml", test_ml},
		    {"test_re", test_re},
		    {"test_standard_streams_redirection", test_standard_streams_redirection},
		    {"test_il", test_il},
		    {"test_interactive", test_interactive},
		    {"test_exception", test_exception}};
		auto it = funcs.find("is_running_checking");
		if (it == funcs.end()) return -2;
		if (!it->second()) return -3;
		return 0;
	}
	catch (...)
	{
		return -4;
	}
}
