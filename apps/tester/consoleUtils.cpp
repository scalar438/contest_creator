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
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), defaultColor);
	}
	else
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color.mTextColor);
	}
	return os;
}

std::ostream &cu::details::operator << (std::ostream &os, const cu::details::Position &p)
{
	os.flush();
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD c;
	c.X = p.mx;
	if(p.my == -1)
	{
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(handle, &info);

		c.Y = info.dwCursorPosition.Y;
	}
	else c.Y = p.my;

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
	return os;
}

cu::details::Color getCurrentColor()
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
	return cu::details::Color(cu::TextColor(info.wAttributes & 15));
}

cu::ColorSaver::ColorSaver()
	: mColor(getCurrentColor())
{
}

struct Initializer
{
	Initializer()
	{
		defaultColor = (int)getCurrentColor().mTextColor;
	}
} initializer;

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

cu::ColorSaver::~ColorSaver()
{
	std::cout << mColor;
}
