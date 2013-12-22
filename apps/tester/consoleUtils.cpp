#include "consoleUtils.h"
#include <QtCore>
#include <iostream>

#ifdef Q_OS_WIN
#include "Windows.h"

int defaultColor;

std::ostream &cu::operator << (std::ostream &os, const cu::Color &color)
{
	os.flush();
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color.mTextColor);
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
		//std::cout << "gcsbi: " << res << " ";

		c.Y = info.dwCursorPosition.Y;
		//std::cout << "c.X = " << c.X;
	}
	else c.Y = p.my;

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
	return os;
}

void cu::initStandard()
{
	//GetConsoleTextAttr
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



