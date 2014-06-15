#pragma once
#include <string>
#include <map>

class Settings
{
public:
	Settings(const std::string &fileName);

	std::string readString(const std::string &key, const std::string &def = "") const;

	int readInt(const std::string &key, bool &success) const;
	int readInt(const std::string &key, int def = 0) const;

	double readDouble(const std::string &key, bool &success) const;
	double readDouble(const std::string &key, double def = 0.0) const;

	bool contains(const std::string &key) const;

private:

	std::map<std::string, std::string> mValues;
};
