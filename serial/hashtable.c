#ifndef __HASHTABLE_C__
#define __HASHTABLE_C__

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define MAX_KEY_LEN 16UL
#define MAX_ENTRIES (2ul << 16)

typedef uint32_t(*hashing_function_t)(const char *key);

struct hash_table_entry {
	char key[MAX_KEY_LEN];
	uint16_t count;
};

struct hash_table_shard {
	uint32_t range_start;
	uint32_t range_end;
	uint32_t entries_count;
	uint32_t curr_max_entries;
	hashing_function_t hashing_function;
	struct hash_table_entry *entries;
};

#define hash_to_id(__shard, __hash) (__hash % __shard->curr_max_entries)
#define get_resize_threshold(__shard) (__shard->curr_max_entries - (__shard->curr_max_entries >> 2))

int hashtable_init(struct hash_table_shard *shard, uint32_t range_start,
		uint32_t range_end, hashing_function_t hashing_function) {
	uint32_t allocation_size = (range_end - range_start);

	shard->entries = calloc(allocation_size, sizeof(struct hash_table_shard));
	if (shard->entries == NULL)
		return 1;

	shard->range_start = range_start;
	shard->range_end = range_end;
	shard->entries_count = 0;
	shard->curr_max_entries = allocation_size;
	shard->hashing_function = hashing_function;

	return 0;
}

void hashtable_deinit(struct hash_table_shard *shard) {
	free(shard->entries);
	shard->entries = NULL;
}

int hashtable_lookup(struct hash_table_shard *shard, const char *key) {
	uint32_t i;
	uint32_t hash = shard->hashing_function(key);
	uint32_t hash_table_id = hash_to_id(shard, hash);
	uint32_t shard_size = shard->curr_max_entries;

	for (i = hash_table_id; i < shard_size; i++) {
		if (shard->entries[i].count == 0)
			return 0;

		if (strncmp(key, shard->entries[i].key, MAX_KEY_LEN) == 0)
			return shard->entries[i].count;
	}

	for (i = 0; i < hash_table_id; i++) {
		if (shard->entries[i].count == 0)
			return 0;

		if (strncmp(key, shard->entries[i].key, MAX_KEY_LEN) == 0)
			return shard->entries[i].count;
	}

	return 0;
}

int hashtable_insert(struct hash_table_shard *shard, const char *key);

int hashtable_resize(struct hash_table_shard *shard, uint32_t target_entries_count) {
	uint32_t i;
	uint32_t original_entries_count = shard->curr_max_entries;
	struct hash_table_entry *original_array;
	int res = 0;

	/*
	printf("INFO: Resize shard %u - %u. Original size %u. New size %u\n",
			shard->range_start, shard->range_end, original_entries_count,
			target_entries_count);
			*/

	assert(target_entries_count > original_entries_count);

	original_array = shard->entries;
	shard->entries = calloc(target_entries_count, sizeof(struct hash_table_entry));
	assert(shard->entries);

	shard->curr_max_entries = target_entries_count;
	shard->entries_count = 0;

	for (i = 0; i < original_entries_count; i++) {
		if (original_array[i].count == 0)
			continue;

		//TODO implement internal insert which doesn't try to reallocate
		res = hashtable_insert(shard, original_array[i].key);
		assert(res == 0);
	}

	free(original_array);

	return 0;
}

int hashtable_insert(struct hash_table_shard *shard, const char *key) {
	uint32_t i;
	uint32_t hash = shard->hashing_function(key);
	uint32_t hash_table_id = hash_to_id(shard, hash);
	uint32_t shard_size = shard->curr_max_entries;
	uint32_t resize_threshold;
	int res = 0;

	assert(strlen(key) < MAX_KEY_LEN);

	for (i = hash_table_id; i < shard_size; i++) {
		if (shard->entries[i].count > 0) {
			if (strncmp(key, shard->entries[i].key, MAX_KEY_LEN) == 0) {
				assert(shard->entries[i].count < MAX_ENTRIES);
				shard->entries[i].count++;
				return 0;
			}
			continue;
		}
		break;
	}

	if (i == shard_size) {
		for (i = 0; i < hash_table_id; i++) {
			if (shard->entries[i].count > 0) {
				if (strncmp(key, shard->entries[i].key, MAX_KEY_LEN) == 0) {
					assert(shard->entries[i].count < MAX_ENTRIES);
					shard->entries[i].count++;
					return 0;
				}
				continue;
			}
			break;
		}
	}

	strncpy(shard->entries[i].key, key, MAX_KEY_LEN);
	shard->entries[i].count = 1;
	shard->entries_count++;
	resize_threshold = get_resize_threshold(shard);

	if (shard->entries_count > resize_threshold)
		res = hashtable_resize(shard, shard->curr_max_entries * 2);

	return res;
}
#endif
