#version 330 core

#include "include/material_input.glsl"

// G-Buffer MRT layout:
//   RT0 (RGBA16F): albedo.rgb, materialAO
//   RT1 (RGBA16F): worldNormal.xyz, metallic
//   RT2 (RGBA16F): emissive.rgb, roughness
layout(location = 0) out vec4 gAlbedoAO;
layout(location = 1) out vec4 gNormalMetallic;
layout(location = 2) out vec4 gEmissiveRoughness;

in vec2 textureCoordinates;
in vec3 worldPosition;
in vec3 tangent;
in vec3 bitangent;
in vec3 normal;

void main()
{
    SurfaceData s = sampleMaterial(textureCoordinates, tangent, bitangent, normal);

    if (s.alpha < material.alphaCutout)
        discard;

    gAlbedoAO          = vec4(s.albedo, s.ao);
    gNormalMetallic    = vec4(s.normal, s.metallic);
    gEmissiveRoughness = vec4(s.emissive, s.roughness);
}
