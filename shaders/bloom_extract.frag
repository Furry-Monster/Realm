#version 330 core

#define GREYSCALE_WEIGHT_VECTOR vec3(0.2126, 0.7152, 0.0722)

out vec4 FragColor;
in vec2  textureCoordinates;

uniform sampler2D inputColorTexture;
uniform float     bloomBrightnessCutoff;

void main()
{
    vec4  color      = texture(inputColorTexture, textureCoordinates);
    float brightness = dot(color.rgb, GREYSCALE_WEIGHT_VECTOR);
    FragColor        = brightness > bloomBrightnessCutoff ? color : vec4(0.0, 0.0, 0.0, 1.0);
}
