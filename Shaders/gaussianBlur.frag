#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform bool horizontal;
//uniform float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);
uniform float weight[5] = float[] (0.0, 0.0, 0.0, 0.0, 0.0);

void main()
{
	//Gets size of a single pixel of the texture
	vec2 offset = 1.0 / textureSize(screenTexture, 0);
	vec3 resultColour = texture(screenTexture, TexCoords).rgb * weight[0];

	if(horizontal)
	{
		for(int i = 1; i < 5; ++i) 
		{
			resultColour += texture(screenTexture, TexCoords + vec2(offset.x * i, 0.0)).rgb * weight[i];
			resultColour += texture(screenTexture, TexCoords - vec2(offset.x * i, 0.0)).rgb * weight[i];
		}
	}
	else
	{
		for(int i = 1; i < 5; ++i) {
			resultColour += texture(screenTexture, TexCoords + vec2(0.0, offset.y * i)).rgb * weight[i];
			resultColour += texture(screenTexture, TexCoords - vec2(0.0, offset.y * i)).rgb * weight[i];
		}
	}
	FragColor = vec4(resultColour, 1.0);	
}