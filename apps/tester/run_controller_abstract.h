#pragma once
#include "params_reader.h"

#include <memory>
#include <boost/asio.hpp>

class RunControllerAbstract
{
public:
	typedef std::shared_ptr<RunControllerAbstract> RunControllerPtr;
	static RunControllerPtr create(boost::asio::io_service &io, ParamsReader &reader);

	virtual ~RunControllerAbstract(){}

	virtual void startTesting() = 0;
};
