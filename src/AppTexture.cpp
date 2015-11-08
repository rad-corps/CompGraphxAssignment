//#include "AppTexture.h"
//#include "gl_core_4_4.h"
//
//#include <GLFW/glfw3.h>
//#include <glm/glm.hpp>
//#include <glm/ext.hpp>
//
//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>
//
//#include <iostream>//debugging
//
//#include "Camera.h"
//#include "GeometryTexture.h"
//
//
//using glm::vec3;
//using glm::vec4;
//using glm::mat4;
//
//
//
//AppTexture::AppTexture()
//: m_camera(nullptr) {
//
//
//}
//
//AppTexture::~AppTexture() {
//
//}
//
//bool AppTexture::startup() {
//
//	// create a basic window
//	createWindow("Complex Game Systems", 1280, 720);
//
//	// start the gizmo system that can draw basic shapes
//	//GeometryTexture::create();
//
//	// create a camera
//	m_camera = new Camera(glm::pi<float>() * 0.25f, 16 / 9.f, 0.1f, 1000.f);
//	m_camera->setLookAtFrom(vec3(10, 10, 10), vec3(0));
//
//	//////////////////////////////////////////////////////////////////////////
//	// YOUR STARTUP CODE HERE
//	//////////////////////////////////////////////////////////////////////////
//	m_pickPosition = glm::vec3(0);
//	
//	gt = new GeometryTexture(); 
//
//	return true;
//}
//
//void AppTexture::shutdown() {
//
//	// delete our camera and cleanup gizmos
//	delete m_camera;
//	//GeometryTexture::destroy();
//
//	// destroy our window properly
//	destroyWindow();
//}
//
//bool AppTexture::update(float deltaTime) {
//
//	// close the application if the window closes
//	if (glfwWindowShouldClose(m_window) ||
//		glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//		return false;
//
//	// update the camera's movement
//	m_camera->update(deltaTime);
//
//	// clear the gizmos out for this frame
//	//GeometryTexture::clear();
//
//	// return true, else the application closes
//	return true;
//}
//
//void AppTexture::draw() {
//
//	// clear the screen for this frame
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	// display the 3D gizmos
//	gt->draw(m_camera->getProjectionView());
//	// get an orthographic projection matrix and draw 2D gizmos
//	int width = 0, height = 0;
//	glfwGetWindowSize(m_window, &width, &height);
//	mat4 guiMatrix = glm::ortho<float>(0, 0, (float)width, (float)height);
//}