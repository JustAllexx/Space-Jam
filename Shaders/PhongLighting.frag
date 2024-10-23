# version 330 core

in vec2 UV;
in vec3 fragPos;
in vec3 normal;

uniform sampler2D textureSampler;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform bool bBloom;
uniform float brightness;
uniform float opacity;
uniform float ambient;

// Output
layout (location = 0) out vec4 fragColor;
//colour attachment 1 (which will be blurred)
layout (location = 1) out vec4 brightColor;

void main ()
{
	float materialAmbient = 0.3f;
	float lightAmbient = 0.3f;
	float materialDiffuse = 1.f;
	float lightDiffuse = 1.f;
	float materialSpecular = 0.1f;
	float lightSpecular = 0.1f;
	float materialShininess = 10.f;

    float dist = length(fragPos - lightPos);
    float power = 100 / (dist * dist);

    vec4 textureColour = texture(textureSampler, UV);

    vec3 lightRay = normalize(lightPos - fragPos);
    //vec3 lightRay = normalize(fragPos - lightPos);
    vec3 viewRay = normalize(viewPos - fragPos);
    vec3 reflectedRay = reflect(-lightRay, normal);

    float diffuse = max(dot(lightRay, normal), 0.0);
    float specular = pow(max(dot(reflectedRay, viewRay), 0.0), materialShininess);

    //vec3 ambient = materialAmbient * lightAmbient * textureColour.rgb;
    vec3 diffuseColor = materialDiffuse * lightDiffuse * diffuse * textureColour.rgb;
    vec3 specularColor = materialSpecular * lightSpecular * specular * vec3(1.0, 1.0, 1.0);
    //specularColor = vec3(0.f, 0.f, 0.f);
    vec3 finalColor;
    if (ambient > 0.9f) {
        finalColor = textureColour.rgb;
    }
    else {
        finalColor = (ambient * textureColour.rgb) + (diffuseColor * power);
    }
    fragColor = vec4(finalColor, opacity);

    if (bBloom) {
		brightColor = vec4(finalColor * brightness, opacity);
	}
	else {
		brightColor = vec4(0.0, 0.0, 0.0, 1.0);
	}

}