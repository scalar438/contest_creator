#pragma once
#include "rp_types.h"
#include <string>
#include <filesystem>
#include <vector>

namespace checklib
{

struct ProcessExecuteParameters
{
	// TODO: remove program, replace program_path to program
	std::string program;
	std::filesystem::path program_path;
	std::vector<std::string> cmdline;
	std::string current_directory;
	std::string standard_input;
	std::string standard_output;
	std::string standard_error;

	Limits limits;
};

} // namespace checklib::details
