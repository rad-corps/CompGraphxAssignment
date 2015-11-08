#include "AppTerrain.h"
#include "AppTexture.h"
#include <GLFW/glfw3.h>
#include "simplexnoise.h"
#include <iostream>

int main() {

	BaseApplication* app = new AppTerrain();
	//BaseApplication* app = new AppTexture();
	if (app->startup())
		app->run();
	app->shutdown();

	return 0;
}