#version 450 core

#include "../include/common.glsl"
#include "../include/brdf.glsl"
#include "../include/lighting.glsl"
#include "../include/shadow.glsl"
#include "../include/sh_lighting.glsl"

layout(location = 0) out vec4 FragColor;

in vec2 textureCoordinates;

// G-Buffer inputs (RT0.a = shadingModelID)
uniform sampler2D gAlbedoModelID;
uniform sampler2D gNormalMetallic;
uniform sampler2D gEmissiveRoughness;
uniform sampler2D gAO;
uniform sampler2D gDepth;

// Camera
uniform vec3 cameraPosition;
uniform mat4 invView;
uniform mat4 invProjection;

// IBL
uniform samplerCube diffuseIrradianceMap;
uniform samplerCube prefilteredEnvMap;
uniform sampler2D   brdfConvolutionMap;

// Viewport display mode
uniform int  displayMode;
uniform mat4 viewMatrix;

// Clustered lighting
uniform bool   useClusteredLights;
uniform float  nearPlane;
uniform float  farPlane;

vec3 reconstructWorldPosition(vec2 uv, float depth)
{
    vec4 ndc     = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos = invProjection * ndc;
    viewPos /= viewPos.w;
    vec4 worldPos = invView * viewPos;
    return worldPos.xyz;
}

uniform bool probesEnabled;

vec3 computeIBL(vec3 albedo, vec3 n, vec3 v, vec3 r, float metallic, float roughness, vec3 f0, vec3 worldPos)
{
    vec3 kSpecular = fresnelSchlickRoughness(max(dot(n, v), 0.0), f0, roughness);
    vec3 kDiffuse  = (1.0 - kSpecular) * (1.0 - metallic);

    vec3 irradiance = texture(diffuseIrradianceMap, n).rgb;

    if (probesEnabled)
    {
        vec3 probeIrr = evaluateProbeIrradiance(worldPos, n);
        // Blend: probe data overrides IBL diffuse where available
        float probeWeight = (probeCount > 0) ? 1.0 : 0.0;
        irradiance        = mix(irradiance, probeIrr, probeWeight);
    }

    vec3 diffuse = irradiance * albedo;

    vec3  prefilteredColor = textureLod(prefilteredEnvMap, r, roughness * PREFILTERED_ENV_MAP_LOD).rgb;
    float NdotV            = max(dot(n, v), 0.0);
    vec2  brdf             = texture(brdfConvolutionMap, vec2(NdotV, roughness)).rg;
    vec3  specular         = prefilteredColor * (kSpecular * brdf.x + brdf.y);

    return kDiffuse * diffuse + specular;
}

// Standard PBR shading (shadingModelID = 0)
vec3 shadePBR(vec3  albedo,
              vec3  n,
              vec3  v,
              vec3  worldPos,
              float metallic,
              float roughness,
              vec3  emissive,
              vec3  r,
              float ao,
              vec2  screenUV,
              float viewDepth)
{
    vec3 f0 = mix(vec3(0.04), albedo, metallic);
    vec3 Lo = vec3(0.0);

    if (useClusteredLights)
    {
        ivec3 c = getClusterIndex(screenUV, viewDepth, nearPlane, farPlane);
        int   clusterIdx = c.x + c.y * clusterDimensions.x + c.z * clusterDimensions.x * clusterDimensions.y;
        uvec2 grid       = clusterGrid[clusterIdx];
        uint  offset     = grid.x;
        uint  count      = grid.y;
        for (uint i = 0u; i < count; i++)
        {
            int idx = int(lightIndices[offset + i]);
            vec3 l, radiance;
            if (!evaluateLight(idx, worldPos, n, l, radiance))
                continue;

            int lightType = int(lights[idx].position.w);
            float shadowBias = max(0.05 * (1.0 - dot(n, l)), 0.005);
            if (lightType == 1)
                radiance *= calculateShadow(worldPos, n, l, viewMatrix);
            else if (lightType == 0 && pointShadowCount > 0)
                radiance *= calculatePointShadow(worldPos, n, lights[idx].position.xyz, idx, shadowBias);
            else if (lightType == 2 && spotShadowCount > 0)
                radiance *= calculateSpotShadow(worldPos, n, idx, shadowBias);

            Lo += cookTorranceBRDF(l, radiance, n, v, albedo, metallic, roughness, f0, false, 0.0, vec3(1.0));
        }
    }
    else
    {
        for (int i = 0; i < lightCount; i++)
        {
            vec3 l, radiance;
            if (!evaluateLight(i, worldPos, n, l, radiance))
                continue;

            int lightType = int(lights[i].position.w);
            float shadowBias = max(0.05 * (1.0 - dot(n, l)), 0.005);
            if (lightType == 1)
                radiance *= calculateShadow(worldPos, n, l, viewMatrix);
            else if (lightType == 0 && pointShadowCount > 0)
                radiance *= calculatePointShadow(worldPos, n, lights[i].position.xyz, i, shadowBias);
            else if (lightType == 2 && spotShadowCount > 0)
                radiance *= calculateSpotShadow(worldPos, n, i, shadowBias);

            Lo += cookTorranceBRDF(l, radiance, n, v, albedo, metallic, roughness, f0, false, 0.0, vec3(1.0));
        }
    }

    vec3 ambient = computeIBL(albedo, n, v, r, metallic, roughness, f0, worldPos) * ao;
    return emissive + ambient + Lo;
}

