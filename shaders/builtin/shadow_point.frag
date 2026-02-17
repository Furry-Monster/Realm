#version 450 core

in vec4 gFragPos;

uniform vec3  lightPos;
uniform float farPlane;

void main()
{
    float dist = length(gFragPos.xyz - lightPos);
    // Normalize to [0,1] range
    gl_FragDepth = dist / farPlane;
}
