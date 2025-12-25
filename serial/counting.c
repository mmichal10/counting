#ifndef __COUNTING_C__
#define __COUNTING_C__

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <pthread.h>

#include "hashtable.c"
#include "parse_json.c"
#include "hash.c"

struct counting_ctx {
	uint32_t shards_count;
	struct hash_table_shard *shards;
	pthread_rwlock_t *locks;
};

int counting_init(struct counting_ctx *ctx, uint32_t shards_count, uint32_t max_hashes) {
	uint64_t init_shards_counter = 0;
	uint64_t init_locks_counter = 0;
	uint64_t i;
	int ret;
	uint32_t shard_range = max_hashes / shards_count;

	ctx->shards = calloc(shards_count, sizeof(struct hash_table_shard));
	assert(ctx->shards);
	ctx->locks = calloc(shards_count, sizeof(pthread_rwlock_t));
	assert(ctx->locks);

	ctx->shards_count = shards_count;

	if (shards_count > 1) {
		for (i = 0; i < shards_count - 1; i++, init_shards_counter++) {
			ret = hashtable_init(&ctx->shards[i], i*shard_range, (i + 1) * shard_range);
			if (ret != 0)
				goto err;
		}
	}

	i = shards_count - 1;
	ret = hashtable_init(&ctx->shards[i], i*shard_range, max_hashes);
	if (ret != 0)
		goto err;

	init_shards_counter++;

	for (i = 0; i < shards_count; i++, init_locks_counter++) {
		ret = pthread_rwlock_init(&ctx->locks[i], 0);
		if (ret != 0)
			goto err;
	}

	return 0;

err:
	for (i = 0; i < init_shards_counter; i++)
			hashtable_deinit(&ctx->shards[i]);
	for (i = 0; i < init_locks_counter; i++)
			assert(pthread_rwlock_destroy(&ctx->locks[i]) == 0);

	free(ctx->shards);
	free(ctx->locks);

	return 1;
}

void counting_deinit(struct counting_ctx *ctx) {
	uint32_t i;

	for (i = 0; i < ctx->shards_count; i++) {
		hashtable_deinit(&ctx->shards[i]);
		assert(pthread_rwlock_destroy(ctx[i].locks) == 0);
	}

	free(ctx->shards);
	free(ctx->locks);
}

int counting_insert_model(struct counting_ctx *ctx, char* model) {
	int res;
	uint32_t hash;
	uint32_t shard_id;
	struct hash_table_entry* entry;

	hash = FNV(model);
	shard_id = hash % ctx->shards_count;

	res = pthread_rwlock_rdlock(&ctx->locks[shard_id]);
	assert(res == 0);

	entry = hashtable_lookup(&ctx->shards[shard_id], model, hash);
	if (!entry) {
		res = pthread_rwlock_unlock(&ctx->locks[shard_id]);
		assert(res == 0);
		goto insert_new_element;
	}

	atomic_fetch_add(&entry->count, 1);

	res = pthread_rwlock_unlock(&ctx->locks[shard_id]);
	assert(res == 0);

	return 0;

insert_new_element:
	res = pthread_rwlock_wrlock(&ctx->locks[shard_id]);
	assert(res == 0);

	res = hashtable_insert(&ctx->shards[shard_id], model, hash, FNV);
	assert(res == 0); // TODO sometimes it's recoverable that the allocation failed 

	res = pthread_rwlock_unlock(&ctx->locks[shard_id]);
	assert(res == 0);

	return res;
}

uint32_t counting_models(struct counting_ctx *ctx, char* buffer, uint32_t remaining_buffer_len) {
	int res;
	uint32_t processed_bytes = 0;
	char* curr_model_start_pos;
	char* curr_model_end_pos;
	char* curr_json_entry_begining;

	//assert(buffer[remaining_buffer_len - 1] == '}');

	while (remaining_buffer_len > 0) {
		curr_json_entry_begining = memchr(buffer, '{', remaining_buffer_len);
		if (curr_json_entry_begining == NULL) {
			processed_bytes += remaining_buffer_len;
			break;
		}

		processed_bytes += (curr_json_entry_begining - buffer);
		remaining_buffer_len -= (curr_json_entry_begining - buffer);
		buffer = curr_json_entry_begining;

		curr_model_start_pos = json_get_next_model(buffer, remaining_buffer_len);
		if (*curr_model_start_pos == '}') {
			processed_bytes += remaining_buffer_len;
			break;
		}

		curr_model_end_pos = memchr(curr_model_start_pos, '\"', remaining_buffer_len);
		assert(curr_model_end_pos != NULL); // The input must be valid JSON

		*curr_model_end_pos = 0;
		res = counting_insert_model(ctx, curr_model_start_pos);
		assert(res == 0);

		curr_model_end_pos++;

		processed_bytes += (curr_model_end_pos - buffer);
		remaining_buffer_len -= (curr_model_end_pos - buffer);

		buffer = curr_model_end_pos;
	}

	return processed_bytes;
}

#endif
