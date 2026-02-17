#version 450 core

#include "../include/common.glsl"
#include "../include/material_input.glsl"
#include "../include/lighting.glsl"
#include "../include/shadow.glsl"

layout(location = 0) out vec4 FragColor;

in vec2 textureCoordinates;
in vec3 worldCoordinates;
in vec3 tangent;
in vec3 bitangent;
in vec3 normal;

uniform vec3 cameraPosition;
uniform mat4 view;
uniform samplerCube diffuseIrradianceMap;
uniform int displayMode;

// Hair-specific uniforms (set via MaterialPropertyBlock)
uniform float layerIndex;
uniform float layerStep;

// Kajiya-Kay diffuse: Kd * sqrt(1 - (T.L)^2)
float kajiyaKayDiffuse(vec3 T, vec3 L)
{
    float TdotL = dot(T, L);
    return max(0.0, sqrt(max(0.0, 1.0 - TdotL * TdotL)));
}

// Kajiya-Kay specular: Ks * pow(max(0, T.H), p)
float kajiyaKaySpecular(vec3 T, vec3 H, float power) {
    return pow(max(0.0, dot(T, H)), power);
}

void main()
{
    SurfaceData s = sampleMaterial(textureCoordinates, tangent, bitangent, normal);

    if (s.alpha < 0.01)
        discard;

    if (displayMode == 1) { FragColor = vec4(s.albedo, s.alpha); return; }
    if (displayMode == 6) { FragColor = vec4(s.emissive, s.alpha); return; }

    vec3 v = normalize(cameraPosition - worldCoordinates);
    vec3 T = tangent;
    if (length(T) < 0.01)
        T = normal;
    T = normalize(T);

    // Hair-specific material properties
    float specStrength = material.roughness;   // reused: lower roughness = more specular
    float specPower    = 64.0;

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < lightCount; i++)
    {
        vec3 l, radiance;
        if (!evaluateLight(i, worldCoordinates, l, radiance))
            continue;

        // Directional light shadow
        if (int(lights[i].position.w) == 1 && shadowEnabled)
            radiance *= calculateShadow(worldCoordinates, normal, l, view);

        vec3 H = normalize(l + v);
        float diff = kajiyaKayDiffuse(T, l);
        float spec = kajiyaKaySpecular(T, H, specPower);

        Lo += radiance * (s.albedo * diff + specStrength * spec * vec3(1.0));
    }

    vec3 irradiance = texture(diffuseIrradianceMap, normal).rgb;
    vec3 ambient = irradiance * s.albedo;
    vec3 color = s.emissive + ambient + Lo;

    FragColor = vec4(color, s.alpha);
}
