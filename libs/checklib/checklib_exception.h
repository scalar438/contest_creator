#pragma once
#include <exception>
#include <QString>

namespace checklib {

class ChecklibException : public std::exception
{
public:
	ChecklibException(const QString &str)
		: std::exception(str.toLocal8Bit().data()) {}
};

class CannotStartProcess : public ChecklibException
{
public:
	CannotStartProcess(const QString &programName)
		: ChecklibException("Cannot start program \"" + programName + "\""),
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

class FileNotFound : public ChecklibException
{
public:
	FileNotFound(const QString &fileName)
		: ChecklibException("Cannot start program \"" + fileName + "\""),
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
