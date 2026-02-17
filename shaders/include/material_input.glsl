struct Material
{
    bool useTextureAlbedo;
    bool useTextureOpacity;
    bool useTextureMetallicRoughness;
    bool useTextureNormal;
    bool useTextureAmbientOcclusion;
    bool useTextureEmissive;

    vec3  albedo;
    float opacity;
    float alphaCutout;
    float metallic;
    float roughness;
    float ambientOcclusion;
    vec3  emissive;
    float emissiveStrength;

    bool  subsurfaceEnabled;
    float subsurfaceRadius;
    vec3  subsurfaceColor;

    sampler2D textureAlbedo;
    sampler2D textureOpacity;
    sampler2D textureMetallicRoughness;
    sampler2D textureNormal;
    sampler2D textureAmbientOcclusion;
    sampler2D textureEmissive;
};

uniform Material material;

// Tangent space to world
vec3 calculateNormal(vec3 tangentNormal, vec3 T, vec3 B, vec3 N)
{
    vec3 norm = normalize(tangentNormal * 2.0 - 1.0);
    return normalize(mat3(T, B, N) * norm);
}

// Sample all standard PBR material properties
struct SurfaceData
{
    vec3  albedo;
    float alpha;
    float metallic;
    float roughness;
    vec3  normal;
    float ao;
    vec3  emissive;
    bool  sssEnabled;
    float sssRadius;
    vec3  sssColor;
};

SurfaceData sampleMaterial(vec2 uv, vec3 T, vec3 B, vec3 N)
{
    SurfaceData s;

    // Albedo
    vec4 albedoSample = vec4(material.albedo, 1.0);
    if (material.useTextureAlbedo)
        albedoSample = texture(material.textureAlbedo, uv);
    s.albedo = albedoSample.rgb;

    // Alpha
    s.alpha = material.opacity;
    if (material.useTextureOpacity)
        s.alpha *= texture(material.textureOpacity, uv).r;
    else if (material.useTextureAlbedo)
        s.alpha *= albedoSample.a;

    // Metallic / Roughness
    s.metallic  = material.metallic;
    s.roughness = material.roughness;
    if (material.useTextureMetallicRoughness)
    {
        vec3 mr     = texture(material.textureMetallicRoughness, uv).rgb;
        s.metallic  = mr.b;
        s.roughness = mr.g;
    }

    // Normal
    s.normal = N;
    if (material.useTextureNormal)
        s.normal = calculateNormal(texture(material.textureNormal, uv).rgb, T, B, N);

    // AO
    s.ao = material.ambientOcclusion;
    if (material.useTextureAmbientOcclusion)
        s.ao = texture(material.textureAmbientOcclusion, uv).r;

    // Emissive
    s.emissive = material.emissive;
    if (material.useTextureEmissive)
        s.emissive = texture(material.textureEmissive, uv).rgb;
    s.emissive *= material.emissiveStrength;

    // SSS
    s.sssEnabled = material.subsurfaceEnabled;
    s.sssRadius  = material.subsurfaceRadius;
    s.sssColor   = material.subsurfaceColor;

    return s;
}
