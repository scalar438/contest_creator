#pragma once
#include <string>
#include <vector>
#include "../rp_types.h"

namespace checklib::details
{

struct ProcessExecuteParameters
{
	std::string program;
	std::vector<std::string> cmdline;
	std::string current_directory;
	std::string standard_input;
	std::string standard_output;
	std::string standard_error;

	Limits limits;
};


} // namespace checklib::details
