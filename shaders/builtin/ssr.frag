#version 450 core

in vec2  textureCoordinates;
out vec4 FragColor;

uniform sampler2D uSceneColor;
uniform sampler2D uHiZDepth;
uniform sampler2D uNormalMetallic;
uniform sampler2D uEmissiveRoughness;
uniform sampler2D uSceneDepth;

uniform mat4  uView;
uniform mat4  uProjection;
uniform mat4  uInvView;
uniform mat4  uInvProjection;
uniform int   uMaxSteps;
uniform float uMaxDistance;
uniform float uNearPlane;
uniform float uFarPlane;
uniform vec2  uScreenSize;
uniform int   uHiZMipCount;

vec3 reconstructViewPos(vec2 uv, float depth)
{
    vec4 ndc  = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 view = uInvProjection * ndc;
    return view.xyz / view.w;
}

vec3 projectToScreen(vec3 viewPos)
{
    vec4 clip = uProjection * vec4(viewPos, 1.0);
    vec3 ndc  = clip.xyz / clip.w;
    return vec3(ndc.xy * 0.5 + 0.5, ndc.z * 0.5 + 0.5);
}

float readHiZDepth(vec2 uv, int mip) { return textureLod(uHiZDepth, uv, float(mip)).r; }

// Hi-Z accelerated screen-space ray march.
// Returns: vec4(hitUV.xy, hitDepth, confidence)
vec4 hiZTrace(vec3 rayOrigin, vec3 rayDir)
{
    vec3 startScreen = projectToScreen(rayOrigin);
    vec3 endView     = rayOrigin + rayDir * uMaxDistance;
    vec3 endScreen   = projectToScreen(endView);

    vec3 rayScreen = endScreen - startScreen;

    // Prevent degenerate rays.
    if (length(rayScreen.xy) < 1e-5)
        return vec4(0.0);

    // Scale so the larger component steps one pixel.
    vec2  pixelStep = rayScreen.xy * uScreenSize;
    float stepScale = 1.0 / max(abs(pixelStep.x), abs(pixelStep.y));
    vec3  stepDir   = rayScreen * stepScale;

    // Start at mip 0 for initial precision, then jump to coarser mips.
    int mip    = 0;
    int maxMip = uHiZMipCount - 1;

    vec3  pos      = startScreen + stepDir * 2.0; // step past origin
    float stepSize = 2.0;

    for (int i = 0; i < uMaxSteps; ++i)
    {
        if (pos.x < 0.0 || pos.x > 1.0 || pos.y < 0.0 || pos.y > 1.0)
            break;

        if (pos.z < 0.0 || pos.z > 1.0)
            break;

        float sceneDepth = readHiZDepth(pos.xy, mip);

        if (pos.z > sceneDepth)
        {
            // Hit detected -- refine to finer mip.
            if (mip == 0)
            {
                // Final hit: compute confidence.
                float depthDiff  = abs(pos.z - sceneDepth);
                float confidence = 1.0 - smoothstep(0.0, 0.005, depthDiff);

                // Fade at screen edges.
                vec2 edgeFade =
                    smoothstep(vec2(0.0), vec2(0.05), pos.xy) * (1.0 - smoothstep(vec2(0.95), vec2(1.0), pos.xy));
                confidence *= edgeFade.x * edgeFade.y;

                return vec4(pos.xy, sceneDepth, confidence);
            }

            // Step back and go to finer mip.
            pos -= stepDir * stepSize;
            mip = max(mip - 1, 0);
            stepSize *= 0.5;
        }
        else
        {
            // No hit: advance and try coarser mip.
            mip      = min(mip + 1, maxMip);
            stepSize = max(stepSize, float(1 << mip));
        }

        pos += stepDir * stepSize;
    }

    return vec4(0.0);
}

void main()
{
    float depth = texture(uSceneDepth, textureCoordinates).r;

    // Sky: no reflection.
    if (depth >= 1.0)
    {
        FragColor = vec4(0.0);
        return;
    }

    vec4  normalMetallic = texture(uNormalMetallic, textureCoordinates);
    vec3  worldNormal    = normalize(normalMetallic.xyz * 2.0 - 1.0);
    float metallic       = normalMetallic.a;

    vec4  emissiveRough = texture(uEmissiveRoughness, textureCoordinates);
    float roughness     = emissiveRough.a;

    // Skip very rough surfaces -- SSR contribution is negligible.
    if (roughness > 0.7)
    {
        FragColor = vec4(0.0);
        return;
    }

    vec3 viewPos    = reconstructViewPos(textureCoordinates, depth);
    vec3 viewNormal = normalize((uView * vec4(worldNormal, 0.0)).xyz);

    vec3 viewDir    = normalize(viewPos);
    vec3 reflectDir = reflect(viewDir, viewNormal);

    vec4  hitResult  = hiZTrace(viewPos, reflectDir);
    float confidence = hitResult.w;

    if (confidence <= 0.0)
    {
        FragColor = vec4(0.0);
        return;
    }

    vec3 reflectedColor = texture(uSceneColor, hitResult.xy).rgb;

    // Fresnel-Schlick approximation for blend factor.
    float NdotV   = max(dot(-viewDir, viewNormal), 0.0);
    float F0      = mix(0.04, 1.0, metallic);
    float fresnel = F0 + (1.0 - F0) * pow(1.0 - NdotV, 5.0);

    // Attenuate by roughness: rougher surfaces get weaker SSR.
    float roughnessAttenuation = 1.0 - roughness;

    float alpha = confidence * fresnel * roughnessAttenuation;

    FragColor = vec4(reflectedColor * alpha, alpha);
}
