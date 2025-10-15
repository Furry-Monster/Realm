#version 330 core

// Input from vertex shader
in vec3 FragPos;           // World space position
in vec3 Normal;            // World space normal
in vec2 TexCoord;          // UV coordinates
in vec3 Tangent;           // World space tangent
in vec3 Bitangent;         // World space bitangent
in vec4 VertexColor;       // Vertex color
in mat3 TBN;               // Tangent space matrix
in vec4 FragPosLightSpace; // Position in light space for shadow mapping

// Output color
out vec4 FragColor;

// Material textures
uniform sampler2D uBaseColorTexture;         // Base color texture
uniform sampler2D uMetallicRoughnessTexture; // Metallic-roughness texture
uniform sampler2D uNormalTexture;            // Normal texture
uniform sampler2D uOcclusionTexture;         // Ambient occlusion texture
uniform sampler2D uEmissiveTexture;          // Emissive texture

// Material parameters
uniform vec4  uBaseColorFactor;   // Base color factor
uniform float uMetallicFactor;    // Metallic factor
uniform float uRoughnessFactor;   // Roughness factor
uniform vec3  uEmissiveFactor;    // Emissive factor
uniform float uNormalScale;       // Normal strength
uniform float uOcclusionStrength; // Ambient occlusion strength

// Camera
uniform vec3 uCameraPos;

// Lighting data
uniform vec3  uLightDirection; // Light direction
uniform vec3  uLightColor;     // Light color
uniform float uLightIntensity; // Light intensity

// Ambient lighting
uniform vec3  uAmbientColor;
uniform float uAmbientIntensity;

// Shadow mapping
uniform sampler2D uShadowMap;  // Shadow map texture
uniform bool      uUseShadows; // Enable/disable shadows

// IBL (Image Based Lighting) - for future enhancement
uniform samplerCube uIrradianceMap; // Irradiance map for diffuse IBL
uniform samplerCube uPrefilterMap;  // Prefiltered environment map for specular IBL
uniform sampler2D   uBRDFLUT;       // BRDF lookup texture
uniform bool        uUseIBL;        // Enable/disable IBL

// Shadow calculation function
float calculateShadow(vec4 fragPosLightSpace, sampler2D shadowMap)
{
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // Check if coordinates are in shadow map bounds
    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;

    // Get closest depth value from light's perspective
    float closestDepth = texture(shadowMap, projCoords.xy).r;

    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // Calculate bias to prevent shadow acne
    float bias = 0.005;

    // Check if current frag pos is in shadow
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}

// Enhanced PBR lighting calculation function
vec3 calculatePBR(vec3  albedo,
                  float metallic,
                  float roughness,
                  vec3  normal,
                  vec3  viewDir,
                  vec3  lightDir,
                  vec3  lightColor,
                  float shadow)
{
    // Calculate half vector
    vec3 halfDir = normalize(viewDir + lightDir);

    // Calculate basic angles
    float NdotL = max(dot(normal, lightDir), 0.0);
    float NdotV = max(dot(normal, viewDir), 0.0);
    float NdotH = max(dot(normal, halfDir), 0.0);
    float VdotH = max(dot(viewDir, halfDir), 0.0);

    // Calculate F0 (base reflectivity)
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // Fresnel term (Schlick's approximation)
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - VdotH, 5.0);

    // Normal Distribution Function (GGX/Trowbridge-Reitz)
    float alpha  = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom  = NdotH * NdotH * (alpha2 - 1.0) + 1.0;
    float D      = alpha2 / (3.14159 * denom * denom);

    // Geometry Function (Smith's method)
    float k   = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    float G1L = NdotL / (NdotL * (1.0 - k) + k);
    float G1V = NdotV / (NdotV * (1.0 - k) + k);
    float G   = G1L * G1V;

    // Cook-Torrance BRDF
    vec3  numerator   = D * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.0001; // Avoid division by zero
    vec3  specular    = numerator / denominator;

    // Diffuse and specular coefficients
    vec3 kS = F;                             // Specular coefficient
    vec3 kD = (1.0 - kS) * (1.0 - metallic); // Diffuse coefficient

    // Final lighting calculation with shadow
    vec3 diffuse  = kD * albedo / 3.14159;
    vec3 lighting = (diffuse + specular) * lightColor * NdotL * (1.0 - shadow);

    return lighting;
}

