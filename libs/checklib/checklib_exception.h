#pragma once
#include <exception>
#include <QString>

namespace checklib {

class Exception : public std::exception
{
public:
	Exception(const QString &str)
		: std::exception(str.toLocal8Bit().data()) {}
};

class CannotStartProcess : public Exception
{
public:
	CannotStartProcess(const QString &programName)
		: Exception("Cannot start program \"" + programName + "\""),
		  mProgram(programName)
	{
	}
	QString programName() const
	{
		return mProgram;
	}
private:
	QString mProgram;
};

class FileNotFound : public Exception
{
public:
	FileNotFound(const QString &fileName)
		: Exception("File not found: \"" + fileName + "\""),
		  mFileName(fileName)
	{
	}
	QString fileName() const
	{
		return mFileName;
	}
private:
	QString mFileName;
};

}
