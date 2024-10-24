#version 330 core
in vec2 TexCoords;
layout (location = 0) out vec4 color;
layout (location = 1) out vec4 BrightColor;

uniform sampler2D text;
uniform vec3 textColor;
uniform int isText;

void main ()
{   
    if (isText == 1) {
        vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
        color = vec4(textColor, 1.0) * sampled;
    }
    else {
        color = texture(text, TexCoords);
    }
    BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}  