uniform sampler2D shadowMap;
uniform bool shadowEnabled;
uniform mat4 lightSpaceMatrix;

// Poisson disk 16-tap sampling
const vec2 poissonDisk[16] = vec2[](
    vec2(-0.613392,  0.617481), vec2( 0.170019, -0.040254),
    vec2(-0.299417,  0.791925), vec2( 0.645680,  0.493210),
    vec2(-0.651784,  0.717887), vec2( 0.421003,  0.027070),
    vec2(-0.817194, -0.271096), vec2(-0.705374, -0.668203),
    vec2( 0.977050, -0.108615), vec2( 0.063326,  0.142369),
    vec2( 0.203528,  0.214331), vec2(-0.667531,  0.326090),
    vec2(-0.098422, -0.295755), vec2(-0.885922,  0.215369),
    vec2( 0.566637,  0.605213), vec2( 0.039766, -0.396100)
);

vec2 rotatePoissonSample(vec2 v, float angle)
{
    float s = sin(angle);
    float c = cos(angle);
    return mat2(c, -s, s, c) * v;
}

// PCF shadow with Poisson disk sampling
float calculateShadow(vec4 fragPosLS, vec3 n, vec3 l)
{
    if (!shadowEnabled)
        return 1.0;

    vec3 proj = fragPosLS.xyz / fragPosLS.w;
    proj = proj * 0.5 + 0.5;

    if (proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0 || proj.z > 1.0)
        return 1.0;

    float depth = proj.z;
    float bias  = max(0.05 * (1.0 - dot(n, l)), 0.005);

    float shadow = 0.0;
    vec2  texel  = 1.0 / textureSize(shadowMap, 0);

    float angle = fract(dot(proj.xy, vec2(12.9898, 78.233)) * 43758.5453) * 6.28318;

    for (int i = 0; i < 16; i++)
    {
        vec2  off    = rotatePoissonSample(poissonDisk[i], angle) * texel;
        float pcf    = texture(shadowMap, proj.xy + off).r;
        shadow += depth - bias > pcf ? 1.0 : 0.0;
    }

    return 1.0 - shadow / 16.0;
}
