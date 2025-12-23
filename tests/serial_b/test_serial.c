#include "radix.c"
#include "unity.h"
#include <string.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void test_radix_insert_1(void)
{
    struct radix_tree_node root = {0};
    const char key[] = "abcd1234";
    uint32_t key_len = (uint32_t)strlen(key);

    strncpy(root.key, key, RADIX_KEY_LEN);
    root.key_len = (uint8_t)key_len;

    radix_tree_insert(&root, (char *)key, key_len);

    TEST_ASSERT_EQUAL_UINT32(1, root.count);

    struct radix_tree_node *found = radix_tree_find(&root, (char *)key, key_len);
    TEST_ASSERT_NOT_NULL(found);
    TEST_ASSERT_EQUAL_PTR(&root, found);
    TEST_ASSERT_EQUAL_UINT32(1, found->count);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_radix_insert_1);

    return UNITY_END();
}
