#version 410 core

out vec4 FragColor; // Output color value

uniform sampler2D map_texture;

in vec2 texture_coords; // Input from vertex shader

void main() {
    // TODO: Do we want to colorize the selection? Maybe based on what the color of the province is?
    FragColor = vec4(1.0, 0.0, 0.0, 1.0) * texture(map_texture, texture_coords);
}

