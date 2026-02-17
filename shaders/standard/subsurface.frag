#version 330 core

// Subsurface scattering shader -- wraps standard PBR with SSS always enabled.
// Uses the same Cook-Torrance BRDF but with wrap diffuse lighting.

#include "../include/common.glsl"
#include "../include/material_input.glsl"
#include "../include/brdf.glsl"
#include "../include/lighting.glsl"
#include "../include/shadow.glsl"

layout(location = 0) out vec4 FragColor;

in vec3 worldCoordinates;
in vec2 textureCoordinates;
in vec3 tangent;
in vec3 bitangent;
in vec3 normal;
in vec4 fragPosLightSpace;

uniform vec3 cameraPosition;

uniform samplerCube diffuseIrradianceMap;
uniform samplerCube prefilteredEnvMap;
uniform sampler2D brdfConvolutionMap;

uniform int displayMode;
uniform bool isTransparentPass;

void main()
{
    SurfaceData s = sampleMaterial(textureCoordinates, tangent, bitangent, normal);

    if (isTransparentPass)
    {
        if (s.alpha < 0.01) discard;
    }
    else if (s.alpha < material.alphaCutout)
        discard;

    // Force SSS on
    bool  sssOn     = true;
    float sssRadius = s.sssRadius > 0.0 ? s.sssRadius : 1.0;
    vec3  sssColor  = s.sssColor;

    vec3 v  = normalize(cameraPosition - worldCoordinates);
    vec3 r  = reflect(-v, s.normal);
    vec3 f0 = mix(vec3(0.04), s.albedo, s.metallic);

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < lightCount; i++)
    {
        vec3 l, radiance;
        if (!evaluateLight(i, worldCoordinates, l, radiance))
            continue;

        if (int(lights[i].position.w) == 1 && shadowEnabled)
            radiance *= calculateShadow(fragPosLightSpace, s.normal, l);

        Lo += cookTorranceBRDF(l, radiance, s.normal, v,
                               s.albedo, s.metallic, s.roughness, f0,
                               sssOn, sssRadius, sssColor);
    }

    vec3 kS = fresnelSchlickRoughness(max(dot(s.normal, v), 0.0), f0, s.roughness);
    vec3 kD = (1.0 - kS) * (1.0 - s.metallic);

    vec3 diffAlbedo = s.albedo * sssColor;
    vec3 irradiance = texture(diffuseIrradianceMap, s.normal).rgb;
    vec3 diffuse    = irradiance * diffAlbedo;

    vec3  prefilteredColor = textureLod(prefilteredEnvMap, r, s.roughness * PREFILTERED_ENV_MAP_LOD).rgb;
    vec2  brdf             = texture(brdfConvolutionMap, vec2(max(dot(s.normal, v), 0.0), s.roughness)).rg;
    vec3  specular         = prefilteredColor * (kS * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * s.ao;
    vec3 color   = s.emissive + ambient + Lo;

    float outAlpha = isTransparentPass ? s.alpha : 1.0;

    if (displayMode == 1) { FragColor = vec4(s.albedo, 1.0); return; }
    if (displayMode == 2) { FragColor = vec4(s.normal * 0.5 + 0.5, 1.0); return; }

    FragColor = vec4(color, outAlpha);
}
