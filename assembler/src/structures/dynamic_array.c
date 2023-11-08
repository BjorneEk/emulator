/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 *
 * dynamic length array
 *
 *==========================================================*/


#include "dynamic_array.h"
#include "../util/error.h"
#include <string.h>
#include <stdio.h>

#define IDX(_dla, i) ((_dla)->data + ((i) * (_dla)->width))

dla_t		*DLA_new_(size_t width, f32_t expand_factor, size_t len)
{
	dla_t	*res;

	res			= malloc(sizeof(dla_t));
	res->data		= calloc(len, width);
	res->_allocated		= len;
	res->len		= 0;
	res->width		= width;
	res->expand_factor	= expand_factor;
	return res;
}

void		DLA_free(dla_t **self)
{
	free((*self)->data);
	free(*self);
}

void		DLA_expand(dla_t *self, size_t size)
{
	self->_allocated += size;
	self->data = realloc(self->data, self->_allocated * self->width);

	ASSERT_(self->data != NULL,
		"DLA_expand, out of memory expanding array of size 0x%x bytes by 0x%x bytes",
		self->_allocated, size * self->width);
}
static inline f32_t max(f32_t a, f32_t b)
{
	return (a >= b) ? a : b;
}

static inline size_t get_expandsize(dla_t *self)
{
	return max(self->expand_factor * self->_allocated, 1.0);
}

void		DLA_insert(dla_t *self, u64_t i, void *data)
{
	if (DLA_isfull(self)) {
		//printf("size: %zu\nfactor: %f\nallocated: %zu\nexpandling: %zu\n",self->len, self->expand_factor, self->_allocated, get_expandsize(self));
		DLA_expand(self, get_expandsize(self));
	}

	/* move elements one step back in array to make room for the new one */
	memmove(
		IDX(self, i + 1	),		/* dest   */
		IDX(self, i	),		/* source */
		(self->len - i) * self->width	/* size   */
	);

	memcpy(IDX(self, i), data, self->width);
	++self->len;
}
