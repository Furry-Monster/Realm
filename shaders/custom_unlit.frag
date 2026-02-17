#version 450 core

// Example custom unlit shader.
// Renders mesh with a flat color, no lighting. Demonstrates how to use
// engine-provided uniforms and custom material parameters.
//
// Engine sets material uniforms via GLSL struct "material.*".
// To use them, declare the same struct as in pbr.frag (partial is OK --
// OpenGL silently ignores uniforms you don't declare).

in vec2 texCoords;
in vec3 worldPos;
in vec3 normal;

out vec4 FragColor;

// Engine-provided (optional to declare)
uniform vec3 cameraPosition;

// Partial PBR material struct -- engine sets all fields, but we only
// need to declare the ones we actually read.
struct Material
{
    vec3 albedo;
    float opacity;
    bool useTextureAlbedo;
    sampler2D textureAlbedo;
};
uniform Material material;

// Custom material parameters (defined per-material in editor)
uniform vec3 baseColor; // Color3 param
uniform float brightness; // Float  param

void main()
{
    // Use baseColor if set, otherwise fall back to material albedo
    vec3 color = baseColor;
    if (material.useTextureAlbedo)
        color *= texture(material.textureAlbedo, texCoords).rgb;

    color *= max(brightness, 0.0);
    FragColor = vec4(color, material.opacity);
}
