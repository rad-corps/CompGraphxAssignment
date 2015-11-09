#version 410

layout(location=0) in vec4 Position; 
layout(location=1) in vec4 Colour; 
layout(location=2) in vec2 TexCoord; 
layout(location=3) in vec4 Normal;

out vec2 vTexCoord; 
out vec4 vColour; 
out vec4 vNormal;

uniform mat4 ProjectionView; 
uniform vec4 lightPosition;

void main() 
{ 
	vNormal = Normal;
	
	vTexCoord = TexCoord; 

	//do lighting calculation
	vec4 lightVector = normalize(lightPosition - Position);
	vColour = Colour * dot(lightVector, Normal);
	 
	gl_Position = ProjectionView * Position;
}