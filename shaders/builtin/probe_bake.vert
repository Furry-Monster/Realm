#version 450 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTextureCoordinates;

out vec3 worldCoordinates;
out vec3 worldNormal;
out vec2 textureCoordinates;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 worldPos      = model * vec4(aPos, 1.0);
    worldCoordinates   = worldPos.xyz;
    worldNormal        = mat3(transpose(inverse(model))) * aNormal;
    textureCoordinates = aTextureCoordinates;
    gl_Position        = projection * view * worldPos;
}
