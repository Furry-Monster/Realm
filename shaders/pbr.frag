#version 330 core

#define PI 3.1415926535897932384626433832795
#define GREYSCALE_WEIGHT_VECTOR vec3(0.2126, 0.7152, 0.0722)
#define PREFILTERED_ENV_MAP_LOD 4.0

layout(location = 0) out vec4 FragColor;

in vec3 worldCoordinates;
in vec2 textureCoordinates;
in vec3 tangent;
in vec3 bitangent;
in vec3 normal;
in vec4 fragPosLightSpace;

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

struct LightData
{
    vec4 position;    // xyz = position, w = type
    vec4 direction;   // xyz = direction, w = intensity
    vec4 color;       // rgb = color, w = constant
    vec4 attenuation; // x = linear, y = quadratic, z = range, w = inner_cone_angle
    vec4 spot_area;   // x = outer_cone_angle, y = width, z = height, w = padding
};

// PBR uniforms
uniform Material material;
uniform vec3     cameraPosition;

layout(std140) uniform LightBlock
{
    int       lightCount;
    LightData lights[16];
};

// IBL precomputed maps
uniform samplerCube diffuseIrradianceMap;
uniform samplerCube prefilteredEnvMap;
uniform sampler2D   brdfConvolutionMap;

// Shadow parameters
uniform sampler2D shadowMap;
uniform bool      shadowEnabled;
uniform mat4      lightSpaceMatrix;

// Viewport display mode: 0=lit, 1=albedo, 2=normals, 3=metallic, 4=roughness, 5=materialAO, 6=emissive
uniform int  displayMode;
uniform bool isTransparentPass;

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

// BSSRDF-inspired wrapped diffuse: wrap(NdotL) = (NdotL + w) / (1 + w)
// Larger w increases subsurface spread (softer falloff at grazing)
float subsurfaceWrap(float nDotL, float wrap) { return (nDotL + wrap) / (1.0 + wrap); }

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
                                    vec3  f0,
                                    bool  subsurfaceEnabled,
                                    float subsurfaceRadius,
                                    vec3  subsurfaceColor)
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
    kDiffuse *= 1.0 - metallic;

    vec3 diffuseAlbedo = albedo;
    if (subsurfaceEnabled)
        diffuseAlbedo *= subsurfaceColor;

    vec3 diffuse          = kDiffuse * diffuseAlbedo / PI;
    vec3 cookTorranceBrdf = diffuse + specular;

    float nDotL      = dot(n, l);
    float diffWeight = subsurfaceEnabled ? subsurfaceWrap(max(nDotL, 0.0), subsurfaceRadius * 0.5) : max(nDotL, 0.0);

    return cookTorranceBrdf * radiance * diffWeight;
}

// Percentage Closer Filtering - Shadow Calculating
//
// PCF samples multiple texels and averages the results to produce soft shadows
//
//           1      N-1
// shadow = ---  *   Σ  [currentDepth - bias > pcfDepth_i ? 1 : 0]
//           N      i=0
//
// where N is the number of samples (using Poisson disk sampling)
//       bias is used to reduce shadow acne
//       pcfDepth_i is the depth value from shadow map at sample position i
//
// The final shadow factor is: 1.0 - shadow

// Poisson disk sampling offsets (16 samples )
const vec2 poissonDisk[16] = vec2[](vec2(-0.613392, 0.617481),
                                    vec2(0.170019, -0.040254),
                                    vec2(-0.299417, 0.791925),
                                    vec2(0.645680, 0.493210),
                                    vec2(-0.651784, 0.717887),
                                    vec2(0.421003, 0.027070),
                                    vec2(-0.817194, -0.271096),
                                    vec2(-0.705374, -0.668203),
                                    vec2(0.977050, -0.108615),
                                    vec2(0.063326, 0.142369),
                                    vec2(0.203528, 0.214331),
                                    vec2(-0.667531, 0.326090),
                                    vec2(-0.098422, -0.295755),
                                    vec2(-0.885922, 0.215369),
                                    vec2(0.566637, 0.605213),
                                    vec2(0.039766, -0.396100));

vec2 rotate2D(vec2 v, float angle)
{
    float s = sin(angle);
    float c = cos(angle);
    mat2  m = mat2(c, -s, s, c);
    return m * v;
}

