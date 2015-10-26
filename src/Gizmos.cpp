#include "Gizmos.h"
#include "gl_core_4_4.h"
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <iostream>
#include "simplexnoise.h"
#include <vector>

Gizmos* Gizmos::sm_singleton = nullptr;

std::vector<std::vector<glm::vec3>> Gizmos::points;
std::vector<std::vector<glm::vec4>> Gizmos::colours;

Gizmos::Gizmos(unsigned int a_maxLines, unsigned int a_maxTris,
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
	
	// create shaders
	//const char* vsSource = "#version 410\n \
	//				 in vec4 Position; \
	//				 in vec4 Colour; \
	//				 out vec4 vColour; \
	//				 uniform mat4 ProjectionView; \
	//				 void main() { vColour = Colour; gl_Position = ProjectionView * Position; }";

	const char* vsSource = 
"#version 410\n \
layout(location=0) in vec4 Position; \
layout(location=1) in vec4 Colour; \
layout(location=2) in vec2 TexCoord; \
out vec2 vTexCoord; \
out vec4 vColour; \
uniform mat4 ProjectionView; \
void main() { \
vColour = Colour; \
vTexCoord = TexCoord; \
gl_Position= ProjectionView * Position;\
}";
		 
	//const char* fsSource = "#version 410\n \
	//				 in vec4 vColour; \
 //                    out vec4 FragColor; \
	//				 void main()	{ FragColor = vColour; }";

	const char* fsSource =
		"#version 410\n \
		in vec2 vTexCoord; \
		in vec4 vColour; \
		out vec4 FragColor; \
		uniform sampler2D diffuse; \
		void main() { \
		FragColor = texture(diffuse,vTexCoord); \
		FragColor = vColour; \
}";
    
    //Create the Vertex and Fragment Shaders
	unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);

	//Set the shaders' source code and compile the shaders
	glShaderSource(vs, 1, (const char**)&vsSource, 0);
	glCompileShader(vs);

	glShaderSource(fs, 1, (const char**)&fsSource, 0);
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
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, sizeof(GizmoVertex), ((char*)0) + 16);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GizmoVertex), ((char*)0) + 32);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Gizmos::~Gizmos() {
	delete[] m_tris;
	delete[] m_transparentTris;
	glDeleteBuffers( 1, &m_triVBO );
	glDeleteBuffers( 1, &m_transparentTriVBO );
	glDeleteVertexArrays( 1, &m_triVAO );
	glDeleteVertexArrays( 1, &m_transparentTriVAO );
	glDeleteProgram(m_shader);
}

void Gizmos::create(unsigned int a_maxLines /* = 0xffff */, unsigned int a_maxTris /* = 0xffff */,
					unsigned int a_max2DLines /* = 0xff */, unsigned int a_max2DTris /* = 0xff */) {
	if (sm_singleton == nullptr)
		sm_singleton = new Gizmos(a_maxLines,a_maxTris,a_max2DLines,a_max2DTris);
}

void Gizmos::destroy() {
	delete sm_singleton;
	sm_singleton = nullptr;
}

void Gizmos::clear() {
	sm_singleton->m_triCount = 0;
	sm_singleton->m_transparentTriCount = 0;
}

