#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 Tangent;
in vec3 Bitangent;
in vec4 VertexColor;
in mat3 TBN;
in vec4 FragPosLightSpace;

out vec4 FragColor;

// Material uniforms
uniform vec4  uBaseColorFactor;
uniform float uMetallicFactor;
uniform float uRoughnessFactor;
uniform vec3  uEmissiveFactor;

// Camera uniforms
uniform vec3 uCameraPos;

// Lighting uniforms
uniform vec3  uLightDirection;
uniform vec3  uLightColor;
uniform float uLightIntensity;
uniform vec3  uAmbientColor;
uniform float uAmbientIntensity;

void main()
{
    // Simple PBR calculation
    vec3  albedo    = uBaseColorFactor.rgb * VertexColor.rgb;
    float metallic  = uMetallicFactor;
    float roughness = uRoughnessFactor;

    // Calculate view direction
    vec3 viewDir = normalize(uCameraPos - FragPos);

    // Calculate light direction
    vec3 lightDir = normalize(-uLightDirection);

    // Calculate normal
    vec3 normal = normalize(Normal);

    // Simple diffuse lighting
    float NdotL   = max(dot(normal, lightDir), 0.0);
    vec3  diffuse = albedo * uLightColor * uLightIntensity * NdotL;

    // Simple ambient lighting
    vec3 ambient = uAmbientColor * uAmbientIntensity * albedo;

    // Combine lighting
    vec3 finalColor = ambient + diffuse;

    // Simple tone mapping
    finalColor = finalColor / (finalColor + vec3(1.0));

    // Gamma correction
    finalColor = pow(finalColor, vec3(1.0 / 2.2));

    FragColor = vec4(finalColor, uBaseColorFactor.a);
}
