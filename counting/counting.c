#ifndef __COUNTING_H__
#define __COUNTING_H__
#include <stdint.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>

#include "tree.h"
#include "config.h"

#define MAX_INPUT 0xffffffff
#define SHARD_SIZE (MAX_INPUT / SHARDS)

struct tree_owner {
	uint32_t tree_size;
	uint32_t repeated_elements;
	struct rb_tree_node *root;

	uint32_t shard_range_min;
	uint32_t shard_range_max;

	pthread_rwlock_t lock;

	//This might be an atomic or per-tree-node lock
	pthread_rwlock_t visited_lock;
};

struct tree_owner *get_shard(struct tree_owner ctx[], uint32_t number) {
	int shard_id = number / SHARD_SIZE;
	struct tree_owner *tmp = &ctx[shard_id];

	if (!(number >= tmp->shard_range_min && number <= tmp->shard_range_max)) {
		printf("Wrong shard. Range %u - %u. Number %u\n",
			tmp->shard_range_min, tmp->shard_range_max, number);
		assert(0);
	}

	return tmp;
}

void prepare_shards(struct tree_owner ctx[], uint32_t shards_count, uint32_t shard_size) {
	uint32_t i;

	for (i = 0; i < shards_count; i++) {
		ctx[i].shard_range_min = shard_size * i;
		ctx[i].shard_range_max = shard_size * (i+1) - 1;

		assert(pthread_rwlock_init(&ctx[i].lock, 0) == 0);
		assert(pthread_rwlock_init(&ctx[i].visited_lock, 0) == 0);
	}

	ctx[shards_count - 1].shard_range_max = MAX_INPUT;
}

void destroy_shards(struct tree_owner ctx[], uint32_t shards_count, uint32_t shard_size) {
	uint32_t i;

	for (i = 0; i < shards_count; i++) {
		assert(pthread_rwlock_destroy(&ctx[i].lock) == 0);
		assert(pthread_rwlock_destroy(&ctx[i].visited_lock) == 0);

		
		rb_tree_deinit(ctx[i].root);
	}

}

int count_numbers(uint32_t *arr, int count, struct tree_owner ctx[]) {
	int i;
	struct rb_tree_node *tmp;
	struct tree_owner *shard;
	int res;


	for (i = 0; i < count; i++) {

		shard = get_shard(ctx, arr[i]);

		res = pthread_rwlock_rdlock(&shard->lock);
		assert(res == 0);

		tmp = rb_tree_find(shard->root, arr[i]);

		res = pthread_rwlock_unlock(&shard->lock);
		assert(res == 0);

		if (tmp != NULL) {
			res = pthread_rwlock_rdlock(&shard->visited_lock);
			assert(res == 0);

			if (tmp->visited == 1) {
				res = pthread_rwlock_unlock(&shard->visited_lock);
				assert(res == 0);
				continue;
			}
			res = pthread_rwlock_unlock(&shard->visited_lock);
			assert(res == 0);

			res = pthread_rwlock_wrlock(&shard->visited_lock);
			assert(res == 0);

			if (tmp->visited == 0) {
				shard->repeated_elements++;
				tmp->visited = 1;
			}
			res = pthread_rwlock_unlock(&shard->visited_lock);
			assert(res == 0);
			continue;
		}

		res = pthread_rwlock_wrlock(&shard->lock);
		assert(res == 0);

		tmp = rb_tree_find(shard->root, arr[i]);
		if (tmp != NULL) {
			if (tmp->visited == 0) {
				shard->repeated_elements++;
				tmp->visited = 1;
			}
			res = pthread_rwlock_unlock(&shard->lock);
			assert(res == 0);
			continue;
		}
		
		tmp = rb_tree_insert_and_fix_violations(shard->root, arr[i]);
		assert(tmp); // TODO proper error handling
		shard->tree_size++;
		shard->root = shard->root ? rb_tree_get_root(shard->root) : tmp;

		res = pthread_rwlock_unlock(&shard->lock);
		assert(res == 0);
	}

	return 0;
}

uint32_t seen_only_once(struct tree_owner *owner) {
	return owner->tree_size - owner->repeated_elements;
}

uint32_t aggregate_unique_numbers(struct tree_owner ctx[], uint32_t shards) {
	uint32_t i;
	int res = 0;
	
	for (i = 0; i < shards; i++)
		res += ctx[i].tree_size;

	return res;
}

uint32_t aggregate_seen_only_once(struct tree_owner ctx[], uint32_t shards) {
	uint32_t i;
	int res = 0;
	
	for (i = 0; i < shards; i++) {
		res += seen_only_once(&ctx[i]);
		//printf("%d: seen numbers %u\n", i, seen_only_once(&ctx[i]));
	}

	return res;
}

#endif
