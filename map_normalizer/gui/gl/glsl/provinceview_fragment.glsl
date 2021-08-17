#version 410 core

out vec4 FragColor; // Output color value

uniform sampler2D map_texture;

in vec2 texture_coords; // Input from vertex shader

void main() {
    FragColor = texture(map_texture, texture_coords);
}