float calculateShadow(vec4 fragPosLightSpace, vec3 n, vec3 l)
{
    if (!shadowEnabled)
        return 1.0;

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords      = projCoords * 0.5 + 0.5;

    if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0 || projCoords.z > 1.0)
        return 1.0;

    float currentDepth = projCoords.z;
    float bias         = max(0.05 * (1.0 - dot(n, l)), 0.005);

    // use Poisson disk sampling with rotation
    float shadow    = 0.0;
    vec2  texelSize = 1.0 / textureSize(shadowMap, 0);

    float randomAngle = dot(projCoords.xy, vec2(12.9898, 78.233)) * 43758.5453;
    randomAngle       = fract(randomAngle) * 6.28318; // 0 to 2*PI

    float sampleRadius = 1.0;

    for (int i = 0; i < 16; i++)
    {
        vec2 offset      = rotate2D(poissonDisk[i], randomAngle) * sampleRadius;
        vec2 sampleCoord = projCoords.xy + offset * texelSize;

        float pcfDepth = texture(shadowMap, sampleCoord).r;
        shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
    }
    shadow /= 16.0;

    return 1.0 - shadow;
}

void main()
{
    // Preprocess:
    // albedo
    vec3 albedo        = material.albedo;
    vec4 albedo_sample = vec4(albedo, 1.0);
    if (material.useTextureAlbedo)
    {
        albedo_sample = texture(material.textureAlbedo, textureCoordinates);
        albedo        = albedo_sample.rgb;
    }

    // alpha: opacity from scalar, texture_opacity, or albedo.a
    float alpha = material.opacity;
    if (material.useTextureOpacity)
        alpha *= texture(material.textureOpacity, textureCoordinates).r;
    else if (material.useTextureAlbedo)
        alpha *= albedo_sample.a;

    if (isTransparentPass)
    {
        if (alpha < 0.01)
            discard;
    }
    else if (alpha < material.alphaCutout)
        discard;

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
        emissive = texture(material.textureEmissive, textureCoordinates).rgb;
    emissive *= material.emissiveStrength;

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

            if (shadowEnabled)
            {
                float shadow = calculateShadow(fragPosLightSpace, n, l);
                radiance *= shadow;
            }
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
            Lo += discreteMonteCarloContribution(l,
                                                 radiance,
                                                 n,
                                                 v,
                                                 albedo,
                                                 metallic,
                                                 roughness,
                                                 f0,
                                                 material.subsurfaceEnabled,
                                                 material.subsurfaceRadius,
                                                 material.subsurfaceColor);
        }
    }

    // Indirect lighting (only use IBL):
    vec3 kSpecular = fresnelSchlickRoughness(max(dot(n, v), 0.0), f0, roughness);
    vec3 kDiffuse  = 1.0 - kSpecular;
    kDiffuse *= 1.0 - metallic;

    vec3 diffuseAlbedo = material.subsurfaceEnabled ? albedo * material.subsurfaceColor : albedo;
    vec3 irradiance    = texture(diffuseIrradianceMap, n).rgb;
    vec3 diffuse       = irradiance * diffuseAlbedo;

    vec3  prefilteredEnvMapColor = textureLod(prefilteredEnvMap, r, roughness * PREFILTERED_ENV_MAP_LOD).rgb;
    float NdotV                  = max(dot(n, v), 0.0);
    vec2  brdf                   = texture(brdfConvolutionMap, vec2(NdotV, roughness)).rg;
    vec3  specular               = prefilteredEnvMapColor * (kSpecular * brdf.x + brdf.y);

    vec3 ambient = (kDiffuse * diffuse + specular) * ao;

    vec3  color    = emissive + ambient + Lo;
    float sssMask  = material.subsurfaceEnabled ? 1.0 : 0.0;
    float outAlpha = isTransparentPass ? alpha : sssMask;

    if (displayMode == 1)
    {
        FragColor = vec4(albedo, 1.0);
        return;
    }
    if (displayMode == 2)
    {
        FragColor = vec4(n * 0.5 + 0.5, 1.0);
        return;
    }
    if (displayMode == 3)
    {
        FragColor = vec4(vec3(metallic), 1.0);
        return;
    }
    if (displayMode == 4)
    {
        FragColor = vec4(vec3(roughness), 1.0);
        return;
    }
    if (displayMode == 5)
    {
        FragColor = vec4(vec3(ao), 1.0);
        return;
    }
    if (displayMode == 6)
    {
        FragColor = vec4(emissive, 1.0);
        return;
    }

    FragColor = vec4(color, outAlpha);
}
