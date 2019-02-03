#include <thread>
#include <chrono>

int main()
{
	std::this_thread::sleep_for(std::chrono::seconds(100500));
}
