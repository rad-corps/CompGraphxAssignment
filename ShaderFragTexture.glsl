#version 410

in vec2 vTexCoord; 
out vec4 FragColor; 

uniform sampler2D diffuse; 

void main() 
{ 
	FragColor = texture(diffuse,vTexCoord);
	FragColor = vec4(1.0,0.0,0.0,1.0);
}