#include "run_controller_abstract.h"
#include "run_controller_simple.h"

RunControllerAbstract::RunControllerPtr RunControllerAbstract::create(boost::asio::io_service &io, ParamsReader &reader)
{
	return std::make_shared<RunControllerSimple>(io, reader);
}