void Gizmos::addTerrain(const int& size_, const float& octaves_, const float& height_, float* RGBFloats_, const float& scale2_, const float& persistance_)
{
	for (int z = 0; z < size_; ++z)
	{
		for (int x = 0; x < size_; ++x)
		{
			glm::vec4 colourBottom(RGBFloats_[0], RGBFloats_[1], RGBFloats_[2], 1.0f);
			glm::vec4 colourTop(RGBFloats_[3], RGBFloats_[4], RGBFloats_[5], 1.0f);
			
			float temp_height = octave_noise_2d(octaves_, persistance_, scale2_, x, z);
			//std::cout << temp_height << std::endl;
			points[x][z] = glm::vec3(x, height_ * temp_height, z);
			//temp_height += 1.0f;

			glm::vec4 newcolour = glm::lerp(colourBottom, colourTop, temp_height);
			//gen colour based on Y height
			//colours[x][z] = glm::vec4(temp_height * 0.2, temp_height, 0.5 - temp_height, 1.0f);
			//colours[x][z] = glm::vec4(temp_height * 0.2, temp_height, temp_height * 0.2, 1.0f);
			colours[x][z] = newcolour;
		}		
	}
	//create the triangles from the points
	for (int z = 0; z < size_ - 1; ++z)
	{
		for (int x = 0; x < size_ - 1; ++x)
		{
			addTri(points[x][z], points[x][z+1], points[x+1][z+1], colours[x][z]);
			addTri(points[x][z], points[x + 1][z + 1], points[x + 1][z], colours[x][z]);
		}
	}
}

//Adds a rectangular prism
void Gizmos::addRectangularPrism(const glm::vec3& startingPoint, const float& w_, const float& h_, const float& d_)
{
	//create an arbitrary colour (white)
	glm::vec4 colour(1.0f, 1.0f, 1.0f, 1.0f);
	//glm::vec4 colour(0.7, 0.4, 0.8, 1);

	//create the 8 points
	glm::vec3 points[8];
	points[0] = startingPoint + glm::vec3(0, 0, 0);
	points[1] = startingPoint + glm::vec3(w_, 0, 0);
	points[2] = startingPoint + glm::vec3(w_, h_, 0);
	points[3] = startingPoint + glm::vec3(0, h_, 0);
	points[4] = startingPoint + glm::vec3(0, 0, d_);
	points[5] = startingPoint + glm::vec3(w_, 0, d_);
	points[6] = startingPoint + glm::vec3(w_, h_, d_);
	points[7] = startingPoint + glm::vec3(0, h_, d_);


	//create triangle 1
	addTri(points[0], points[2], points[1], colour);
	addTri(points[2], points[0], points[3], colour);
	addTri(points[3], points[0], points[4], colour);
	addTri(points[3], points[4], points[7], colour);

	addTri(points[7], points[6], points[2], colour);
	addTri(points[2], points[3], points[7], colour);
	
	addTri(points[1], points[2], points[6], colour);
	addTri(points[6], points[5], points[1], colour);

	addTri(points[4], points[6], points[7], colour);
	addTri(points[4], points[5], points[6], colour);

	addTri(points[1], points[5], points[4], colour);
	addTri(points[4], points[0], points[1], colour);
	
	//addTri(startingPoint, startingPoint + points[1], startingPoint + points[2], colour);
	//addTri(startingPoint, startingPoint + points[1], startingPoint + points[2], colour);

	
	//create triangle 2	
}

