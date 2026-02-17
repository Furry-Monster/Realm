#version 330 core

in vec3 fragWorldPos;

uniform vec3 lightPos;
uniform float farPlane;

void main()
{
    gl_FragDepth = length(fragWorldPos - lightPos) / farPlane;
}
