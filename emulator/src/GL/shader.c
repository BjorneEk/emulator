/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 *
 * shader functions
 *
 *==========================================================*/

#include "shader.h"
#include "../../../common/util/error.h"
#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>



static const char *shader_type_string(u32_t t)
{
	switch(t) {
		case GL_VERTEX_SHADER:		return "vertex";
		case GL_FRAGMENT_SHADER:	return "fragment";
		default: return "invalid";
	}
}

u32_t create_shader(const char *shader_src, u32_t shader_type)
{
	i32_t	success;
	u32_t	result;
	char	info_log[512];

	result = glCreateShader(shader_type);
	glShaderSource(result, 1, &shader_src, NULL);
	glCompileShader(result);
	glGetShaderiv(result, GL_COMPILE_STATUS, &success);

	if (success)
		return result;

	glGetShaderInfoLog(result, 512, NULL, info_log);
	exit_error("loading %s shader:\n%s",
		shader_type_string(shader_type), info_log);
}

void shader_new(shader_t * self, const char *vert_src, const char *frag_src)
{
	i32_t	success;
	char	info_log[512];
	u32_t	vert_shader;
	u32_t	frag_shader;


	vert_shader = create_shader(vert_src, GL_VERTEX_SHADER);
	frag_shader = create_shader(frag_src, GL_FRAGMENT_SHADER);

	*self = glCreateProgram();
	glAttachShader(*self, vert_shader);
	glAttachShader(*self, frag_shader);
	glLinkProgram(*self);
	glGetProgramiv(*self, GL_LINK_STATUS, &success);

	if(!success) {
		glGetProgramInfoLog(*self, 512, NULL, info_log);
		exit_error("linking shader:\n%s\n", info_log);
	}

	glDeleteShader(vert_shader);
	glDeleteShader(frag_shader);
}

void shader_use(shader_t * self)
{
	glUseProgram(*self);
}

u32_t shader_get_uniform(shader_t * self, const char * name)
{
	return glGetUniformLocation(*self, name);
}
void shader_setb(shader_t * self, const char *name, bool value)
{
	glUniform1i(glGetUniformLocation(*self, name), (int)value);
}
void shader_seti(shader_t * self, const char *name, int value)
{
	glUniform1i(glGetUniformLocation(*self, name), value);
}
void shader_setf(shader_t * self, const char *name, float value)
{
	glUniform1f(glGetUniformLocation(*self, name), value);
}
