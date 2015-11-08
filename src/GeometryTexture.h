#pragma once

#include <glm/fwd.hpp>
#include <vector>

class GeometryTexture {
public:

	GeometryTexture();

	// removes all Gizmos
	static void		clear();

	// draws current Gizmo buffers, either using a combined (projection * view) matrix, or separate matrices
	void		draw(const glm::mat4& a_projectionView);

private:

	unsigned int grassTexture;
	unsigned int shaderProgram;
	unsigned int vbo;
	unsigned int ibo;
	unsigned int vao;
};