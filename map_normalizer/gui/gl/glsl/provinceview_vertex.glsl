#version 410 core

layout(location=0) in vec4 position;

uniform mat4 projection;
uniform mat4 transform;

out vec2 texture_coords;

void main() {
    texture_coords = position.zw;
    gl_Position = projection * transform * vec4(position.xy, 0, 1);
}

