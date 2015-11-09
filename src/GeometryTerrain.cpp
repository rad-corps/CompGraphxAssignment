#include "GeometryTerrain.h"
#include "gl_core_4_4.h"
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <iostream>
#include "simplexnoise.h"
#include <vector>
#include "FileIO.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

GeometryTerrain* GeometryTerrain::sm_singleton = nullptr;

std::vector<std::vector<glm::vec3>> GeometryTerrain::points;
std::vector<std::vector<glm::vec4>> GeometryTerrain::colours;
unsigned int GeometryTerrain::grassTexture;

GeometryTerrain::GeometryTerrain(unsigned int maxTerrainTris_, unsigned int maxPlainTris_ )
	: 
	maxTerrainTris(maxTerrainTris_),
	terrainTriCount(0),
	terrainTris(new GizmoTri[maxTerrainTris_]),
	maxPlainTris(maxPlainTris_),
	plainTriCount(0),
	plainTris(new GizmoTri[maxPlainTris_]),
	lightPosition(glm::vec4(0,3,0,1))
	{
	
	//preallocate vector memory for terrain points and colours
	points.resize(200);
	colours.resize(200);
	for (int i = 0; i < 200; ++i)
	{
		points[i].resize(200);
		colours[i].resize(200);
	}

	{
		//set up shaders for terrain
		std::string vsSource = FileIO::read_file("ShaderVertTerrain.glsl");
		std::string fsSource = FileIO::read_file("ShaderFragTerrain.glsl");
		const char * vsSourceCstr = vsSource.c_str();
		const char * fsSourceCstr = fsSource.c_str();

		//Create the Vertex and Fragment Shaders
		unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
		unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);

		//Set the shaders' source code and compile the shaders
		//glShaderSource(vs, 1, &vsSourceCstr, 0);
		glShaderSource(vs, 1, &vsSourceCstr, 0);
		glCompileShader(vs);

		glShaderSource(fs, 1, &fsSourceCstr, 0);
		glCompileShader(fs);

		//Create the shader program, attach the shaders 
		terrainShaderProgram = glCreateProgram();
		glAttachShader(terrainShaderProgram, vs);
		glAttachShader(terrainShaderProgram, fs);

		//link the glsl program once the shaders are attached
		glLinkProgram(terrainShaderProgram);

		//shader error checking and reporting
		int success = GL_FALSE;
		int infoLogLength = 0;
		glGetShaderiv(terrainShaderProgram, GL_INFO_LOG_LENGTH, &infoLogLength);

		if (infoLogLength > 0)
		{
			char* infoLog = new char[infoLogLength];
			//cout << infoLog << endl;
			glGetShaderInfoLog(terrainShaderProgram, infoLogLength, 0, infoLog);
			printf("Error: glGetShaderiv failed\n%s\n", infoLog);
			delete[] infoLog;
		}

		//glsl program error checking
		glGetProgramiv(terrainShaderProgram, GL_LINK_STATUS, &success);
		if (success == GL_FALSE)
		{
			printf("Error: glGetProgramiv Failed!\n\n");
		}

		//we can delete the original shader memory once the program is linked, as the program has its own copy
		glDeleteShader(vs);
		glDeleteShader(fs);
	}

	//setup simple shaders
	{
		//set up shaders for terrain
		std::string vsSource = FileIO::read_file("SimpleVertShader.glsl");
		std::string fsSource = FileIO::read_file("SimpleFragShader.glsl");
		const char * vsSourceCstr = vsSource.c_str();
		const char * fsSourceCstr = fsSource.c_str();

		//Create the Vertex and Fragment Shaders
		unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
		unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);

		//Set the shaders' source code and compile the shaders
		//glShaderSource(vs, 1, &vsSourceCstr, 0);
		glShaderSource(vs, 1, &vsSourceCstr, 0);
		glCompileShader(vs);

		glShaderSource(fs, 1, &fsSourceCstr, 0);
		glCompileShader(fs);

		//Create the shader program, attach the shaders 
		simpleShaderProgram = glCreateProgram();
		glAttachShader(simpleShaderProgram, vs);
		glAttachShader(simpleShaderProgram, fs);

		//link the glsl program once the shaders are attached
		glLinkProgram(simpleShaderProgram);

		//we can delete the original shader memory once the program is linked, as the program has its own copy
		glDeleteShader(vs);
		glDeleteShader(fs);
	}

	//Setup the terrain data
	glGenBuffers( 1, &terrainVBO );
	glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
	glBufferData(GL_ARRAY_BUFFER, maxTerrainTris * sizeof(GizmoTri), terrainTris, GL_DYNAMIC_DRAW);
	glGenVertexArrays(1, &terrainVAO);
	glBindVertexArray(terrainVAO);
	glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(GizmoVertex), 0);			//position
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(GizmoVertex), ((char*)0) + 16);//colour
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GizmoVertex), ((char*)0) + 32);//tex coord
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(GizmoVertex), ((char*)0) + 40);//normal
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Setup the plain tri data
	glGenBuffers(1, &plainVBO);
	glBindBuffer(GL_ARRAY_BUFFER, plainVBO);
	glBufferData(GL_ARRAY_BUFFER, maxPlainTris * sizeof(GizmoTri), plainTris, GL_DYNAMIC_DRAW);
	glGenVertexArrays(1, &plainVAO);
	glBindVertexArray(plainVAO);
	glBindBuffer(GL_ARRAY_BUFFER, plainVBO);
	glEnableVertexAttribArray(0); //position vec4
	glEnableVertexAttribArray(1); //colour vec4
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(GizmoVertex), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(GizmoVertex), ((char*)0) + 16);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//load texture
	//Load a texture
	glEnable(GL_TEXTURE_2D);
	int imageWidth = 0, imageHeight = 0, imageFormat = 0;
	unsigned char* data = stbi_load("./data/textures/grass.jpg", &imageWidth, &imageHeight, &imageFormat, STBI_rgb);
	grassTexture = 0;
	glGenTextures(1, &grassTexture);
	glBindTexture(GL_TEXTURE_2D, grassTexture);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexImage2D(GL_TEXTURE_2D, 0, imageFormat, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	stbi_image_free(data);
}

