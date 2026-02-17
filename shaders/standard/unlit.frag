#version 330 core

#include "../include/material_input.glsl"

layout(location = 0) out vec4 FragColor;

in vec2 textureCoordinates;

void main()
{
    vec3 albedo = material.albedo;
    if (material.useTextureAlbedo)
        albedo = texture(material.textureAlbedo, textureCoordinates).rgb;

    float alpha = material.opacity;
    if (material.useTextureOpacity)
        alpha *= texture(material.textureOpacity, textureCoordinates).r;

    if (alpha < material.alphaCutout)
        discard;

    vec3 emissive = material.emissive * material.emissiveStrength;
    if (material.useTextureEmissive)
        emissive = texture(material.textureEmissive, textureCoordinates).rgb * material.emissiveStrength;

    FragColor = vec4(albedo + emissive, alpha);
}
