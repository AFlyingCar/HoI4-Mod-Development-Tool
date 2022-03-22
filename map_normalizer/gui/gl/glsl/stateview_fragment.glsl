#version 410 core

out vec4 FragColor; // Output color value

uniform usampler2D state_id_matrix;

uniform vec3 state_color;
uniform uint state_id;

// This is necessary for dFd*Exact to work
uniform ivec2 tex_dimensions;

in vec2 texture_coords; // Input from vertex shader

// edge-detection code adapted from both:
//   https://blog.ruofeidu.com/simplest-fatest-glsl-edge-detection-using-fwidth/
//   https://stackoverflow.com/a/43982907
// We need to define our own *Exact dFdx, dFdy, and fwidth functions because
//   (as explained in the stackoverflow link), most GPUs compute the derivatives
//   on blocks of 2x2 pixels, rather than on individual pixels. While this is
//   nice for speed, it does mean that for what we're trying to do here we get
//   some pretty bad gaps and bad-looking borders. So we define our own here
//   instead

float dFdxExact(float p) {
    float step_u = 1.0 / tex_dimensions.x;

    uint right = texture(state_id_matrix, texture_coords + vec2(step_u, 0.0)).r;

    return length(right - p) / step_u;
}

float dFdyExact(float p) {
    float step_v = 1.0 / tex_dimensions.y;

    uint bottom = texture(state_id_matrix, texture_coords + vec2(0.0, step_v)).r;

    return length(bottom - p) / step_v;
}

float fwidthExact(float p) {
    return abs(dFdxExact(p)) + abs(dFdyExact(p));
}

void main() {
    uint pixel_id = texture(state_id_matrix, texture_coords).r;

    float edge_strength = length(fwidthExact(texture(state_id_matrix, texture_coords).r));

    bool is_state_id = (pixel_id == state_id);

    // Each state is a solid color, so it should be safe to just check != 0
    // TODO: The borders seem a bit thin imo, we might want to look into a way
    //  to increase the thickness of the borders
    bool is_border = edge_strength != 0;

    vec4 color_array[2];
    color_array[0] = vec4(0, 0, 0, 0);
    color_array[1] = vec4(state_color, 1.0);

    // Commented out code allows us to view just the border. Leaving here in
    //   case we want a debug utility to switch to viewing _only_ the borders or
    //   not.
    // FragColor = vec4(vec3(edge_strength), 1.0);
    FragColor = color_array[uint(is_state_id && !is_border)];
}

