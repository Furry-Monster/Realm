struct LightData
{
    vec4 position;    // xyz = position, w = type
    vec4 direction;   // xyz = direction, w = intensity
    vec4 color;       // rgb = color, w = constant
    vec4 attenuation; // x = linear, y = quadratic, z = range, w = inner_cone_angle
    vec4 spot_area;   // x = outer_cone_angle, y = width, z = height, w = padding
};

layout(std430, binding = 1) readonly buffer LightBuffer
{
    int       lightCount;
    LightData lights[];
};

layout(std430, binding = 4) readonly buffer LightGrid
{
    uvec2 clusterGrid[]; // x = offset, y = count
};

layout(std430, binding = 3) readonly buffer LightIndices
{
    uint globalIndexCount;
    uint lightIndices[];
};

uniform ivec3 clusterDimensions; // (16, 9, 24)

// Map screen UV + view-space depth to a cluster index
ivec3 getClusterIndex(vec2 screenUV, float viewDepth, float nearPlane, float farPlane)
{
    int x = int(screenUV.x * float(clusterDimensions.x));
    int y = int(screenUV.y * float(clusterDimensions.y));

    // Exponential depth slicing (inverse of: slice = near * pow(far/near, z/Z))
    int z = int(log(viewDepth / nearPlane) / log(farPlane / nearPlane) * float(clusterDimensions.z));

    x = clamp(x, 0, clusterDimensions.x - 1);
    y = clamp(y, 0, clusterDimensions.y - 1);
    z = clamp(z, 0, clusterDimensions.z - 1);

    return ivec3(x, y, z);
}

// Compute light direction and radiance for a single light.
// Returns false if the fragment is out of range.
bool evaluateLight(int idx, vec3 worldPos, out vec3 l, out vec3 radiance)
{
    int   lightType   = int(lights[idx].position.w);
    vec3  lightPos    = lights[idx].position.xyz;
    vec3  lightDir    = lights[idx].direction.xyz;
    float intensity   = lights[idx].direction.w;
    vec3  lightColor  = lights[idx].color.rgb;
    float lightConst  = lights[idx].color.w;
    float lightLinear = lights[idx].attenuation.x;
    float lightQuad   = lights[idx].attenuation.y;
    float lightRange  = lights[idx].attenuation.z;
    float innerCone   = lights[idx].attenuation.w;
    float outerCone   = lights[idx].spot_area.x;

    radiance = vec3(0.0);
    l        = vec3(0.0);

    // Point (0)
    if (lightType == 0)
    {
        vec3  d    = lightPos - worldPos;
        float dist = length(d);
        if (dist > lightRange)
            return false;
        l           = normalize(d);
        float atten = 1.0 / (lightConst + lightLinear * dist + lightQuad * dist * dist);
        radiance    = lightColor * intensity * atten;
    }
    // Directional (1)
    else if (lightType == 1)
    {
        l        = normalize(-lightDir);
        radiance = lightColor * intensity;
    }
    // Spot (2)
    else if (lightType == 2)
    {
        vec3  d    = lightPos - worldPos;
        float dist = length(d);
        if (dist > lightRange)
            return false;
        l = normalize(d);

        float theta  = dot(l, -normalize(lightDir));
        float innerC = cos(radians(innerCone));
        float outerC = cos(radians(outerCone));
        float spot   = clamp((theta - outerC) / (innerC - outerC), 0.0, 1.0);
        if (spot <= 0.0)
            return false;

        float atten = 1.0 / (lightConst + lightLinear * dist + lightQuad * dist * dist);
        radiance    = lightColor * intensity * atten * spot;
    }
    // Area (3)
    else if (lightType == 3)
    {
        vec3  d     = lightPos - worldPos;
        float dist  = length(d);
        l           = normalize(d);
        float atten = 1.0 / (dist * dist);
        radiance    = lightColor * intensity * atten;
    }

    return length(radiance) > 0.0;
}
