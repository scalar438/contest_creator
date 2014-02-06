#pragma once
#include <ostream>
#include <string>
#include <map>

class Settings
{
public:
	Settings(const std::string &fileName);

	template<class T> void param(const std::string &str)
	{

	}

private:

	std::map<std::string, std::string> mValues;

};
