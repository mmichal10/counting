#ifndef __COUNTING_C__
#define __COUNTING_C__

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <pthread.h>

#include "hashtable.c"
#include "hash.c"

#define SHARD_COUNT 4ul
#define MAX_HASHES (2ul << 28)
// (2 << 28) * 4 bytes == 1G of preallocated memory

struct counting_ctx {
	struct hash_table_shard *shards;
	pthread_rwlock_t *locks;
};

int counting_init(struct counting_ctx *ctx, uint32_t shard_count, uint32_t max_hashes) {
	int i;
	int ret;
	uint32_t shard_range = MAX_HASHES / SHARD_COUNT;

	ctx->shards = calloc(shard_count, sizeof(struct hash_table_shard));
	assert(ctx->shards);
	ctx->locks = calloc(shard_count, sizeof(pthread_rwlock_t));
	assert(ctx->locks);

	for (i = 0; i < SHARD_COUNT - 1; i++) {
		ret = hashtable_init(ctx->shards[i], i*shard_range, (i + 1) * shard_range);
		if (ret != 0)
			goto err;
	}
	i = SHARD_COUNT - 1;
	ret = hashtable_init(ctx->shards[i], i*shard_range, MAX_HASHES);
	if (ret != 0)
		goto err;

	for (i = 0; i < SHARD_COUNT; i++) {
		ret = pthread_rwlock_init(&ctx[i].lock, 0);
		if (ret != 0)
			goto err;
	}

	return 0;
err:
	for (i = 0; i < SHARD_COUNT; i++) {
		if (ctx->shards[i] != NULL)
			hashtable_deinit(ctx->shards[i]);
		if (ctx->locks[i] != NULL)
			assert(pthread_rwlock_destroy(&ctx[i].lock) == 0)
	}

	free(ctx->shards);
	free(ctx->locks);

	return 1;
}

void counting_deinit(struct counting_ctx *ctx) {
	int i;

	for (i = 0; i < SHARD_COUNT; i++) {
		hashtable_deinit(ctx->shards[i]);
		assert(pthread_rwlock_destroy(&ctx[i].lock) == 0)
	}

	free(ctx->shards);
	free(ctx->locks);
}
#endif
