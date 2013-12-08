#include "consoleUtils.h"
#include <QtCore>

#ifdef Q_OS_WIN
#include "Windows.h"

std::ostream &cu::operator << (std::ostream &os, const cu::Color &color)
{
	os.flush();
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color.mTextColor);
	return os;
}

std::ostream &cu::operator << (std::ostream &os, const cu::Position &p)
{
	os.flush();
	COORD c = {p.mx, p.my};

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
