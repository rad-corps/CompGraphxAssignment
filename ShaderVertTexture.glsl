#version 410

layout(location=0) in vec4 Position; 
layout(location=1) in vec4 Colour; 
layout(location=2) in vec2 TexCoord; 

out vec2 vTexCoord; 
out vec4 vColour; 

uniform mat4 ProjectionView; 

void main() 
{ 
	vColour = Colour; 
	vTexCoord = TexCoord; 
	gl_Position= ProjectionView * Position;
}