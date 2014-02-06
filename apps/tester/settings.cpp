#include "settings.h"
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

		mValue[key] = value;
	}
}

