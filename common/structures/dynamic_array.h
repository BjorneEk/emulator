/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 *
 * dynamic array header file
 *
 *==========================================================*/

#ifndef _DYNAMIC_ARRAY_H_
#define _DYNAMIC_ARRAY_H_

#include "../util/types.h"
#include <stdlib.h>
#include <stdbool.h>

#define DEFAULT_EXPAND_FACTOR 0.7f

typedef struct dynamic_length_array {

	u8_t *data;

	size_t width;
	/**
	 * number of elements currently in the array
	 **/
	size_t len;

	/**
	 * currently allocated space in elements
	 **/
	size_t _allocated;

	f32_t expand_factor;
} dla_t;

#define FOREACH_DLA(_dla, _iter, _type, _name, __stmt__) do {	\
	i32_t _iter;						\
	_type _name;						\
	for (_iter = 0; _iter < _dla->len; _iter++) {		\
		_name = *(_type*)DLA_get(_dla, _iter);		\
		__stmt__;					\
	}							\
}while(0);

#define FOREACH_DLA_PTR(_dla, _iter, _type, _name, __stmt__) do {	\
	i32_t _iter;							\
	_type * _name;							\
	for (_iter = 0; _iter < _dla->len; _iter++) {			\
		_name = (_type*)DLA_get(_dla, _iter);			\
		__stmt__;						\
	}								\
}while(0);

/**
 * create a new dynamic length array
 * @width:		the size in bytes of a single element.
 * @expand_factor:	when the array grows it grows to become
 * 			@_allocated * @width * (1.0 + expand_factor) bytes long
 * @len:		the initial lenght to be allocated
 **/
dla_t		*DLA_new_(size_t width, f32_t expand_factor, size_t len);

/**
 * create a new dynamic length array with a expand factor of 0.7
 * @width:		the size in bytes of a single element.
 * @len:		the initial lenght to be allocated
 **/
static inline dla_t	*DLA_new(size_t width, size_t len)
{
	return DLA_new_(width, DEFAULT_EXPAND_FACTOR, len);
}

/**
 * free the dynamic length array and the data it stores
 * @self: the dynamic length array to free
 **/
void		DLA_free(dla_t **self);

/**
 * clear the dynamic length array of its content (does not free the memory)
 * @self: the dynamic length array to clear
 **/
static inline void	DLA_clear(dla_t *self)
{
	self->len = 0;
}

/**
 * sets the expand factor of the dynamic length
 * @self: the dynamic length array
 * @expand_factor: the dynamic length array
 **/
static inline void	DLA_set_expand_factor(dla_t *self, f32_t expand_factor)
{
	self->expand_factor = expand_factor;
}

/**
 * expand the array by @size * @self->width ie make room for @size more elements
 * @self: the dynamic length array
 * @size: the number of elements to resize by
 **/
void		DLA_expand(dla_t *self, size_t size);

/**
 * returns true if the array is fully populated
* @self: the dynamic length array
 **/
static inline bool	DLA_isfull(dla_t *self)
{
	return self->len == self->_allocated;
}

/**
 * insert @data to @self at index @i
 * @self: the dynamic length array
 * @i: the index to insert at
 * @data: the data to insert
 **/
void		DLA_insert(dla_t *self, u64_t i, void *data);

/**
 * append @data to @self at last index
 * @self: the dynamic length array
 * @data: the data to insert
 **/
static inline void	DLA_append(dla_t *self, void *data)
{
	DLA_insert(self, self->len, data);
}

/**
 * prepend @data to @self at first index
 * @self: the dynamic length array
 * @data: the data to insert
 **/
static inline void	DLA_prepend(dla_t *self, void *data)
{
	DLA_insert(self, 0, data);
}

/**
 * return the data at index @i
 * @self: the dynamic length array
 * @i: the index to return
 **/
static inline void	*DLA_get(dla_t *self, u64_t i)
{
	return self->data + (size_t)(i * self->width);
}

#endif /* _DYNAMIC_ARRAY_H_ */