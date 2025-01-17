﻿#include "console_utils.h"
#include <iostream>

#ifdef OS_WIN32

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

#include <unistd.h>

// Для преобразования номера цвета в enum-e в номер цвета из ansi escape codes необходимо
// поменять местами 1 и 3 бит
int getIndex(int a)
{
	a ^= (a & 7) >> 2;
	a ^= (a << 2) & 7;
	a ^= (a & 7) >> 2;
	return a;
}

namespace cu
{
namespace details
{
cu::TextColor currentColor = cu::standard;

// Не придумал, как лучше передать в isatty дескриптор потока
bool isEscapeCodesAcceptable(std::ostream &os)
{
	if(&os == &std::cout) return isatty(1);
	if(&os == &std::cerr) return isatty(2);
	return false;
}

}
}

std::ostream &cu::details::operator << (std::ostream &os, const cu::details::Color &color)
{
	if(cu::details::isEscapeCodesAcceptable(os))
	{
		if(color.mTextColor == standard) os << "\033[0m";
		else
		{
			int code = getIndex(static_cast<int>(color.mTextColor));
			os << "\033[" << (code >> 3) << ";3" << (code & 7) << "m";
		}
	}
	cu::details::currentColor = color.mTextColor;
	return os;
}

std::ostream &cu::details::operator << (std::ostream &os, const cu::details::Position &p)
{
	if(cu::details::isEscapeCodesAcceptable(os))
	{
		if(p.my == -1) std::cout << "\033[" << p.mx + 1 << "G";
		else std::cout << "\033[" << p.mx + 1 << ";" << p.my + 1 << "H";
	}
	return os;
}

cu::ColorSaver::ColorSaver()
	: mColor(cu::details::currentColor)
{
}
#endif

cu::ColorSaver::~ColorSaver()
{
	std::cout << mColor;
}

cu::details::Color cu::textColor(cu::TextColor textColor)
{
	return details::Color(textColor);
}

cu::details::Position cu::cursorPosition(int x, int y)
{
	return details::Position(x, y);
}

cu::details::Position cu::cursorPosition(int x)
{
	return details::Position(x, -1);
}
