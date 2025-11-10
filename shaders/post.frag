#version 330 core

out vec4 FragColor;
in vec2 textureCoordinates;

uniform sampler2D colorTexture;
uniform sampler2D bloomTexture;
uniform bool bloomEnabled;
uniform float bloomIntensity;
uniform bool tonemappingEnabled;
uniform float gammaCorrectionFactor;

void main() {
	vec3 color = texture(colorTexture, textureCoordinates).rgb;

	// bloom
	if (bloomEnabled) {
		vec3 bloomColor = vec3(0.0, 0.0, 0.0);
		for (int i = 0; i <= 5; i++) {
			bloomColor += textureLod(bloomTexture, textureCoordinates, i).rgb;
		}

		color += bloomColor * bloomIntensity;
	}

	// tonemapping
	if (tonemappingEnabled) {
		// apply Reinhard tonemapping C = C / (1 + C)
		color = color / (color + vec3(1.0));
	}

	// gamma correction
	color = pow(color, vec3(1.0 / gammaCorrectionFactor)); // gamma correction to account for monitor, raise to the (1 / 2.2)

	FragColor = vec4(color, 1.0);
}

