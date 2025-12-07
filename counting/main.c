#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>

#include "tree.h"
#include "counting.h"

struct tree_owner trees[SHARDS] = {};

struct pthread_ctx {
	uint32_t start_pos;
	uint32_t end_pos;
	uint32_t id;
	int file_descryptor;
};

#define MIN(__A, __B) (__A < __B ? __A : __B)

#define THREAD_COUNT 8

void *sharded_counting(void *param) {
	struct pthread_ctx *ctx = param;
	uint32_t file_position = 0;
	int read_bytes;
	int read_size;
	uint32_t id;

	//printf("Start %u, end %u\n", ctx->start_pos, ctx->end_pos);
	file_position = ctx->start_pos;

	while (1) {
		uint32_t arr[1024] = {};
		read_size = MIN(sizeof(arr), ctx->end_pos - file_position);

		printf("Start %u, end %u file pos %u, read size %u, read bytes %u\n",
			ctx->start_pos, ctx->end_pos, file_position, read_size, read_bytes);

		read_bytes = pread(ctx->file_descryptor, arr, read_size, file_position);
		if (read_bytes < 1)
			break;

		file_position += read_bytes;
		
		count_numbers(arr, read_bytes/sizeof(uint32_t), trees);
	}

	printf("Exit thread %u\n", ctx->id);

	return NULL;
}

int main() {	
	int i;
	int res;
	char *inputfile_name = "random_numbers";
	uint32_t tree_size_aggregate = 0;
	uint32_t unique_elements_aggregate = 0;

	pthread_t threads[THREAD_COUNT] = {};
	struct pthread_ctx *thread_params[THREAD_COUNT] = {};

	int fd = open(inputfile_name, O_RDONLY);
	if (fd == 0) {
		printf("Failed to open file\n");
		return 1;
	}

	prepare_shards(trees, SHARDS, SHARD_SIZE);

	assert(trees[SHARDS - 1].shard_range_max == MAX_INPUT);

	for (i = 0; i < THREAD_COUNT; i++) {
		thread_params[i] = malloc(sizeof(struct pthread_ctx));
		if (thread_params[i] == NULL)
			goto end;

		thread_params[i]->start_pos = i * 8192;
		thread_params[i]->end_pos = (i+1) * 8192;
		thread_params[i]->file_descryptor = fd;
		thread_params[i]->id = i;

		res = pthread_create(&threads[i], NULL, sharded_counting, thread_params[i]);
		if (res != 0) {
			printf("Failed to initialize thread %d\n", i);
			goto end;
		}
	}
end:

	for (i = 0; i < THREAD_COUNT; i++) {
		if (threads[i] != 0) {
			printf("Joining %u\n", i);
			res = pthread_join(threads[i], NULL);
			assert(res == 0);
		}

		free(thread_params[i]);
	}

	printf("Unique numbers %u\n", aggregate_unique_numbers(trees, SHARDS));
	printf("Seen only once %u\n", aggregate_seen_only_once(trees, SHARDS));

	destroy_shards(trees, SHARDS, SHARD_SIZE);

	close(fd);
}
