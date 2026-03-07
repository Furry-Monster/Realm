#version 450 core

#include "../include/common.glsl"
#include "../include/material_input.glsl"
#include "../include/brdf.glsl"
#include "../include/lighting.glsl"
#include "../include/shadow.glsl"
#include "../include/sh_lighting.glsl"

layout(location = 0) out vec4 FragColor;

in vec3 worldCoordinates;
in vec2 textureCoordinates;
in vec3 tangent;
in vec3 bitangent;
in vec3 normal;

uniform vec3 cameraPosition;
uniform mat4 view;

// IBL precomputed maps
uniform samplerCube diffuseIrradianceMap;
uniform samplerCube prefilteredEnvMap;
uniform sampler2D   brdfConvolutionMap;

// Viewport display mode: 0=lit, 1=albedo, 2=normals, 3=metallic, 4=roughness, 5=materialAO, 6=emissive
uniform int  displayMode;
uniform bool isTransparentPass;
uniform bool probesEnabled;

void main()
{
    SurfaceData s = sampleMaterial(textureCoordinates, tangent, bitangent, normal);

    // Alpha test
    if (isTransparentPass)
    {
        if (s.alpha < 0.01)
            discard;
    }
    else if (s.alpha < material.alphaCutout)
        discard;

    vec3 v  = normalize(cameraPosition - worldCoordinates);
    vec3 r  = reflect(-v, s.normal);
    vec3 f0 = mix(vec3(0.04), s.albedo, s.metallic);

    // Direct lighting
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < lightCount; i++)
    {
        vec3 l, radiance;
        if (!evaluateLight(i, worldCoordinates, s.normal, l, radiance))
            continue;

        // Shadow only for directional lights
        if (int(lights[i].position.w) == 1 && shadowEnabled)
            radiance *= calculateShadow(worldCoordinates, s.normal, l, view);

        Lo += cookTorranceBRDF(
            l, radiance, s.normal, v, s.albedo, s.metallic, s.roughness, f0, s.sssEnabled, s.sssRadius, s.sssColor);
    }

    // IBL (indirect lighting)
    vec3 kS = fresnelSchlickRoughness(max(dot(s.normal, v), 0.0), f0, s.roughness);
    vec3 kD = (1.0 - kS) * (1.0 - s.metallic);

    vec3 diffAlbedo = s.sssEnabled ? s.albedo * s.sssColor : s.albedo;
    vec3 irradiance = texture(diffuseIrradianceMap, s.normal).rgb;
    if (probesEnabled)
    {
        vec3  probeIrr    = evaluateProbeIrradiance(worldCoordinates, s.normal);
        float probeWeight = (probeCount > 0) ? 1.0 : 0.0;
        irradiance        = mix(irradiance, probeIrr, probeWeight);
    }
    vec3 diffuse = irradiance * diffAlbedo;

    vec3 prefilteredColor = textureLod(prefilteredEnvMap, r, s.roughness * PREFILTERED_ENV_MAP_LOD).rgb;
    vec2 brdf             = texture(brdfConvolutionMap, vec2(max(dot(s.normal, v), 0.0), s.roughness)).rg;
    vec3 specular         = prefilteredColor * (kS * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * s.ao;
    vec3 color   = s.emissive + ambient + Lo;

    float sssMask  = s.sssEnabled ? 1.0 : 0.0;
    float outAlpha = isTransparentPass ? s.alpha : sssMask;

    // Debug display modes
    if (displayMode == 1)
    {
        FragColor = vec4(s.albedo, 1.0);
        return;
    }
    if (displayMode == 2)
    {
        FragColor = vec4(s.normal * 0.5 + 0.5, 1.0);
        return;
    }
    if (displayMode == 3)
    {
        FragColor = vec4(vec3(s.metallic), 1.0);
        return;
    }
    if (displayMode == 4)
    {
        FragColor = vec4(vec3(s.roughness), 1.0);
        return;
    }
    if (displayMode == 5)
    {
        FragColor = vec4(vec3(s.ao), 1.0);
        return;
    }
    if (displayMode == 6)
    {
        FragColor = vec4(s.emissive, 1.0);
        return;
    }

    FragColor = vec4(color, outAlpha);
}
