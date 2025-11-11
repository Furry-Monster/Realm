#version 330 core

out vec4 FragColor;
in vec2  textureCoordinates;

// first weight is for the central pixel, the remaining are on either side of it
// meaning we are working with a 9x9 kernel
const float gaussianBlurWeights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

uniform sampler2D inputColorTexture;
uniform int       sampleMipLevel;
uniform vec2      blurDirection;

void main()
{
    // size of 1 pixel in [0-1] coordinates
    vec2 texelSize = 1.0 / textureSize(inputColorTexture, sampleMipLevel);

    vec3 result = vec3(0.0, 0.0, 0.0);

    // center pixel
    result += textureLod(inputColorTexture, textureCoordinates, sampleMipLevel).rgb * gaussianBlurWeights[0];

    for (int i = 1; i < 5; i++)
    {
        vec2 sampleOffset = vec2(texelSize.x * i * blurDirection.x, texelSize.y * i * blurDirection.y);

        result += texture(inputColorTexture, textureCoordinates + sampleOffset).rgb * gaussianBlurWeights[i];
        result += texture(inputColorTexture, textureCoordinates - sampleOffset).rgb * gaussianBlurWeights[i];
    }

    FragColor = vec4(result, 1.0);
}
