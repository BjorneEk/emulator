/*==========================================================*
 *
 * @author Gustaf Franzén :: https://github.com/BjorneEk;
 *
 * hashmap
 *
 *==========================================================*/


#include "hashmap.h"
#include <stdlib.h>
#include <string.h>



u64_t HASH_fnv_1a(const void* a, u64_t size)
{
	i64_t	nblocks;
	i64_t	i;
	u64_t	hash;
	u64_t	last;
	u8_t	*data;

	nblocks	= size / 8;
	hash	= 2166136261u;
	data	= (u8_t*)a;

	for (i = 0; i < nblocks; ++i) {
		hash ^=	(u64_t)data[0] << 0  | (u64_t)data[1] << 8  |
			(u64_t)data[2] << 16 | (u64_t)data[3] << 24 |
			(u64_t)data[4] << 32 | (u64_t)data[5] << 40 |
			(u64_t)data[6] << 48 | (u64_t)data[7] << 56;
		hash *= 0xbf58476d1ce4e5b9; /* FNV PRIME */
		data += 8;
	}

	last = size & 0xff;
	switch (size % 8) {
	case 7:
		last |= (u64_t)data[6] << 56;
	case 6:
		last |= (u64_t)data[5] << 48;
	case 5:
		last |= (u64_t)data[4] << 40;
	case 4:
		last |= (u64_t)data[3] << 32;
	case 3:
		last |= (u64_t)data[2] << 24;
	case 2:
		last |= (u64_t)data[1] << 16;
	case 1:
		last |= (u64_t)data[0] << 8;
		hash ^= last;
		hash *= 0xd6e8feb86659fd93; /* FNV PRIME */
	}

	return hash ^ hash >> 32;
}

hashmap_t	*HMAP_new(u64_t len, hash_f hashf)
{
	hashmap_t	*res;

	res = malloc(sizeof(hashmap_t));
	res->buckets = calloc(len, sizeof(struct bucket));
	res->n_buckets = 0;
	res->len = len;
	res->hashf = hashf;

	return res;
}

static u64_t hash_idx(hashmap_t *m, const void *key, u64_t kwidth)
{
	return m->hashf(key, kwidth) % m->len;
}

static inline bool bucket_equals(u64_t hash, const void *key, u64_t kwidth, const struct bucket *b)
{
	return (hash == b->hash && kwidth == b->kwidth && memcmp(key, b->key, kwidth) == 0);
}

static struct bucket *get_bucket(hashmap_t *m, const void *key, u64_t kwidth)
{
	struct bucket *b;
	u64_t hash;
	u64_t idx;


	hash = m->hashf(key, kwidth);
	idx = hash % m->len;
	b = &m->buckets[idx];

	do {
		if (bucket_equals(hash, key, kwidth, b))
			return b;
	} while((b = b->next) != NULL);
	return NULL;
}



bool	HMAP_contins_key(hashmap_t *m, const void *key, u64_t kwidth)
{
	return get_bucket(m, key, kwidth) != NULL;
}

void HMAP_remove(hashmap_t *m, void *key, u64_t kwidth)
{
	struct bucket *b, *tmp;
	u64_t hash;
	u64_t idx;


	hash = m->hashf(key, kwidth);
	idx = hash % m->len;
	b = &m->buckets[idx];

	if(b->next == NULL && bucket_equals(hash, key, kwidth, b)) {
		memset(b, 0, sizeof(struct bucket));
		return;
	} else if (bucket_equals(hash, key, kwidth, b)) {
		tmp = b->next;
		memcpy(b, tmp, sizeof(struct bucket));
		free(tmp);
		return;
	}

	while (b->next != NULL) {

		if (bucket_equals(hash, key, kwidth, b->next)) {
			tmp = b->next;
			b->next = b->next->next;
			free(tmp);
			return;
		}
		b = b->next;
	}

}

void	*HMAP_get(hashmap_t *m, const void *key, u64_t kwidth)
{
	struct bucket *entry;

	entry = get_bucket(m, key, kwidth);

	if(entry == NULL)
		return NULL;

	return entry->value;
}

struct bucket new_bucket(hashmap_t *m, void *key, u64_t kwidth, void *value)
{
	struct bucket res;

	res.value	= value;
	res.key		= key;

	res.hash	= m->hashf(key, kwidth);
	res.next	= NULL;
	res.kwidth	= kwidth;
	return res;
}
struct bucket *new_bucket_ptr(hashmap_t *m, void *key, u64_t kwidth, void *value)
{
	struct bucket *res;

	res = malloc(sizeof(struct bucket));
	res->value	= value;
	res->key	= key;

	res->hash	= m->hashf(key, kwidth);
	res->next	= NULL;
	res->kwidth	= kwidth;
	return res;
}

void	HMAP_add(hashmap_t *m, void *key, u64_t kwidth, void *value)
{
	u64_t i;
	struct bucket *b;

	++m->n_buckets;
	i = hash_idx(m, key, kwidth);

	if(m->buckets[i].key == NULL && m->buckets[i].value == NULL) {
		m->buckets[i] = new_bucket(m, key, kwidth, value);
	} else {
		for(b = &m->buckets[i]; b->next != NULL; b = b->next)
			;
		b->next = new_bucket_ptr(m, key, kwidth, value);
	}
}

void free_bucket(struct bucket b)
{
	struct bucket *next, *tmp;

	if(!b.next) {
		return;
	}
	next = b.next;

	do {
		tmp = next;
		next = next->next;
		free(tmp);
	} while(next != NULL);
}

void		HMAP_free(hashmap_t **m)
{
	u64_t i;

	for(i = 0; i < (*m)->len; i++)
		free_bucket((*m)->buckets[i]);
	free((*m)->buckets);
	free(*m);
}