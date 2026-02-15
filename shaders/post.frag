#version 330 core

out vec4 FragColor;
in vec2  textureCoordinates;

uniform sampler2D colorTexture;
uniform sampler2D bloomTexture;
uniform sampler2D ssaoTexture;
uniform sampler2D depthTexture;
uniform bool      ssaoEnabled;
uniform float     ssaoPower;
uniform bool      bloomEnabled;
uniform float     bloomIntensity;
uniform bool      tonemappingEnabled;
uniform float     gammaCorrectionFactor;

void main()
{
    vec3 color = texture(colorTexture, textureCoordinates).rgb;

    if (ssaoEnabled)
    {
        // Apply AO: C' = C · AO^p (p controls contrast)
        float depth = texture(depthTexture, textureCoordinates).r;
        float ao    = texture(ssaoTexture, textureCoordinates).r;
        if (depth < 1.0)
            color *= pow(ao, ssaoPower);
    }

    // bloom
    if (bloomEnabled)
    {
        vec3 bloomColor = vec3(0.0, 0.0, 0.0);
        for (int i = 0; i <= 5; i++)
        {
            bloomColor += textureLod(bloomTexture, textureCoordinates, i).rgb;
        }

        color += bloomColor * bloomIntensity;
    }

    // Reinhard: C' = C / (1 + C)
    if (tonemappingEnabled)
        color = color / (color + vec3(1.0));

    // Gamma: C' = C^(1/γ)
    color = pow(color, vec3(1.0 / gammaCorrectionFactor));

    FragColor = vec4(color, 1.0);
}
