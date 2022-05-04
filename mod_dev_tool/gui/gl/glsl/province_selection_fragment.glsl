#version 410 core

out vec4 FragColor; // Output color value

uniform sampler2D selection;
uniform usampler2D label_matrix;

uniform uint province_labels[MAX_SELECTED_PROVINCES];
uniform uint num_selected; // Will be no larger than MAX_SELECTED_PROVINCES

// The color that the selection will appear rendered as
uniform vec3 selection_color;

in vec2 texture_coords; // Input from vertex shader

/**
 * @brief Checks if the given label is selected. Only iterates up to num_selected.
 */
bool isSelected(uint pixel_label) {
    for(uint i = 0; i < num_selected; ++i) {
        if(province_labels[i] == pixel_label) {
            return true;
        }
    }

    return false;
}

void main() {
    vec4 sel_color1 = texture(selection, texture_coords * 16); // TODO: This should either be a constant, or passed in via uniform

    uint pixel_label = texture(label_matrix, texture_coords).r;

    // Only draw if the label for this fragment/pixel matches the one that is currently selected
    // float alpha = uint(pixel_label == province_label);
    float alpha = uint(isSelected(pixel_label));

    FragColor = sel_color1 * vec4(selection_color.rgb, alpha);
}

