#version 450 core

out vec4 FragColor;
in vec3  modelCoordinates;

// equirectangular projection HDRI
uniform sampler2D hdri;

const vec2 inverseAtan = vec2(0.1591, 0.3183);
// Maps a 3D direction (cartesian) to equirectangular UV coordinates
vec2 cartesianToSpherical(vec3 v)
{
    vec2 xy = vec2(atan(v.z, v.x), asin(v.y));
    xy *= inverseAtan;
    xy += 0.5;
    return xy;
}

void main()
{
    vec3 sampleDirection = normalize(modelCoordinates);
    vec2 uv              = cartesianToSpherical(sampleDirection);
    vec3 color           = texture(hdri, uv).rgb;

    FragColor = vec4(color, 1.0);
}
