#version 410

layout (std140) uniform Matrices {
	mat4 projModelViewMatrix;
	mat3 normalMatrix;
};

layout(location=0) in uvec2 position;

out vec2 texture_coords;

void main()
{
	texture_cords = /* calculate from position */
	gl_Position = projModelViewMatrix * uvec4(position, 0, 1);
}

