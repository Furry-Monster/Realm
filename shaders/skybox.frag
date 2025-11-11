#version 330 core

#define GREYSCALE_WEIGHT_VECTOR vec3(0.2126, 0.7152, 0.0722)

layout(location = 0) out vec4 FragColor;  // regular output
layout(location = 1) out vec4 BloomColor; // output to be used by bloom shader

in vec3 textureCoordinates;

uniform samplerCube skybox;

// Post parameters
uniform float bloomBrightnessCutoff;

void main()
{
    FragColor = texture(skybox, textureCoordinates); // for samplerCube the coordinates are a vector

    // bloom color output
    // use greyscale conversion here because not all colors are equally "bright"
    float greyscaleBrightness = dot(FragColor.rgb, GREYSCALE_WEIGHT_VECTOR);
    BloomColor                = greyscaleBrightness > bloomBrightnessCutoff ? FragColor : vec4(0.0, 0.0, 0.0, 1.0);
}
