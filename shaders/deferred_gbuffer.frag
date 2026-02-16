#version 330 core

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

struct Material
{
    bool useTextureAlbedo;
    bool useTextureOpacity;
    bool useTextureMetallicRoughness;
    bool useTextureNormal;
    bool useTextureAmbientOcclusion;
    bool useTextureEmissive;

    vec3 albedo;
    float opacity;
    float alphaCutout;
    float metallic;
    float roughness;
    float ambientOcclusion;
    vec3 emissive;
    float emissiveStrength;

    bool subsurfaceEnabled;
    float subsurfaceRadius;
    vec3 subsurfaceColor;

    sampler2D textureAlbedo;
    sampler2D textureOpacity;
    sampler2D textureMetallicRoughness;
    sampler2D textureNormal;
    sampler2D textureAmbientOcclusion;
    sampler2D textureEmissive;
};

uniform Material material;

vec3 calculateNormal(vec3 tangentNormal)
{
    vec3 norm = normalize(tangentNormal * 2.0 - 1.0);
    mat3 TBN = mat3(tangent, bitangent, normal);
    return normalize(TBN * norm); // tangent --> world
}

void main()
{
    // Albedo
    vec3 albedo = material.albedo;
    vec4 albedo_sample = vec4(albedo, 1.0);
    if (material.useTextureAlbedo)
    {
        albedo_sample = texture(material.textureAlbedo, textureCoordinates);
        albedo = albedo_sample.rgb;
    }

    // Alpha test
    float alpha = material.opacity;
    if (material.useTextureOpacity)
        alpha *= texture(material.textureOpacity, textureCoordinates).r;
    else if (material.useTextureAlbedo)
        alpha *= albedo_sample.a;

    if (alpha < material.alphaCutout)
        discard;

    // Metallic / roughness
    float metallic = material.metallic;
    float roughness = material.roughness;
    if (material.useTextureMetallicRoughness)
    {
        vec3 mr = texture(material.textureMetallicRoughness, textureCoordinates).rgb;
        metallic = mr.b;
        roughness = mr.g;
    }

    // Normal
    vec3 n = normal;
    if (material.useTextureNormal)
        n = calculateNormal(texture(material.textureNormal, textureCoordinates).rgb);

    // AO
    float ao = material.ambientOcclusion;
    if (material.useTextureAmbientOcclusion)
        ao = texture(material.textureAmbientOcclusion, textureCoordinates).r;

    // Emissive
    vec3 emissive = material.emissive;
    if (material.useTextureEmissive)
        emissive = texture(material.textureEmissive, textureCoordinates).rgb;
    emissive *= material.emissiveStrength;

    // Write G-Buffer
    gAlbedoAO = vec4(albedo, ao);
    gNormalMetallic = vec4(n, metallic);
    gEmissiveRoughness = vec4(emissive, roughness);
}
