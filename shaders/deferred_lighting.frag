#version 330 core

#define PI 3.1415926535897932384626433832795
#define PREFILTERED_ENV_MAP_LOD 4.0

layout(location = 0) out vec4 FragColor;

in vec2 textureCoordinates;

// G-Buffer inputs
uniform sampler2D gAlbedoAO;
uniform sampler2D gNormalMetallic;
uniform sampler2D gEmissiveRoughness;
uniform sampler2D gDepth;

// Camera
uniform vec3 cameraPosition;
uniform mat4 invView;
uniform mat4 invProjection;

// Lights
struct LightData
{
    vec4 position;
    vec4 direction;
    vec4 color;
    vec4 attenuation;
    vec4 spot_area;
};

layout(std140) uniform LightBlock
{
    int lightCount;
    LightData lights[16];
};

// IBL
uniform samplerCube diffuseIrradianceMap;
uniform samplerCube prefilteredEnvMap;
uniform sampler2D brdfConvolutionMap;

// Shadow
uniform sampler2D shadowMap;
uniform bool shadowEnabled;
uniform mat4 lightSpaceMatrix;

// Viewport display mode
uniform int displayMode;

// Reconstruct world position from depth
vec3 reconstructWorldPosition(vec2 uv, float depth)
{
    // NDC [-1, 1]
    vec4 ndc = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos = invProjection * ndc;
    viewPos /= viewPos.w;
    vec4 worldPos = invView * viewPos;
    return worldPos.xyz;
}

