/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 *
 * image representation for the raytracer and for drawing to opengl
 *
 *==========================================================*/

#ifndef _IMAGE_H_
#define _IMAGE_H_

#include "../../../common/util/types.h"

typedef struct __attribute__((packed)) rgb {
	u8_t	red;
	u8_t	green;
	u8_t	blue;
} rgb_t;

typedef struct image {

	u32_t width;
	u32_t height;

	rgb_t *data;

	u32_t GL_texture;

} image_t;

static inline rgb_t rgb(u8_t r, u8_t g, u8_t b) {return (rgb_t){r, g, b};};

image_t *new_image(u32_t width, u32_t height, u32_t scr_width, u32_t scr_hight);
void draw_image(image_t *img);
#endif /* _IMAGE_H_ */