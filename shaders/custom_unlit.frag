#version 330 core

// Example custom unlit shader.
// Renders mesh with a flat color, no lighting. Demonstrates how to use
// engine-provided uniforms and custom material parameters.

in vec2 texCoords;
in vec3 worldPos;
in vec3 normal;

out vec4 FragColor;

// Engine-provided uniforms (available but optional to use)
uniform vec3 cameraPosition;

// Standard material uniforms (set by engine, optional to use)
uniform vec3 material_albedo; // from RenderMaterial::albedo
uniform float material_opacity; // from RenderMaterial::opacity

// Custom material parameters (defined per-material in editor)
uniform vec3 baseColor; // custom color param
uniform float brightness; // custom brightness multiplier

void main()
{
    vec3 color = baseColor * max(brightness, 0.0);
    FragColor = vec4(color, material_opacity);
}
