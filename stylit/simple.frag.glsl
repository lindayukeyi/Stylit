#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 fs_uv;

uniform sampler2D texture1;

void main() {
	vec3 lightPos = vec3(10.0, 10.0, -2.0);
	float ambient = 0.2f;
	float specular = 0.0f;

	FragColor = texture2D(texture1, fs_uv);// +ambient;
}
