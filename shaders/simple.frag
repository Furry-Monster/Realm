#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 VertexColor;

out vec4 FragColor;

void main()
{
    // Simple test: output bright red color
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
