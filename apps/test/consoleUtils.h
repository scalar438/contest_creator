#pragma once

namespace ConsoleUtils
{

enum TextColor {};
void textColor(TextColor textColor);
void cursorPosition(int x, int y);
bool getCursorPosition(int &x, int &y);

}
