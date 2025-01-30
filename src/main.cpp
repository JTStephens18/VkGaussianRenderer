// main.cpp : Defines the entry point for the application.
//

#include "main.h"
#include "first_app.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main()
{
	vr::FirstApp app{};

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << "\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
