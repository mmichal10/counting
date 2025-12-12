#include "counting.c"
#include "unity.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_prepare_shards_cover_full_range(void)
{
	TEST_ASSERT_TRUE(SHARDS > 0);

	struct tree_owner ctx[SHARDS] = {0};
	uint32_t i;
	uint64_t expected_min64;
	uint64_t expected_max64;

	prepare_shards(ctx, SHARDS, SHARD_SIZE);

	for (i = 0; i < SHARDS; i++) {
		expected_min64 = (uint64_t)i * SHARD_SIZE;
		expected_max64 = (i == SHARDS - 1)
			? (uint64_t)MAX_INPUT
			: ((uint64_t)(i + 1) * SHARD_SIZE) - 1;

		TEST_ASSERT_EQUAL_UINT32((uint32_t)expected_min64, ctx[i].shard_range_min);
		TEST_ASSERT_EQUAL_UINT32((uint32_t)expected_max64, ctx[i].shard_range_max);
		TEST_ASSERT_TRUE(ctx[i].shard_range_min <= ctx[i].shard_range_max);

		if (i > 0) {
			TEST_ASSERT_EQUAL_UINT32(
				(uint32_t)(((uint64_t)ctx[i - 1].shard_range_max) + 1),
				ctx[i].shard_range_min);
		}
	}

	TEST_ASSERT_EQUAL_UINT32(0u, ctx[0].shard_range_min);
	TEST_ASSERT_EQUAL_UINT32(MAX_INPUT, ctx[SHARDS - 1].shard_range_max);

	destroy_shards(ctx, SHARDS, SHARD_SIZE);
}

void test_countint_1(void)
{
	uint32_t i;

	uint32_t arr[1000];
	for (i = 0; i < 990; i++) {
		arr[i] = i % 900;
	}
	arr[990] = 0;
	arr[991] = 1;
	arr[992] = 2;
	arr[993] = 3;
	arr[994] = 4;
	arr[996] = 5;
	arr[997] = 6;
	arr[998] = 7;
	arr[999] = 8;
	arr[990] = 0;

	struct tree_owner ctx[16] = {};
	ctx[0].shard_range_min = 0;
	ctx[0].shard_range_max = 10000;

	TEST_ASSERT_EQUAL(count_numbers(arr, 1000, ctx), 0);

	TEST_ASSERT_EQUAL(aggregate_unique_numbers(ctx, 16), 900);
	TEST_ASSERT_EQUAL(aggregate_seen_only_once(ctx, 16), 810);
}

void test_countint_2(void)
{
	uint32_t arr[10];
	arr[0] = 1;
	arr[1] = 2;
	arr[2] = 2;
	arr[3] = 3;
	arr[4] = 3;
	arr[5] = 4;
	arr[6] = 2;
	arr[7] = 3;

	struct tree_owner ctx[16] = {};
	ctx[0].shard_range_min = 0;
	ctx[0].shard_range_max = 10000;

	TEST_ASSERT_EQUAL(count_numbers(arr, 6, ctx), 0);

	TEST_ASSERT_EQUAL(aggregate_unique_numbers(ctx, 16), 4);
	TEST_ASSERT_EQUAL(aggregate_seen_only_once(ctx, 16), 2);

	uint32_t arr2[2];
	arr2[0] = 1;
	arr2[1] = 2;
	TEST_ASSERT_EQUAL(count_numbers(arr2, 2, ctx), 0);

	TEST_ASSERT_EQUAL(aggregate_unique_numbers(ctx, 16), 4);
	TEST_ASSERT_EQUAL(aggregate_seen_only_once(ctx, 16), 1);

}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_prepare_shards_cover_full_range);
    RUN_TEST(test_countint_1);
    RUN_TEST(test_countint_2);

    return UNITY_END();
}
