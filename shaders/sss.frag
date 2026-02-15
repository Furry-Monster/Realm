#version 330 core

out vec4 FragColor;
in vec2  textureCoordinates;

uniform sampler2D colorTexture;
uniform sampler2D depthTexture;
uniform mat4      projection;
uniform mat4      invProjection;
uniform vec2      blurDirection;
uniform float     radius;
uniform int       samples;

// Gaussian weights for 9-tap separable kernel (center + 4 offsets each side)
const float gaussianWeights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

vec3 viewPosFromDepth(float depth, vec2 uv)
{
    vec4 ndc      = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos4 = invProjection * ndc;
    return viewPos4.xyz / viewPos4.w;
}

void main()
{
    vec4  centerSample = texture(colorTexture, textureCoordinates);
    float centerDepth  = texture(depthTexture, textureCoordinates).r;

    if (centerDepth >= 1.0)
    {
        FragColor = centerSample;
        return;
    }

    float sssMask = centerSample.a;
    if (sssMask < 0.01)
    {
        FragColor = vec4(centerSample.rgb, 0.0);
        return;
    }

    vec3 centerViewPos = viewPosFromDepth(centerDepth, textureCoordinates);
    vec3 gradX         = dFdx(centerViewPos);
    vec3 gradY         = dFdy(centerViewPos);
    vec3 normal        = normalize(cross(gradX, gradY));
    if (length(normal) < 1e-6)
        normal = vec3(0.0, 0.0, 1.0);

    vec2  texelSize = 1.0 / textureSize(colorTexture, 0);
    vec3  result    = centerSample.rgb * gaussianWeights[0];
    float weightSum = gaussianWeights[0];

    for (int i = 1; i < 5; ++i)
    {
        vec2 offset = texelSize * float(i) * blurDirection * radius;
        for (int s = -1; s <= 1; s += 2)
        {
            vec2  sampleUV    = textureCoordinates + offset * float(s);
            vec4  sampleTex   = texture(colorTexture, sampleUV);
            float sampleDepth = texture(depthTexture, sampleUV).r;

            if (sampleDepth >= 1.0)
                continue;

            vec3  sampleViewPos = viewPosFromDepth(sampleDepth, sampleUV);
            float depthDiff     = abs(centerViewPos.z - sampleViewPos.z);
            float depthScale    = length(centerViewPos) * 0.01;
            float rangeCheck    = 1.0 - smoothstep(0.0, depthScale, depthDiff);

            float w = gaussianWeights[i] * rangeCheck * sampleTex.a;
            result += sampleTex.rgb * w;
            weightSum += w;
        }
    }

    result /= max(weightSum, 0.0001);
    FragColor = vec4(result, sssMask);
}
