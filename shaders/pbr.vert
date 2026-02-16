#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTextureCoordinates;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

out vec2 textureCoordinates;
out vec3 worldCoordinates;
out vec3 tangent;
out vec3 bitangent;
out vec3 normal;
out vec4 fragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;
uniform mat3 normalMatrix; // transpose(inverse(mat3(model))), computed on CPU

void main()
{
    textureCoordinates = aTextureCoordinates;
    worldCoordinates   = (model * vec4(aPos, 1.0f)).xyz;
    gl_Position        = projection * view * model * vec4(aPos, 1.0f);
    fragPosLightSpace  = lightSpaceMatrix * model * vec4(aPos, 1.0f);

    tangent   = normalize(normalMatrix * aTangent);
    bitangent = normalize(normalMatrix * aBitangent);
    normal    = normalize(normalMatrix * aNormal);
}
