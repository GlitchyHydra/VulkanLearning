
#include "Application.h"

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "Log.h"

int main()
{
	Vipera::Log::Init();

	Vipera::Application app{};

	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}