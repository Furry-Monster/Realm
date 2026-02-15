#version 330 core

out float FragColor;

in vec2 textureCoordinates;

uniform sampler2D ssaoTexture;
uniform sampler2D depthTexture;
uniform vec2      texelSize;

// Bilateral: w = w_d · w_s, AO_out = Σ(AO_i · w_i) / Σ w_i
// w_d = exp(-|d_c - d_s|·k), w_s = exp(-||r||²/(2σ²))
void main()
{
    float centerDepth = texture(depthTexture, textureCoordinates).r;
    float centerAO    = texture(ssaoTexture, textureCoordinates).r;

    float totalWeight = 0.0;
    float totalAO     = 0.0;

    for (int x = -2; x <= 2; ++x)
    {
        for (int y = -2; y <= 2; ++y)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            vec2 uv     = textureCoordinates + offset;

            float sampleDepth = texture(depthTexture, uv).r;
            float sampleAO    = texture(ssaoTexture, uv).r;

            float depthDiff   = abs(centerDepth - sampleDepth);
            float depthWeight = exp(-depthDiff * 10.0);

            float dist          = length(vec2(x, y));
            float spatialWeight = exp(-dist * dist / 2.0);

            float weight = depthWeight * spatialWeight;
            totalWeight += weight;
            totalAO += sampleAO * weight;
        }
    }

    FragColor = totalWeight > 0.0 ? totalAO / totalWeight : centerAO;
}