void Gizmos::addTri(const glm::vec3& a_rv0, const glm::vec3& a_rv1, const glm::vec3& a_rv2, const glm::vec4& a_colour) {
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
				sm_singleton->m_tris[sm_singleton->m_triCount].v0.w = a_colour.a;
				sm_singleton->m_tris[sm_singleton->m_triCount].v1.r = a_colour.r;
				sm_singleton->m_tris[sm_singleton->m_triCount].v1.g = a_colour.g;
				sm_singleton->m_tris[sm_singleton->m_triCount].v1.b = a_colour.b;
				sm_singleton->m_tris[sm_singleton->m_triCount].v1.w = a_colour.a;
				sm_singleton->m_tris[sm_singleton->m_triCount].v2.r = a_colour.r;
				sm_singleton->m_tris[sm_singleton->m_triCount].v2.g = a_colour.g;
				sm_singleton->m_tris[sm_singleton->m_triCount].v2.b = a_colour.b;
				sm_singleton->m_tris[sm_singleton->m_triCount].v2.w = a_colour.a;

				//set vertex UV's
				sm_singleton->m_tris[sm_singleton->m_transparentTriCount].v0.s = 0;
				sm_singleton->m_tris[sm_singleton->m_transparentTriCount].v0.t = 0;
				sm_singleton->m_tris[sm_singleton->m_transparentTriCount].v1.s = 1;
				sm_singleton->m_tris[sm_singleton->m_transparentTriCount].v1.t = 1;
				sm_singleton->m_tris[sm_singleton->m_transparentTriCount].v2.s = 0;
				sm_singleton->m_tris[sm_singleton->m_transparentTriCount].v2.t = 1;

				sm_singleton->m_triCount++;
			}
		}
		else
		{
			if (sm_singleton->m_transparentTriCount < sm_singleton->m_maxTris)
			{
				//set vertex location
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v0.x = a_rv0.x;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v0.y = a_rv0.y;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v0.z = a_rv0.z;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v0.w = 1;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v1.x = a_rv1.x;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v1.y = a_rv1.y;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v1.z = a_rv1.z;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v1.w = 1;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v2.x = a_rv2.x;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v2.y = a_rv2.y;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v2.z = a_rv2.z;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v2.w = 1;

				//set vertex colour
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v0.r = a_colour.r;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v0.g = a_colour.g;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v0.b = a_colour.b;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v0.a = a_colour.a;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v1.r = a_colour.r;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v1.g = a_colour.g;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v1.b = a_colour.b;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v1.a = a_colour.a;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v2.r = a_colour.r;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v2.g = a_colour.g;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v2.b = a_colour.b;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v2.a = a_colour.a;

				//set vertex UV's
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v0.s = 0;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v0.t = 0;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v1.s = 0;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v1.t = 1;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v2.s = 1;
				sm_singleton->m_transparentTris[sm_singleton->m_transparentTriCount].v2.t = 1;

				sm_singleton->m_transparentTriCount++;
			}
		}
	}
}

void Gizmos::draw(const glm::mat4& a_projectionView, const unsigned int& texture_) {
	if ( sm_singleton != nullptr && ( sm_singleton->m_triCount > 0 || sm_singleton->m_transparentTriCount > 0))
	{
		int shader = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &shader);

		glUseProgram(sm_singleton->m_shader);
		
		unsigned int loc = glGetUniformLocation(sm_singleton->m_shader,"ProjectionView");
		glUniformMatrix4fv(loc, 1, false, glm::value_ptr(a_projectionView));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_);		loc = glGetUniformLocation(sm_singleton->m_shader, "diffuse");
		glUniform1i(loc, 0);

		if (sm_singleton->m_triCount > 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER, sm_singleton->m_triVBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sm_singleton->m_triCount * sizeof(GizmoTri), sm_singleton->m_tris);

			//by binding this vertex array object, we dont have to set our vertex attrib pointers each time
			glBindVertexArray(sm_singleton->m_triVAO);			
			glDrawArrays(GL_TRIANGLES, 0, sm_singleton->m_triCount * 3);
		}

		if (sm_singleton->m_transparentTriCount > 0)
		{
			// not ideal to store these, but Gizmos must work stand-alone
			GLboolean blendEnabled = glIsEnabled(GL_BLEND);
			GLboolean depthMask = GL_TRUE;
			glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
			int src, dst;
			glGetIntegerv(GL_BLEND_SRC, &src);
			glGetIntegerv(GL_BLEND_DST, &dst);

			// setup blend states
			if (blendEnabled == GL_FALSE)
				glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDepthMask(GL_FALSE);

			glBindBuffer(GL_ARRAY_BUFFER, sm_singleton->m_transparentTriVBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sm_singleton->m_transparentTriCount * sizeof(GizmoTri), sm_singleton->m_transparentTris);

			glBindVertexArray(sm_singleton->m_transparentTriVAO);
			glDrawArrays(GL_TRIANGLES, 0, sm_singleton->m_transparentTriCount * 3);

			// reset state
			glDepthMask(depthMask);
			glBlendFunc(src, dst);
			if (blendEnabled == GL_FALSE)
				glDisable(GL_BLEND);
		}

		glUseProgram(shader);
	}
}