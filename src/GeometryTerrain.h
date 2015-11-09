#pragma once

#include <glm/fwd.hpp>
#include <glm/vec4.hpp>
#include <vector>

class GeometryTerrain {
public:

	static void		create(unsigned int a_maxLines = 0xffff, unsigned int a_maxTris = 0xffff,
						   unsigned int a_max2DLines = 0xff, unsigned int a_max2DTris = 0xff);
	static void		destroy();

	// removes all Gizmos
	static void		clear();

	// draws current Gizmo buffers, either using a combined (projection * view) matrix, or separate matrices
	static void		draw(const glm::mat4& a_projectionView);
	
	// Adds a triangle.
	static void		addTri(const glm::vec3& a_rv0, const glm::vec3& a_rv1, const glm::vec3& a_rv2, const glm::vec4& a_colour, const int& triNum_);

	static glm::vec4 calculateSurfaceNormal(const glm::vec3& a_rv0, const glm::vec3& a_rv1, const glm::vec3& a_rv2);

	//Adds a rectangular prism
	static void		addRectangularPrism(const glm::vec3& startingPoint, const float& w_, const float& h_, const float& d_);

	static void		addTerrain(const int& size_, const float& octaves_, const float& height_, float* RGBFloats_, const float& scale2_, const float& persistance_);

private:

	GeometryTerrain(unsigned int a_maxLines, unsigned int a_maxTris,
		   unsigned int a_max2DLines, unsigned int a_max2DTris);
	~GeometryTerrain();

	struct GizmoVertex {
		float x, y, z, w;
		float r, g, b, a;
		float s, t; //UV coords
		float nx, ny, nz, nw; //surface normal
	};

	struct GizmoLine {
		GizmoVertex v0;
		GizmoVertex v1;
	};

	struct GizmoTri {
		GizmoVertex v0;
		GizmoVertex v1;
		GizmoVertex v2;
	};

	unsigned int	terrainShaderProgram;

	// triangle data
	unsigned int	maxTerrainTris;
	unsigned int	terrainTriCount;
	GizmoTri*		terrainTris;
	unsigned int	terrainVAO;
	unsigned int 	terrainVBO;
	
	//singleton
	static GeometryTerrain*	sm_singleton;

	//stores the vertex info for the terrain
	static std::vector<std::vector<glm::vec3>> points;
	static std::vector<std::vector<glm::vec4>> colours;
	
	static unsigned int grassTexture;

	glm::vec4 lightPosition;
};