#pragma once
#include <ostream>

// cu - "console utils"
namespace cu
{

enum TextColor {black, navy, green, teal, brown, purple, olive, lightGray,
				darkGray, blue, lime, cyan, red, magenta, yellow, white, standard};

namespace details
{

class Color
{
public:
	Color(TextColor textColor = standard) : mTextColor(textColor){}
	friend std::ostream& operator << (std::ostream &os, const details::Color &color);
	TextColor mTextColor;
};

class Position
{
public:
	Position(int x, int y) : mx(x), my(y){}
	friend std::ostream& operator << (std::ostream &os, const Position &p);
	int mx, my;
};

}

// Манипулятор потока цвета текста в консоли
details::Color textColor(TextColor textColor);

// Манипулятор потока для задания положения курсора
details::Position cursorPosition(int x, int y);

details::Position cursorPosition(int x);

// Сохраняет, а затем автоматически восстанавливает текущий цвет шрифта консоли
class ColorSaver
{
public:
	ColorSaver();
	~ColorSaver();
private:
	details::Color mColor;
};

}
