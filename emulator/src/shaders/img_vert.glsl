#version 330 core
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec2 in_texture_coord;

out vec3 color;
out vec2 texture_coord;

uniform mat4 projection;

void main()
{
	gl_Position = vec4(in_pos, 1.0);
	texture_coord = vec2(in_texture_coord.x, in_texture_coord.y);
}