#version 450 core

out float FragColor;

in vec2 textureCoordinates;

uniform sampler2D depthTexture;
uniform sampler2D noiseTexture;

uniform mat4  projection;
uniform mat4  invProjection;
uniform vec2  noiseScale;
uniform vec2  texelSize;
uniform float radius;
uniform int   numDirections;
uniform int   numSteps;

const float PI             = 3.14159265359;
const float TWO_PI         = 6.28318530718;
const float HALF_PI        = 1.57079632679;
const float FALLOFF_ALBEDO = 0.2; // default albedo for multi-bounce approximation

vec3 reconstructViewPos(vec2 uv, float depth)
{
    vec4 ndc  = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 view = invProjection * ndc;
    return view.xyz / view.w;
}

// Cos-weighted hemisphere integral for a single horizon slice
// h1, h2: horizon angles (from tangent plane), n: projected normal angle
float integrateArc(float h1, float h2, float n)
{
    float sinN = sin(n);
    float cosN = cos(n);
    // Integral: 0.25 * (-cos(2h - n) + cos(n) + 2h*sin(n))
    return 0.25 * (-cos(2.0 * h1 - n) + cosN + 2.0 * h1 * sinN) + 0.25 * (-cos(2.0 * h2 - n) + cosN + 2.0 * h2 * sinN);
}

void main()
{
    float depth = texture(depthTexture, textureCoordinates).r;
    if (depth >= 1.0)
    {
        FragColor = 1.0;
        return;
    }

    vec3 viewPos = reconstructViewPos(textureCoordinates, depth);

    // Normal from depth gradient: n = normalize(cross(dP/dx, dP/dy))
    vec3 grad   = cross(dFdx(viewPos), dFdy(viewPos));
    vec3 normal = length(grad) > 1e-6 ? normalize(grad) : vec3(0.0, 0.0, 1.0);

    // View direction from origin to fragment in view space (normalized)
    vec3 viewDir = normalize(-viewPos);

    // Noise: (cos(angle), sin(angle), step_jitter)
    vec2  noiseUV    = textureCoordinates * noiseScale;
    vec3  noise      = texture(noiseTexture, noiseUV).xyz;
    float rotAngle   = atan(noise.y, noise.x);
    float stepJitter = noise.z;

    // Screen-space step radius: project world-space radius to pixels
    float screenRadius = (radius * projection[1][1]) / (-viewPos.z * 2.0);
    screenRadius       = max(screenRadius, 3.0);

    float ao        = 0.0;
    float angleStep = TWO_PI / float(numDirections);

    for (int dir = 0; dir < numDirections; ++dir)
    {
        float phi      = angleStep * (float(dir) + 0.5) + rotAngle;
        vec2  sliceDir = vec2(cos(phi), sin(phi));

        // Project normal onto the slice plane defined by (sliceDir, viewDir)
        // tangent in the slice: T = normalize(sliceDir.x * right + sliceDir.y * up)
        // For screen-space: tangent is (sliceDir, 0) in screen, we project normal onto slice
        vec3 sliceTangent = vec3(sliceDir, 0.0);
        vec3 orthoDir     = normal - sliceTangent * dot(normal, sliceTangent);
        vec3 projNormal   = normalize(orthoDir);

        // Normal angle in the slice: angle between viewDir and projNormal
        float cosNormalAngle = clamp(dot(projNormal, viewDir), -1.0, 1.0);
        float normalAngle    = -sign(dot(orthoDir, vec3(sliceDir, 0.0))) * acos(cosNormalAngle);

        // March in both directions along the slice
        float horizonAngle1 = -HALF_PI; // negative direction
        float horizonAngle2 = -HALF_PI; // positive direction

        float stepSize = screenRadius / float(numSteps);

        for (int step = 0; step < numSteps; ++step)
        {
            float marchDist = (float(step) + stepJitter) * stepSize;
            if (marchDist < 1.0)
                marchDist = 1.0;

            // Positive direction
            vec2 sampleUV1 = textureCoordinates + sliceDir * marchDist * texelSize;
            if (sampleUV1.x >= 0.0 && sampleUV1.x <= 1.0 && sampleUV1.y >= 0.0 && sampleUV1.y <= 1.0)
            {
                float sampleDepth1 = texture(depthTexture, sampleUV1).r;
                if (sampleDepth1 < 1.0)
                {
                    vec3  samplePos1 = reconstructViewPos(sampleUV1, sampleDepth1);
                    vec3  horizon1   = samplePos1 - viewPos;
                    float dist1      = length(horizon1);
                    if (dist1 > 1e-6 && dist1 < radius)
                    {
                        float elevAngle = asin(clamp(dot(normalize(horizon1), viewDir), -1.0, 1.0));
                        // Falloff: attenuate far samples
                        float falloff = 1.0 - (dist1 / radius) * (dist1 / radius);
                        elevAngle     = mix(-HALF_PI, elevAngle, falloff);
                        horizonAngle2 = max(horizonAngle2, elevAngle);
                    }
                }
            }

            // Negative direction
            vec2 sampleUV2 = textureCoordinates - sliceDir * marchDist * texelSize;
            if (sampleUV2.x >= 0.0 && sampleUV2.x <= 1.0 && sampleUV2.y >= 0.0 && sampleUV2.y <= 1.0)
            {
                float sampleDepth2 = texture(depthTexture, sampleUV2).r;
                if (sampleDepth2 < 1.0)
                {
                    vec3  samplePos2 = reconstructViewPos(sampleUV2, sampleDepth2);
                    vec3  horizon2   = samplePos2 - viewPos;
                    float dist2      = length(horizon2);
                    if (dist2 > 1e-6 && dist2 < radius)
                    {
                        float elevAngle = asin(clamp(dot(normalize(horizon2), viewDir), -1.0, 1.0));
                        float falloff   = 1.0 - (dist2 / radius) * (dist2 / radius);
                        elevAngle       = mix(-HALF_PI, elevAngle, falloff);
                        horizonAngle1   = max(horizonAngle1, elevAngle);
                    }
                }
            }
        }

        // Clamp horizon angles to hemisphere
        horizonAngle1 = clamp(horizonAngle1, -HALF_PI, HALF_PI);
        horizonAngle2 = clamp(horizonAngle2, -HALF_PI, HALF_PI);

        ao += integrateArc(horizonAngle1, horizonAngle2, normalAngle);
    }

    ao = clamp(ao / float(numDirections), 0.0, 1.0);

    // Multi-bounce approximation: ao_mb = ao / (1 - ao * (1 - albedo))
    float aoMultiBounce = ao / (1.0 - ao * (1.0 - FALLOFF_ALBEDO));
    aoMultiBounce       = clamp(aoMultiBounce, 0.0, 1.0);

    FragColor = aoMultiBounce;
}
