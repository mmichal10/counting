#include "counting.c"
#include "unity.h"
#include <string.h>

static struct tree_owner *find_expected_shard(struct tree_owner ctx[], uint32_t number)
{
	uint32_t i;

	for (i = 0; i < SHARDS; i++) {
		if (number >= ctx[i].shard_range_min && number <= ctx[i].shard_range_max)
			return &ctx[i];
	}

	return NULL;
}

static uint32_t total_unique_numbers(struct tree_owner ctx[])
{
	return aggregate_unique_numbers(ctx, SHARDS);
}

static uint32_t total_seen_once_numbers(struct tree_owner ctx[])
{
	return aggregate_seen_only_once(ctx, SHARDS);
}

static uint32_t total_repeated_numbers(struct tree_owner ctx[])
{
	return total_unique_numbers(ctx) - total_seen_once_numbers(ctx);
}

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
			? ((uint64_t)COUNTING_MAX_INPUT)
			: ((uint64_t)(i + 1) * (uint64_t)SHARD_SIZE) - 1;

		TEST_ASSERT_EQUAL_UINT64((uint64_t)expected_min64, (uint64_t)ctx[i].shard_range_min);
		TEST_ASSERT_EQUAL_UINT64((uint64_t)expected_max64, (uint64_t)ctx[i].shard_range_max);
		TEST_ASSERT_TRUE(ctx[i].shard_range_min < ctx[i].shard_range_max);

		if (i > 0) {
			TEST_ASSERT_EQUAL_UINT32(
				(uint32_t)(((uint64_t)ctx[i - 1].shard_range_max) + 1),
				ctx[i].shard_range_min);
		}
	}

	TEST_ASSERT_EQUAL_UINT64(0ul, ctx[0].shard_range_min);
	TEST_ASSERT_EQUAL_UINT64(COUNTING_MAX_INPUT, ctx[SHARDS - 1].shard_range_max);

	destroy_shards(ctx, SHARDS);
}

void test_get_shard_returns_correct_shard(void)
{
	struct tree_owner ctx[SHARDS] = {0};
	uint32_t i;

	TEST_ASSERT_TRUE(SHARDS > 0);
	TEST_ASSERT_TRUE(SHARD_SIZE > 0);

	prepare_shards(ctx, SHARDS, SHARD_SIZE);

	for (i = 0; i < SHARDS; i++) {
		struct tree_owner *expected = &ctx[i];
		struct tree_owner *actual;

		actual = get_shard(ctx, ctx[i].shard_range_min);
		TEST_ASSERT_EQUAL_PTR(expected, actual);

		actual = get_shard(ctx, ctx[i].shard_range_max);
		TEST_ASSERT_EQUAL_PTR(expected, actual);

		if (ctx[i].shard_range_max > ctx[i].shard_range_min) {
			uint64_t midpoint64 = (uint64_t)ctx[i].shard_range_min +
				((uint64_t)ctx[i].shard_range_max - (uint64_t)ctx[i].shard_range_min) / 2;
			uint32_t midpoint = (uint32_t)midpoint64;

			actual = get_shard(ctx, midpoint);
			TEST_ASSERT_EQUAL_PTR(expected, actual);

			actual = get_shard(ctx, ctx[i].shard_range_min + 1);
			TEST_ASSERT_EQUAL_PTR(expected, actual);

			actual = get_shard(ctx, ctx[i].shard_range_max - 1);
			TEST_ASSERT_EQUAL_PTR(expected, actual);
		}

		if (i > 0) {
			uint64_t handoff_value = ctx[i].shard_range_min;
			struct tree_owner *previous_expected = &ctx[i - 1];
			struct tree_owner *previous_actual = get_shard(ctx, handoff_value - 1);

			TEST_ASSERT_EQUAL_PTR(previous_expected, previous_actual);

			actual = get_shard(ctx, handoff_value);
			TEST_ASSERT_EQUAL_PTR(expected, actual);
		}

		if (i < SHARDS - 1) {
			uint64_t handoff_value = ctx[i].shard_range_max;
			struct tree_owner *next_expected = &ctx[i + 1];
			struct tree_owner *next_actual = get_shard(ctx, handoff_value + 1);

			TEST_ASSERT_EQUAL_PTR(next_expected, next_actual);

			actual = get_shard(ctx, handoff_value);
			if (expected != actual)
				TEST_ASSERT_EQUAL_PTR(expected, actual);
		}
	}

	struct tree_owner *expected_last = &ctx[SHARDS-1];
	struct tree_owner *actual_last = get_shard(ctx, COUNTING_MAX_INPUT);

	TEST_ASSERT_NOT_NULL(expected_last);
	TEST_ASSERT_EQUAL_PTR(expected_last, actual_last);

	if (COUNTING_MAX_INPUT > 0) {
		struct tree_owner *expected_near_last = find_expected_shard(ctx, COUNTING_MAX_INPUT - 1);
		struct tree_owner *actual_near_last = get_shard(ctx, COUNTING_MAX_INPUT - 1);

		TEST_ASSERT_NOT_NULL(expected_near_last);
		TEST_ASSERT_EQUAL_PTR(expected_near_last, actual_near_last);
	}

	destroy_shards(ctx, SHARDS);
}

