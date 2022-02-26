#version 410 core

out vec4 FragColor; // Output color value

uniform sampler2D selection;
uniform usampler2D label_matrix;

uniform uint province_label;

in vec2 texture_coords; // Input from vertex shader

void main() {
    vec4 sel_color1 = texture(selection, texture_coords * 16); // TODO: This should either be a constant, or passed in via uniform

    uint pixel_label = texture(label_matrix, texture_coords).r;

    // Only draw if the label for this fragment/pixel matches the one that is currently selected
    float alpha = uint(pixel_label == province_label);

    FragColor = sel_color1 * vec4(1, 0, 0, alpha);
}

