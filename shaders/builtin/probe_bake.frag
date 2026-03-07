#version 450 core

#include "../include/lighting.glsl"

layout(location = 0) out vec4 FragColor;

in vec3 worldCoordinates;
in vec3 worldNormal;
in vec2 textureCoordinates;

uniform vec3 cameraPosition;

void main()
{
    vec3 n  = normalize(worldNormal);
    vec3 Lo = vec3(0.0);

    for (int i = 0; i < lightCount; ++i)
    {
        vec3 l, radiance;
        if (!evaluateLight(i, worldCoordinates, n, l, radiance))
            continue;
        float NdotL = max(dot(n, l), 0.0);
        Lo += radiance * NdotL;
    }

    FragColor = vec4(Lo, 1.0);
}
