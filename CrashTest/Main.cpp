#include <iostream>
#include <Windows.h>

// This program is just a demonstration for testing.

int main()
{
	std::cout << "Press any key to crash the program\n";
	std::cin.get();

	volatile int* p = 0;
	*p = 0;

	while (1) 
	{
		std::cout << "I'm still running!\n";
		Sleep(1000);
	}
	return 0;
}