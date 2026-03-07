uniform sampler2DArray shadowMapArray;
uniform bool           shadowEnabled;
uniform int            cascadeCount;
uniform mat4           cascadeVP[4];
uniform float          cascadeSplits[4]; // far depth of each cascade in view space
uniform float          lightSize;        // virtual light size for PCSS

// Poisson disk 16-tap sampling
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

vec2 rotatePoissonSample(vec2 v, float angle)
{
    float s = sin(angle);
    float c = cos(angle);
    return mat2(c, -s, s, c) * v;
}

int selectCascade(float viewDepth)
{
    for (int i = 0; i < cascadeCount; ++i)
    {
        if (viewDepth < cascadeSplits[i])
            return i;
    }
    return cascadeCount - 1;
}

// Blocker search for PCSS: returns average blocker depth, or -1 if no blockers
float blockerSearch(vec3 projCoords, int cascade, float searchRadius)
{
    float blockerSum   = 0.0;
    int   blockerCount = 0;
    vec2  texel        = 1.0 / vec2(textureSize(shadowMapArray, 0).xy);
    float angle        = fract(dot(projCoords.xy, vec2(12.9898, 78.233)) * 43758.5453) * 6.28318;

    for (int i = 0; i < 16; ++i)
    {
        vec2  off = rotatePoissonSample(poissonDisk[i], angle) * texel * searchRadius;
        float d   = texture(shadowMapArray, vec3(projCoords.xy + off, float(cascade))).r;
        if (d < projCoords.z)
        {
            blockerSum += d;
            blockerCount++;
        }
    }
    return blockerCount > 0 ? blockerSum / float(blockerCount) : -1.0;
}

// PCF with variable kernel radius
float pcfFilter(vec3 projCoords, int cascade, float filterRadius)
{
    float shadow = 0.0;
    vec2  texel  = 1.0 / vec2(textureSize(shadowMapArray, 0).xy);
    float angle  = fract(dot(projCoords.xy, vec2(12.9898, 78.233)) * 43758.5453) * 6.28318;

    for (int i = 0; i < 16; ++i)
    {
        vec2  off = rotatePoissonSample(poissonDisk[i], angle) * texel * filterRadius;
        float d   = texture(shadowMapArray, vec3(projCoords.xy + off, float(cascade))).r;
        shadow += projCoords.z > d ? 1.0 : 0.0;
    }
    return 1.0 - shadow / 16.0;
}

// PCSS: distance-dependent penumbra
float pcssShadow(vec3 projCoords, int cascade, float bias)
{
    projCoords.z -= bias;
    float avgBlockerDepth = blockerSearch(projCoords, cascade, lightSize * 20.0);
    if (avgBlockerDepth < 0.0)
        return 1.0;
    // Penumbra estimation: w_penumbra = lightSize * (d_receiver - d_blocker) / d_blocker
    float penumbra     = lightSize * (projCoords.z - avgBlockerDepth) / avgBlockerDepth;
    float filterRadius = max(penumbra * 20.0, 1.0);
    return pcfFilter(projCoords, cascade, filterRadius);
}

// Standard PCF shadow for a single cascade
float pcfShadow(vec3 projCoords, int cascade, float bias)
{
    projCoords.z -= bias;
    return pcfFilter(projCoords, cascade, 1.0);
}

// Main entry: compute shadow factor for a world-space fragment
float calculateShadow(vec3 worldPos, vec3 n, vec3 l, mat4 viewMatrix)
{
    if (!shadowEnabled)
        return 1.0;

    // View-space depth for cascade selection
    float viewDepth = -(viewMatrix * vec4(worldPos, 1.0)).z;
    int   cascade   = selectCascade(viewDepth);

    // Project into cascade light space
    vec4 lsPos = cascadeVP[cascade] * vec4(worldPos, 1.0);
    vec3 proj  = lsPos.xyz / lsPos.w;
    proj       = proj * 0.5 + 0.5;

    if (proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0 || proj.z > 1.0)
        return 1.0;

    float bias = max(0.05 * (1.0 - dot(n, l)), 0.005);

    float shadow;
    if (lightSize > 0.0)
        shadow = pcssShadow(proj, cascade, bias);
    else
        shadow = pcfShadow(proj, cascade, bias);

    // Fade out shadow at cascade boundary to reduce visible seams
    float fade = smoothstep(cascadeSplits[cascade] * 0.9, cascadeSplits[cascade], viewDepth);
    if (cascade < cascadeCount - 1)
    {
        vec4 lsNext   = cascadeVP[cascade + 1] * vec4(worldPos, 1.0);
        vec3 projNext = lsNext.xyz / lsNext.w;
        projNext      = projNext * 0.5 + 0.5;

        float shadowNext;
        if (lightSize > 0.0)
            shadowNext = pcssShadow(projNext, cascade + 1, bias);
        else
            shadowNext = pcfShadow(projNext, cascade + 1, bias);

        shadow = mix(shadow, shadowNext, fade);
    }

    return shadow;
}

// Point light shadow: cubemap stores linear depth in R channel, normalized to [0,1]
uniform int   pointShadowCount;
uniform vec3  pointShadowPos[4];
uniform float pointShadowRange[4];
uniform int   pointShadowLightIndex[4];
uniform samplerCube pointShadowMap[4];

float calculatePointShadow(vec3 worldPos, vec3 n, vec3 lightPos, int lightIndex, float bias)
{
    if (pointShadowCount <= 0)
        return 1.0;
    for (int i = 0; i < pointShadowCount; i++)
    {
        if (pointShadowLightIndex[i] != lightIndex)
            continue;
        vec3 fragToLight = worldPos - pointShadowPos[i];
        float dist       = length(fragToLight);
        float depth      = dist / pointShadowRange[i];
        if (depth > 1.0)
            return 1.0;
        vec3 dir     = fragToLight / dist;
        float stored = texture(pointShadowMap[i], dir).r;
        return (depth - bias <= stored) ? 1.0 : 0.0;
    }
    return 1.0;
}

// Spot light shadow: 2D depth map with light view-proj
uniform int   spotShadowCount;
uniform mat4  spotShadowVP[4];
uniform int   spotShadowLightIndex[4];
uniform sampler2D spotShadowMap[4];

float calculateSpotShadow(vec3 worldPos, vec3 n, int lightIndex, float bias)
{
    if (spotShadowCount <= 0)
        return 1.0;
    for (int i = 0; i < spotShadowCount; i++)
    {
        if (spotShadowLightIndex[i] != lightIndex)
            continue;
        vec4 lsPos = spotShadowVP[i] * vec4(worldPos, 1.0);
        vec3 proj  = lsPos.xyz / lsPos.w;
        proj       = proj * 0.5 + 0.5;
        if (proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0 || proj.z > 1.0)
            return 1.0;
        float d = texture(spotShadowMap[i], proj.xy).r;
        return (proj.z - bias <= d) ? 1.0 : 0.0;
    }
    return 1.0;
}
