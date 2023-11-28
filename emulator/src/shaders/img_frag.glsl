#version 330 core

out vec4 out_color;

in vec3 color;
in vec2 texture_coord;

// texture sampler
uniform sampler2D texture1;

void main()
{
	out_color = texture(texture1, texture_coord);
}