vec3 fresnelSchlick(float cosTheta, vec3 f0)
{
    return f0 + (1.0 - f0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 f0, float roughness)
{
    return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// NDF: Trowbridge-Reitz GGX
float ndfTrowbridgeReitzGGX(vec3 n, vec3 h, float roughness)
{
    float alpha = roughness * roughness;
    float alphaSquared = alpha * alpha;
    float nDotH = max(dot(n, h), 0.0);
    float nDotHSquared = nDotH * nDotH;
    float innerTerms = nDotHSquared * (alphaSquared - 1.0) + 1.0;
    float denomenator = PI * innerTerms * innerTerms;
    return alphaSquared / max(denomenator, 0.0001);
}

float geometrySchlickGGX(vec3 n, vec3 v, float k)
{
    float nDotV = max(dot(n, v), 0.0);
    return nDotV / (nDotV * (1.0 - k) + k);
}

float geometrySmith(vec3 n, vec3 v, vec3 l, float roughness)
{
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    return geometrySchlickGGX(n, v, k) * geometrySchlickGGX(n, l, k);
}

// Cook-Torrance BRDF
vec3 cookTorranceBRDF(vec3 l, vec3 radiance, vec3 n, vec3 v, vec3 albedo, float metallic, float roughness, vec3 f0)
{
    vec3 h = normalize(v + l);

    float D = ndfTrowbridgeReitzGGX(n, h, roughness);
    vec3 F = fresnelSchlick(max(dot(h, v), 0.0), f0);
    float G = geometrySmith(n, v, l, roughness);

    vec3 numerator = D * F * G;
    float denominator = 4.0 * max(dot(v, n), 0.0) * max(dot(l, n), 0.0);
    vec3 specular = numerator / max(denominator, 0.001);

    vec3 kDiffuse = (vec3(1.0) - F) * (1.0 - metallic);
    vec3 diffuse = kDiffuse * albedo / PI;

    float nDotL = max(dot(n, l), 0.0);
    return (diffuse + specular) * radiance * nDotL;
}

// Poisson disk PCF shadow
const vec2 poissonDisk[16] = vec2[](
        vec2(-0.613392, 0.617481), vec2(0.170019, -0.040254),
        vec2(-0.299417, 0.791925), vec2(0.645680, 0.493210),
        vec2(-0.651784, 0.717887), vec2(0.421003, 0.027070),
        vec2(-0.817194, -0.271096), vec2(-0.705374, -0.668203),
        vec2(0.977050, -0.108615), vec2(0.063326, 0.142369),
        vec2(0.203528, 0.214331), vec2(-0.667531, 0.326090),
        vec2(-0.098422, -0.295755), vec2(-0.885922, 0.215369),
        vec2(0.566637, 0.605213), vec2(0.039766, -0.396100));

vec2 rotate2D(vec2 v, float angle)
{
    float s = sin(angle);
    float c = cos(angle);
    return mat2(c, -s, s, c) * v;
}

float calculateShadow(vec3 worldPos, vec3 n, vec3 l)
{
    if (!shadowEnabled)
        return 1.0;

    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(worldPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
            projCoords.y < 0.0 || projCoords.y > 1.0 ||
            projCoords.z > 1.0)
        return 1.0;

    float currentDepth = projCoords.z;
    float bias = max(0.05 * (1.0 - dot(n, l)), 0.005);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    float randomAngle = fract(dot(projCoords.xy, vec2(12.9898, 78.233)) * 43758.5453) * 6.28318;

    for (int i = 0; i < 16; i++)
    {
        vec2 offset = rotate2D(poissonDisk[i], randomAngle);
        float pcfDepth = texture(shadowMap, projCoords.xy + offset * texelSize).r;
        shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
    }
    shadow /= 16.0;
    return 1.0 - shadow;
}

void main()
{
    // Sample G-Buffer
    vec4 albedoAO_sample = texture(gAlbedoAO, textureCoordinates);
    vec4 normalMet_sample = texture(gNormalMetallic, textureCoordinates);
    vec4 emissRough_sample = texture(gEmissiveRoughness, textureCoordinates);
    float depth = texture(gDepth, textureCoordinates).r;

    // Discard sky pixels (depth == 1.0)
    if (depth >= 1.0)
        discard;

    vec3 albedo = albedoAO_sample.rgb;
    float ao = albedoAO_sample.a;
    vec3 n = normalize(normalMet_sample.xyz);
    float metallic = normalMet_sample.w;
    vec3 emissive = emissRough_sample.rgb;
    float roughness = emissRough_sample.w;

    vec3 worldPos = reconstructWorldPosition(textureCoordinates, depth);
    vec3 v = normalize(cameraPosition - worldPos);
    vec3 r = reflect(-v, n);

    // F0: 0.04 for dielectrics, albedo for metals
    vec3 f0 = mix(vec3(0.04), albedo, metallic);

    // Display modes (match forward pipeline)
    if (displayMode == 1) {
        FragColor = vec4(albedo, 1.0);
        return;
    }
    if (displayMode == 2) {
        FragColor = vec4(n * 0.5 + 0.5, 1.0);
        return;
    }
    if (displayMode == 3) {
        FragColor = vec4(vec3(metallic), 1.0);
        return;
    }
    if (displayMode == 4) {
        FragColor = vec4(vec3(roughness), 1.0);
        return;
    }
    if (displayMode == 5) {
        FragColor = vec4(vec3(ao), 1.0);
        return;
    }
    if (displayMode == 6) {
        FragColor = vec4(emissive, 1.0);
        return;
    }

    // Direct lighting
    vec3 Lo = vec3(0.0);

    for (int i = 0; i < lightCount; i++)
    {
        int lightType = int(lights[i].position.w);
        vec3 lightPosition = lights[i].position.xyz;
        vec3 lightDirection = lights[i].direction.xyz;
        float lightIntensity = lights[i].direction.w;
        vec3 lightColor = lights[i].color.rgb;
        float lightConstant = lights[i].color.w;
        float lightLinear = lights[i].attenuation.x;
        float lightQuadratic = lights[i].attenuation.y;
        float lightRange = lights[i].attenuation.z;
        float innerConeAngle = lights[i].attenuation.w;
        float outerConeAngle = lights[i].spot_area.x;

        vec3 radiance = vec3(0.0);
        vec3 l = vec3(0.0);

        // Point light
        if (lightType == 0)
        {
            vec3 lightDir = lightPosition - worldPos;
            float distance = length(lightDir);
            if (distance > lightRange) continue;
            l = normalize(lightDir);
            float atten = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * distance * distance);
            radiance = lightColor * lightIntensity * atten;
        }
        // Directional light
        else if (lightType == 1)
        {
            l = normalize(-lightDirection);
            radiance = lightColor * lightIntensity;
            if (shadowEnabled)
                radiance *= calculateShadow(worldPos, n, l);
        }
        // Spot light
        else if (lightType == 2)
        {
            vec3 lightDir = lightPosition - worldPos;
            float distance = length(lightDir);
            if (distance > lightRange) continue;
            l = normalize(lightDir);
            vec3 spotDir = normalize(lightDirection);
            float theta = dot(l, -spotDir);
            float innerCos = cos(radians(innerConeAngle));
            float outerCos = cos(radians(outerConeAngle));
            float spotFactor = clamp((theta - outerCos) / (innerCos - outerCos), 0.0, 1.0);
            if (spotFactor <= 0.0) continue;
            float atten = 1.0 / (lightConstant + lightLinear * distance + lightQuadratic * distance * distance);
            radiance = lightColor * lightIntensity * atten * spotFactor;
        }
        // Area light
        else if (lightType == 3)
        {
            vec3 lightDir = lightPosition - worldPos;
            float distance = length(lightDir);
            l = normalize(lightDir);
            float atten = 1.0 / (distance * distance);
            float facingFactor = max(dot(-normalize(lightDirection), n), 0.0);
            radiance = lightColor * lightIntensity * atten * facingFactor;
        }

        if (length(radiance) > 0.0)
            Lo += cookTorranceBRDF(l, radiance, n, v, albedo, metallic, roughness, f0);
    }

    // IBL: indirect lighting
    vec3 kSpecular = fresnelSchlickRoughness(max(dot(n, v), 0.0), f0, roughness);
    vec3 kDiffuse = (1.0 - kSpecular) * (1.0 - metallic);

    vec3 irradiance = texture(diffuseIrradianceMap, n).rgb;
    vec3 diffuse = irradiance * albedo;

    vec3 prefilteredColor = textureLod(prefilteredEnvMap, r, roughness * PREFILTERED_ENV_MAP_LOD).rgb;
    float NdotV = max(dot(n, v), 0.0);
    vec2 brdf = texture(brdfConvolutionMap, vec2(NdotV, roughness)).rg;
    vec3 specular = prefilteredColor * (kSpecular * brdf.x + brdf.y);

    vec3 ambient = (kDiffuse * diffuse + specular) * ao;
    vec3 color = emissive + ambient + Lo;

    FragColor = vec4(color, 0.0);
}
