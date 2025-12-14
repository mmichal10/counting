#include "counting.c"
#include "unity.h"

static struct tree_owner *find_expected_shard(struct tree_owner ctx[], uint32_t number)
{
	uint32_t i;

	for (i = 0; i < SHARDS; i++) {
		if (number >= ctx[i].shard_range_min && number <= ctx[i].shard_range_max)
			return &ctx[i];
	}

	return NULL;
}

static struct rb_tree_node *insert_value_into_owner(struct tree_owner *owner, uint32_t value)
{
	struct rb_tree_node *node = rb_tree_find(owner->root, value);

	if (node == NULL) {
		node = rb_tree_insert_and_fix_violations(owner->root, value);
		TEST_ASSERT_NOT_NULL(node);

		owner->tree_size++;
		owner->root = owner->root ? rb_tree_get_root(owner->root) : node;

		node = rb_tree_find(owner->root, value);
		TEST_ASSERT_NOT_NULL(node);
	}

	return node;
}

static void mark_value_as_repeated(struct tree_owner *owner, struct rb_tree_node *node, uint32_t value)
{
	TEST_ASSERT_NOT_NULL(node);

	if (RB_TREE_VALUE_WAS_VISITED(node, value) == 0) {
		owner->repeated_elements++;
		RB_TREE_SET_WAS_VISITED(node, value);
	}
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
			? (uint64_t)MAX_INPUT
			: ((uint64_t)(i + 1) * (uint64_t)SHARD_SIZE) - 1;
		printf("i %u, expected min %llu expected max %llu."
				"Actual min %u actual max %u. MAX INPUT %u\n",
				i, expected_min64, expected_max64,
				ctx[i].shard_range_min, ctx[i].shard_range_max,
				MAX_INPUT);

		TEST_ASSERT_EQUAL_UINT64((uint64_t)expected_min64, (uint64_t)ctx[i].shard_range_min);
		if (expected_max64 != (uint64_t)ctx[i].shard_range_max) {
			printf("asdf");
		}
		TEST_ASSERT_EQUAL_UINT64((uint64_t)expected_max64, (uint64_t)ctx[i].shard_range_max);
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
			uint32_t handoff_value = ctx[i].shard_range_min - 1;
			struct tree_owner *previous_expected = &ctx[i - 1];
			struct tree_owner *previous_actual = get_shard(ctx, handoff_value);

			TEST_ASSERT_EQUAL_PTR(previous_expected, previous_actual);

			actual = get_shard(ctx, handoff_value);
			TEST_ASSERT_EQUAL_PTR(expected, actual);
		}

		if (i < SHARDS - 1) {
			uint32_t handoff_value = ctx[i].shard_range_max + 1;
			struct tree_owner *next_expected = &ctx[i + 1];
			struct tree_owner *next_actual = get_shard(ctx, handoff_value);

			TEST_ASSERT_EQUAL_PTR(next_expected, next_actual);

			actual = get_shard(ctx, handoff_value);
			TEST_ASSERT_EQUAL_PTR(expected, actual);
		}
	}

	struct tree_owner *expected_last = &ctx[SHARDS-1];
	struct tree_owner *actual_last = get_shard(ctx, MAX_INPUT);

	TEST_ASSERT_NOT_NULL(expected_last);
	TEST_ASSERT_EQUAL_PTR(expected_last, actual_last);

	if (MAX_INPUT > 0) {
		struct tree_owner *expected_near_last = find_expected_shard(ctx, MAX_INPUT - 1);
		struct tree_owner *actual_near_last = get_shard(ctx, MAX_INPUT - 1);

		TEST_ASSERT_NOT_NULL(expected_near_last);
		TEST_ASSERT_EQUAL_PTR(expected_near_last, actual_near_last);
	}

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

void test_tree_api_handles_high_range_duplicates(void)
{
	struct tree_owner ctx[SHARDS] = {0};
	const uint32_t range_start = 0xffffffffu - (1024u * 1024u);
	const uint32_t range_end = 0xffffffffu;
	const uint32_t range_count = (1024u * 1024u) + 1u;
	const uint32_t segment = 16384u;
	const uint32_t hot_start = 0xffffff00u;
	const uint32_t expected_seen_once = range_count - (2u * segment);

	prepare_shards(ctx, SHARDS, SHARD_SIZE);

	for (uint32_t value = range_start;;) {
		struct tree_owner *owner = get_shard(ctx, value);
		TEST_ASSERT_NOT_NULL(owner);

		struct rb_tree_node *node = insert_value_into_owner(owner, value);
		TEST_ASSERT_NOT_NULL(node);
		TEST_ASSERT_TRUE(RB_TREE_VALUE_EXISTS(node, value));

		if (value == range_end)
			break;

		++value;
	}

	TEST_ASSERT_EQUAL_UINT32(range_count, total_unique_numbers(ctx));
	TEST_ASSERT_EQUAL_UINT32(range_count, total_seen_once_numbers(ctx));
	TEST_ASSERT_EQUAL_UINT32(0u, total_repeated_numbers(ctx));

	for (uint32_t value = range_start; value < range_start + segment; ++value) {
		struct tree_owner *owner = get_shard(ctx, value);
		struct rb_tree_node *node = insert_value_into_owner(owner, value);
		TEST_ASSERT_TRUE(RB_TREE_VALUE_EXISTS(node, value));
		mark_value_as_repeated(owner, node, value);
	}

	for (uint32_t value = (range_end - segment) + 1u;;) {
		struct tree_owner *owner = get_shard(ctx, value);
		struct rb_tree_node *node = insert_value_into_owner(owner, value);
		TEST_ASSERT_TRUE(RB_TREE_VALUE_EXISTS(node, value));
		mark_value_as_repeated(owner, node, value);

		if (value == range_end)
			break;

		++value;
	}

	TEST_ASSERT_EQUAL_UINT32(range_count, total_unique_numbers(ctx));
	TEST_ASSERT_EQUAL_UINT32(expected_seen_once, total_seen_once_numbers(ctx));
	TEST_ASSERT_EQUAL_UINT32(2u * segment, total_repeated_numbers(ctx));

	for (int loop = 0; loop < 10; ++loop) {
		for (uint32_t value = hot_start;;) {
			struct tree_owner *owner = get_shard(ctx, value);
			struct rb_tree_node *node = insert_value_into_owner(owner, value);
			TEST_ASSERT_TRUE(RB_TREE_VALUE_EXISTS(node, value));
			mark_value_as_repeated(owner, node, value);

			if (value == 0xffffffffu)
				break;

			++value;
		}

		TEST_ASSERT_EQUAL_UINT32(range_count, total_unique_numbers(ctx));
		TEST_ASSERT_EQUAL_UINT32(expected_seen_once, total_seen_once_numbers(ctx));
		TEST_ASSERT_EQUAL_UINT32(2u * segment, total_repeated_numbers(ctx));
	}

	destroy_shards(ctx, SHARDS, SHARD_SIZE);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_prepare_shards_cover_full_range);
    RUN_TEST(test_tree_api_handles_high_range_duplicates);
    //RUN_TEST(test_get_shard_returns_correct_shard);
    //RUN_TEST(test_countint_1);
    //RUN_TEST(test_countint_2);

    return UNITY_END();
}
