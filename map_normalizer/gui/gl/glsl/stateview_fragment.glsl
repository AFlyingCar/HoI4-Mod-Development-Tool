#version 410 core

out vec4 FragColor; // Output color value

uniform usampler2D state_id_matrix;

uniform vec3 state_color;
uniform uint state_id;

in vec2 texture_coords; // Input from vertex shader

void main() {
    uint pixel_id = texture(state_id_matrix, texture_coords).r;

    bool is_state_id = (pixel_id == state_id);
    // bool is_border = ???;

    vec3 color_array[2];
    color_array[0] = vec3(0, 0, 0);
    color_array[1] = state_color;

    FragColor = vec4(color_array[uint(is_state_id)].rgb, 1.0);
}

