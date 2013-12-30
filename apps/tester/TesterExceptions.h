#pragma once
#include <stdexcept>
#include <exception>

#include <QString>

class TesterException : public std::exception
{
public:
	TesterException(const QString &msg) : mMsg(msg) {}
	~TesterException() noexcept {}
	const char * const what()
	{
		return mMsg.toLocal8Bit().data();
	}
private:
	QString mMsg;
};
