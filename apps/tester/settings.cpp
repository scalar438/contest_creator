#include "settings.h"
#include <fstream>
#include <sstream>
#include <boost/algorithm/string.hpp>
using namespace std;

Settings::Settings(const std::string &fileName)
{
	ifstream is(fileName);
	string str;
	while(getline(is, str))
	{
		int c = 0;
		while(c < (int)str.length() && str[c] != '=') ++c;
		if(c >= (int)str.length() - 1 || c == 0) continue;

		string key = str.substr(0, c), value = str.substr(c + 1);
		boost::algorithm::trim(key);
		boost::algorithm::trim(value);
		if(key.empty() || value.empty()) continue;

		mValues[key] = value;
	}
}

std::string Settings::readString(const std::string &key, const std::string &def) const
{
	auto it = mValues.find(key);
	if(it != mValues.end()) return it->second;
	return def;
}

int Settings::readInt(const std::string &key, int def) const
{
	auto it = mValues.find(key);
	if(it != mValues.end())
	{
		std::istringstream is(it->second);
		int val;
		if(is >> val) return val;
	}
	return def;
}

double Settings::readDouble(const std::string &key, double def) const
{
	auto it = mValues.find(key);
	if(it != mValues.end())
	{
		std::istringstream is(it->second);
		double val;
		if(is >> val) return val;
	}
	return def;
}

bool Settings::contains(const string &key) const
{
	return mValues.find(key) != mValues.end();
}
