#version 450 core

out float FragColor;

in vec2 textureCoordinates;

uniform sampler2D gtaoTexture;
uniform sampler2D depthTexture;
uniform vec2      texelSize;

void main()
{
    float centerDepth = texture(depthTexture, textureCoordinates).r;
    float centerAO    = texture(gtaoTexture, textureCoordinates).r;

    float totalWeight = 0.0;
    float totalAO     = 0.0;

    // 5x5 bilateral blur: spatial gaussian + depth-aware edge preservation
    for (int x = -2; x <= 2; ++x)
    {
        for (int y = -2; y <= 2; ++y)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            vec2 uv     = textureCoordinates + offset;

            float sampleDepth = texture(depthTexture, uv).r;
            float sampleAO    = texture(gtaoTexture, uv).r;

            // w_depth = exp(-|d_c - d_s| * k)
            float depthDiff   = abs(centerDepth - sampleDepth);
            float depthWeight = exp(-depthDiff * 10.0);

            // w_spatial = exp(-||offset||^2 / (2 * sigma^2))
            float dist          = length(vec2(x, y));
            float spatialWeight = exp(-dist * dist / 2.0);

            float weight = depthWeight * spatialWeight;
            totalWeight += weight;
            totalAO += sampleAO * weight;
        }
    }

    FragColor = totalWeight > 0.0 ? totalAO / totalWeight : centerAO;
}
