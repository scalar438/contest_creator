#include <cstdlib>
#include <iostream>

int main()
{
	int a = rand() % 100;
	a = a + a / 10 + a % 10;
	a /= (a % 9);
	std::cout << a;
	return 0;
}
