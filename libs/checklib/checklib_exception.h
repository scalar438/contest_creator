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
		ExceptionCannotOpenFile(const QString &fileName, const QString &info)
			: ExceptionBase("File " + fileName + " cannot be open: " + info)
		{

		}
	};

	class ExseptionCannotStartProcess : public ExceptionBase
	{
	public:
		ExseptionCannotStartProcess(const QString &process, const QString &)
			: ExceptionBase("123")
		{

		}
	};
}
