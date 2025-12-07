#include "counting.c"
#include "unity.h"

void setUp(void)
{
}

void tearDown(void)
{
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
    RUN_TEST(test_countint_1);
    RUN_TEST(test_countint_2);

    return UNITY_END();
}
