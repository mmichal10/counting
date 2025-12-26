#ifndef __PARSE_JSON_C__
#define __PARSE_JSON_C__

#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define MAX(__A, __B) (__A > __B ? __A : __B)
#define MIN(__A, __B) (__A < __B ? __A : __B)
#define JSON_BUFFER_SIZE 128ULL

uint64_t json_find_offset_of_first_occurance(int fd, uint64_t offset_in_file, char character) {
	int64_t i;
	int64_t read_bytes;

	assert(sizeof(off_t) >= sizeof(uint64_t));

	char buffer[JSON_BUFFER_SIZE] = {};

	while (1) {
		// uint64_t should never overflow
		assert(offset_in_file + JSON_BUFFER_SIZE > offset_in_file);

		read_bytes = pread(fd, buffer, JSON_BUFFER_SIZE, offset_in_file);
		assert(!(read_bytes < 0));

		if (read_bytes == 0)
			break;

		for (i = 0; i < read_bytes; i++) {
			if (buffer[i] == character)
				goto end;
		}

		offset_in_file += JSON_BUFFER_SIZE;
	}
end:

	assert(buffer[i] == character);

	return offset_in_file + i;
}

uint64_t json_find_begin_offset(int fd, uint64_t offset_in_file) {
	return json_find_offset_of_first_occurance(fd, offset_in_file, '{');
}

uint64_t json_find_end_offset(int fd, uint64_t offset_in_file) {
	return json_find_offset_of_first_occurance(fd, offset_in_file, '}');
}

void json_shard_the_file(int fd, uint64_t shards_beginings[],
		uint64_t shards_endings[], uint32_t shards_count) {
	uint64_t i;
	uint64_t previous_end_offset = 0;
	uint64_t file_size;
	uint64_t file_chunk_size;

 	file_size = lseek(fd, 0L, SEEK_END);
	file_chunk_size = file_size / shards_count;

	for (i = 0; i < shards_count; i++) {
		shards_beginings[i] = json_find_begin_offset(fd, previous_end_offset);

		if (i == shards_count - 1)
			shards_endings[i] = file_size;
		else
			shards_endings[i] = json_find_end_offset(fd, file_chunk_size * (i + 1));

		previous_end_offset = shards_endings[i];
	}
}

#define JSON_MODEL_KEY "\"model\""
#define JSON_MODEL_KEY_LEN 7

char *json_get_next_model(char *src_buffer, uint32_t src_buffer_size) {
	char *uuid_key;
	char *uuid_val;

	if (src_buffer[0] != '{') {
		printf("%s %u: Parsing failed\n", __FILE__, __LINE__);
		return NULL;
	}
	uuid_key = memmem(src_buffer, src_buffer_size, JSON_MODEL_KEY, strlen(JSON_MODEL_KEY));

	if (!uuid_key) {
		assert(src_buffer[src_buffer_size - 1] == '}' || src_buffer[src_buffer_size - 1] == ']');
		return src_buffer + src_buffer_size - 1;
	}

	uuid_val = uuid_key + JSON_MODEL_KEY_LEN;

	while (*uuid_val != '\"' && *uuid_val != '}')
		uuid_val++;

	if (*uuid_val != '\"') {
		assert(src_buffer[src_buffer_size - 1] == '}');
		return src_buffer + src_buffer_size - 1;
	}

	uuid_val++;

	return uuid_val;
}

#endif
