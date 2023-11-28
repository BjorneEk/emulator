/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 *
 * image representation for the raytracer and for drawing to opengl
 *
 *==========================================================*/

#include "image.h"
#include "../GL/shader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>


typedef struct gliobject {
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;
	unsigned int length;
} GL_element_object_t;

f32_t img_verts[] = {
	// positions		// texture coords
	 1.0f,  1.0f, 0.0f,	1.0f, 1.0f, // top right
	 1.0f, -1.0f, 0.0f,	1.0f, 0.0f, // bottom right
	-1.0f, -1.0f, 0.0f,	0.0f, 0.0f, // bottom left
	-1.0f,  1.0f, 0.0f,	0.0f, 1.0f  // top left
};
u32_t img_mesh[] = {
	0, 1, 3, // first triangle
	1, 2, 3  // second triangle
};

static const char *IMAGE_VERT_PATH = "src/shaders/img_vert.glsl";
static const char *IMAGE_FRAG_PATH = "src/shaders/img_frag.glsl";

extern f32_t GLOBL_DELTA_TIME;

shader_t image_shader()
{
	static shader_t shader;
	static bool shader_loaded = false;

	if (shader_loaded)
		return shader;
	else
		shader_load(&shader, IMAGE_VERT_PATH, IMAGE_FRAG_PATH);
	shader_loaded = true;
	return shader;
}


GL_element_object_t get_img_obj()
{
	static GL_element_object_t res;
	static bool has_generated = false;

	if (has_generated)
		return res;

	has_generated = true;
	glGenVertexArrays(1, &res.VAO);
	glGenBuffers(1, &res.VBO);
	glGenBuffers(1, &res.EBO);

	glBindVertexArray(res.VAO);

	glBindBuffer(GL_ARRAY_BUFFER, res.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(img_verts), img_verts, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, res.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(img_mesh), img_mesh, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32_t), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	res.length = sizeof(img_mesh);
	return res;
}

image_t *new_image(u32_t width, u32_t height, rgb_t *data)
{
	image_t *res;

	res = malloc(sizeof(image_t));

	res->height = height;
	res->width = width;
	//res->data = calloc(res->height * res->width, sizeof(rgb_t));
	res->data = data;

	glGenTextures(1, &res->GL_texture);

	glBindTexture(GL_TEXTURE_2D, res->GL_texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object

	// set the texture wrapping parameters
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, res->height, res->width, 0, GL_RGB, GL_UNSIGNED_BYTE, res->data);

	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	return res;
}

void draw_image(image_t *img)
{
	u32_t			texture;
	GL_element_object_t	obj;
	shader_t		shader;

	shader = image_shader();
	shader_use(&shader);

	//glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, img->GL_texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, img->height, img->width, 0, GL_RGB, GL_UNSIGNED_BYTE, img->data);

	glGenerateMipmap(GL_TEXTURE_2D);

	obj = get_img_obj();

	glBindTexture(GL_TEXTURE_2D, img->GL_texture);
	glBindVertexArray(obj.VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
