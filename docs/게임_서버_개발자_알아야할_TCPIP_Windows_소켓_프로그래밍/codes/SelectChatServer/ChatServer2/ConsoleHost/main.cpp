#include <iostream>
#include <thread>
#include "../LogicLib/Main.h"

int main()
{
	NLogicLib::Main main;
	main.Init();

	std::thread logicThread([&]() { 		
		main.Run(); }
	);
	
	std::cout << "press any key to exit...";
	getchar();

	main.Stop();
	logicThread.join();

	return 0;
}