#version 450

layout (binding = 0) uniform UniformBufferObject {
	mat4 proj;
	vec3 dirLight;
} ubo;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout (location = 0) out vec3 fragColor;

layout (push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

const float AMBIENT = 0.02;

void main() {
	gl_Position = ubo.proj * push.modelMatrix * vec4(inPosition, 1.0);
	fragColor = inColor;
}