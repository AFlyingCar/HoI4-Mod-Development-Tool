#version 410 core

out vec4 FragColor; // Output color value

uniform sampler2D selection_area;
uniform sampler2D selection;

in vec2 texture_coords; // Input from vertex shader

void main() {
    vec4 sel_color1 = texture(selection, texture_coords);

    FragColor = vec4(1, 0, 0, texture(selection_area, texture_coords).a) * sel_color1;
}

