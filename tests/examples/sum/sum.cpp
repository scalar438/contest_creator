#include <iostream>

int main()
{
	int a, b;
	while(1)
	{
		if(!(std::cin >> a >> b)) break;
		if(a == 0 && b == 0) break;
		std::cout << a + b << std::endl;
	}
	return 0;
}
