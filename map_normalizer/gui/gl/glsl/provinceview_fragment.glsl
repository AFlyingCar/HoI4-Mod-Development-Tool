
#version 410

uniform sampler2D map_texture;

in vec2 texture_coords;

void main() {
    FragColor = texture(map_texture, texture_coords);
}