// Subsurface shading (shadingModelID = 1): PBR + subsurface wrap diffuse
vec3 shadeSubsurface(vec3  albedo,
                     vec3  n,
                     vec3  v,
                     vec3  worldPos,
                     float metallic,
                     float roughness,
                     vec3  emissive,
                     vec3  r,
                     float ao,
                     vec2  screenUV,
                     float viewDepth)
{
    vec3 f0 = mix(vec3(0.04), albedo, metallic);
    vec3 Lo = vec3(0.0);

    if (useClusteredLights)
    {
        ivec3 c = getClusterIndex(screenUV, viewDepth, nearPlane, farPlane);
        int   clusterIdx = c.x + c.y * clusterDimensions.x + c.z * clusterDimensions.x * clusterDimensions.y;
        uvec2 grid       = clusterGrid[clusterIdx];
        uint  offset     = grid.x;
        uint  count      = grid.y;
        for (uint i = 0u; i < count; i++)
        {
            int idx = int(lightIndices[offset + i]);
            vec3 l, radiance;
            if (!evaluateLight(idx, worldPos, n, l, radiance))
                continue;

            int lightType = int(lights[idx].position.w);
            float shadowBias = max(0.05 * (1.0 - dot(n, l)), 0.005);
            if (lightType == 1)
                radiance *= calculateShadow(worldPos, n, l, viewMatrix);
            else if (lightType == 0 && pointShadowCount > 0)
                radiance *= calculatePointShadow(worldPos, n, lights[idx].position.xyz, idx, shadowBias);
            else if (lightType == 2 && spotShadowCount > 0)
                radiance *= calculateSpotShadow(worldPos, n, idx, shadowBias);

            Lo += cookTorranceBRDF(l, radiance, n, v, albedo, metallic, roughness, f0, true, 1.0, vec3(1.0));
        }
    }
    else
    {
        for (int i = 0; i < lightCount; i++)
        {
            vec3 l, radiance;
            if (!evaluateLight(i, worldPos, n, l, radiance))
                continue;

            int lightType = int(lights[i].position.w);
            float shadowBias = max(0.05 * (1.0 - dot(n, l)), 0.005);
            if (lightType == 1)
                radiance *= calculateShadow(worldPos, n, l, viewMatrix);
            else if (lightType == 0 && pointShadowCount > 0)
                radiance *= calculatePointShadow(worldPos, n, lights[i].position.xyz, i, shadowBias);
            else if (lightType == 2 && spotShadowCount > 0)
                radiance *= calculateSpotShadow(worldPos, n, i, shadowBias);

            Lo += cookTorranceBRDF(l, radiance, n, v, albedo, metallic, roughness, f0, true, 1.0, vec3(1.0));
        }
    }

    vec3 ambient = computeIBL(albedo, n, v, r, metallic, roughness, f0, worldPos) * ao;
    return emissive + ambient + Lo;
}

void main()
{
    vec4  albedoModelID = texture(gAlbedoModelID, textureCoordinates);
    vec4  normalMet     = texture(gNormalMetallic, textureCoordinates);
    vec4  emissRough    = texture(gEmissiveRoughness, textureCoordinates);
    float ao            = texture(gAO, textureCoordinates).r;
    float depth         = texture(gDepth, textureCoordinates).r;

    if (depth >= 1.0)
        discard;

    vec3  albedo    = albedoModelID.rgb;
    int   modelID   = int(albedoModelID.a + 0.5);
    vec3  n         = normalize(normalMet.xyz);
    float metallic  = normalMet.w;
    vec3  emissive  = emissRough.rgb;
    float roughness = emissRough.w;

    vec3 worldPos = reconstructWorldPosition(textureCoordinates, depth);
    vec3 v        = normalize(cameraPosition - worldPos);
    vec3 r        = reflect(-v, n);
    float viewDepth = -(viewMatrix * vec4(worldPos, 1.0)).z;

    // Debug display modes
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

    vec3 color;
    if (modelID == 1)
        color = shadeSubsurface(albedo, n, v, worldPos, metallic, roughness, emissive, r, ao, textureCoordinates,
                                viewDepth);
    else
        color = shadePBR(albedo, n, v, worldPos, metallic, roughness, emissive, r, ao, textureCoordinates, viewDepth);

    FragColor = vec4(color, 0.0);
}
