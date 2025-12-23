#ifndef __COUNTING_H__
#define __COUNTING_H__
#include <stdint.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#include "config.h"

#define COUNTING_MAX_INPUT (0xffffffffUL)
#define SHARD_SIZE (COUNTING_MAX_INPUT / SHARDS)

struct tree_owner {
	uint64_t elements_in_map;
	uint64_t repeated_elements;

	uint64_t shard_range_min;
	uint64_t shard_range_max;

	uint64_t *added_once;
	uint64_t *added_twice;

	pthread_rwlock_t lock;

	pthread_rwlock_t visited_lock;
};

struct tree_owner *get_shard(struct tree_owner ctx[], uint64_t number) {
	uint64_t shard_id = number / SHARD_SIZE;
	struct tree_owner *tmp;

	if (shard_id == SHARDS) {
		assert(number >= ctx[SHARDS - 1].shard_range_min);
		shard_id = SHARDS - 1;
	}

	tmp = &ctx[shard_id];

	if (!(number >= tmp->shard_range_min && number <= tmp->shard_range_max)) {
		printf("Wrong shard. Range %lu - %lu. Number %lu\n",
			tmp->shard_range_min, tmp->shard_range_max, number);
		assert(0);
	}

	return tmp;
}

void prepare_shards(struct tree_owner ctx[], uint64_t shards_count, uint64_t shard_size) {
	uint64_t i;
	ssize_t allocation_size;

	for (i = 0; i < shards_count; i++) {
		ctx[i].shard_range_min = shard_size * i;
		ctx[i].shard_range_max = shard_size * (i+1) - 1;

		assert(pthread_rwlock_init(&ctx[i].lock, 0) == 0);
		assert(pthread_rwlock_init(&ctx[i].visited_lock, 0) == 0);
	}

	ctx[shards_count - 1].shard_range_max = COUNTING_MAX_INPUT;

	for (i = 0; i < shards_count; i++) {
		assert(ctx[i].shard_range_max > ctx[i].shard_range_min);
		assert(ctx[i].shard_range_max - ctx[i].shard_range_min + 1 >= shard_size);

		allocation_size = (ctx[i].shard_range_max - ctx[i].shard_range_min) / 64;
		allocation_size += 1; // To not to bother with rounding up/down etc

		ctx[i].added_once = calloc(allocation_size, sizeof(uint64_t));
		assert(ctx[i].added_once != NULL);

		ctx[i].added_twice = calloc(allocation_size, sizeof(uint64_t));
		assert(ctx[i].added_twice != NULL);
	}
}

void destroy_shards(struct tree_owner ctx[], uint64_t shards_count) {
	uint64_t i;

	for (i = 0; i < shards_count; i++) {
		assert(pthread_rwlock_destroy(&ctx[i].lock) == 0);
		assert(pthread_rwlock_destroy(&ctx[i].visited_lock) == 0);

		free(ctx[i].added_once);
		free(ctx[i].added_twice);
	}

}

#define COUNTING_VALUE_EXISTS(__shard, __cell_id, __bit_id) \
	(__shard->added_once[__cell_id] & (uint64_t)((uint64_t)1 << __bit_id))

#define COUNTING_VALUE_WAS_VISITED(__shard, __cell_id, __bit_id) \
	(__shard->added_twice[__cell_id] & (uint64_t)((uint64_t)1 << __bit_id))

#define COUNTING_SET_EXISTS(__shard, __cell_id, __bit_id) \
	(__shard->added_once[__cell_id] |= (uint64_t)((uint64_t)1 << __bit_id))

#define COUNTING_SET_WAS_VISITED(__shard, __cell_id, __bit_id) \
	(__shard->added_twice[__cell_id] |= (uint64_t)((uint64_t)1 << __bit_id))

int count_numbers(uint32_t *arr, int count, struct tree_owner ctx[]) {
	int i;
	uint64_t exists;
	struct tree_owner *shard;
	int res;


	for (i = 0; i < count; i++) {
		shard = get_shard(ctx, arr[i]);

		uint32_t val_id_in_shard = arr[i] - shard->shard_range_min;
		uint32_t cell_id_in_array = val_id_in_shard / 64;
		uint32_t bit_id_in_cell = val_id_in_shard % 64;

		res = pthread_rwlock_rdlock(&shard->lock);
		assert(res == 0);

		exists = COUNTING_VALUE_EXISTS(shard, cell_id_in_array, bit_id_in_cell);

		res = pthread_rwlock_unlock(&shard->lock);
		assert(res == 0);

		if (exists != 0) {
			res = pthread_rwlock_rdlock(&shard->visited_lock);
			assert(res == 0);

			if (COUNTING_VALUE_WAS_VISITED(shard, cell_id_in_array, bit_id_in_cell)) {
				res = pthread_rwlock_unlock(&shard->visited_lock);
				assert(res == 0);
				continue;
			}
			res = pthread_rwlock_unlock(&shard->visited_lock);
			assert(res == 0);

			res = pthread_rwlock_wrlock(&shard->visited_lock);
			assert(res == 0);

			if (COUNTING_VALUE_WAS_VISITED(shard, cell_id_in_array, bit_id_in_cell) == 0) {
				shard->repeated_elements++;
				COUNTING_SET_WAS_VISITED(shard, cell_id_in_array, bit_id_in_cell);
			}
			res = pthread_rwlock_unlock(&shard->visited_lock);
			assert(res == 0);
			continue;
		}

		res = pthread_rwlock_wrlock(&shard->lock);
		assert(res == 0);

		exists = COUNTING_VALUE_EXISTS(shard, cell_id_in_array, bit_id_in_cell);
		if (exists != 0) {
			res = pthread_rwlock_rdlock(&shard->visited_lock);
			assert(res == 0);
			if (COUNTING_VALUE_WAS_VISITED(shard, cell_id_in_array, bit_id_in_cell) == 0) {
				res = pthread_rwlock_unlock(&shard->visited_lock);
				assert(res == 0);

				res = pthread_rwlock_wrlock(&shard->visited_lock);
				assert(res == 0);

				if (COUNTING_VALUE_WAS_VISITED(shard, cell_id_in_array, bit_id_in_cell) == 0) {
					shard->repeated_elements++;
					COUNTING_SET_WAS_VISITED(shard, cell_id_in_array, bit_id_in_cell);
				}

				res = pthread_rwlock_unlock(&shard->visited_lock);
				assert(res == 0);
			} else {
				res = pthread_rwlock_unlock(&shard->visited_lock);
				assert(res == 0);
			}
			// TODO Unlock earlier and test.
			res = pthread_rwlock_unlock(&shard->lock);
			assert(res == 0);
			continue;
		}
		
		COUNTING_SET_EXISTS(shard, cell_id_in_array, bit_id_in_cell);
		shard->elements_in_map++;

		res = pthread_rwlock_unlock(&shard->lock);
		assert(res == 0);
	}

	return 0;
}

uint64_t seen_only_once(struct tree_owner *owner) {
	return owner->elements_in_map - owner->repeated_elements;
}

uint64_t aggregate_unique_numbers(struct tree_owner ctx[], uint64_t shards) {
	uint64_t i;
	int res = 0;
	
	for (i = 0; i < shards; i++)
		res += ctx[i].elements_in_map;

	return res;
}

uint64_t aggregate_seen_only_once(struct tree_owner ctx[], uint64_t shards) {
	uint64_t i;
	int res = 0;
	
	for (i = 0; i < shards; i++) {
		res += seen_only_once(&ctx[i]);
		//printf("%d: seen numbers %u\n", i, seen_only_once(&ctx[i]));
	}

	return res;
}

#endif
