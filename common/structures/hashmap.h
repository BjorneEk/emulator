/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 *
 * hashmap header file
 *
 *==========================================================*/

#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include "../util/types.h"
#include <stdbool.h>

typedef u64_t (*hash_f)(const void *a, u64_t size);

struct bucket {

	u64_t hash;

	void *key;
	u64_t kwidth;

	void *value;

	struct bucket *next;
};


typedef struct hashmap {

	struct bucket *buckets;

	u64_t len;

	u64_t n_buckets;

	hash_f hashf;
} hashmap_t;


#define FOREACH_HMAP(map, _ktype, _kname, _vtype, _vname, __stmt__)	\
do {									\
i32_t i;								\
_ktype _kname;								\
_vtype _vname;								\
var_t * _var;								\
struct bucket *b;							\
for(i = 0; i <map->len; i++){						\
	if(map->buckets[i].hash != 0) {					\
		b = &map->buckets[i];					\
		do {							\
			_kname = b->key;				\
			_vname = b->value;				\
			__stmt__					\
		} while ((b = b->next) != NULL);			\
	}								\
}} while(0);

#define FOREACH_VALUE(map, _vtype, _vname, __stmt__)			\
do {									\
i32_t i;								\
_vtype _vname;								\
struct bucket *b;							\
for(i = 0; i <map->len; i++){						\
	if(map->buckets[i].hash != 0) {					\
		b = &map->buckets[i];					\
		do {							\
			_vname = b->value;				\
			__stmt__					\
		} while ((b = b->next) != NULL);			\
	}								\
}} while(0);
#define FOREACH_KEY(map, _ktype, _kname, __stmt__)			\
do {									\
i32_t i;								\
_ktype _kname;								\
var_t * _var;								\
for(i = 0; i <map->len; i++){						\
	if(map->buckets[i].hash != 0) {					\
		b = &map->buckets[i];					\
		do {							\
			_kname = b->key;				\
			__stmt__					\
		} while ((b = b->next) != NULL);			\
	}								\
}} while(0);


/**
 * a fnv-1a hash algorithm.
 * https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
 *
 * @param a   : the data to generate hash from,
 * @param size: the length in bytes of @a
 **/
u64_t HASH_fnv_1a(const void* a, u64_t size);


/**
 * allocates and returns a new hashmap with the given type,
 * length and hash functions
 *
 * @param vwidth: the size in bytes of the values to be stored in the hashmap
 * @param len	: the length of the hashmap
 * @param hashf	: the hashfunction of the hashmap
 **/
hashmap_t	*HMAP_new(u64_t len, hash_f hashf);

/**
 * frees a hashmap and its contents fully, this includes freeing the
 * key and value pointers themselves
 *
 * @param m: the hashmap to be freed
 **/
void		HMAP_free(hashmap_t **m);


/**
 * returns true if the hashmap contains a entry with the given key
 *
 * @param m	: the hashmap
 * @param key	: the key to search for
 * @param kwidth: the width in bytes of the @key
 **/
bool		HMAP_contins_key(hashmap_t *m, const void *key, u64_t kwidth);

/**
 * if the hashmap contains the given key return the value, else return NULL
 *
 * @param m	: the hashmap
 * @param key	: the key to search for
 * @param kwidth: the width in bytes of the @key
 **/
void		*HMAP_get(hashmap_t *m, const void *key, u64_t kwidth);

/**
 * remove the entrie with the coresponding key
 *
 * @param m	: the hashmap
 * @param key	: the key to search for
 * @param kwidth: the width in bytes of the @key
 **/
void		HMAP_remove(hashmap_t *m, void *key, u64_t kwidth);

/**
 * adds the entry created by @key and @value to the hashmap
 *
 * @param m	: the hashmap
 * @param key	: the key to the entry
 * @param kwidth: the width in bytes of the @key
 * @param value	: the value to be added
 **/
void		HMAP_add(hashmap_t *m, void *key, u64_t kwidth, void *value);

#endif /* _HASHMAP_H_ */