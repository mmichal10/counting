#ifndef __HASHTABLE_C__
#define __HASHTABLE_C__

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdatomic.h>

#include "hash.c"

#define MAX_KEY_LEN 16UL
#define ENTRY_MAX_OCCURANCES (2ul << 32) // HO

struct hash_table_entry {
	char key[MAX_KEY_LEN];
	atomic_ulong count;
};

struct hash_table_shard {
	uint32_t range_start;
	uint32_t range_end;
	uint32_t entries_count;
	uint32_t curr_max_entries;
	struct hash_table_entry *entries;
};

#define hash_to_id(__shard, __hash) (__hash % __shard->curr_max_entries)
#define get_resize_threshold(__shard) (__shard->curr_max_entries - (__shard->curr_max_entries >> 2))
#define get_resize_critical_threshold(__shard) (__shard->curr_max_entries - (__shard->curr_max_entries >> 4))

int hashtable_init(struct hash_table_shard *shard, uint32_t range_start, uint32_t range_end) {
	uint32_t allocation_size = (range_end - range_start);

	shard->entries = calloc(allocation_size, sizeof(struct hash_table_entry));
	if (shard->entries == NULL)
		return 1;

	shard->range_start = range_start;
	shard->range_end = range_end;
	shard->entries_count = 0;
	shard->curr_max_entries = allocation_size;

	return 0;
}

void hashtable_deinit(struct hash_table_shard *shard) {
	free(shard->entries);
	shard->entries = NULL;
}

struct hash_table_entry* hashtable_lookup(struct hash_table_shard *shard, const char *key, uint32_t hash) {
	uint32_t i;
	uint32_t hash_table_id = hash_to_id(shard, hash);
	uint32_t shard_size = shard->curr_max_entries;

	for (i = hash_table_id; i < shard_size; i++) {
		if (shard->entries[i].key[0] == 0)
			return NULL;

		if (strncmp(key, shard->entries[i].key, MAX_KEY_LEN) == 0)
			return &shard->entries[i];
	}

	for (i = 0; i < hash_table_id; i++) {
		if (shard->entries[i].key[0] == 0)
			return NULL;

		if (strncmp(key, shard->entries[i].key, MAX_KEY_LEN) == 0)
			return &shard->entries[i];
	}

	return 0;
}

void hashtable_insert_on_resize(struct hash_table_shard *shard, const char *key, uint32_t count,
		uint32_t hash) {
	uint32_t i;
	uint32_t hash_table_id = hash_to_id(shard, hash);
	uint32_t shard_size = shard->curr_max_entries;

	assert(strlen(key) < MAX_KEY_LEN);

	for (i = hash_table_id; i < shard_size; i++) {
		if (shard->entries[i].key[0] == 0)
			break;
	}

	if (i == shard_size) {
		for (i = 0; i < hash_table_id; i++) {
			if (shard->entries[i].key[0] == 0)
				break;
		}
	}

	strncpy(shard->entries[i].key, key, MAX_KEY_LEN);
	atomic_init(&shard->entries[i].count, count);
	shard->entries_count++;
}

int hashtable_resize(struct hash_table_shard *shard, uint32_t target_entries_count,
		hashing_function_t hashing_function) {
	uint32_t i;
	uint32_t original_entries_count = shard->curr_max_entries;
	struct hash_table_entry *original_array;

	assert(target_entries_count > original_entries_count);

	original_array = shard->entries;
	shard->entries = calloc(target_entries_count, sizeof(struct hash_table_entry));
	if (!shard->entries) {
		shard->entries = original_array;
		return 1;
	}

	shard->curr_max_entries = target_entries_count;
	shard->entries_count = 0;

	for (i = 0; i < original_entries_count; i++) {
		if (original_array[i].key[0] == 0)
			continue;

		hashtable_insert_on_resize(shard,
				original_array[i].key,
				atomic_load(&original_array[i].count),
				hashing_function(original_array[i].key));
	}

	free(original_array);

	return 0;
}

int hashtable_insert(struct hash_table_shard *shard, const char *key, uint32_t hash,
		hashing_function_t hashing_function) {
	uint32_t i;
	uint32_t hash_table_id = hash_to_id(shard, hash);
	uint32_t shard_size = shard->curr_max_entries;
	uint32_t resize_threshold;
	int res = 0;
	
	if (strlen(key) >= MAX_KEY_LEN) {
		printf("ERROR: Key %s too long!\n", key);
		return 1;
	}

	for (i = hash_table_id; i < shard_size; i++) {
		if (shard->entries[i].key[0] != 0) {
			if (strncmp(key, shard->entries[i].key, MAX_KEY_LEN) == 0) {
				if (atomic_load(&shard->entries[i].count) == ENTRY_MAX_OCCURANCES) {
					printf("ERROR: Key %s has reached the max possible occurances\n", shard->entries[i].key);
					return 1;
				}
				atomic_fetch_add(&shard->entries[i].count, 1);
				return 0;
			}
			continue;
		}
		break;
	}

	if (i == shard_size) {
		for (i = 0; i < hash_table_id; i++) {
			if (shard->entries[i].key[0] != 0) {
				if (strncmp(key, shard->entries[i].key, MAX_KEY_LEN) == 0) {
					if (atomic_load(&shard->entries[i].count) == ENTRY_MAX_OCCURANCES) {
						printf("ERROR: Key %s has reached the max possible occurances\n", shard->entries[i].key);
						return 1;
					}
					atomic_fetch_add(&shard->entries[i].count, 1);
					return 0;
				}
				continue;
			}
			break;
		}
	}

	strncpy(shard->entries[i].key, key, MAX_KEY_LEN);
	atomic_init(&shard->entries[i].count, 1);
	shard->entries_count++;
	resize_threshold = get_resize_threshold(shard);

	if (shard->entries_count > resize_threshold) {
		res = hashtable_resize(shard, shard->curr_max_entries * 2, hashing_function);
		if (res && shard->entries_count > get_resize_critical_threshold(shard)) {
			printf("WARNING: Growing hashtable shard failed, but it continues to operate\n");
			res = 0;
		} else if (res) {
			printf("ERROR: Growing hashtable shard failed\n");
		}
	}

	return res;
}
#endif
