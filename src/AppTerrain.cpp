#include "AppTerrain.h"
#include "gl_core_4_4.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>//debugging

#include "Camera.h"
#include "Gizmos.h"


using glm::vec3;
using glm::vec4;
using glm::mat4;


void OnMouseButton(GLFWwindow*, int b, int a, int m) {
	TwEventMouseButtonGLFW(b, a);
}
void OnMousePosition(GLFWwindow*, double x, double y) {
	TwEventMousePosGLFW((int)x, (int)y);
}
void OnMouseScroll(GLFWwindow*, double x, double y) {
	TwEventMouseWheelGLFW((int)y);
}
void OnKey(GLFWwindow*, int k, int s, int a, int m) {
	TwEventKeyGLFW(k, a);
}
void OnChar(GLFWwindow*, unsigned int c) {
	TwEventCharGLFW(c, GLFW_PRESS);
}
void OnWindowResize(GLFWwindow*, int w, int h) {
	TwWindowSize(w, h);
	glViewport(0, 0, w, h);
}


AppTerrain::AppTerrain()
	: m_camera(nullptr) {

	
}

AppTerrain::~AppTerrain() {

}

bool AppTerrain::startup() {

	// create a basic window
	createWindow("Complex Game Systems", 1280, 720);

	// start the gizmo system that can draw basic shapes
	Gizmos::create();

	// create a camera
	m_camera = new Camera(glm::pi<float>() * 0.25f, 16 / 9.f, 0.1f, 1000.f);
	m_camera->setLookAtFrom(vec3(10, 10, 10), vec3(0));
	
	//////////////////////////////////////////////////////////////////////////
	// YOUR STARTUP CODE HERE
	//////////////////////////////////////////////////////////////////////////
	m_pickPosition = glm::vec3(0);

	//AntTweakBar stuff
	TwInit(TW_OPENGL_CORE, nullptr);
//	TwInit(TW_OPENGL, nullptr);
	TwWindowSize(1280, 720);


	//Load a texture
	glEnable(GL_TEXTURE_2D);
	int imageWidth = 0, imageHeight = 0, imageFormat = 0;
	unsigned char* data = stbi_load("./data/textures/star.png",	&imageWidth, &imageHeight, &imageFormat, STBI_default);	glGenTextures(1, &grassTexture);
	glBindTexture(GL_TEXTURE_2D, grassTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	stbi_image_free(data);

	//GUI Controls
	//m_clearColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	terrainSize = 100;
	octaves = 1.0f;
//	smoothness = 0.03f;
	scale2 = 0.05f;
	persistance = 0.5f;
	height = 4.0f;
	bottomRed = 26;
	bottomGreen = 57;
	bottomBlue = 12;
	topRed = 91;
	topGreen = 135;
	topBlue = 10;
	guiBar = TwNewBar("Control");	TwAddVarRW(guiBar, "size", TW_TYPE_INT32, &terrainSize, " min=0 max=200 label='Terrain Size'");	TwAddVarRW(guiBar, "octave", TW_TYPE_FLOAT, &octaves, " min=0 max=10 label='Perlin Octaves'");	TwAddVarRW(guiBar, "scale", TW_TYPE_FLOAT, &scale2, " min=0.001 max=1.0 label='Scale'");	TwAddVarRW(guiBar, "persistance", TW_TYPE_FLOAT, &persistance, " min=0.0 max=1.0 label='Persistance'");	TwAddVarRW(guiBar, "height", TW_TYPE_FLOAT, &height, " min=0.0 max=10 label='Height'");	TwAddVarRW(guiBar, "bottomRed", TW_TYPE_INT32, &bottomRed, " min=0 max=255 label='Bottom Red'");	TwAddVarRW(guiBar, "bottomGreen", TW_TYPE_INT32, &bottomGreen, " min=0 max=255 label='Bottom Green'");	TwAddVarRW(guiBar, "bottomBlue", TW_TYPE_INT32, &bottomBlue, " min=0 max=255 label='Bottom Blue'");	TwAddVarRW(guiBar, "topRed", TW_TYPE_INT32, &topRed, " min=0 max=255 label='Top Red'");	TwAddVarRW(guiBar, "topGreen", TW_TYPE_INT32, &topGreen, " min=0 max=255 label='Top Green'");	TwAddVarRW(guiBar, "topBlue", TW_TYPE_INT32, &topBlue, " min=0 max=255 label='Top Blue'");	glfwSetMouseButtonCallback(m_window, OnMouseButton);
	glfwSetCursorPosCallback(m_window, OnMousePosition);
	glfwSetScrollCallback(m_window, OnMouseScroll);
	glfwSetKeyCallback(m_window, OnKey);
	glfwSetCharCallback(m_window, OnChar);
	glfwSetWindowSizeCallback(m_window, OnWindowResize);
	return true;
}

void AppTerrain::shutdown() {

	//////////////////////////////////////////////////////////////////////////
	// YOUR SHUTDOWN CODE HERE
	//////////////////////////////////////////////////////////////////////////	
	//AntTweakBar
	TwDeleteAllBars();
	TwTerminate();

	// delete our camera and cleanup gizmos
	delete m_camera;
	Gizmos::destroy();

	// destroy our window properly
	destroyWindow();
}

bool AppTerrain::update(float deltaTime) {
	
	// close the application if the window closes
	if (glfwWindowShouldClose(m_window) ||
		glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		return false;

	// update the camera's movement
	m_camera->update(deltaTime);

	// clear the gizmos out for this frame
	Gizmos::clear();

	//convert RGB values to a float array then pass to addTerrain
	float RGBFloats[6];
	RGBFloats[0] = (float)bottomRed / 255.f;
	RGBFloats[1] = (float)bottomGreen / 255.f;
	RGBFloats[2] = (float)bottomBlue / 255.f;
	RGBFloats[3] = (float)topRed / 255.f;
	RGBFloats[4] = (float)topGreen / 255.f;
	RGBFloats[5] = (float)topBlue / 255.f;

	Gizmos::addTerrain(terrainSize, octaves, height, RGBFloats, scale2, persistance);

	// return true, else the application closes
	return true;
}

void AppTerrain::draw() {

	// clear the screen for this frame
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// display the 3D gizmos
	Gizmos::draw(m_camera->getProjectionView(), grassTexture);
	// get an orthographic projection matrix and draw 2D gizmos
	int width = 0, height = 0;
	glfwGetWindowSize(m_window, &width, &height);
	mat4 guiMatrix = glm::ortho<float>(0, 0, (float)width, (float)height);	TwDraw();
}