void test_countint_1(void)
{
	uint32_t i;
	struct tree_owner ctx[SHARDS] = {};
	prepare_shards(ctx, SHARDS, SHARD_SIZE);

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

	TEST_ASSERT_EQUAL(count_numbers(arr, 1000, ctx), 0);

	TEST_ASSERT_EQUAL(aggregate_unique_numbers(ctx, 16), 900);
	TEST_ASSERT_EQUAL(aggregate_seen_only_once(ctx, 16), 810);

	destroy_shards(ctx, SHARDS);
}

void test_countint_2(void)
{
	struct tree_owner ctx[SHARDS] = {};
	prepare_shards(ctx, SHARDS, SHARD_SIZE);

	uint32_t arr[10];
	arr[0] = 1;
	arr[1] = 2;
	arr[2] = 2;
	arr[3] = 3;
	arr[4] = 3;
	arr[5] = 4;
	arr[6] = 2;
	arr[7] = 3;

	TEST_ASSERT_EQUAL(count_numbers(arr, 6, ctx), 0);

	TEST_ASSERT_EQUAL(aggregate_unique_numbers(ctx, 16), 4);
	TEST_ASSERT_EQUAL(aggregate_seen_only_once(ctx, 16), 2);

	uint32_t arr2[2];
	arr2[0] = 1;
	arr2[1] = 2;
	TEST_ASSERT_EQUAL(count_numbers(arr2, 2, ctx), 0);

	TEST_ASSERT_EQUAL(aggregate_unique_numbers(ctx, 16), 4);
	TEST_ASSERT_EQUAL(aggregate_seen_only_once(ctx, 16), 1);

	destroy_shards(ctx, SHARDS);
}

void test_tree_api_handles_high_range_duplicates(void)
{
	struct tree_owner ctx[SHARDS] = {0};
	const uint64_t range_start = 0xfffffffflu - (1024lu * 1024lu);
	const uint64_t range_end = 0xfffffffflu;
	const uint64_t range_count = (1024lu * 1024lu) + 1lu;
	const uint64_t segment = 16384lu;
	const uint64_t expected_seen_once = range_count - (2lu * segment);
	uint32_t buffer[16384] = {};

	prepare_shards(ctx, SHARDS, SHARD_SIZE);

	for (int i = 0; i < 64; i++) {
		for (int j = 0; j < (int)segment; j++) {
			buffer[j] = (uint32_t)(range_start + i*segment + j);
		}
		count_numbers(buffer, segment, ctx);
		memset(buffer, 0, sizeof(uint32_t) * segment);
	}

	buffer[0] = 0xfffffffful;
	count_numbers(buffer, 1, ctx);
	memset(buffer, 0, sizeof(uint32_t) * segment);

	TEST_ASSERT_EQUAL_UINT64(range_count, total_unique_numbers(ctx));
	TEST_ASSERT_EQUAL_UINT64(range_count, total_seen_once_numbers(ctx));
	TEST_ASSERT_EQUAL_UINT64(0ul, total_repeated_numbers(ctx));

	for (int i = 0; i < (int)segment; i++)
		buffer[i] = (uint32_t)(range_start + (uint32_t)i);

	count_numbers(buffer, (int)segment, ctx);
	memset(buffer, 0, sizeof(uint32_t) * 16384);

	TEST_ASSERT_EQUAL_UINT64(range_count, total_unique_numbers(ctx));
	TEST_ASSERT_EQUAL_UINT64(range_count - segment, total_seen_once_numbers(ctx));
	TEST_ASSERT_EQUAL_UINT64(segment, total_repeated_numbers(ctx));

	for (int i = 0; i < (int)(segment); i++)
		buffer[i] = (uint32_t)(range_end - segment + (uint32_t)i + 1ul);

	count_numbers(buffer, segment, ctx);
	memset(buffer, 0, sizeof(uint32_t) * 16384);

	TEST_ASSERT_EQUAL_UINT64(range_count, total_unique_numbers(ctx));
	TEST_ASSERT_EQUAL_UINT64(expected_seen_once, total_seen_once_numbers(ctx));
	TEST_ASSERT_EQUAL_UINT64(2lu * segment, total_repeated_numbers(ctx));

	for (int i = 0; i < (int)(segment); i++)
		buffer[i] = (uint32_t)(range_end - segment + (uint32_t)i + 1ul);

	for (int loop = 0; loop < 10; ++loop) {
		count_numbers(buffer, segment, ctx);

		TEST_ASSERT_EQUAL_UINT64(range_count, total_unique_numbers(ctx));
		TEST_ASSERT_EQUAL_UINT64(expected_seen_once, total_seen_once_numbers(ctx));
		TEST_ASSERT_EQUAL_UINT64(2lu * segment, total_repeated_numbers(ctx));
	}

	destroy_shards(ctx, SHARDS);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_prepare_shards_cover_full_range);
    RUN_TEST(test_tree_api_handles_high_range_duplicates);
    RUN_TEST(test_get_shard_returns_correct_shard);
    RUN_TEST(test_countint_1);
    RUN_TEST(test_countint_2);

    return UNITY_END();
}
