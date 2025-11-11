#version 330 core

out vec4 FragColor;
in vec3  modelCoordinates;

uniform samplerCube environmentCubemap;

const float PI = 3.14159265359;
const vec3  up = vec3(0.0, 0.0, 1.0);

void main()
{
    vec3 normal    = normalize(modelCoordinates);
    vec3 tangent   = normalize(cross(up, normal));
    vec3 bitangent = normalize(cross(normal, tangent));

    vec3 irradiance = vec3(0.0);

    // integrate the incoming light from the environment cubemap
    // over the hemisphere centered around our sample direction
    // this uses spherical coordinates phi/theta

    float numSamples = 0.0;
    float delta      = 0.025; // radians

    for (float phi = 0.0; phi < 2.0 * PI; phi += delta)
    { // 360 degrees around
        for (float theta = 0.0; theta < PI / 2.0; theta += delta)
        { // 90 degrees up/down

            vec3 sampleDirectionTangent = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

            vec3 sampleDirectionWorld = sampleDirectionTangent.x * tangent + sampleDirectionTangent.y * bitangent +
                                        sampleDirectionTangent.z * normal;

            vec3 sampleValue = texture(environmentCubemap, sampleDirectionWorld).rgb;

            // compensate for the fact that angles at the base of the hemisphere
            // have bigger deltaArea
            sampleValue *= sin(theta);

            // light rays more aligned with the normal are stronger (think lambertion diffuse)
            sampleValue *= cos(theta);

            irradiance += sampleValue;
            numSamples++;
        }
    }

    irradiance = PI * irradiance * (1.0 / numSamples);
    FragColor  = vec4(irradiance, 1.0);
}
