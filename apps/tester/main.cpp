#include "params_reader.h"
#include "console_utils.h"
#include "checklib/checklib_exception.h"
#include "tester_exceptions.h"
#include "run_controller.h"

#include <iostream>
#include <stdexcept>

#include <boost/asio.hpp>

int main(int argc, char *argv[])
{
	cu::ColorSaver saver;
	// Delete warning
	((void*)(&saver));

	try
	{
		std::vector<std::string> arguments(argv + 1, argv + argc);

		std::string settingsFileName = "test.ini";

		for(int i = 1; i < argc; ++i)
		{
			if(arguments[i] == "-ini")
			{
				++i;
				if(i > argc) throw TesterException("Wrong argument format. Usage: -ini <settings-file>");
				settingsFileName = arguments[i];
			}
			else if(arguments[i] == "-v" || arguments[i] == "-version")
			{
				std::cout << "Tester version: 1.0.1" << std::endl;
				return 0;
			}
		}

		ParamsReader reader(settingsFileName);
		boost::asio::io_service io;
		RunController runController(io, reader);
		runController.startTesting();
		io.run();
		return 0;
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
