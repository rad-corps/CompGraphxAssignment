#include "AppTerrain.h"
#include <GLFW/glfw3.h>
#include "simplexnoise.h"
#include <iostream>

int main() {

	BaseApplication* app = new AppTerrain();
	if (app->startup())
		app->run();
	app->shutdown();

	return 0;
}