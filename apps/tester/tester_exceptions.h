#pragma once
#include <string>
#include <stdexcept>
#include <exception>

class TesterException : public std::exception
{
public:
	TesterException(const std::string &msg) : mMsg(msg) {}
	~TesterException() {}
	const char * what() const noexcept
	{
		return mMsg.c_str();
	}
private:
	std::string mMsg;
};
