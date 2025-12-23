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

/*
void test_json_find_model_real_data(void) {
	char input_buffer[] = {"{\"id\":0,\"model\":\"RDV2\",\"serial\":\"HD92731454\"},{\"id\":1,\"model\":\"DRV1\",\"serial\":\"HD78784271\"},{\"id\":2,\"model\":\"SSDLP2\",\"serial\":\"HD74286234\"},{\"id\":3,\"model\":\"SSDF1\",\"serial\":\"HD18368977\"},{\"id\":4,\"model\":\"123456789\",\"serial\":\"HD71847139\"},{\"id\":5,\"model\":\"HGST2048T\",\"serial\":\"HD60894593\"},{\"id\":6,\"model\":\"DRV1\",\"serial\":\"HD61514441\"},{\"id\":7,\"model\":\"DSD07461\",\"serial\":\"HD69247082\"},{\"id\":8,\"model\":\"broken\",\"serial\":\"HD30186042\"},{\"id\":9,\"model\":\"broken\",\"serial\":\"HD46171584\"},{\"id\":10,\"model\":\"broken\",\"serial\":\"HD96999858\"},{\"id\":11,\"model\":\"SSDDC1\",\"serial\":\"HD19987737\"},{\"id\":12,\"model\":\"RDV2\",\"serial\":\"HD23665165\"},{\"id\":13,\"model\":\"DRV1\",\"serial\":\"HD89836002\"},{\"id\":14,\"model\":\"SSDLP2\",\"serial\":\"HD71643186\"},{\"id\":15,\"model\":\"RDV2\",\"serial\":\"HD96068276\"},{\"id\":16,\"model\":\"HGST2048T\",\"serial\":\"HD55004428\"},{\"id\":17,\"model\":\"broken\",\"serial\":\"HD57141352\"},{\"id\":18,\"model\":\"SSDF1\",\"serial\":\"HD47013370\"},{\"id\":19,\"model\":\"SSDF1\",\"serial\":\"HD69336988\"}"};
	uint32_t input_buffer_len = strlen(input_buffer);

	char dst_buffer[16] = {};
}
*/

int main(void) {
    UNITY_BEGIN();
	RUN_TEST(test_json_find_begin);
	RUN_TEST(test_json_find_end);
	RUN_TEST(test_json_find_model);
	RUN_TEST(test_json_find_model_negative);

    return UNITY_END();
}
