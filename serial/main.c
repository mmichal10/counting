#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include "counting.c"
#include "parse_json.c"

struct pthread_ctx {
	uint64_t start_pos;
	uint64_t end_pos;
	int file_descryptor;
	int shard_id;
	struct counting_ctx *hash_table; 
};

#define MIN(__A, __B) (__A < __B ? __A : __B)

#define READ_BATCH_SIZE 16384
#define LOG_INTERVAL (READ_BATCH_SIZE * READ_BATCH_SIZE)

#define THREAD_COUNT 2ULL

#define SHARD_COUNT 4ul
#define MAX_HASHES (2ul << 8)
// (2 << 28) * 4 bytes == 1G of preallocated memory


void *sharded_counting(void *param) {
	struct pthread_ctx *ctx = param;
	uint64_t file_position = 0;
	int read_bytes;
	int read_size;
	uint64_t processed_bytes;
	char arr[READ_BATCH_SIZE] = {};
	char* last_entry_end;
	char* next_entry_begin;
	uint64_t log_threshold = ctx->start_pos + LOG_INTERVAL;

	printf("Worker %d: STARTED\n", ctx->shard_id);
	file_position = ctx->start_pos;

	printf("Worker %d: start %lu end %lu\n", ctx->shard_id, ctx->start_pos, ctx->end_pos);

	while (1) {
		read_size = MIN(sizeof(arr), ctx->end_pos - file_position);

		if (read_size < sizeof(arr))
			memset(arr, 0, sizeof(arr));

		assert(file_position >= ctx->start_pos);
		assert(file_position + read_size <= ctx->end_pos);

		read_bytes = pread(ctx->file_descryptor, arr, read_size, file_position);
		if (read_bytes < 1)
			break;

		next_entry_begin = memrchr(arr, '{', read_bytes);

		last_entry_end = memrchr(arr, '}', read_bytes);
		if (last_entry_end)
			read_bytes = last_entry_end - arr;

		if (read_bytes < 1)
			break;

		processed_bytes = counting_models(ctx->hash_table, arr, read_bytes);
		if (next_entry_begin && last_entry_end && next_entry_begin > last_entry_end)
			read_bytes = next_entry_begin - arr;

		file_position += read_bytes;

		if (file_position > log_threshold) {
			printf("Worker %d: processed %.2f%%\n", ctx->shard_id,
					100 * ((float)(file_position - ctx->start_pos)/(float)(ctx->end_pos - ctx->start_pos)));
			log_threshold += LOG_INTERVAL;
		}
	}
	printf("Worker %d: FINISHED\n", ctx->shard_id);

	return NULL;
}

int main(int argc, char *argv[]) {	
	uint32_t i, j;
	int res;
	uint32_t file_size;
	uint64_t file_chunk_size;
	struct counting_ctx ctx = {};
	pthread_t threads[THREAD_COUNT] = {};
	struct pthread_ctx *thread_params[THREAD_COUNT] = {};
	uint64_t file_shards_beginings[THREAD_COUNT];
	uint64_t file_shards_endings[THREAD_COUNT];

	if (argc != 2) {
		printf("Usage: %s <path>\n", argv[0]);
		return 1;
	}

	int fd = open(argv[1], O_RDONLY);
	if (fd == 0) {
		printf("Failed to open file\n");
		return 1;
	}

 	file_size = lseek(fd, 0L, SEEK_END);
	file_chunk_size = file_size / THREAD_COUNT;

	res = counting_init(&ctx, SHARD_COUNT, MAX_HASHES);
	if (res != 0) {
		printf("Failed to initialize context\n");
		close(fd);
		return 1;
	}

	assert(ctx.shards[SHARD_COUNT - 1].range_end == MAX_HASHES);

	json_shard_the_file(fd, file_shards_beginings, file_shards_endings, THREAD_COUNT);

	for (i = 0; i < THREAD_COUNT; i++) {
		thread_params[i] = malloc(sizeof(struct pthread_ctx));
		if (thread_params[i] == NULL)
			goto end;

		thread_params[i]->hash_table = &ctx;
		thread_params[i]->file_descryptor = fd;
		thread_params[i]->shard_id = i;
		thread_params[i]->start_pos = file_shards_beginings[i];
		thread_params[i]->end_pos = file_shards_endings[i];

		res = pthread_create(&threads[i], NULL, sharded_counting, thread_params[i]);
		if (res != 0) {
			printf("Failed to initialize thread %d\n", i);
			goto end;
		}
	}

end:

	for (i = 0; i < THREAD_COUNT; i++) {
		if (threads[i] != 0) {
			res = pthread_join(threads[i], NULL);
			assert(res == 0);
		}

		free(thread_params[i]);
	}

	for (i = 0; i < SHARD_COUNT; i++) {
		struct hash_table_shard *shard = &ctx.shards[i];
		for (j = 0; j < shard->curr_max_entries; j++) {
			struct hash_table_entry* entry = &shard->entries[j];
			if (entry->key[0] == 0)
				continue;

			printf("%lu %s\n",
					atomic_load(&entry->count),
					shard->entries[j].key);
		}
	}

	counting_deinit(&ctx);

	close(fd);
}
