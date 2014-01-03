#pragma once
#include "checklib/noexcept.h"
#include <stdexcept>
#include <exception>

#include <QString>

class TesterException : public std::exception
{
public:
	TesterException(const QString &msg) : mMsg(msg) {}
	~TesterException() NOEXCEPT {}
	const char * what() const NOEXCEPT
	{
		return mMsg.toLocal8Bit().data();
	}
private:
	QString mMsg;
};
