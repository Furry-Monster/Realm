#version 450 core

layout(location = 0) out vec4 FragColor;

in vec3 textureCoordinates;

uniform samplerCube skybox;

void main() { FragColor = texture(skybox, textureCoordinates); }
