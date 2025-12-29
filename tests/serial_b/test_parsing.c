#include "counting.c"
#include "unity.h"
#include <string.h>
#include <stdint.h>

void setUp(void)
{
}

void tearDown(void)
{
}

uint32_t entry_count_test_helper(struct hash_table_entry *entry) {
	return atomic_load(&entry->count);
}

struct hash_table_shard *hash_to_shard_test_helper(struct counting_ctx *ctx, const char *key) {
	uint32_t hash = FNV(key);

	return &ctx->shards[hash % ctx->shards_count];
}

void test_count_models_1(void) {
	char input_buffer[] = {
		"{\"id\":0,\"model\":\"RDV2\",\"serial\":\"HD92731454\"},\
		{\"id\":1,\"model\":\"DRV1\",\"serial\":\"HD78784271\"},\
		{\"id\":2,\"model\":\"SSDLP2\",\"serial\":\"HD74286234\"},\
		{\"id\":3,\"model\":\"SSDF1\",\"serial\":\"HD18368977\"},\
		{\"id\":4,\"model\":\"123456789\",\"serial\":\"HD71847139\"},\
		{\"id\":5,\"model\":\"HGST2048T\",\"serial\":\"HD60894593\"},\
		{\"id\":6,\"model\":\"DRV1\",\"serial\":\"HD61514441\"},\
		{\"id\":7,\"model\":\"DSD07461\",\"serial\":\"HD69247082\"},\
		{\"id\":8,\"model\":\"broken\",\"serial\":\"HD30186042\"},\
		{\"id\":9,\"model\":\"broken\",\"serial\":\"HD46171584\"},\
		{\"id\":10,\"model\":\"broken\",\"serial\":\"HD96999858\"},\
		{\"id\":11,\"model\":\"SSDDC1\",\"serial\":\"HD19987737\"},\
		{\"id\":12,\"model\":\"RDV2\",\"serial\":\"HD23665165\"},\
		{\"id\":13,\"model\":\"DRV1\",\"serial\":\"HD89836002\"},\
		{\"id\":14,\"model\":\"SSDLP2\",\"serial\":\"HD71643186\"},\
		{\"id\":15,\"model\":\"RDV2\",\"serial\":\"HD96068276\"},\
		{\"id\":16,\"model\":\"HGST2048T\",\"serial\":\"HD55004428\"},\
		{\"id\":17,\"model\":\"broken\",\"serial\":\"HD57141352\"},\
		{\"id\":18,\"model\":\"SSDF1\",\"serial\":\"HD47013370\"},\
		{\"id\":19,\"model\":\"SSDF1\",\"serial\":\"HD69336988\"},\
		{\"id\":19,\"modl\":\"SSDF1\",\"serial\":\"HD69336988\"}" // THIS ENTRY IS INVALID
	};
	uint32_t input_buffer_len = strlen(input_buffer);
	struct counting_ctx ctx = {};
	uint32_t shards_count = 2;
	uint32_t max_hashes = 8;
	uint32_t i;
	uint32_t expected_entries_count = 0;
	uint32_t actaul_entries_count = 0;
	struct hash_table_entry *entry;
	struct hash_table_shard *shard;
	int ret;

	ret = counting_init(&ctx, shards_count, max_hashes);
	TEST_ASSERT_EQUAL(0, ret);

	ret = counting_models(&ctx, input_buffer, input_buffer_len);

	{
		const char *key = "RDV2";
		shard = hash_to_shard_test_helper(&ctx, key);
		entry = hashtable_lookup(shard, key, FNV(key));
		TEST_ASSERT_EQUAL(3, entry_count_test_helper(entry));
		expected_entries_count++;
	}

	{
		const char *key = "DRV1";
		shard = hash_to_shard_test_helper(&ctx, key);
		entry = hashtable_lookup(shard, key, FNV(key));
		TEST_ASSERT_EQUAL(3, entry_count_test_helper(entry));
		expected_entries_count++;
	}

	{
		const char *key = "SSDLP2";
		shard = hash_to_shard_test_helper(&ctx, key);
		entry = hashtable_lookup(shard, key, FNV(key));
		TEST_ASSERT_EQUAL(2, entry_count_test_helper(entry));
		expected_entries_count++;
	}

	{
		const char *key = "SSDF1";
		shard = hash_to_shard_test_helper(&ctx, key);
		entry = hashtable_lookup(shard, key, FNV(key));
		TEST_ASSERT_EQUAL(3, entry_count_test_helper(entry));
		expected_entries_count++;
	}

	{
		const char *key = "123456789";
		shard = hash_to_shard_test_helper(&ctx, key);
		entry = hashtable_lookup(shard, key, FNV(key));
		TEST_ASSERT_EQUAL(1, entry_count_test_helper(entry));
		expected_entries_count++;
	}

	{
		const char *key = "HGST2048T";
		shard = hash_to_shard_test_helper(&ctx, key);
		entry = hashtable_lookup(shard, key, FNV(key));
		TEST_ASSERT_EQUAL(2, entry_count_test_helper(entry));
		expected_entries_count++;
	}

	{
		const char *key = "DSD07461";
		shard = hash_to_shard_test_helper(&ctx, key);
		entry = hashtable_lookup(shard, key, FNV(key));
		TEST_ASSERT_EQUAL(1, entry_count_test_helper(entry));
		expected_entries_count++;
	}

	{
		const char *key = "broken";
		shard = hash_to_shard_test_helper(&ctx, key);
		entry = hashtable_lookup(shard, key, FNV(key));
		TEST_ASSERT_EQUAL(4, entry_count_test_helper(entry));
		expected_entries_count++;
	}

	{
		const char *key = "SSDDC1";
		shard = hash_to_shard_test_helper(&ctx, key);
		entry = hashtable_lookup(shard, key, FNV(key));
		TEST_ASSERT_EQUAL(1, entry_count_test_helper(entry));
		expected_entries_count++;
	}

	for (i = 0; i < shards_count; i++)
		actaul_entries_count += ctx.shards[i].entries_count;

	TEST_ASSERT_EQUAL(expected_entries_count, actaul_entries_count);

	counting_deinit(&ctx);
}

int main(void) {
    UNITY_BEGIN();
	RUN_TEST(test_count_models_1);

    return UNITY_END();
}
