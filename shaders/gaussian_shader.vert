#version 450

layout (binding = 0) uniform UniformBufferObject {
	mat4 proj;
	vec3 dirLight;
} ubo;

layout (location = 4) in vec3 inPosition;
layout (location = 5) in vec3 normal;
layout (location = 6) in vec4 color;
layout (location = 18) in float opacity;
layout (location = 19) in vec3 scale;
layout (location = 20) in vec3 rotation;

layout (location = 0) out vec3 fragColor;

layout (push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

const float AMBIENT = 0.02;

void main() {
	gl_Position = ubo.proj * push.modelMatrix * vec4(inPosition, 1.0);
	fragColor = vec3(color.x, color.y, color.z);
}