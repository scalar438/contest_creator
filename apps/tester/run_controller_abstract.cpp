#include "run_controller_abstract.h"
#include "run_controller_simple.h"
#include "run_controller_interactive.h"

RunControllerAbstract::RunControllerPtr RunControllerAbstract::create(boost::asio::io_service &io, ParamsReader &reader)
{
	if(reader.isInteractive) return std::make_shared<RunControllerInteractive>(io, reader);
	return std::make_shared<RunControllerSimple>(io, reader);
}
