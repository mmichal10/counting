#include "hashtable.c"
#include "hash.c"
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

void test_hashtable_basic_insert(void) {
	uint32_t range_begin = 0;
	uint32_t range_end = 16;
	uint32_t i;
	struct hash_table_shard hash_table = {};
	struct hash_table_entry *entry;
	const char *keys[] = {"asdf", "zxvc", "qwer"};
	int ret;

	ret = hashtable_init(&hash_table, range_begin,  range_end, FNV);
	TEST_ASSERT_EQUAL(0, ret);

	for (i = 0; i < 3; i++) {
		ret = hashtable_insert(&hash_table, keys[i]);
		TEST_ASSERT_EQUAL(0, ret);
	}

	for (i = 0; i < 3; i++) {
		entry = hashtable_lookup(&hash_table, keys[i]);
		TEST_ASSERT_EQUAL(1, entry_count_test_helper(entry));
	}
		
	hashtable_deinit(&hash_table);
}

void test_hashtable_insert_resize(void) {
	uint32_t range_begin = 0;
	uint32_t range_end = 8;
	uint32_t i;
	struct hash_table_shard hash_table = {};
	const char *keys[] = {"asdf", "zxvc", "qwer", "qazx", "uiop", "hjkl", "vbnm", "sdfg", "wert", "xcvb", "sdaf"};
	struct hash_table_entry *entry;
	int ret;

	ret = hashtable_init(&hash_table, range_begin,  range_end, FNV);
	TEST_ASSERT_EQUAL(0, ret);

	for (i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
		ret = hashtable_insert(&hash_table, keys[i]);
		TEST_ASSERT_EQUAL(0, ret);
	}

	for (i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
		entry = hashtable_lookup(&hash_table, keys[i]);
		TEST_ASSERT_EQUAL(1, entry_count_test_helper(entry));
	}
		
	hashtable_deinit(&hash_table);
}

void test_hashtable_insert_the_same_key_after_resize(void) {
	uint32_t range_begin = 0;
	uint32_t range_end = 8;
	uint32_t i;
	struct hash_table_shard hash_table = {};
	struct hash_table_entry *entry;
	const char *keys[] = {"asdf", "zxvc", "qwer", "qazx", "uiop", "hjkl", "vbnm", "sdfg", "wert", "xcvb", "sdaf"};
	int ret;

	ret = hashtable_init(&hash_table, range_begin,  range_end, FNV);
	TEST_ASSERT_EQUAL(0, ret);

	for (i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
		ret = hashtable_insert(&hash_table, keys[i]);
		TEST_ASSERT_EQUAL(0, ret);
	}

	for (i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
		entry = hashtable_lookup(&hash_table, keys[i]);
		TEST_ASSERT_EQUAL(1, entry_count_test_helper(entry));
	}

	for (i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
		ret = hashtable_insert(&hash_table, keys[i]);
		TEST_ASSERT_EQUAL(0, ret);
	}

	for (i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
		entry = hashtable_lookup(&hash_table, keys[i]);
		TEST_ASSERT_EQUAL(2, entry_count_test_helper(entry));
	}
		
	hashtable_deinit(&hash_table);
}

void test_hashtable_lookup_missing_keys(void) {
	uint32_t range_begin = 0;
	uint32_t range_end = 8;
	uint32_t i;
	struct hash_table_shard hash_table = {};
	struct hash_table_entry *entry;
	const char *valid_keys[] = {"asdf", "zxvc", "qwer", "qazx", "uiop", "hjkl", "vbnm", "sdfg", "wert", "xcvb", "sdaf"};
	const char *invalid_keys[] = {"asd", "zxv", "qwe", "qaz", "uio", "hjk", "vbn", "sdf", "wer", "xcv", "sda",
		"asdfx", "zxvcx", "qwerx", "qazxx", "uiopx", "hjklx", "vbnmx", "sdfgx", "wertx", "xcvbx", "sdafx"};
	int ret;

	ret = hashtable_init(&hash_table, range_begin,  range_end, FNV);
	TEST_ASSERT_EQUAL(0, ret);

	for (i = 0; i < sizeof(valid_keys) / sizeof(valid_keys[0]); i++) {
		ret = hashtable_insert(&hash_table, valid_keys[i]);
		TEST_ASSERT_EQUAL(0, ret);
	}

	for (i = 0; i < sizeof(valid_keys) / sizeof(valid_keys[0]); i++) {
		entry = hashtable_lookup(&hash_table, valid_keys[i]);
		TEST_ASSERT_EQUAL(1, entry_count_test_helper(entry));
	}

	for (i = 0; i < sizeof(invalid_keys) / sizeof(invalid_keys[0]); i++) {
		entry = hashtable_lookup(&hash_table, invalid_keys[i]);
		TEST_ASSERT_NULL(entry);
	}
		
	hashtable_deinit(&hash_table);
}

