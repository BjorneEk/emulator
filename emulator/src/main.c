/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * emulator
 *==========================================================*/

#include "IO/io_emulator.h"
#include "IO/video_card.h"
#include "emulator.h"
#include "cpu.h"
#include "memory.h"
#include "util/types.h"
#include "image/image.h"
#include "../../arch/interface.h"
#include "../../common/util/error.h"
#include "window/window.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define FAKE_KEY_PRESS IO_SIGNAL_CUSTOM

static window_t *WINDOW;

static int CURRENT_RESOLUTION	= VC_RESOLUTION_800X600;
static int WINDOW_WIDTH		= 800;
static int WINDOW_HEIGHT	= 600;

static bool HAS_RESOLUTION_CHANGE = false;

static image_t *FRAMEBUFFER_IMAGE;

/*
static void *run_emulator(void *arg)
{
	emulator_t *em = arg;
	int res;
	do {
		res = emulator_execute(em);
	} while(res != 1);
	return NULL;
}
*/

void	vc_resolution_change_callback(video_card_t *vc, int resolution);

void text_input_callback(	window_t *window, unsigned int	codepoint);
void keypress_callback(		window_t* window, int		key,		int	scancode,	int action,	int mods);
void mouse_button_callback(	window_t* window, int		button, 	int	action,		int mods);
void mouse_motion_callback(	window_t* window, double	x_in,		double	y_in);
void scroll_callback(		window_t* window, double	x_off,		double	y_off);
void framebuffer_size_callback(	window_t* window, int		width,		int	height);

static void *run_emulator(void *arg)
{
	emulator_t *em = arg;

	int res;
	do {
		res = emulator_execute(em);
	} while(res != 1);
	return NULL;
}

static pthread_t start_emulator(emulator_t *em)
{
	pthread_t thread;
	pthread_create(&thread, NULL, &run_emulator, em);
	return thread;
}

static void	update_framebuffer(video_card_t *vc)
{
	free(FRAMEBUFFER_IMAGE->data);
	FRAMEBUFFER_IMAGE->data = vc_get_render_buffer(vc);
	// u32_t addr = (u16_t)0 << 16 | (u16_t)0;
	// BUG("color: %02X %02X %02X\n", FRAMEBUFFER_IMAGE->data[addr].red, FRAMEBUFFER_IMAGE->data[addr].green, FRAMEBUFFER_IMAGE->data[addr].blue);
}

static void	main_loop(io_t *io, video_card_t *vc)
{
	while (!window_should_close(WINDOW)) {
		if (HAS_RESOLUTION_CHANGE) {
			window_set_size(WINDOW, WINDOW_WIDTH, WINDOW_HEIGHT);
			FRAMEBUFFER_IMAGE = new_image(WINDOW_WIDTH, WINDOW_HEIGHT, vc_get_render_buffer(vc));
			HAS_RESOLUTION_CHANGE = false;
		}
		update_framebuffer(vc);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		// glClear(GL_COLOR_BUFFER_BIT);

		draw_image(FRAMEBUFFER_IMAGE);
		// printf("image width: %i\n", FRAMEBUFFER_IMAGE->width);
		// printf("image height: %i\n", FRAMEBUFFER_IMAGE->height);

		window_swap_buffers(WINDOW);
		window_poll_events();
	}
	window_destroy();
}

int main(int argc, const char *argv[])
{
	cpu_t		*cpu;
	memory_t	*mem;
	io_t		*io;
	video_card_t	*vc;
	emulator_t	*em;
	pthread_t	emulator_thread;

	window_init(&WINDOW, "Emulator", WINDOW_WIDTH, WINDOW_HEIGHT);

	cpu = new_cpu(MEMORY_SIZE - 8);
	mem = new_memory();
	io = new_io_emulator();
	vc = new_video_card(
		mem,
		ADDRESS_GRAPHICS_CARD_DATA,
		ADDRESS_GRAPHICS_CARD_ADDRESS,
		vc_resolution_change_callback);
	FRAMEBUFFER_IMAGE = new_image(WINDOW_WIDTH, WINDOW_HEIGHT, vc_get_render_buffer(vc));

	em = new_emulator(cpu, mem, io, vc);

	memory_from_file(mem, argv[1]);


	window_set_callbacks(WINDOW,
		keypress_callback,
		text_input_callback,
		mouse_button_callback,
		mouse_motion_callback,
		scroll_callback,
		framebuffer_size_callback);
	/*
	do {
		getchar();
		res = emulator_execute(em);
		emulator_debug(em);
	} while(res != 1);
	*/
	emulator_thread = start_emulator(em);
	main_loop(io, vc);


	pthread_join(emulator_thread, NULL);
}

void	vc_resolution_change_callback(video_card_t *vc, int resolution)
{
	if (CURRENT_RESOLUTION == resolution)
		return;

	switch(resolution) {
		case VC_RESOLUTION_800X600:
			WINDOW_WIDTH = 800;
			WINDOW_HEIGHT = 600;
			HAS_RESOLUTION_CHANGE = true;
			printf("resolution set: 800x600\n");
			break;
		case VC_RESOLUTION_640X480:
			WINDOW_WIDTH = 640;
			WINDOW_HEIGHT = 480;
			HAS_RESOLUTION_CHANGE = true;
			printf("resolution set: 640x480\n");
			break;
		default:
			exit_error("Unsupported resolution\n");
	}
	CURRENT_RESOLUTION = resolution;
}

void keypress_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{/* unused */}
void text_input_callback(GLFWwindow *window, unsigned int codepoint)
{/* unused */}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{/* unused */}
void mouse_motion_callback(GLFWwindow* window, double x_in, double y_in)
{/* unused */}
void scroll_callback(GLFWwindow* window, double x_off, double y_off)
{/* unused */}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and
	// height will be significantly larger than specified on retina displays.
	WINDOW_WIDTH = width;
	WINDOW_HEIGHT = height;
	//img_proj = M4_orthographic(-1.0 * WINDOW_HEIGHT / WINDOW_WIDTH, 1.0 * WINDOW_HEIGHT / WINDOW_WIDTH, -1.0, 1.0);
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}