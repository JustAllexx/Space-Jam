#version 330 core
layout (location = 0) in vec4 pos;

out vec2 TexCoords;

void main()
{
	gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
	TexCoords = pos.zw;
}