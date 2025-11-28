#version 330 core

#define PI 3.1415926535897932384626433832795
#define GREYSCALE_WEIGHT_VECTOR vec3(0.2126, 0.7152, 0.0722)

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BloomColor; // for bloom shader

// vertex attributes
in vec3 worldCoordinates;
in vec2 textureCoordinates;
in vec3 tangent;
in vec3 bitangent;
in vec3 normal;

struct Material
{
    bool useTextureAlbedo;
    bool useTextureMetallicRoughness;
    bool useTextureNormal;
    bool useTextureAmbientOcclusion;
    bool useTextureEmissive;

    vec3  albedo;
    float metallic;
    float roughness;
    float ambientOcclusion;
    vec3  emissive;

    sampler2D textureAlbedo;
    sampler2D textureMetallicRoughness;
    sampler2D textureNormal;
    sampler2D textureAmbientOcclusion;
    sampler2D textureEmissive;
};

uniform Material material;

uniform vec3 cameraPosition;

struct LightData
{
    vec4 position;    // xyz = position, w = type
    vec4 direction;   // xyz = direction, w = intensity
    vec4 color;       // rgb = color, w = constant
    vec4 attenuation; // x = linear, y = quadratic, z = range, w = inner_cone_angle
    vec4 spot_area;   // x = outer_cone_angle, y = width, z = height, w = padding
};

layout(std140) uniform LightBlock
{
    int       lightCount;
    LightData lights[16];
};

// PBR
// IBL precomputed maps
const float PREFILTERED_ENV_MAP_LOD = 4.0; // how many mipmap levels

uniform samplerCube diffuseIrradianceMap;
uniform samplerCube prefilteredEnvMap;
uniform sampler2D   brdfConvolutionMap;

// Post parameters
uniform float bloomBrightnessCutoff;

// Fresnel function (Fresnel-Schlick approximation)
//
// F_schlick = f0 + (1 - f0)(1 - (h * v))^5
vec3 fresnelSchlick(float cosTheta, vec3 f0) { return f0 + (1.0 - f0) * pow(max(1 - cosTheta, 0.0), 5.0); }

