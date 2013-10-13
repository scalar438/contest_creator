#pragma once
#include <exception>
#include <QString>

namespace checklib
{
	class ExceptionBase : public std::exception
	{
	public:
		ExceptionBase(const QString &)
		{
		}
	};

	class ExceptionCannotOpenFile : public ExceptionBase
	{
	public:
		ExceptionCannotOpenFile(const QString &fileName)
			: ExceptionBase(fileName)
		{

		}
	};
}
