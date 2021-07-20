#version 410

layout(location=0) in vec4 position;

uniform in mat4 transform;

out vec2 texture_coords;

void main() {
	texture_cords = position.zw;
	gl_Position = transform * vec4(position.xy, 0, 1);
}

