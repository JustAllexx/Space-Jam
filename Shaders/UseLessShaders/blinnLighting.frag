# version 330 core
// Do not modify the version directive to anything older than 330.

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

// Output
out vec4 fragColor;

void main () 
{        
	vec4 color = texture(textureSampler, UV);
	vec3 lightColor = vec3(1.f, 1.f, 1.f);
	//fragColor = vec4(Color, 1.0f);
	//fragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	vec3 lightDir   = normalize(lightPos - fragPos);
	vec3 viewDir    = normalize(viewPos - fragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	vec3 norm = normalize(normal);
	//float shininess = 0.5f;

	//float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
	//fragColor = color;

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;
	vec3 shadedColor = (ambient + diffuse) * color.rgb;
	fragColor = vec4(shadedColor, opacity);
}
