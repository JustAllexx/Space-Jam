#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 vertexUV;
layout (location = 2) in vec3 normalVert;

out vec2 UV;
out vec3 fragPos;
out vec3 normal;

uniform mat4 modelview;
uniform mat4 projection;

void main() 
{
	gl_Position = projection * modelview * vec4(position, 1.0f);	
	//Learn OpenGL does this a weird way (look into it if this doesn't work)
	fragPos = position;
	UV = vertexUV;
	normal = mat3(transpose(inverse(modelview))) * normalVert;
}