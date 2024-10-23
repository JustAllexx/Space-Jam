#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D bloomBlur;

void main()
{ 
	const float gamma = 1;
	vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
	vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;

	hdrColor += bloomColor;
	//FragColor = vec4(hdrColor, 1.0);

	vec3 result = vec3(1.0) - exp(-hdrColor * 1.f);
	result = pow(result, vec3(1.0 / gamma));
	FragColor = vec4(result, 1.0);
}