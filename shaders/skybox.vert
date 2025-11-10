#version 330 core

layout (location = 0) in vec3 aPos;

out vec3 textureCoordinates;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 position = projection * view * model * vec4(aPos, 1.0f);

	// we set z = w so that after perspective divide z will be 1.0, which is max depth value
	// this keeps the skybox behind everything
	// got z-fighting for (w / w), hence the subtractio to make the depth a little less than max
	gl_Position = vec4(position.xy, position.w, position.w); 

	textureCoordinates = aPos; // treat position as a vector for cubemap texture coords
}

