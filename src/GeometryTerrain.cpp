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

GeometryTerrain::GeometryTerrain(unsigned int a_maxLines, unsigned int a_maxTris,
			   unsigned int a_max2DLines, unsigned int a_max2DTris)
	: 
	m_maxTris(a_maxTris),
	m_triCount(0),
	m_tris(new GizmoTri[a_maxTris]),
	m_transparentTriCount(0),
	m_transparentTris(new GizmoTri[a_maxTris])
	{
	
	//preallocate vector memory
	points.resize(200);
	colours.resize(200);
	for (int i = 0; i < 200; ++i)
	{
		points[i].resize(200);
		colours[i].resize(200);
	}

	std::string vsSource = FileIO::read_file("ShaderVertTerrain.glsl");
	std::string fsSource = FileIO::read_file("ShaderFragTerrain.glsl");	const char * vsSourceCstr = vsSource.c_str();
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
	m_shader = glCreateProgram();
	glAttachShader(m_shader, vs);
	glAttachShader(m_shader, fs);

	//set the locations for the "in vars" in the shaders. this can also be done in the shader code itself
	glBindAttribLocation(m_shader, 0, "Position");
	glBindAttribLocation(m_shader, 1, "Colour");
	glBindAttribLocation(m_shader, 2, "TexCoord");
	
	//link the glsl program once the shaders are attached
	glLinkProgram(m_shader);
    
	
	//shader error checking and reporting
	int success = GL_FALSE;
	int infoLogLength = 0;
	glGetShaderiv(m_shader, GL_INFO_LOG_LENGTH, &infoLogLength);

	if (infoLogLength > 0)
	{
		char* infoLog = new char[infoLogLength];
		//cout << infoLog << endl;
		glGetShaderInfoLog(m_shader, infoLogLength, 0, infoLog);
		printf("Error: glGetShaderiv failed\n%s\n", infoLog);
		delete[] infoLog;
	}

	//glsl program error checking
	glGetProgramiv(m_shader, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{	
		printf("Error: glGetProgramiv Failed!\n\n");
	}

	//we can delete the original shader memory once the program is linked, as the program has its own copy
	glDeleteShader(vs);
	glDeleteShader(fs);

	////ARG1: reserve buffer data for the VBO currently bound to GL_ARRAY_BUFFER (m_lineVBO). 
	////ARG2: reserve maxlines * multiplied by the size of a GizmoLine to get the number of bytes to reserve
	////ARG3: m_lines is an array of GizmoLines whose length is m_maxLines (see constructor initialiser list)
	////void glBufferData(GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage);	
	//glBufferData(GL_ARRAY_BUFFER, m_maxLines * sizeof(GizmoLine), m_lines, GL_DYNAMIC_DRAW);

	glGenBuffers( 1, &m_triVBO );
	glBindBuffer(GL_ARRAY_BUFFER, m_triVBO);
	glBufferData(GL_ARRAY_BUFFER, m_maxTris * sizeof(GizmoTri), m_tris, GL_DYNAMIC_DRAW);


	//The vertex array tracks the glBindBuffer, glEnableVertexAttribArray and glVertexAttribPointer state of the bound buffer m_triVBO
	glGenVertexArrays(1, &m_triVAO);
	glBindVertexArray(m_triVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_triVBO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(GizmoVertex), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(GizmoVertex), ((char*)0) + 16);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GizmoVertex), ((char*)0) + 32);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//load texture
	//Load a texture
	glEnable(GL_TEXTURE_2D);
	int imageWidth = 0, imageHeight = 0, imageFormat = 0;
	unsigned char* data = stbi_load("./data/textures/grass.jpg", &imageWidth, &imageHeight, &imageFormat, STBI_rgb);	grassTexture = 0;	glGenTextures(1, &grassTexture);
	glBindTexture(GL_TEXTURE_2D, grassTexture);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexImage2D(GL_TEXTURE_2D, 0, imageFormat, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	stbi_image_free(data);


	glUseProgram(m_shader);
}

	GeometryTerrain::~GeometryTerrain() {
	delete[] m_tris;
	delete[] m_transparentTris;
	glDeleteBuffers( 1, &m_triVBO );
	glDeleteBuffers( 1, &m_transparentTriVBO );
	glDeleteVertexArrays( 1, &m_triVAO );
	glDeleteVertexArrays( 1, &m_transparentTriVAO );
	glDeleteProgram(m_shader);
}

	void GeometryTerrain::create(unsigned int a_maxLines /* = 0xffff */, unsigned int a_maxTris /* = 0xffff */,
					unsigned int a_max2DLines /* = 0xff */, unsigned int a_max2DTris /* = 0xff */) {
	if (sm_singleton == nullptr)
		sm_singleton = new GeometryTerrain(a_maxLines, a_maxTris, a_max2DLines, a_max2DTris);
}

	void GeometryTerrain::destroy() {
	delete sm_singleton;
	sm_singleton = nullptr;
}

	void GeometryTerrain::clear() {
	sm_singleton->m_triCount = 0;
	sm_singleton->m_transparentTriCount = 0;
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
			addTri(points[x][z], points[x][z+1], points[x+1][z+1], colours[x][z], 0);
			addTri(points[x][z], points[x + 1][z + 1], points[x + 1][z], colours[x][z], 1);
		}
	}
}