GeometryTerrain::~GeometryTerrain() {
	delete[] terrainTris;
	glDeleteBuffers( 1, &terrainVBO );
	glDeleteVertexArrays( 1, &terrainVAO);
	glDeleteProgram(terrainShaderProgram);

	delete[] plainTris;
	glDeleteBuffers(1, &plainVBO);
	glDeleteVertexArrays(1, &plainVAO);
	glDeleteProgram(simpleShaderProgram);
}

void GeometryTerrain::create(unsigned int maxTris_, unsigned int maxPlainTris_ ) {
	if (sm_singleton == nullptr)
		sm_singleton = new GeometryTerrain(maxTris_, maxPlainTris_);
}

void GeometryTerrain::destroy() {
	delete sm_singleton;
	sm_singleton = nullptr;
}

void GeometryTerrain::clear() {
	sm_singleton->terrainTriCount = 0;
	sm_singleton->plainTriCount = 0;
}

void GeometryTerrain::addTerrain(const int& size_, const float& octaves_, const float& height_, float* RGBFloats_, const float& scale2_, const float& persistance_)
{
	for (int z = 0; z < size_; ++z)
	{
		for (int x = 0; x < size_; ++x)
		{
			glm::vec4 colourBottom(RGBFloats_[0], RGBFloats_[1], RGBFloats_[2], 1.0f);
			glm::vec4 colourTop(RGBFloats_[3], RGBFloats_[4], RGBFloats_[5], 1.0f);
			
			float temp_height = octave_noise_2d(octaves_, persistance_, scale2_, x, z);
			points[x][z] = glm::vec3(x, height_ * temp_height, z);

			glm::vec4 newcolour = glm::lerp(colourBottom, colourTop, temp_height);
			//gen colour based on Y height
			colours[x][z] = newcolour;
		}		
	}
	//create the triangles from the points
	for (int z = 0; z < size_ - 1; ++z)
	{
		for (int x = 0; x < size_ - 1; ++x)
		{
			addTri(TriType::TRI_TERRAIN, points[x][z], points[x][z+1], points[x+1][z+1], colours[x][z], 0);
			addTri(TriType::TRI_TERRAIN, points[x][z], points[x + 1][z + 1], points[x + 1][z], colours[x][z], 1);
		}
	}
}

