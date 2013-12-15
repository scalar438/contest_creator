#pragma once
#include <ostream>

// cu - акроним от "console utils"
// содержит манипуляторы потоков для задания цвета консоли
// Использование: std::cout << textColor(tcRed) << "Error!";
namespace cu
{

class Color;
enum TextColor {black, navy, green, teal, brown, purple, olive, lightGray,
				darkGray, blue, lime, cyan, red, magenta, yellow, white};
Color textColor(TextColor textColor);
class Color
{
	Color(TextColor textColor) : mTextColor(textColor){}
	friend std::ostream& operator << (std::ostream &os, const Color &color);
	friend Color textColor(TextColor);
	TextColor mTextColor;
};

static Color textColor(TextColor textColor)
{
	return Color(textColor);
}

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
