#include "Boy/Environment.h"
#include "Demo2.h"

int main(int argc, char* argv[])
{
	// initialize the platform:
	Boy::Environment::instance()->init(
		Demo2::instance(),
		1024, // width
		768, // height
		false, // fullscreen
		"demo1", // window title
		"demo1.dat", // persistence layer file name
		NULL); // no encryption

	// start the main loop
	Boy::Environment::instance()->startMainLoop();

	// destroy the game:
	Demo2::destroy();

	// destroy the environment:
	Boy::Environment::instance()->destroy();

	return 0;
}

