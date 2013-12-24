#include "consoleUtils.h"
#include <QtCore>
#include <iostream>

#ifdef Q_OS_WIN
#include "Windows.h"

int defaultColor;

std::ostream &cu::details::operator << (std::ostream &os, const cu::details::Color &color)
{
	os.flush();
	if(color.mTextColor == standard)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), lightGray);
	}
	else
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color.mTextColor);
	}
	return os;
}

std::ostream &cu::operator << (std::ostream &os, const cu::Position &p)
{
	os.flush();
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD c;
	c.X = p.mx;
	if(p.my == -1)
	{
		CONSOLE_SCREEN_BUFFER_INFO info;
		auto res = GetConsoleScreenBufferInfo(handle, &info);

		c.Y = info.dwCursorPosition.Y;
	}
	else c.Y = p.my;

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
	return os;
}

#else
std::ostream &ConsoleUtils::operator << (std::ostream &os, const ConsoleUtils::Color &color)
{
	return os;
}

std::ostream &ConsoleUtils::operator << (std::ostream &os, const ConsoleUtils::Position &p)
{
	return os;
}
#endif


cu::ColorSaver::ColorSaver()
{

}

cu::ColorSaver::~ColorSaver()
{
	std::cout << mColor;
}
