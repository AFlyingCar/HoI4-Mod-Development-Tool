#version 410 core

out vec4 FragColor; // Output color value

uniform sampler2D selection_area;
uniform sampler2D selection;

in vec2 texture_coords; // Input from vertex shader

void main() {
    // TODO: Do we want to colorize the selection? Maybe based on what the color of the province is?

    FragColor = vec4(1.0, 0.0, 0.0, 1.0) * texture(selection, texture_coords) * vec4(1, 1, 1, texture(selection_area, texture_coords).a);
}

