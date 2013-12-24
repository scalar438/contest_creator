#pragma once
#include <ostream>

// cu - акроним от "console utils"
// содержит манипуляторы потоков для задания цвета консоли
// Использование: std::cout << textColor(tcRed) << "Error!";
namespace cu
{

namespace details {
class Color;
}
enum TextColor {black, navy, green, teal, brown, purple, olive, lightGray,
				darkGray, blue, lime, cyan, red, magenta, yellow, white, standard};
details::Color textColor(TextColor textColor);

namespace details {
class Color
{
public:
	Color(TextColor textColor = standard) : mTextColor(textColor){}
	friend std::ostream& operator << (std::ostream &os, const details::Color &color);
	details::Color textColor(TextColor);
	TextColor mTextColor;
};
}

static details::Color textColor(TextColor textColor)
{
	return details::Color(textColor);
}

class ColorSaver
{
public:
	ColorSaver();
	~ColorSaver();
private:
	details::Color mColor;
};

class Position;
Position cursorPosition(int x, int y);
// Задает позицию в текущей строке.
Position cursorPosition(int y);
class Position
{
	Position(int x, int y) : mx(x), my(y){}
	friend std::ostream& operator << (std::ostream &os, const Position &p);
	friend Position cursorPosition(int, int);
	friend Position cursorPosition(int);
	int mx, my;
};

static Position cursorPosition(int x, int y)
{
	return Position(x, y);
}

static Position cursorPosition(int x)
{
	return Position(x, -1);
}

}
