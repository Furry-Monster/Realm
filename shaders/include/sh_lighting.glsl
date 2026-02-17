#define SH_MAX_PROBES 64

struct ProbeData
{
    vec4 positionRadius; // xyz = position, w = influence radius
    vec4 sh[9];          // xyz = SH coefficient per band
};

layout(std430, binding = 5) readonly buffer ProbeBuffer
{
    int probeCount;
    ProbeData probeList[];
};

// Evaluate L2 Spherical Harmonics irradiance at given normal direction.
// shCoeffs[0..8] = 9 RGB coefficients (pre-convolved with cosine lobe).
//
// Band 0: Y_00  = 0.282095
// Band 1: Y_1-1 = 0.488603*y, Y_10 = 0.488603*z, Y_11 = 0.488603*x
// Band 2: Y_2-2 = 1.092548*x*y, Y_2-1 = 1.092548*y*z, Y_20 = 0.315392*(3z^2-1)
//          Y_21 = 1.092548*x*z, Y_22 = 0.546274*(x^2-y^2)
vec3 evaluateSH(vec3 normal, vec3 shCoeffs[9])
{
    vec3 n = normalize(normal);

    vec3 result = shCoeffs[0] * 0.282095;

    result += shCoeffs[1] * 0.488603 * n.y;
    result += shCoeffs[2] * 0.488603 * n.z;
    result += shCoeffs[3] * 0.488603 * n.x;

    result += shCoeffs[4] * 1.092548 * n.x * n.y;
    result += shCoeffs[5] * 1.092548 * n.y * n.z;
    result += shCoeffs[6] * 0.315392 * (3.0 * n.z * n.z - 1.0);
    result += shCoeffs[7] * 1.092548 * n.x * n.z;
    result += shCoeffs[8] * 0.546274 * (n.x * n.x - n.y * n.y);

    return max(result, vec3(0.0));
}

// Compute SH indirect diffuse by blending nearby probes weighted by inverse distance.
// Returns blended SH irradiance for the given world position and normal.
vec3 evaluateProbeIrradiance(vec3 worldPos, vec3 normal)
{
    if (probeCount <= 0)
        return vec3(0.0);

    vec3 blendedSH[9];
    for (int i = 0; i < 9; ++i)
        blendedSH[i] = vec3(0.0);

    float totalWeight = 0.0;

    int count = min(probeCount, SH_MAX_PROBES);
    for (int p = 0; p < count; ++p)
    {
        vec3  probePos = probeList[p].positionRadius.xyz;
        float radius   = probeList[p].positionRadius.w;
        float dist     = distance(worldPos, probePos);

        if (dist > radius)
            continue;

        // Smooth falloff: 1 - (d/r)^2
        float t = dist / radius;
        float w = max(1.0 - t * t, 0.0);

        for (int i = 0; i < 9; ++i)
            blendedSH[i] += probeList[p].sh[i].rgb * w;

        totalWeight += w;
    }

    if (totalWeight <= 0.0)
        return vec3(0.0);

    for (int i = 0; i < 9; ++i)
        blendedSH[i] /= totalWeight;

    return evaluateSH(normal, blendedSH);
}
