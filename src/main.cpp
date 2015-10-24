#include "ComplexApplication.h"
#include <GLFW/glfw3.h>
#include "simplexnoise.h"
#include <iostream>

int main() {

	//const int SAMPLE_SIZE = 5;

	//float results[SAMPLE_SIZE][SAMPLE_SIZE];

	//for (int i = 0; i < SAMPLE_SIZE; ++i)
	//{
	//	for (int j = 0; j < SAMPLE_SIZE; ++j)
	//	{
	//		results[i][j] = octave_noise_2d(1.f, 0.5f, 2.0f, i * 0.01, j * 0.01);
	//		std::cout << "results[" << i << "][" << j << "]: " << results[i][j] << std::endl;
	//	}
	//}

	//for (int i = 0; i < SAMPLE_SIZE; ++i)
	//{
	//	for (int j = 0; j < SAMPLE_SIZE; ++j)
	//	{
	//		results[i][j] = octave_noise_2d(1.f, 0.5f, 2.0f, i * 0.01, j * 0.01);
	//		std::cout << "results[" << i << "][" << j << "]: " << results[i][j] << std::endl;
	//	}
	//}

	//	system("pause");

	BaseApplication* app = new ComplexApplication();
	if (app->startup())
		app->run();
	app->shutdown();



	return 0;
}