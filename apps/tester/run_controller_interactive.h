#pragma once
#include "run_controller_abstract.h"

class RunControllerInteractive : public RunControllerAbstract
{
public:
	RunControllerInteractive(boost::asio::io_service &io, ParamsReader &reader);
	~RunControllerInteractive();

	void startTesting();
};
