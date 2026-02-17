#version 450 core

out vec4 FragColor;
in vec2  textureCoordinates;

uniform sampler2D colorTexture;
uniform sampler2D bloomTexture;
uniform sampler2D ssaoTexture;
uniform sampler2D depthTexture;
uniform bool      ssaoEnabled;
uniform float     ssaoPower;
uniform float     ssaoIntensity;
uniform bool      bloomEnabled;
uniform float     bloomIntensity;
uniform int       bloomMaxMip;
uniform bool      tonemappingEnabled;
uniform float     gammaCorrectionFactor;

// 0=lit, 7=ssao, 8=depth (modes 1-6 from geometry pass)
uniform int displayMode;

void main()
{
    if (displayMode == 7)
    {
        float ao = texture(ssaoTexture, textureCoordinates).r;
        FragColor = vec4(vec3(ao), 1.0);
        return;
    }
    if (displayMode == 8)
    {
        float d = texture(depthTexture, textureCoordinates).r;
        FragColor = vec4(vec3(d), 1.0);
        return;
    }

    vec3 color = texture(colorTexture, textureCoordinates).rgb;

    if (ssaoEnabled)
    {
        // AO factor: mix(1, ao^power, intensity) to avoid over-darkening
        float depth = texture(depthTexture, textureCoordinates).r;
        float ao    = texture(ssaoTexture, textureCoordinates).r;
        if (depth < 1.0)
        {
            float aoFactor = mix(1.0, pow(ao, ssaoPower), ssaoIntensity);
            color *= aoFactor;
        }
    }

    if (bloomEnabled)
    {
        vec3 bloomColor = vec3(0.0, 0.0, 0.0);
        for (int i = 0; i <= bloomMaxMip; i++)
            bloomColor += textureLod(bloomTexture, textureCoordinates, i).rgb;
        color += bloomColor * bloomIntensity;
    }

    // Reinhard: C' = C / (1 + C)
    if (tonemappingEnabled)
        color = color / (color + vec3(1.0));

    // Gamma: C' = C^(1/γ)
    color = pow(color, vec3(1.0 / gammaCorrectionFactor));

    FragColor = vec4(color, 1.0);
}
