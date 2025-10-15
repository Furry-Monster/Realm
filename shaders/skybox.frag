#version 330 core

// Input from vertex shader
in vec3 TexCoord;

// Output color
out vec4 FragColor;

// Skybox texture
uniform samplerCube uSkyboxTexture;

// Optional: exposure control for HDR skyboxes
uniform float uExposure;

void main()
{
    // Sample skybox texture
    vec3 color = texture(uSkyboxTexture, TexCoord).rgb;

    // Apply exposure (for HDR skyboxes)
    if (uExposure > 0.0)
    {
        color = vec3(1.0) - exp(-color * uExposure);
    }

    // Output final color
    FragColor = vec4(color, 1.0);
}
