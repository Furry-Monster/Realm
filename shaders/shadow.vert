#version 330 core

layout(location = 0) in vec3 aPosition;

uniform mat4 uLightSpaceMatrix;

void main() { gl_Position = uLightSpaceMatrix * vec4(aPosition, 1.0); }
