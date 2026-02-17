#version 330 core

out float FragColor;

in vec2 textureCoordinates;

uniform sampler2D depthTexture;
uniform sampler2D noiseTexture;

uniform mat4 projection;
uniform mat4 invProjection;

uniform vec2  noiseScale;
uniform float radius;
uniform float bias;
uniform int   kernelSize;

uniform vec3 samples[64];

void main()
{
    float depth = texture(depthTexture, textureCoordinates).r;
    if (depth >= 1.0)
    {
        FragColor = 1.0;
        return;
    }

    // Reconstruct view-space position: P_view = (invProj * [uv*2-1, depth*2-1, 1]) / w
    vec4 ndc      = vec4(textureCoordinates * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos4 = invProjection * ndc;
    vec3 viewPos  = viewPos4.xyz / viewPos4.w;

    // Normal from depth gradient: n = normalize(∂P/∂x × ∂P/∂y)
    vec3 grad = cross(dFdx(viewPos), dFdy(viewPos));
    vec3 normal = length(grad) > 1e-6 ? normalize(grad) : vec3(0.0, 0.0, 1.0);

    vec2 noiseUV   = textureCoordinates * noiseScale;
    vec3 randomVec = texture(noiseTexture, noiseUV).xyz;

    // TBN: T = normalize(r - n·(r·n)), B = n × T
    vec3 tangentVec = randomVec - normal * dot(randomVec, normal);
    vec3 tangent    = length(tangentVec) > 1e-6 ? normalize(tangentVec) : vec3(1.0, 0.0, 0.0);
    vec3 bitangent = cross(normal, tangent);
    mat3 tbn       = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; ++i)
    {
        // Sample in hemisphere: P_s = P + TBN·s_i·r
        vec3 sampleOffset = tbn * samples[i];
        vec3 samplePos    = viewPos + sampleOffset * radius;

        // Project to screen: uv = (Proj·P_s).xy / w * 0.5 + 0.5
        vec4 sampleClip = projection * vec4(samplePos, 1.0);
        vec3 sampleNDC  = sampleClip.xyz / sampleClip.w;
        vec2 sampleUV   = sampleNDC.xy * 0.5 + 0.5;

        if (sampleUV.x < 0.0 || sampleUV.x > 1.0 || sampleUV.y < 0.0 || sampleUV.y > 1.0)
            continue;

        float sampleDepth = texture(depthTexture, sampleUV).r;
        float sampleNDCz  = sampleDepth * 2.0 - 1.0;
        vec4  sampleView4 = invProjection * vec4(sampleNDC.xy, sampleNDCz, 1.0);
        float sampleViewZ = sampleView4.z / sampleView4.w;

        // Range check: attenuate samples beyond radius
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(viewPos.z - sampleViewZ));
        // Occluded if geometry z_s >= sample z + bias
        occlusion += (sampleViewZ >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }

    // AO = 1 - (occluded_count / N)
    occlusion = 1.0 - (occlusion / float(kernelSize));
    FragColor = occlusion;
}