void test_hashtable_insert_multiple_times(void) {
	uint32_t range_begin = 0;
	uint32_t range_end = 8;
	uint32_t i, j;
	const uint32_t insert_repetitions = 1024;
	struct hash_table_shard hash_table = {};
	struct hash_table_entry *entry;
	const char *valid_keys[] = {"asdf", "zxvc", "qwer", "qazx", "uiop", "hjkl", "vbnm", "sdfg", "wert", "xcvb", "sdaf"};
	int ret;

	ret = hashtable_init(&hash_table, range_begin,  range_end, FNV);
	TEST_ASSERT_EQUAL(0, ret);

	for (j = 0; j < insert_repetitions; j++) {
		for (i = 0; i < sizeof(valid_keys) / sizeof(valid_keys[0]); i++) {
			ret = hashtable_insert(&hash_table, valid_keys[i]);
			TEST_ASSERT_EQUAL(0, ret);
		}
	}

	for (i = 0; i < sizeof(valid_keys) / sizeof(valid_keys[0]); i++) {
		entry = hashtable_lookup(&hash_table, valid_keys[i]);
		TEST_ASSERT_EQUAL(insert_repetitions, entry_count_test_helper(entry));
	}

	hashtable_deinit(&hash_table);
}

void test_hashtable_multiple_insert_and_resize(void) {
	uint32_t range_begin = 0;
	uint32_t range_end = 8;
	uint32_t i, j;
	const uint32_t insert_repetitions = 1024;
	struct hash_table_shard hash_table = {};
	struct hash_table_entry *entry;
	const char *multiple_inserted_key = "insert0";
	const char *valid_keys[] = {"asdf", "zxvc", "qwer", "qazx", "uiop", "hjkl", "vbnm", "sdfg", "wert", "xcvb", "sdaf"};
	int ret;

	ret = hashtable_init(&hash_table, range_begin,  range_end, FNV);
	TEST_ASSERT_EQUAL(0, ret);

	for (j = 0; j < insert_repetitions; j++) {
		ret = hashtable_insert(&hash_table, multiple_inserted_key);
		TEST_ASSERT_EQUAL(0, ret);
	}

	for (j = 0; j < insert_repetitions; j++) {
		for (i = 0; i < sizeof(valid_keys) / sizeof(valid_keys[0]); i++) {
			ret = hashtable_insert(&hash_table, valid_keys[i]);
			TEST_ASSERT_EQUAL(0, ret);

			entry = hashtable_lookup(&hash_table, multiple_inserted_key);
			TEST_ASSERT_EQUAL(insert_repetitions, entry_count_test_helper(entry));
		}
	}


	for (i = 0; i < sizeof(valid_keys) / sizeof(valid_keys[0]); i++) {
		entry = hashtable_lookup(&hash_table, valid_keys[i]);
		TEST_ASSERT_EQUAL(insert_repetitions, entry_count_test_helper(entry));
	}

	hashtable_deinit(&hash_table);
}

int main(void) {
    UNITY_BEGIN();
	RUN_TEST(test_hashtable_basic_insert);
	RUN_TEST(test_hashtable_insert_resize);
	RUN_TEST(test_hashtable_insert_the_same_key_after_resize);
	RUN_TEST(test_hashtable_lookup_missing_keys);
	RUN_TEST(test_hashtable_insert_multiple_times);
	RUN_TEST(test_hashtable_multiple_insert_and_resize);

    return UNITY_END();
}
