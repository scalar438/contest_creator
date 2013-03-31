#include <iostream>

int f(int a)
{
	if(a < 0) return 1;
	return f(a) + f(a - 1);
}

int main()
{
	std::cout << f(1000);
	return 0;
}
