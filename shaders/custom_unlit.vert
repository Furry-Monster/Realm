#version 450 core

// Engine vertex attributes (same layout as all RealmEngine meshes)
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTextureCoordinates;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

out vec2 texCoords;
out vec3 worldPos;
out vec3 normal;

// Engine-provided uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

void main()
{
    texCoords   = aTextureCoordinates;
    worldPos    = (model * vec4(aPos, 1.0)).xyz;
    normal      = normalize(normalMatrix * aNormal);
    gl_Position = projection * view * vec4(worldPos, 1.0);
}
