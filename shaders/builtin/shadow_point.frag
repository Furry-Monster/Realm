#version 450 core

layout(location = 0) out float fragDepth;

in vec4 gFragPos;

uniform vec3  lightPos;
uniform float farPlane;

void main()
{
    float dist = length(gFragPos.xyz - lightPos);
    float d    = dist / farPlane;
    fragDepth  = d;
    gl_FragDepth = d;
}
