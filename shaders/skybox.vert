#version 330 core

layout(location = 0) in vec3 aPosition;

out vec3 TexCoord;

uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
    TexCoord = aPosition;

    vec4 pos    = uProjection * mat4(mat3(uView)) * vec4(aPosition, 1.0);
    gl_Position = pos.xyww;
}
