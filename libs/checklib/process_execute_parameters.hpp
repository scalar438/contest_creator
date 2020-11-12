#pragma once
#include "rp_types.h"
#include <string>
#include <vector>

namespace checklib
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
