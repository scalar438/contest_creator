#pragma once
#include <exception>
#include <QString>

namespace checlib
{
	class ExceptionBase : public std::exception
	{
	public:
		ExceptionBase(const QString &)
		{
		}
	};

	class exception_cannot_open_file : public ExceptionBase
	{
	public:
		exception_cannot_open_file(const QString &fileName)
			: ExceptionBase(fileName)
		{

		}
	};
}
