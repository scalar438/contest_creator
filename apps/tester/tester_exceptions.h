#pragma once
#include "checklib/noexcept.h"
#include <string>
#include <stdexcept>
#include <exception>

class TesterException : public std::exception
{
public:
	TesterException(const std::string &msg) : mMsg(msg) {}
	~TesterException() NOEXCEPT {}
	const char * what() const NOEXCEPT
	{
		return mMsg.c_str();
	}
private:
	std::string mMsg;
};
