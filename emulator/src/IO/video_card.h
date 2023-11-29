/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 * @author Adam Månsson
 *
 * video card emulator
 *==========================================================*/
#ifndef _VIDEO_CARD_H_
#define _VIDEO_CARD_H_

#include "../../../common/util/types.h"
#include "../../../common/util/error.h"
#include "../memory.h"
#include <pthread.h>


#define VC_DEFAULT_RESOLUTION VC_RESOLUTION_800X600

enum vc_signals {
	VC_WRITE_RGB,
	VC_READ_RGB,
	VC_SWAP_BUFFERS,
	VC_SET_RESOLUTION,
	VC_RESOLUTION_800X600,
	VC_RESOLUTION_640X480
};

typedef struct __attribute((packed)) color {
	u8_t	red;
	u8_t	green;
	u8_t	blue;
} vc_color_t;

typedef struct video_card video_card_t;

typedef void (*vc_update_resolution_callback_func_t)(video_card_t *vc, int resolution);

struct video_card {
	vc_color_t			*active_frame_buffer;
	vc_color_t			*rendered_frame_buffer;

	pthread_mutex_t		mutex;

	bool			waiting_for_resolution;
	memory_t		*memory;
	u32_t			color_address;
	u32_t			address_address;
	int			resolution;

	vc_update_resolution_callback_func_t	resolution_change_callback;
};

static const inline int vc_resolution_height(int res)
{
	switch (res) {
		case VC_RESOLUTION_640X480: return 480;
		case VC_RESOLUTION_800X600: return 600;
	}
	return 0;
}

static const inline int vc_resolution_width(int res)
{
	switch (res) {
		case VC_RESOLUTION_640X480: return 640;
		case VC_RESOLUTION_800X600: return 800;
	}
	return 0;
}

vc_color_t	*new_vc_frame_buffer(int res);

video_card_t	*new_video_card(
		memory_t				*memory,
		u32_t					color_address,
		u32_t					address_address,
		vc_update_resolution_callback_func_t	resolution_change_callback);

void	vc_send(video_card_t *vc, u8_t signal);

vc_color_t	*vc_get_render_buffer(video_card_t *vc);

#endif /* _VIDEO_CARD_H_*/