void GeometryTerrain::addCube(glm::vec3& position_, const float& size_)
{
	glm::vec3 pos2 = position_;
	glm::vec3 pos3 = position_;
	glm::vec4 colour = glm::vec4(1,1,1,1);
	pos2.y += 1;
	pos3.x += 1;
	addTri(TRI_PLAIN, position_, pos2, pos3, colour, 0);
}

void GeometryTerrain::addLight(glm::vec3& position_)
{
	sm_singleton->lightPosition = glm::vec4(position_, 1);
	addCube(position_, 1);
}

glm::vec4 GeometryTerrain::calculateSurfaceNormal(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3)
{
	glm::vec3 v = p2 - p1;
	glm::vec3 w = p3 - p1;
	glm::vec4 n;
	n.x = (v.y * w.z) - (v.z * w.y);
	n.y = (v.z * w.x) - (v.x * w.z);
	n.z = (v.x * w.y) - (v.y * w.x);
	n.w = 1;

	////may need to normalise n? 
	float length = sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
	n.x = n.x / length;
	n.y = n.y / length;
	n.z = n.z / length;
	return n;
}

void GeometryTerrain::addTri(TriType type_, const glm::vec3& a_rv0, const glm::vec3& a_rv1, const glm::vec3& a_rv2, const glm::vec4& a_colour, const int& triNum_) 
{
	if (type_ == TRI_TERRAIN)
	{
		if (sm_singleton->terrainTriCount < sm_singleton->maxTerrainTris)
		{
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v0.x = a_rv0.x;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v0.y = a_rv0.y;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v0.z = a_rv0.z;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v0.w = 1;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v1.x = a_rv1.x;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v1.y = a_rv1.y;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v1.z = a_rv1.z;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v1.w = 1;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v2.x = a_rv2.x;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v2.y = a_rv2.y;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v2.z = a_rv2.z;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v2.w = 1;

			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v0.r = a_colour.r;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v0.g = a_colour.g;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v0.b = a_colour.b;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v0.a = 1;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v1.r = a_colour.r;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v1.g = a_colour.g;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v1.b = a_colour.b;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v1.a = 1;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v2.r = a_colour.r;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v2.g = a_colour.g;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v2.b = a_colour.b;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v2.a = 1;

			if (triNum_ <= 0)
			{
				//set vertex UV's
				sm_singleton->terrainTris[sm_singleton->terrainTriCount].v0.s = 0;
				sm_singleton->terrainTris[sm_singleton->terrainTriCount].v0.t = 0;
				sm_singleton->terrainTris[sm_singleton->terrainTriCount].v1.s = 0;
				sm_singleton->terrainTris[sm_singleton->terrainTriCount].v1.t = 1;
				sm_singleton->terrainTris[sm_singleton->terrainTriCount].v2.s = 1;
				sm_singleton->terrainTris[sm_singleton->terrainTriCount].v2.t = 1;
			}
			if (triNum_ >= 1)
			{
				//set vertex UV's
				sm_singleton->terrainTris[sm_singleton->terrainTriCount].v0.s = 0;
				sm_singleton->terrainTris[sm_singleton->terrainTriCount].v0.t = 0;
				sm_singleton->terrainTris[sm_singleton->terrainTriCount].v1.s = 1;
				sm_singleton->terrainTris[sm_singleton->terrainTriCount].v1.t = 1;
				sm_singleton->terrainTris[sm_singleton->terrainTriCount].v2.s = 1;
				sm_singleton->terrainTris[sm_singleton->terrainTriCount].v2.t = 0;
			}

			//add surface normals
			glm::vec4 n = calculateSurfaceNormal(a_rv0, a_rv1, a_rv2);
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v0.nx = n.x;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v0.ny = n.y;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v0.nz = n.z;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v0.nw = 1;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v1.nx = n.x;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v1.ny = n.y;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v1.nz = n.z;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v1.nw = 1;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v2.nx = n.x;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v2.ny = n.y;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v2.nz = n.z;
			sm_singleton->terrainTris[sm_singleton->terrainTriCount].v2.nw = 1;

			sm_singleton->terrainTriCount++;
		}
	}
	else if (type_ == TRI_PLAIN)
	{
		sm_singleton->plainTris[sm_singleton->plainTriCount].v0.x = a_rv0.x;
		sm_singleton->plainTris[sm_singleton->plainTriCount].v0.y = a_rv0.y;
		sm_singleton->plainTris[sm_singleton->plainTriCount].v0.z = a_rv0.z;
		sm_singleton->plainTris[sm_singleton->plainTriCount].v0.w = 1;
		sm_singleton->plainTris[sm_singleton->plainTriCount].v1.x = a_rv1.x;
		sm_singleton->plainTris[sm_singleton->plainTriCount].v1.y = a_rv1.y;
		sm_singleton->plainTris[sm_singleton->plainTriCount].v1.z = a_rv1.z;
		sm_singleton->plainTris[sm_singleton->plainTriCount].v1.w = 1;
		sm_singleton->plainTris[sm_singleton->plainTriCount].v2.x = a_rv2.x;
		sm_singleton->plainTris[sm_singleton->plainTriCount].v2.y = a_rv2.y;
		sm_singleton->plainTris[sm_singleton->plainTriCount].v2.z = a_rv2.z;
		sm_singleton->plainTris[sm_singleton->plainTriCount].v2.w = 1;

		sm_singleton->plainTris[sm_singleton->plainTriCount].v0.r = a_colour.r;
		sm_singleton->plainTris[sm_singleton->plainTriCount].v0.g = a_colour.g;
		sm_singleton->plainTris[sm_singleton->plainTriCount].v0.b = a_colour.b;
		sm_singleton->plainTris[sm_singleton->plainTriCount].v0.a = 1;
		sm_singleton->plainTris[sm_singleton->plainTriCount].v1.r = a_colour.r;
		sm_singleton->plainTris[sm_singleton->plainTriCount].v1.g = a_colour.g;
		sm_singleton->plainTris[sm_singleton->plainTriCount].v1.b = a_colour.b;
		sm_singleton->plainTris[sm_singleton->plainTriCount].v1.a = 1;
		sm_singleton->plainTris[sm_singleton->plainTriCount].v2.r = a_colour.r;
		sm_singleton->plainTris[sm_singleton->plainTriCount].v2.g = a_colour.g;
		sm_singleton->plainTris[sm_singleton->plainTriCount].v2.b = a_colour.b;
		sm_singleton->plainTris[sm_singleton->plainTriCount].v2.a = 1;

		sm_singleton->plainTriCount++;
	}
}

