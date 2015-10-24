#include "ComplexApplication.h"
#include <GLFW/glfw3.h>

int main() {
	
	BaseApplication* app = new ComplexApplication();
	if (app->startup())
		app->run();
	app->shutdown();

	return 0;
}