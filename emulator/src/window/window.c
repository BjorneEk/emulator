/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 *
 * GLFW window context
 *
 *==========================================================*/


#include "window.h"
#include <stdio.h>
#include <stdlib.h>



void window_init(window_t **window, const char *title, unsigned int width, unsigned int height)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GL_TRUE);
#endif
	*window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to create GLFW window\n");
		glfwTerminate();
		exit(-1);
	}
	glfwMakeContextCurrent(*window);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		printf("Failed to initialize GLAD\n");
		exit(-1);
	}
}

/* set calback functions for window event handling */
void window_set_callbacks(
	window_t			*window,
	keypress_callback_t		kpc,
	text_input_callback_t		tic,
	mouse_button_callback_t		mbc,
	mouse_motion_callback_t		mmc,
	scroll_callback_t		sc,
	framebuffer_size_callback_t	fsc)
{
	glfwSetKeyCallback(window, kpc);
	glfwSetCharCallback(window, tic);
	glfwSetMouseButtonCallback(window, mbc);
	glfwSetCursorPosCallback(window, mmc);
	glfwSetScrollCallback(window, sc);
	glfwSetFramebufferSizeCallback(window, fsc);
}