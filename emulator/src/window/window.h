/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 *
 * GLFW window context
 *
 *==========================================================*/

#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>

typedef GLFWwindow window_t;

typedef void (*keypress_callback_t)(		window_t *, int, int, int, int);
typedef void (*text_input_callback_t)(		window_t *, unsigned int);
typedef void (*mouse_button_callback_t)(	window_t *, int, int, int);
typedef void (*mouse_motion_callback_t)(	window_t *, double, double);
typedef void (*scroll_callback_t)(		window_t *, double, double);
typedef void (*framebuffer_size_callback_t)(	window_t *, int, int);



void window_init(window_t **window, const char *title, unsigned int width, unsigned int height);

#define window_destroy 		glfwTerminate
#define window_should_close	glfwWindowShouldClose
#define window_swap_buffers	glfwSwapBuffers
#define window_poll_events	glfwPollEvents

/* set calback functions for window event handling */
void window_set_callbacks(
	window_t			*window,
	keypress_callback_t		kpc,
	text_input_callback_t		tic,
	mouse_button_callback_t		mbc,
	mouse_motion_callback_t		mmc,
	scroll_callback_t		sc,
	framebuffer_size_callback_t	fsc);


#endif /* _WINDOW_H_ */