void GeometryTerrain::draw(const glm::mat4& a_projectionView)
{
	//draw the terrain
	glUseProgram(sm_singleton->terrainShaderProgram);

	//used to temporarily store the uniform locations of the vertex shader
	unsigned int loc = 0;
		
	//send through the projection view matrix
	loc = glGetUniformLocation(sm_singleton->terrainShaderProgram, "ProjectionView");
	glUniformMatrix4fv(loc, 1, false, glm::value_ptr(a_projectionView));

	//send through the light position
	loc = glGetUniformLocation(sm_singleton->terrainShaderProgram, "lightPosition");
	std::cout << sm_singleton->lightPosition[0] << ',' << sm_singleton->lightPosition[1] << ',' << sm_singleton->lightPosition[2] << std::endl;
	glUniform4fv(loc, 1, &sm_singleton->lightPosition[0]);

	//set the active texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, grassTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	if (sm_singleton->terrainTriCount > 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, sm_singleton->terrainVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sm_singleton->terrainTriCount * sizeof(GizmoTri), sm_singleton->terrainTris);

		//by binding this vertex array object, we dont have to set our vertex attrib pointers each time
		glBindVertexArray(sm_singleton->terrainVAO);
		glDrawArrays(GL_TRIANGLES, 0, sm_singleton->terrainTriCount * 3);
	}


	//draw the camera
	glUseProgram(sm_singleton->simpleShaderProgram);

	//send through the projection view matrix
	loc = glGetUniformLocation(sm_singleton->terrainShaderProgram, "ProjectionView");
	glUniformMatrix4fv(loc, 1, false, glm::value_ptr(a_projectionView));

	if (sm_singleton->plainTriCount > 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, sm_singleton->plainVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sm_singleton->plainTriCount * sizeof(GizmoTri), sm_singleton->plainTris);

		//by binding this vertex array object, we dont have to set our vertex attrib pointers each time
		glBindVertexArray(sm_singleton->plainVAO);
		glDrawArrays(GL_TRIANGLES, 0, sm_singleton->plainTriCount * 3);
	}
}