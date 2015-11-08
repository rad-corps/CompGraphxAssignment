#version 410

in vec2 vTexCoord; 
in vec4 vColour; 

out vec4 FragColor; 

uniform sampler2D mysampler; 

void main() 
{ 
	//FragColor = vColour; 
	//FragColor = texture(mysampler,vTexCoord); 

	FragColor = texture(mysampler,vTexCoord) * vColour; 
	
}