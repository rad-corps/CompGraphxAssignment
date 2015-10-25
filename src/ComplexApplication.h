#pragma once

#include "BaseApplication.h"
#include "AntTweakBar.h"

// only needed for the camera picking
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class Camera;

class ComplexApplication : public BaseApplication {
public:

	ComplexApplication();
	virtual ~ComplexApplication();

	virtual bool startup();
	virtual void shutdown();

	virtual bool update(float deltaTime);
	virtual void draw();

private:

	Camera*		m_camera;

	// this is an example position for camera picking
	glm::vec3	m_pickPosition;

	//GUI Controls
	TwBar* guiBar;
	int terrainSize;
	float octaves;
	float smoothness;
	float height;

	int bottomRed;
	int bottomGreen;
	int bottomBlue;

	int topRed;
	int topGreen;
	int topBlue;

	float persistance;
	float scale2;
};