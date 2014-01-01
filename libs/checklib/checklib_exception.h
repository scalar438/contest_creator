#pragma once
#include <exception>
#include <QString>

namespace checklib {

class Exception : public std::exception
{
public:
	Exception(const QString &str)
		: mStr(str)
	{
	}
	virtual ~Exception() noexcept {}
	virtual const char* what() const noexcept
	{
		return mStr.toLocal8Bit().data();
	}
private:
	QString mStr;
};

class CannotStartProcess : public Exception
{
public:
	CannotStartProcess(const QString &programName)
		: Exception("Cannot start program \"" + programName + "\""),
		  mProgram(programName)
	{
	}

	CannotStartProcess(const QString &programName, const QString &msg)
		: Exception("Cannot start program \"" + programName + "\": " + msg),
		  mProgram(programName)
	{
	}

	~CannotStartProcess() noexcept {}

	QString programName() const
	{
		return mProgram;
	}

private:
	QString mProgram;
};

class CannotOpenFile : public Exception
{
public:
	CannotOpenFile(const QString &fileName)
		: Exception("File not found: \"" + fileName + "\""),
		  mFileName(fileName)
	{
	}
	~CannotOpenFile() noexcept {}
	QString fileName() const
	{
		return mFileName;
	}
private:
	QString mFileName;
};

}
