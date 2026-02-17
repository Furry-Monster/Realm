#version 450 core

layout(location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 shadowMatrices[6];

out vec4 fragPos;

void main()
{
    fragPos     = model * vec4(aPos, 1.0);
    gl_Position = fragPos;
}