// IBL (Image Based Lighting) calculation - for future enhancement
vec3 calculateIBL(vec3 albedo, float metallic, float roughness, vec3 normal, vec3 viewDir)
{
    if (!uUseIBL)
        return vec3(0.0);

    // Diffuse IBL
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 F  = F0 + (1.0 - F0) * pow(1.0 - max(dot(normal, viewDir), 0.0), 5.0);
    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - metallic);

    vec3 irradiance = texture(uIrradianceMap, normal).rgb;
    vec3 diffuse    = kD * albedo * irradiance;

    // Specular IBL
    vec3  R     = reflect(-viewDir, normal);
    float NdotV = max(dot(normal, viewDir), 0.0);

    // Sample prefiltered environment map
    const float MAX_REFLECTION_LOD = 4.0;
    vec3        prefilteredColor   = textureLod(uPrefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;

    // Sample BRDF lookup texture
    vec2 brdf     = texture(uBRDFLUT, vec2(NdotV, roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    return diffuse + specular;
}

void main()
{
    // Sample material textures
    vec4  baseColorTex         = texture(uBaseColorTexture, TexCoord);
    vec4  metallicRoughnessTex = texture(uMetallicRoughnessTexture, TexCoord);
    vec3  normalTex            = texture(uNormalTexture, TexCoord).rgb;
    float occlusionTex         = texture(uOcclusionTexture, TexCoord).r;
    vec3  emissiveTex          = texture(uEmissiveTexture, TexCoord).rgb;

    // Calculate final material parameters
    vec3  albedo    = baseColorTex.rgb * uBaseColorFactor.rgb * VertexColor.rgb;
    float metallic  = metallicRoughnessTex.b * uMetallicFactor;
    float roughness = metallicRoughnessTex.g * uRoughnessFactor;
    vec3  emissive  = emissiveTex * uEmissiveFactor;

    // Normal mapping
    vec3 normal = Normal;
    if (normalTex != vec3(0.5, 0.5, 1.0)) // Check if normal map exists
    {
        // Convert normal from [0,1] to [-1,1]
        normalTex = normalTex * 2.0 - 1.0;
        normalTex.xy *= uNormalScale;

        // Transform normal from tangent space to world space
        normal = normalize(TBN * normalTex);
    }

    // Calculate view direction
    vec3 viewDir = normalize(uCameraPos - FragPos);

    // Calculate light direction (directional light)
    vec3 lightDir = normalize(-uLightDirection);

    // Calculate shadow
    float shadow = 0.0;
    if (uUseShadows)
    {
        shadow = calculateShadow(FragPosLightSpace, uShadowMap);
    }

    // PBR lighting calculation
    vec3 lighting =
        calculatePBR(albedo, metallic, roughness, normal, viewDir, lightDir, uLightColor * uLightIntensity, shadow);

    // Ambient lighting
    vec3 ambient = uAmbientColor * uAmbientIntensity * albedo * occlusionTex * uOcclusionStrength;

    // IBL (Image Based Lighting) - for future enhancement
    vec3 ibl = calculateIBL(albedo, metallic, roughness, normal, viewDir);

    // Final color calculation
    vec3 finalColor = ambient + lighting + ibl + emissive;

    // Tone mapping (simple Reinhard)
    finalColor = finalColor / (finalColor + vec3(1.0));

    // Gamma correction
    finalColor = pow(finalColor, vec3(1.0 / 2.2));

    // Output final color
    FragColor = vec4(finalColor, uBaseColorFactor.a * baseColorTex.a);
}
