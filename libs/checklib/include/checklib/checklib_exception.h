#pragma once

#include <exception>
#include <string>

namespace checklib {

class Exception : public std::exception
{
public:
	Exception(const std::string &str)
		: mStr(str)
	{
	}
	virtual ~Exception() {}
	virtual const char* what() const noexcept
	{
		return mStr.c_str();
	}
private:
	std::string mStr;
};

class CannotStartProcess : public Exception
{
public:
	CannotStartProcess(const std::string &programName)
		: Exception("Cannot start program \"" + programName + "\""),
		  mProgram(programName)
	{
	}

	CannotStartProcess(const std::string &programName, const std::string &msg)
		: Exception("Cannot start program \"" + programName + "\": " + msg),
		  mProgram(programName)
	{
	}

	~CannotStartProcess() {}

	std::string programName() const
	{
		return mProgram;
	}

private:
	std::string mProgram;
};

class CannotOpenFile : public Exception
{
public:
	CannotOpenFile(const std::string &fileName)
		: Exception("File not found: \"" + fileName + "\""),
		  mFileName(fileName)
	{
	}
	~CannotOpenFile() {}
	std::string fileName() const
	{
		return mFileName;
	}
private:
	std::string mFileName;
};

}
