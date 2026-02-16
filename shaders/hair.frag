#version 330 core

#define PI 3.1415926535897932384626433832795

layout(location = 0) out vec4 FragColor;

in vec2 textureCoordinates;
in vec3 worldCoordinates;
in vec3 tangent;
in vec3 bitangent;
in vec3 normal;

struct HairMaterial
{
    bool useTextureAlbedo;
    bool useTextureOpacity;
    bool useTextureEmissive;
    vec3 albedo;
    float opacity;
    vec3 emissive;
    float emissiveStrength;
    float specularStrength;
    float specularPower;

    sampler2D textureAlbedo;
    sampler2D textureOpacity;
    sampler2D textureEmissive;
};

struct LightData
{
    vec4 position;
    vec4 direction;
    vec4 color;
    vec4 attenuation;
    vec4 spot_area;
};

uniform HairMaterial material;
uniform vec3 cameraPosition;

layout(std140) uniform LightBlock
{
    int lightCount;
    LightData lights[16];
};

uniform sampler2D shadowMap;
uniform bool shadowEnabled;
uniform mat4 lightSpaceMatrix;
uniform samplerCube diffuseIrradianceMap;

uniform int displayMode;

// Kajiya-Kay diffuse: Kd * sqrt(1 - (T·L)^2)
float kajiyaKayDiffuse(vec3 T, vec3 L)
{
    float TdotL = dot(T, L);
    return max(0.0, sqrt(max(0.0, 1.0 - TdotL * TdotL)));
}

// Kajiya-Kay specular: Ks * pow(max(0, T·H), p)
float kajiyaKaySpecular(vec3 T, vec3 H, float power) {
    return pow(max(0.0, dot(T, H)), power);
}

float calcShadow(vec4 fragPosLightSpace)
{
    if (!shadowEnabled)
        return 1.0;

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0)
        return 1.0;

    float currentDepth = projCoords.z;
    float bias = 0.005;
    float shadow = currentDepth - bias > texture(shadowMap, projCoords.xy).r ? 1.0 : 0.0;
    return 1.0 - shadow;
}

void main()
{
    vec3 albedo = material.albedo;
    vec4 albedo_sample = vec4(albedo, 1.0);
    if (material.useTextureAlbedo)
    {
        albedo_sample = texture(material.textureAlbedo, textureCoordinates);
        albedo = albedo_sample.rgb;
    }

    float alpha = material.opacity;
    if (material.useTextureOpacity)
        alpha *= texture(material.textureOpacity, textureCoordinates).r;
    else if (material.useTextureAlbedo)
        alpha *= albedo_sample.a;
    if (alpha < 0.01)
        discard;

    vec3 emissive = material.emissive;
    if (material.useTextureEmissive)
        emissive = texture(material.textureEmissive, textureCoordinates).rgb;
    emissive *= material.emissiveStrength;

    if (displayMode == 1)
    {
        FragColor = vec4(albedo, alpha);
        return;
    }
    if (displayMode == 6)
    {
        FragColor = vec4(emissive, alpha);
        return;
    }

    vec3 v = normalize(cameraPosition - worldCoordinates);
    vec3 T = tangent;
    if (length(T) < 0.01)
        T = normal;
    T = normalize(T);

    vec3 Lo = vec3(0.0);

    for (int i = 0; i < lightCount && i < 16; ++i)
    {
        LightData l = lights[i];
        int typ = int(l.position.w);

        vec3 lightPos = l.position.xyz;
        vec3 lightDir = normalize(l.direction.xyz);
        vec3 lightColor = l.color.rgb;
        float intensity = l.direction.w;
        float lightConst = l.color.w;

        vec3 L;
        float attenuation = 1.0;

        if (typ == 0)
        {
            vec3 toLight = lightPos - worldCoordinates;
            float dist = length(toLight);
            L = normalize(toLight);
            float range = l.attenuation.z;
            if (dist > range)
                continue;
            attenuation = 1.0 / (lightConst + l.attenuation.x * dist + l.attenuation.y * dist * dist);
        }
        else if (typ == 1)
        {
            L = -lightDir;
        }
        else if (typ == 2)
        {
            vec3 toLight = lightPos - worldCoordinates;
            float dist = length(toLight);
            L = normalize(toLight);
            float range = l.attenuation.z;
            if (dist > range)
                continue;
            attenuation = 1.0 / (lightConst + l.attenuation.x * dist + l.attenuation.y * dist * dist);
            float theta = dot(L, -lightDir);
            float outerCos = cos(radians(l.spot_area.x));
            float innerCos = cos(radians(l.attenuation.w));
            if (theta < outerCos)
                continue;
            attenuation *= clamp((theta - outerCos) / (innerCos - outerCos), 0.0, 1.0);
        }
        else
        {
            continue;
        }

        vec3 H = normalize(L + v);

        float diffuse = kajiyaKayDiffuse(T, L);
        float specular = kajiyaKaySpecular(T, H, material.specularPower);

        vec3 radiance = lightColor * intensity * attenuation *
                (albedo * diffuse + material.specularStrength * specular * vec3(1.0));

        // Shadow map is only valid for directional light (type 1)
        float shadow = 1.0;
        if (typ == 1)
        {
            vec4 fragPosLightSpace = lightSpaceMatrix * vec4(worldCoordinates, 1.0);
            shadow = calcShadow(fragPosLightSpace);
        }
        Lo += radiance * shadow;
    }

    vec3 irradiance = texture(diffuseIrradianceMap, normal).rgb;
    vec3 ambient = irradiance * albedo;
    vec3 color = emissive + ambient + Lo;

    FragColor = vec4(color, alpha);
}
