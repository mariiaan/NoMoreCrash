#include <iostream>
#include <Windows.h>

// This program is just a demonstration for testing.

int main()
{
	std::cout << "Press any key to crash the program";
	std::cin.get();

	volatile int* p = 0;
	*p = 0;

	std::cout << "Again!";
	std::cin.get();

	volatile int a = 1;
	volatile int b = 0;
	volatile int c = a / b;

	for (int i = 0; i < 5; ++i)
	{
		std::cout << "I'm still running!\n";
		if (i < 5) Sleep(1000);
	}

	std::cout << "Trying abort...\n";
	std::abort();
	std::cout << "Still running!\n";
	Sleep(1000);

	int cnt = 0;
	while (true)
	{
		std::cout << "Memory corruption " << cnt << "...\n";
		p[cnt] = ++cnt;
	}
	std::cout << "We broke out!\n";

	for (int i = 0; i < 5; ++i)
	{
		std::cout << "I'm still running!\n";
		if (i < 5) Sleep(1000);
	}

	return 0;
}