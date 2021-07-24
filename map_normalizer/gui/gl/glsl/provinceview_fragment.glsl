#version 410 core

out vec4 FragColor; // Output color value

uniform sampler2D map_texture;

in vec2 texture_coords; // Input from vertex shader

void main() {
    FragColor = vec4(texture_coords, 0, 1); // texture(map_texture, texture_coords);
}

