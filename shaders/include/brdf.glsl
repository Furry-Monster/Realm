// Fresnel-Schlick approximation
// F_schlick = f0 + (1 - f0)(1 - cos(theta))^5
vec3 fresnelSchlick(float cosTheta, vec3 f0) { return f0 + (1.0 - f0) * pow(max(1.0 - cosTheta, 0.0), 5.0); }

// Fresnel-Schlick with roughness attenuation
vec3 fresnelSchlickRoughness(float cosTheta, vec3 f0, float roughness)
{
    return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Normal Distribution Function (Trowbridge-Reitz GGX)
//       a^2 / (PI * ((n.h)^2 * (a^2 - 1) + 1)^2)
// a = roughness^2 (Disney/Epic convention)
float ndfTrowbridgeReitzGGX(vec3 n, vec3 h, float roughness)
{
    float alpha = roughness * roughness;
    float a2    = alpha * alpha;
    float nDotH = max(dot(n, h), 0.0);
    float denom = nDotH * nDotH * (a2 - 1.0) + 1.0;
    denom       = PI * denom * denom;
    return a2 / max(denom, 0.0001);
}

// Schlick-GGX geometry term
float geometrySchlickGGX(vec3 n, vec3 v, float k)
{
    float nDotV = max(dot(n, v), 0.0);
    return nDotV / (nDotV * (1.0 - k) + k);
}

// Smith's method: G = G_schlick(n,v) * G_schlick(n,l)
float geometrySmith(vec3 n, vec3 v, vec3 l, float roughness)
{
    // k remapping for direct lighting
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    return geometrySchlickGGX(n, v, k) * geometrySchlickGGX(n, l, k);
}

// BSSRDF-inspired wrapped diffuse: (NdotL + w) / (1 + w)
float subsurfaceWrap(float nDotL, float wrap) { return (nDotL + wrap) / (1.0 + wrap); }

// Cook-Torrance specular BRDF + Lambertian diffuse
// f_r = kd * (c / PI) + ks * (DFG / (4 * (v.n) * (l.n)))
vec3 cookTorranceBRDF(vec3  l,
                      vec3  radiance,
                      vec3  n,
                      vec3  v,
                      vec3  albedo,
                      float metallic,
                      float roughness,
                      vec3  f0,
                      bool  sssEnabled,
                      float sssRadius,
                      vec3  sssColor)
{
    vec3 h = normalize(v + l);

    float D = ndfTrowbridgeReitzGGX(n, h, roughness);
    vec3  F = fresnelSchlick(max(dot(h, v), 0.0), f0);
    float G = geometrySmith(n, v, l, roughness);

    vec3 specular = (D * F * G) / max(4.0 * max(dot(v, n), 0.0) * max(dot(l, n), 0.0), 0.001);

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

    vec3 diffuseAlbedo = sssEnabled ? albedo * sssColor : albedo;
    vec3 diffuse       = kD * diffuseAlbedo / PI;

    float nDotL  = dot(n, l);
    float weight = sssEnabled ? max(subsurfaceWrap(nDotL, sssRadius * 0.5), 0.0) : max(nDotL, 0.0);

    return (diffuse + specular) * radiance * weight;
}