void GeometryTerrain::addTri(const glm::vec3& a_rv0, const glm::vec3& a_rv1, const glm::vec3& a_rv2, const glm::vec4& a_colour, const int& triNum_) 
{
	if (sm_singleton != nullptr)
	{
		if (a_colour.w == 1)
		{
			if (sm_singleton->m_triCount < sm_singleton->m_maxTris)
			{
				sm_singleton->m_tris[sm_singleton->m_triCount].v0.x = a_rv0.x;
				sm_singleton->m_tris[sm_singleton->m_triCount].v0.y = a_rv0.y;
				sm_singleton->m_tris[sm_singleton->m_triCount].v0.z = a_rv0.z;
				sm_singleton->m_tris[sm_singleton->m_triCount].v0.w = 1;
				sm_singleton->m_tris[sm_singleton->m_triCount].v1.x = a_rv1.x;
				sm_singleton->m_tris[sm_singleton->m_triCount].v1.y = a_rv1.y;
				sm_singleton->m_tris[sm_singleton->m_triCount].v1.z = a_rv1.z;
				sm_singleton->m_tris[sm_singleton->m_triCount].v1.w = 1;
				sm_singleton->m_tris[sm_singleton->m_triCount].v2.x = a_rv2.x;
				sm_singleton->m_tris[sm_singleton->m_triCount].v2.y = a_rv2.y;
				sm_singleton->m_tris[sm_singleton->m_triCount].v2.z = a_rv2.z;
				sm_singleton->m_tris[sm_singleton->m_triCount].v2.w = 1;

				sm_singleton->m_tris[sm_singleton->m_triCount].v0.r = a_colour.r;
				sm_singleton->m_tris[sm_singleton->m_triCount].v0.g = a_colour.g;
				sm_singleton->m_tris[sm_singleton->m_triCount].v0.b = a_colour.b;
				sm_singleton->m_tris[sm_singleton->m_triCount].v0.a = 1;
				sm_singleton->m_tris[sm_singleton->m_triCount].v1.r = a_colour.r;
				sm_singleton->m_tris[sm_singleton->m_triCount].v1.g = a_colour.g;
				sm_singleton->m_tris[sm_singleton->m_triCount].v1.b = a_colour.b;
				sm_singleton->m_tris[sm_singleton->m_triCount].v1.a = 1;
				sm_singleton->m_tris[sm_singleton->m_triCount].v2.r = a_colour.r;
				sm_singleton->m_tris[sm_singleton->m_triCount].v2.g = a_colour.g;
				sm_singleton->m_tris[sm_singleton->m_triCount].v2.b = a_colour.b;
				sm_singleton->m_tris[sm_singleton->m_triCount].v2.a = 1;

				if (triNum_ <= 0)
				{
					//set vertex UV's
					sm_singleton->m_tris[sm_singleton->m_triCount].v0.s = 0;
					sm_singleton->m_tris[sm_singleton->m_triCount].v0.t = 0;
					sm_singleton->m_tris[sm_singleton->m_triCount].v1.s = 0;
					sm_singleton->m_tris[sm_singleton->m_triCount].v1.t = 1;
					sm_singleton->m_tris[sm_singleton->m_triCount].v2.s = 1;
					sm_singleton->m_tris[sm_singleton->m_triCount].v2.t = 1;
				}
				if (triNum_ >= 1)
				{
					//set vertex UV's
					sm_singleton->m_tris[sm_singleton->m_triCount].v0.s = 0;
					sm_singleton->m_tris[sm_singleton->m_triCount].v0.t = 0;
					sm_singleton->m_tris[sm_singleton->m_triCount].v1.s = 1;
					sm_singleton->m_tris[sm_singleton->m_triCount].v1.t = 1;
					sm_singleton->m_tris[sm_singleton->m_triCount].v2.s = 1;
					sm_singleton->m_tris[sm_singleton->m_triCount].v2.t = 0;
				}

				sm_singleton->m_triCount++;
			}
		}
	}
}

	void GeometryTerrain::addTextureSquare()
{


}

//void GeometryTerrain::draw(const glm::mat4& a_projectionView) 
//{
//	if ( sm_singleton != nullptr && ( sm_singleton->m_triCount > 0 || sm_singleton->m_transparentTriCount > 0))
//	{
//		//send through the projection view matrix
//		unsigned int loc = glGetUniformLocation(sm_singleton->m_shader,"ProjectionView");
//		glUniformMatrix4fv(loc, 1, false, glm::value_ptr(a_projectionView));
//
//		//set the active texture
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, grassTexture);//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//
//		if (sm_singleton->m_triCount > 0)
//		{
//			glBindBuffer(GL_ARRAY_BUFFER, sm_singleton->m_triVBO);
//			glBufferSubData(GL_ARRAY_BUFFER, 0, sm_singleton->m_triCount * sizeof(GizmoTri), sm_singleton->m_tris);
//
//			//by binding this vertex array object, we dont have to set our vertex attrib pointers each time
//			glBindVertexArray(sm_singleton->m_triVAO);			
//			glDrawArrays(GL_TRIANGLES, 0, sm_singleton->m_triCount * 3);
//		}
//	}
//}

void GeometryTerrain::draw(const glm::mat4& a_projectionView)
{
	if (sm_singleton != nullptr && (sm_singleton->m_triCount > 0 || sm_singleton->m_transparentTriCount > 0))
	{
		//send through the projection view matrix
		unsigned int loc = glGetUniformLocation(sm_singleton->m_shader, "ProjectionView");
		glUniformMatrix4fv(loc, 1, false, glm::value_ptr(a_projectionView));

		//set the active texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, grassTexture);		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		if (sm_singleton->m_triCount > 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER, sm_singleton->m_triVBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sm_singleton->m_triCount * sizeof(GizmoTri), sm_singleton->m_tris);

			//by binding this vertex array object, we dont have to set our vertex attrib pointers each time
			glBindVertexArray(sm_singleton->m_triVAO);
			glDrawArrays(GL_TRIANGLES, 0, sm_singleton->m_triCount * 3);
		}
	}
}