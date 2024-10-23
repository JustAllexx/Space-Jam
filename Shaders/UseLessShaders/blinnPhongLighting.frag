# version 330 core
// Do not modify the version directive to anything older than 330.

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 BrightColor;

// Fragment shader inputs are outputs of the same name from vertex shader
in vec2 UV;
in vec3 fragPos;
in vec3 normal;

// Uniform variables
uniform sampler2D textureSampler;
uniform float opacity;
uniform float ambient;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float brightness;

// Output

void main () 
{        
	vec3 color = texture(textureSampler, UV).rgb;
	vec3 nNormal = normalize(normal);
	vec3 ambient = 0.0 * color;

	//Lighting Calculations
	vec3 lighting = vec3(0.0);
	vec3 viewDir = normalize(viewPos - fragPos);

	//diffuse
	vec3 lightDir = normalize(lightPos - fragPos);
	float diff = max(dot(lightDir, normal), 0.0);
	//Replace vec3 with light color parameter
	vec3 lightResult = vec3(20.0, 20.0, 20.0) * diff * color;

	float dist = length(fragPos - lightPos);
	lightResult *= 1.0 / (dist * dist);
	lighting += lightResult;

	vec3 result = ambient + lighting;

	fragColor = vec4(result, 1.0);

	float luminence = dot(result, vec3(0.2126, 0.7152, 0.0722));
	
	if (luminence > 1) {
		BrightColor = vec4(result * brightness, 1.0);
	}
	else {
		BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
	}	
	//fragColor = vec4(1.0, 2.0, 3.0, 1.0);
}
