# version 330 core
// Do not modify the above version directive to anything older than 330, as
// modern OpenGL will not work properly due to it not being core at the time.

// Shader inputs
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 vertexUV;
layout (location = 2) in vec3 normalVert;

// Shader outputs, if any
out vec2 UV;
out vec3 fragPos;
out vec3 normal;

// Uniform variables
uniform mat4 modelview;
uniform mat4 projection;

void main() {
    gl_Position = projection * modelview * vec4(position, 1.0f);
    //gl_Position = vec4(position, 1.0f);
    //Color = color; // Just forward this color to the fragment shader
    UV = vertexUV;
    fragPos = position;
    normal = mat3(transpose(inverse(modelview))) * normalVert;
}
