#include "unity.h"
#include "parse_json.c"
#include <string.h>
#include <stdint.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void test_json_find_begin(void) {
	const char *inputfile_name = "test_json.txt";
	FILE *generated_file = fopen(inputfile_name, "wb");
	uint64_t result;
	int i;
	int ret;
	char c = '}';
	

	for (i = 0; i < 4096; i++) {
		ret = fwrite(&c, sizeof(c), 1, generated_file);		
		if (ret != 1) {
			printf("Failed to generate input file.\n");
			TEST_ASSERT_EQUAL(ret, 1);
		}

		if (i % 2048 == 2047)
			c = '{';
	}

	fclose(generated_file);

	int f = open(inputfile_name, O_RDONLY);
	TEST_ASSERT_NOT_EQUAL(f, 0);

	result = json_find_begin_offset(f, 0);
	TEST_ASSERT_EQUAL_UINT64(2048ULL, result);

	close(f);
}

void test_json_find_end(void) {
	const char *inputfile_name = "test_json.txt";
	FILE *generated_file = fopen(inputfile_name, "wb");
	uint64_t result;
	int i;
	int ret;
	char c = '{';

	// Test what happens if the target char is at the end of the file and the file size is unaligned
	for (i = 0; i < 2048 + 57; i++) {
		ret = fwrite(&c, sizeof(c), 1, generated_file);		
		if (ret != 1) {
			printf("Failed to generate input file.\n");
			TEST_ASSERT_EQUAL(ret, 1);
		}
	}

	c = '}';
	ret = fwrite(&c, sizeof(c), 1, generated_file);		
	if (ret != 1) {
		printf("Failed to generate input file.\n");
		TEST_ASSERT_EQUAL(ret, 1);
	}

	fclose(generated_file);

	int f = open(inputfile_name, O_RDONLY);
	TEST_ASSERT_NOT_EQUAL(f, 0);

	result = json_find_end_offset(f, 123ULL);
	TEST_ASSERT_EQUAL_UINT64(2048ULL + 57ULL, result);

	close(f);
}

void test_json_find_model(void) {
	char input_buffer[128] = {"{\"id\": 0,\"model\":\"RDV2\",\"serial\":\"costam\"}"};
	char *model_ptr;

	model_ptr = json_get_next_model(input_buffer, strlen(input_buffer));

	TEST_ASSERT_EQUAL_MEMORY(model_ptr, model_ptr, 4);
}

void test_json_find_model_negative(void) {
	char input_buffer[128] = {"{\"id\": 0,\"serial\":\"costam\"}"};
	char *model_ptr;

	model_ptr = json_get_next_model(input_buffer, strlen(input_buffer));

	TEST_ASSERT_EQUAL('}', *model_ptr);
}

int main(void) {
    UNITY_BEGIN();
	RUN_TEST(test_json_find_begin);
	RUN_TEST(test_json_find_end);
	RUN_TEST(test_json_find_model);
	RUN_TEST(test_json_find_model_negative);

    return UNITY_END();
}
