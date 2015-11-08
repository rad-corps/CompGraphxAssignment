#include "GeometryTexture.h"
#include "gl_core_4_4.h"
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <iostream>
#include "simplexnoise.h"
#include <vector>
#include "FileIO.hpp"
#include <stb_image.h>
//#include "SOIL.h"


GeometryTexture::GeometryTexture()
{
	std::cout << "GeometryTexture" << std::endl;

	std::string vsSource = FileIO::read_file("ShaderVertTexture.glsl");
	std::string fsSource = FileIO::read_file("ShaderFragTexture.glsl");	const char * vsSourceCstr = vsSource.c_str();
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
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vs);
	glAttachShader(shaderProgram, fs);

	//set the locations for the "in vars" in the shaders. this can also be done in the shader code itself
	//glBindAttribLocation(shaderProgram, 0, "Position");
	//glBindAttribLocation(shaderProgram, 1, "Colour");
	//glBindAttribLocation(shaderProgram, 1, "TexCoord");

	//link the glsl program once the shaders are attached
	glLinkProgram(shaderProgram);


	//shader error checking and reporting
	int success = GL_FALSE;
	int infoLogLength = 0;

	glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(vs, maxLength, &maxLength, &errorLog[0]);

		for (int i = 0; i < errorLog.size(); ++i)
		{
			std::cout << errorLog[i];
		}

		// Provide the infolog in whatever manor you deem best.
		// Exit with failure.
		glDeleteShader(vs); // Don't leak the shader.
		return;
	}

	glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(fs, maxLength, &maxLength, &errorLog[0]);

		for (int i = 0; i < errorLog.size(); ++i)
		{
			std::cout << errorLog[i];
		}

		// Provide the infolog in whatever manor you deem best.
		// Exit with failure.
		glDeleteShader(fs); // Don't leak the shader.
		return;
	}

	//glsl program error checking
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{		
		printf("Error: glGetProgramiv Failed!\n\n");
		
		GLint maxLength = 0;
		glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);

		//The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, &infoLog[0]);

		
		for (int i = 0; i < infoLog.size(); ++i)
		{
			std::cout << infoLog[i];
		}

		//The program is useless now. So delete it.
		glDeleteProgram(shaderProgram);

		//Provide the infolog in whatever manner you deem best.
		//Exit with failure.
		return;
	}

	//we can delete the original shader memory once the program is linked, as the program has its own copy
	glDeleteShader(vs);
	glDeleteShader(fs);

	float vertexData[] = {
		0, 0, 0, 1, 0, 0,
		0, 0, 1, 1, 0, 1,
		1, 0, 1, 1, 1, 1,
		1, 0, 0, 1, 1, 0,
	};
	
	unsigned int indexData[] = {
		0, 1, 2,
		0, 2, 3,
	};

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* 6 * 4, vertexData, GL_DYNAMIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, m_maxTris * sizeof(GizmoTri), m_tris, GL_STREAM_DRAW);
	
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)* 6, indexData, GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float)* 6, 0);				//the vertex position coordinates	
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float)* 6, ((char*)0) + 16); //UV coordinates
	
	//unbind the opengl buffers
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glUseProgram(shaderProgram);

	////Load a texture
	glEnable(GL_TEXTURE_2D);
	int imageWidth = 0, imageHeight = 0, imageFormat = 0;
	unsigned char* data = stbi_load("./data/textures/dot.png",	&imageWidth, &imageHeight, &imageFormat, STBI_default);	glGenTextures(1, &grassTexture);
	glBindTexture(GL_TEXTURE_2D, grassTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	stbi_image_free(data);
}



void GeometryTexture::draw(const glm::mat4& a_projectionView) 
{
	//std::cout << "draw" << std::endl;
	// use our texture program
	glUseProgram(shaderProgram);
	// bind the camera
	int loc = glGetUniformLocation(shaderProgram, "ProjectionView");
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(a_projectionView));
	// set texture slot
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, grassTexture);
	// tell the shader where it is
	loc = glGetUniformLocation(shaderProgram, "diffuse");
	glUniform1i(loc, 0);
	// draw
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}