// Fresnel schlick roughness
//
// Same as above except with a roughness term
// F_schlick_roughness = f0 + ((1 - roughness) - f0)(1 - (h * v))^5
vec3 fresnelSchlickRoughness(float cosTheta, vec3 f0, float roughness)
{
    return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Normal distribution function (Trowbridge-Reitz GGX)
//
//                a ^ 2
//     ---------------------------------
//      PI((n * h)^2(a^2 - 1) + 1)^2
//
// Note: a for alpha, alpha = roughness^2
//       h for halfway
float ndfTrowbridgeReitzGGX(vec3 n, vec3 h, float roughness)
{
    float alpha        = roughness * roughness; // recommended by disney/epic papers
    float alphaSquared = alpha * alpha;

    float nDotH        = max(dot(n, h), 0.0);
    float nDotHSquared = nDotH * nDotH;
    float innerTerms   = nDotHSquared * (alphaSquared - 1.0) + 1.0;

    float numerator   = alphaSquared;
    float denomenator = PI * innerTerms * innerTerms;
    denomenator       = max(denomenator, 0.0001); // avoid zero-div err

    return numerator / denomenator;
}

// Geometry function
//
//         n * v
//   -------------------
//   (n * v)(1 - k) + k
//
// Note: k for roughness-related param
float geometrySchlickGGX(vec3 n, vec3 v, float k)
{
    float nDotV = max(dot(n, v), 0.0);

    float numerator   = nDotV;
    float denomenator = nDotV * (1.0 - k) + k;

    return numerator / denomenator;
}

// Geometry function - smiths method
//
// G_smith = G(n,v) * G(n,l)
float geometrySmith(vec3 n, vec3 v, vec3 l, float roughness)
{
    // remapping for direct lighting (doesn't work for IBL)
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;

    return geometrySchlickGGX(n, v, k) * geometrySchlickGGX(n, l, k);
}

// Tangent space to world
vec3 calculateNormal(vec3 tangentNormal)
{
    vec3 norm = normalize(tangentNormal * 2.0 - 1.0);
    mat3 TBN  = mat3(tangent, bitangent, normal);
    return normalize(TBN * norm); // tangent --> world
}

// Cook-Torrance specular BRDF term
//
//                DFG
//        --------------------
//         4(w_0 * n)(w_i * n)
//
// Cook-Torrance with both diffuse + specular term
//
// f_r = kd * f_lambert + ks * f_cook-torrance
//
// where f_lambert = c / pi
vec3 discreteMonteCarloContribution(vec3  l,
                                    vec3  radiance,
                                    vec3  n,
                                    vec3  v,
                                    vec3  albedo,
                                    float metallic,
                                    float roughness,
                                    vec3  f0)
{
    vec3 h = normalize(v + l);

    float D = ndfTrowbridgeReitzGGX(n, h, roughness);
    vec3  F = fresnelSchlick(max(dot(h, v), 0.0), f0);
    float G = geometrySmith(n, v, l, roughness);

    vec3  numerator   = D * F * G;
    float denominator = 4.0 * max(dot(v, n), 0.0) * max(dot(l, n), 0.0);

    vec3 specular = numerator / max(denominator, 0.001);

    vec3 kSpecular = F;
    vec3 kDiffuse  = vec3(1.0) - kSpecular;
    kDiffuse *= 1.0 - metallic; // metallic materials should have less diffuse component

    vec3  diffuse          = kDiffuse * albedo / PI;
    vec3  cookTorranceBrdf = diffuse + specular;
    float nDotL            = max(dot(n, l), 0.0);

    return cookTorranceBrdf * radiance * nDotL;
}

void main()
{
    // Preprocess:
    // albedo
    vec3 albedo = material.albedo;
    if (material.useTextureAlbedo)
    {
        albedo = texture(material.textureAlbedo, textureCoordinates).rgb;
    }

    // metallic/roughness
    float metallic  = material.metallic;
    float roughness = material.roughness;
    if (material.useTextureMetallicRoughness)
    {
        vec3 metallicRoughness = texture(material.textureMetallicRoughness, textureCoordinates).rgb;
        metallic               = metallicRoughness.b;
        roughness              = metallicRoughness.g;
    }

    // normal
    vec3 n = normal; // interpolated vertex normal
    if (material.useTextureNormal)
    {
        n = calculateNormal(texture(material.textureNormal, textureCoordinates).rgb);
    }

    // ambient occlusion
    float ao = material.ambientOcclusion;
    if (material.useTextureAmbientOcclusion)
    {
        ao = texture(material.textureAmbientOcclusion, textureCoordinates).r;
    }

    // emissive
    vec3 emissive = material.emissive;
    if (material.useTextureEmissive)
    {
        emissive = texture(material.textureEmissive, textureCoordinates).rgb;
    }

    vec3 v = normalize(cameraPosition - worldCoordinates);
    vec3 r = reflect(-v, n);

    // for PBR-metallic we assume dialectrics all have 0.04
    // for metals the value comes from the albedo map
    vec3 f0 = vec3(0.04);
    f0      = mix(f0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    // Direct lighting :
    for (int i = 0; i < lightCount; i++)
    {
        int  lightType = int(lights[i].position.w);
        vec3 radiance  = vec3(0.0);
        vec3 l         = vec3(0.0);

        vec3  lightPosition       = lights[i].position.xyz;
        vec3  lightDirection      = lights[i].direction.xyz;
        float lightIntensity      = lights[i].direction.w;
        vec3  lightColor          = lights[i].color.rgb;
        float lightConstant       = lights[i].color.w;
        float lightLinear         = lights[i].attenuation.x;
        float lightQuadratic      = lights[i].attenuation.y;
        float lightRange          = lights[i].attenuation.z;
        float lightInnerConeAngle = lights[i].attenuation.w;
        float lightOuterConeAngle = lights[i].spot_area.x;
        float lightWidth          = lights[i].spot_area.y;
        float lightHeight         = lights[i].spot_area.z;

        // Point Light (0)
        if (lightType == 0)
        {
            vec3  lightDir = lightPosition - worldCoordinates;
            float distance = length(lightDir);

            if (distance > lightRange)
                continue;

            l = normalize(lightDir);

            float attenuation = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * distance * distance);

            radiance = lightColor * lightIntensity * attenuation;
        }
        // Directional Light (1)
        else if (lightType == 1)
        {
            l        = normalize(-lightDirection);
            radiance = lightColor * lightIntensity;
        }
        // Spot Light (2)
        else if (lightType == 2)
        {
            vec3  lightDir = lightPosition - worldCoordinates;
            float distance = length(lightDir);

            if (distance > lightRange)
                continue;

            l            = normalize(lightDir);
            vec3 spotDir = normalize(lightDirection);

            float theta    = dot(l, -spotDir);
            float innerCos = cos(radians(lightInnerConeAngle));
            float outerCos = cos(radians(lightOuterConeAngle));

            float epsilon    = innerCos - outerCos;
            float spotFactor = clamp((theta - outerCos) / epsilon, 0.0, 1.0);

            if (spotFactor <= 0.0)
                continue;

            float attenuation = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * distance * distance);

            radiance = lightColor * lightIntensity * attenuation * spotFactor;
        }
        // Area Light (3)
        else if (lightType == 3)
        {
            vec3  lightDir = lightPosition - worldCoordinates;
            float distance = length(lightDir);
            l              = normalize(lightDir);

            float attenuation  = 1.0 / (distance * distance);
            vec3  areaDir      = normalize(lightDirection);
            float facingFactor = max(dot(-areaDir, n), 0.0);

            radiance = lightColor * lightIntensity * attenuation * facingFactor;
        }

        if (length(radiance) > 0.0)
        {
            Lo += discreteMonteCarloContribution(l, radiance, n, v, albedo, metallic, roughness, f0);
        }
    }

    // Indirect lighting (only use IBL):
    // IBL = prefilteredEnvMap * (LUT.r * F + LUT.g)
    vec3 kSpecular = fresnelSchlickRoughness(max(dot(n, v), 0.0), f0, roughness);
    vec3 kDiffuse  = 1.0 - kSpecular;
    kDiffuse *= 1.0 - metallic; // metallic materials should have less diffuse component

    vec3 irradiance = texture(diffuseIrradianceMap, n).rgb;
    vec3 diffuse    = irradiance * albedo;

    vec3  prefilteredEnvMapColor = textureLod(prefilteredEnvMap, r, roughness * PREFILTERED_ENV_MAP_LOD).rgb;
    float NdotV                  = max(dot(n, v), 0.0);
    vec2  brdf                   = texture(brdfConvolutionMap, vec2(NdotV, roughness)).rg;
    vec3  specular               = prefilteredEnvMapColor * (kSpecular * brdf.x + brdf.y);

    vec3 ambient = (kDiffuse * diffuse + specular) * ao;

    // Outputs:
    // color = emissive + indirect + direct
    vec3 color = emissive + ambient + Lo;
    FragColor  = vec4(color, 1.0);
    // use greyscale conversion here because not all colors are equally "bright"
    float greyscaleBrightness = dot(FragColor.rgb, GREYSCALE_WEIGHT_VECTOR);
    BloomColor = greyscaleBrightness > bloomBrightnessCutoff ? vec4(emissive, 1.0) : vec4(0.0, 0.0, 0.0, 1.0);
}
