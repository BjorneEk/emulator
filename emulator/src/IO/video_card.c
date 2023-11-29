/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * video card emulator
 *==========================================================*/

#include "video_card.h"

#include <stdlib.h>
#include <string.h>

#define DO_LOCKED(_lock, _stmt) do {		\
	pthread_mutex_lock((_lock));		\
	_stmt;					\
	pthread_mutex_unlock((_lock));		\
	} while(0)
#define MONITOR(_lock, _fn) {			\
	pthread_mutex_lock((_lock));		\
	_fn;					\
	pthread_mutex_unlock((_lock));		\
	}

vc_color_t	*new_vc_frame_buffer(int resolution)
{
	return calloc(vc_resolution_height(resolution)*vc_resolution_width(resolution), sizeof(vc_color_t));
}

vc_color_t	*copy_vc_framebuffer(vc_color_t *fb, int resolution)
{
	vc_color_t *res;

	res = new_vc_frame_buffer(resolution);
	memmove(res, fb, vc_resolution_height(resolution)*vc_resolution_width(resolution)*sizeof(vc_color_t));
	return res;
}

video_card_t	*new_video_card(
		memory_t				*memory,
		u32_t					color_address,
		u32_t					address_address,
		vc_update_resolution_callback_func_t	resolution_change_callback)
{
	video_card_t	*res;

	res				= calloc(1, sizeof(video_card_t));
	res->memory			= memory;
	res->color_address		= color_address;
	res->address_address		= address_address;
	res->resolution			= VC_DEFAULT_RESOLUTION;
	res->active_frame_buffer	= new_vc_frame_buffer(res->resolution);
	res->rendered_frame_buffer	= new_vc_frame_buffer(res->resolution);
	res->waiting_for_resolution	= false;
	res->resolution_change_callback	= resolution_change_callback;
	pthread_mutex_init(&res->mutex, NULL);
	return res;
}

static void swap_buffers(video_card_t *vc) MONITOR(&vc->mutex,
{
	memmove(
		vc->rendered_frame_buffer,
		vc->active_frame_buffer,
		vc_resolution_height(vc->resolution)*vc_resolution_width(vc->resolution)*sizeof(vc_color_t));
})

static void update_resolution(video_card_t *vc, int new_res) MONITOR(&vc->mutex,
{
	free(vc->active_frame_buffer);
	free(vc->rendered_frame_buffer);

	vc->active_frame_buffer		= new_vc_frame_buffer(new_res);
	vc->rendered_frame_buffer	= new_vc_frame_buffer(new_res);
	vc->resolution			= new_res;
})

static u32_t	get_addr(video_card_t *vc)
{
	u32_t a = memory_read_long(vc->memory, vc->address_address);
	return a;
}

static vc_color_t	get_color(video_card_t *vc)
{
	u8_t r, g, b;
	r = memory_read_byte(vc->memory, vc->color_address);
	g = memory_read_byte(vc->memory, vc->color_address + 1);
	b = memory_read_byte(vc->memory, vc->color_address + 2);
	return (vc_color_t){r, g, b};
}

static void read_color(video_card_t *vc) MONITOR(&vc->mutex,
{
	vc_color_t c;

	c = vc->rendered_frame_buffer[get_addr(vc)];
	memory_write_byte(vc->memory, vc->color_address, c.red);
	memory_write_byte(vc->memory, vc->color_address + 1, c.green);
	memory_write_byte(vc->memory, vc->color_address + 2, c.blue);
})

void	vc_send(video_card_t *vc, u8_t signal)
{
	if (vc->waiting_for_resolution) {
		update_resolution(vc, signal);
		vc->resolution_change_callback(vc, signal);
		vc->waiting_for_resolution = false;
		return;
	}
	static int cnt = 0;
	switch(signal) {
		case VC_WRITE_RGB:
			// u32_t addr = get_addr(vc);
			// if (addr > vc_resolution_height(vc))

			vc->active_frame_buffer[get_addr(vc)] = get_color(vc);
			break;
		case VC_READ_RGB:
			read_color(vc);
			break;
		case VC_SWAP_BUFFERS:
			swap_buffers(vc);

			break;
		case VC_SET_RESOLUTION:
			vc->waiting_for_resolution = true;
			break;
	}
}

vc_color_t	*vc_get_render_buffer(video_card_t *vc)
{
	vc_color_t *res;

	DO_LOCKED(&vc->mutex, {
		res = copy_vc_framebuffer(vc->rendered_frame_buffer, vc->resolution);
	});
	return res;
}