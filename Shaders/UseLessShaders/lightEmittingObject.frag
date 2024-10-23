#version 330 core
//This is colour attachment 0
layout (location = 0) out vec4 FragColor;
//This is colour attachment 1, this is what's going to be blurred
layout (location = 1) out vec4 BrightColor;

in vec2 UV;
in vec3 fragPos;
in vec3 normal;

//out vec4 fragColor;

//uniform vec3 objectColour;

void main() 
{
	vec3 objectColour = vec3(1.0, 2.0, 3.0);

	FragColor = vec4(objectColour, 1.0);
	//Redundent test that our objectColour is brighter than 1, somewhat irrelebant might remove
	float luminence = dot(objectColour, vec3(0.2126, 0.7152, 0.0722));
	
	if (luminence > 1) {
		BrightColor = vec4(objectColour, 1.0);
	}
	else {
		BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
	}	
}