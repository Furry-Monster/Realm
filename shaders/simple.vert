#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;
layout(location = 5) in vec4 aColor;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec4 VertexColor;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
    vec4 worldPos = uModel * vec4(aPosition, 1.0);
    FragPos       = worldPos.xyz;
    Normal        = mat3(transpose(inverse(uModel))) * aNormal;
    TexCoord      = aTexCoord;
    VertexColor   = aColor;
    gl_Position   = uProjection * uView * worldPos;
}
