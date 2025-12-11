#include "tree.c"
#include "unity.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

static size_t count_nodes(struct rb_tree_node *node)
{
	if (node == NULL)
		return 0;

	return 1 + count_nodes(node->left) + count_nodes(node->right);
}

static void assert_parent_links(struct rb_tree_node *node)
{
	if (node == NULL)
		return;

	if (node->left != NULL) {
		TEST_ASSERT_EQUAL_PTR(node, node->left->parent);
		assert_parent_links(node->left);
	}

	if (node->right != NULL) {
		TEST_ASSERT_EQUAL_PTR(node, node->right->parent);
		assert_parent_links(node->right);
	}
}

static void assert_bst_order_helper(struct rb_tree_node *node, uint32_t *previous, int *is_first)
{
	if (node == NULL)
		return;

	assert_bst_order_helper(node->left, previous, is_first);

	if (!*is_first)
		TEST_ASSERT_TRUE(*previous < node->range_min);

	*previous = node->range_min;
	*is_first = 0;

	assert_bst_order_helper(node->right, previous, is_first);
}

static void assert_bst_order(struct rb_tree_node *root)
{
	uint32_t previous = 0;
	int is_first = 1;

	assert_bst_order_helper(root, &previous, &is_first);
}

static void assert_no_red_red(struct rb_tree_node *node)
{
	if (node == NULL)
		return;

	if (node->color == RB_TREE_COLOR_RED) {
		if (node->left != NULL)
			TEST_ASSERT_EQUAL(RB_TREE_COLOR_BLACK, node->left->color);
		if (node->right != NULL)
			TEST_ASSERT_EQUAL(RB_TREE_COLOR_BLACK, node->right->color);
	}

	assert_no_red_red(node->left);
	assert_no_red_red(node->right);
}

static int compute_black_height(struct rb_tree_node *node)
{
	int left_height;
	int right_height;
	int self_black;

	if (node == NULL)
		return 1;

	left_height = compute_black_height(node->left);
	right_height = compute_black_height(node->right);

	TEST_ASSERT_EQUAL(left_height, right_height);

	self_black = (node->color == RB_TREE_COLOR_BLACK) ? 1 : 0;

	return left_height + self_black;
}

static void assert_tree_properties(struct rb_tree_node *root)
{
	if (root == NULL)
		return;

	TEST_ASSERT_NULL(root->parent);
	if (root->color != RB_TREE_COLOR_BLACK)
		printf("asdf\n");
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_BLACK, root->color);

	assert_parent_links(root);
	assert_bst_order(root);
	assert_no_red_red(root);
	(void)compute_black_height(root);
}

static void collect_inorder(struct rb_tree_node *node, uint32_t *buffer, size_t *index)
{
	if (node == NULL)
		return;

	collect_inorder(node->left, buffer, index);
	buffer[*index] = node->range_min;
	(*index)++;
	collect_inorder(node->right, buffer, index);
}

static void assert_tree_matches(struct rb_tree_node *root, const uint32_t *expected_values, size_t expected_count)
{
	size_t total = count_nodes(root);
	TEST_ASSERT_EQUAL(expected_count, total);

	if (expected_count == 0) {
		TEST_ASSERT_NULL(root);
		return;
	}

	uint32_t *buffer = malloc(sizeof(uint32_t) * expected_count);
	TEST_ASSERT_NOT_NULL(buffer);

	size_t idx = 0;
	collect_inorder(root, buffer, &idx);
	TEST_ASSERT_EQUAL(expected_count, idx);

	for (size_t i = 0; i < expected_count; i++)
		TEST_ASSERT_EQUAL_UINT32(expected_values[i], buffer[i]);

	free(buffer);
}

static size_t build_expected(const int *present, size_t size, uint32_t *buffer)
{
	size_t count = 0;

	for (size_t i = 0; i < size; i++) {
		if (present[i]) {
			buffer[count] = (uint32_t)i;
			count++;
		}
	}

	return count;
}

void setUp(void)
{
}

void tearDown(void)
{
}

void test_find_1(void)
{
	TEST_ASSERT_NULL(rb_tree_find(NULL, 0));
}

void test_rotation_left_1(void)
{
/*
	Before Rotation:

	      dummy
		 /
		5
		 \
		  9
		 / \
		7   11

	After Left Rotation:

	  	    dummy
		   /
		  9
		 / \
		5   11
		 \
		  7
*/
	struct rb_tree_node dummy = {};

	struct rb_tree_node n5 = {};
	n5.range_min = 5;
	n5.range_max = 5;
	struct rb_tree_node n7 = {};;
	n7.range_min = 7;
	n7.range_max = 7;
	struct rb_tree_node n9 = {};
	n9.range_min = 9;
	n9.range_max = 9;
	struct rb_tree_node n11 = {};
	n11.range_min = 11;
	n11.range_max = 11;

	dummy.left = &n5;

	n5.parent = &dummy;
	n5.right = &n9;

	n9.left = &n7;
	n7.parent = &n9;
	n9.right = &n11;
	n11.parent = &n9;

	// Store dummy values in uninvolved fields to make sure the pointers aren't corrupted
	dummy.right = (void*)0x123;
	n5.left = (void*)0xdead5;
	n7.left = (void*)0xdead71;
	n7.right = (void*)0xdead72;
	n11.left = (void*)0xdead111;
	n11.right = (void*)0xdead112;

	rb_tree_rotate_left(&n5);

	TEST_ASSERT_EQUAL_PTR(dummy.left, &n9);
	TEST_ASSERT_EQUAL_PTR(n9.parent, &dummy);
	TEST_ASSERT_EQUAL_PTR(n9.left, &n5);
	TEST_ASSERT_EQUAL_PTR(n9.right, &n11);

	TEST_ASSERT_EQUAL_PTR(n5.parent, &n9);
	TEST_ASSERT_EQUAL_PTR(n11.parent, &n9);

	TEST_ASSERT_EQUAL_PTR(n5.right, &n7);
	TEST_ASSERT_EQUAL_PTR(n7.parent, &n5);

	TEST_ASSERT_EQUAL(dummy.right, 0x123);
	TEST_ASSERT_EQUAL(n5.left, 0xdead5);
	TEST_ASSERT_EQUAL(n7.left, 0xdead71);
	TEST_ASSERT_EQUAL(n7.right, 0xdead72);
	TEST_ASSERT_EQUAL(n11.left, 0xdead111);
	TEST_ASSERT_EQUAL(n11.right, 0xdead112);
}

void test_rotation_right_1(void)
{
/*
	Before Rotation:

	       dummy
		   /
		  11
		 /
		7
	   / \
	  5   9

After Right Rotation:

	    dummy
   		 /
		7
	   / \
	  5   11
		 /
		9
*/
	struct rb_tree_node dummy = {};

	struct rb_tree_node n5 = {};
	n5.range_min = 5;
	struct rb_tree_node n7 = {};;
	n7.range_min = 7;
	struct rb_tree_node n9 = {};
	n9.range_min = 9;
	struct rb_tree_node n11 = {};
	n11.range_min = 11;

	dummy.left = &n11;

	n11.parent = &dummy;
	n11.left = &n7;

	n7.parent = &n11;
	n7.left = &n5;
	n7.right = &n9;
	n5.parent = &n7;
	n9.parent = &n7;

	// Store dummy values in uninvolved fields to make sure the pointers aren't corrupted
	dummy.right = (void*)0x123;
	n5.left = (void*)0xdead51;
	n5.right = (void*)0xdead52;
	n11.right = (void*)0xdead112;
	n9.left = (void*)0xdead91;
	n9.right = (void*)0xdead92;

	rb_tree_rotate_right(&n11);

	TEST_ASSERT_EQUAL_PTR(dummy.left, &n7);
	TEST_ASSERT_EQUAL_PTR(n7.parent, &dummy);
	TEST_ASSERT_EQUAL_PTR(n7.left, &n5);
	TEST_ASSERT_EQUAL_PTR(n7.right, &n11);

	TEST_ASSERT_EQUAL_PTR(n5.parent, &n7);
	TEST_ASSERT_EQUAL_PTR(n11.parent, &n7);

	TEST_ASSERT_EQUAL_PTR(n11.left, &n9);
	TEST_ASSERT_EQUAL_PTR(n9.parent, &n11);

	TEST_ASSERT_EQUAL(dummy.right, 0x123);
	TEST_ASSERT_EQUAL(n5.left, 0xdead51);
	TEST_ASSERT_EQUAL(n5.right, 0xdead52);
	TEST_ASSERT_EQUAL(n9.left, 0xdead91);
	TEST_ASSERT_EQUAL(n9.right, 0xdead92);
	TEST_ASSERT_EQUAL(n11.right, 0xdead112);
}

void test_alloc_1(void)
{
	struct rb_tree_node *root = rb_tree_insert(NULL, 12);
	TEST_ASSERT_NOT_NULL(root);
	TEST_ASSERT_NULL(root->parent);

	TEST_ASSERT_EQUAL_PTR(rb_tree_insert(root, 15), root);
	
	free(root);
}

void test_insert_within_single_range(void)
{
	uint32_t values[] = {10, 42, 63};
	size_t value_count = sizeof(values) / sizeof(values[0]);
	struct rb_tree_node *root = NULL;

	for (size_t i = 0; i < value_count; i++) {
		struct rb_tree_node *node = rb_tree_insert_and_fix_violations(root, values[i]);
		TEST_ASSERT_NOT_NULL(node);
		root = rb_tree_get_root(node);
		TEST_ASSERT_NOT_NULL(root);
		assert_tree_properties(root);
	}

	TEST_ASSERT_NULL(root->left);
	TEST_ASSERT_NULL(root->right);

	for (size_t i = 0; i < value_count; i++) {
		struct rb_tree_node *found = rb_tree_find(root, values[i]);
		TEST_ASSERT_NOT_NULL(found);
	}

	uint32_t expected_ranges[] = {
		RB_TREE_GET_MIN_NODE_RANGE(values[0]),
	};
	assert_tree_matches(root, expected_ranges, sizeof(expected_ranges) / sizeof(expected_ranges[0]));

	assert_tree_properties(root);
	rb_tree_deinit(root);
}

void test_insert_cross_range_boundary(void)
{
	uint32_t values[] = {0, 64, 1, 127};
	size_t value_count = sizeof(values) / sizeof(values[0]);
	struct rb_tree_node *root = NULL;

	for (size_t i = 0; i < value_count; i++) {
		struct rb_tree_node *node = rb_tree_insert_and_fix_violations(root, values[i]);
		TEST_ASSERT_NOT_NULL(node);
		root = rb_tree_get_root(node);
		TEST_ASSERT_NOT_NULL(root);
		assert_tree_properties(root);
	}

	for (size_t i = 0; i < value_count; i++) {
		struct rb_tree_node *found = rb_tree_find(root, values[i]);
		TEST_ASSERT_NOT_NULL(found);
	}

	TEST_ASSERT_NULL(root->left);
	TEST_ASSERT_NOT_NULL(root->right);
	TEST_ASSERT_EQUAL_PTR(root, root->right->parent);

	uint32_t expected_ranges[] = {
		RB_TREE_GET_MIN_NODE_RANGE(values[0]),
		RB_TREE_GET_MIN_NODE_RANGE(values[1]),
	};
	assert_tree_matches(root, expected_ranges, sizeof(expected_ranges) / sizeof(expected_ranges[0]));

	assert_tree_properties(root);
	rb_tree_deinit(root);
}

void test_insert_multiple_ranges_with_rebalancing(void)
{
	uint32_t values[] = {200, 40, 320, 41, 260, 258};
	size_t value_count = sizeof(values) / sizeof(values[0]);
	struct rb_tree_node *root = NULL;

	for (size_t i = 0; i < value_count; i++) {
		struct rb_tree_node *node = rb_tree_insert_and_fix_violations(root, values[i]);
		TEST_ASSERT_NOT_NULL(node);
		root = rb_tree_get_root(node);
		TEST_ASSERT_NOT_NULL(root);
		assert_tree_properties(root);
	}

	for (size_t i = 0; i < value_count; i++) {
		struct rb_tree_node *found = rb_tree_find(root, values[i]);
		TEST_ASSERT_NOT_NULL(found);
	}

	struct rb_tree_node *node40 = rb_tree_find(root, 40);
	struct rb_tree_node *node200 = rb_tree_find(root, 200);
	struct rb_tree_node *node260 = rb_tree_find(root, 260);
	struct rb_tree_node *node320 = rb_tree_find(root, 320);

	TEST_ASSERT_NOT_NULL(node40);
	TEST_ASSERT_NOT_NULL(node200);
	TEST_ASSERT_NOT_NULL(node260);
	TEST_ASSERT_NOT_NULL(node320);

	TEST_ASSERT_EQUAL_PTR(node200, root);
	TEST_ASSERT_EQUAL_PTR(root->left, node40);
	TEST_ASSERT_EQUAL_PTR(root->right, node320);
	TEST_ASSERT_NULL(root->parent);

	TEST_ASSERT_EQUAL_PTR(node40->parent, root);
	TEST_ASSERT_NULL(node40->left);
	TEST_ASSERT_NULL(node40->right);

	TEST_ASSERT_EQUAL_PTR(node320->parent, root);
	TEST_ASSERT_NOT_NULL(node320->left);
	TEST_ASSERT_EQUAL_PTR(node320->left, node260);
	TEST_ASSERT_NULL(node320->right);

	TEST_ASSERT_EQUAL_PTR(node260->parent, node320);

	uint32_t expected_ranges[] = {
		RB_TREE_GET_MIN_NODE_RANGE(40),
		RB_TREE_GET_MIN_NODE_RANGE(200),
		RB_TREE_GET_MIN_NODE_RANGE(260),
		RB_TREE_GET_MIN_NODE_RANGE(320),
	};
	assert_tree_matches(root, expected_ranges, sizeof(expected_ranges) / sizeof(expected_ranges[0]));

	assert_tree_properties(root);
	rb_tree_deinit(root);
}

void test_root_remains_black_after_deleting_black_leaf(void)
{
	uint32_t values[] = {0, 1, 2, 3, 4, 5};
	size_t value_count = sizeof(values) / sizeof(values[0]);
	struct rb_tree_node *root = NULL;
	size_t i;

	for (i = 0; i < value_count; i++) {
		struct rb_tree_node *node = rb_tree_insert_and_fix_violations(root, values[i]);
		TEST_ASSERT_NOT_NULL(node);
		root = rb_tree_get_root(node);
	}

	assert_tree_properties(root);

	root = rb_tree_delete(root, 0);
	TEST_ASSERT_NOT_NULL(root);
	root = rb_tree_get_root(root);
	TEST_ASSERT_NOT_NULL(root);

	uint32_t expected[] = {1, 2, 3, 4, 5};
	size_t expected_count = sizeof(expected) / sizeof(expected[0]);

	assert_tree_matches(root, expected, expected_count);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_BLACK, root->color);

	assert_tree_properties(root);
	rb_tree_deinit(root);
}

void test_mixed_stress_insert_delete(void)
{
	const size_t max_key = 700;
	int present[700] = {0};
	uint32_t expected[700];
	size_t expected_count;
	struct rb_tree_node *root = NULL;
	struct rb_tree_node *node;
	size_t i;

	for (i = 0; i < 600; i++) {
		node = rb_tree_insert_and_fix_violations(root, (uint32_t)i);
		TEST_ASSERT_NOT_NULL(node);
		root = rb_tree_get_root(node);
		present[i] = 1;

		expected_count = build_expected(present, max_key, expected);
		assert_tree_properties(root);
		assert_tree_matches(root, expected, expected_count);
	}

	for (i = 600; i < 700; i++) {
		node = rb_tree_insert_and_fix_violations(root, (uint32_t)i);
		TEST_ASSERT_NOT_NULL(node);
		root = rb_tree_get_root(node);
		present[i] = 1;

		expected_count = build_expected(present, max_key, expected);
		assert_tree_properties(root);
		assert_tree_matches(root, expected, expected_count);

		uint32_t remove_key = (uint32_t)(i - 600);
		root = rb_tree_delete(root, remove_key);
		if (root != NULL)
			root = rb_tree_get_root(root);
		present[remove_key] = 0;

		expected_count = build_expected(present, max_key, expected);
		assert_tree_properties(root);
		return;
		assert_tree_matches(root, expected, expected_count);
	}

	rb_tree_deinit(root);
}

void test_delete_red_leaf(void)
{
	struct rb_tree_node *root = rb_tree_insert_and_fix_violations(NULL, 10);
	TEST_ASSERT_NOT_NULL(root);

	struct rb_tree_node *n5 = rb_tree_insert_and_fix_violations(root, 5);
	TEST_ASSERT_NOT_NULL(n5);
	root = rb_tree_get_root(n5);
	TEST_ASSERT_EQUAL_PTR(root->left, n5);
	TEST_ASSERT_EQUAL(n5->color, RB_TREE_COLOR_RED);

	struct rb_tree_node *n15 = rb_tree_insert_and_fix_violations(root, 15);
	TEST_ASSERT_NOT_NULL(n15);
	root = rb_tree_get_root(n15);
	TEST_ASSERT_EQUAL_PTR(root->right, n15);
	TEST_ASSERT_EQUAL(n15->color, RB_TREE_COLOR_RED);

	rb_tree_delete(root, 15);
	root = rb_tree_get_root(root);

	TEST_ASSERT_NULL(rb_tree_find(root, 15));

	struct rb_tree_node *n5_after = rb_tree_find(root, 5);
	TEST_ASSERT_NOT_NULL(n5_after);
	TEST_ASSERT_EQUAL_PTR(root->left, n5_after);
	TEST_ASSERT_EQUAL_PTR(n5_after->parent, root);
	TEST_ASSERT_NULL(n5_after->left);
	TEST_ASSERT_NULL(n5_after->right);

	TEST_ASSERT_NULL(root->right);
	TEST_ASSERT_EQUAL(root->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n5_after->color, RB_TREE_COLOR_RED);
	TEST_ASSERT_NULL(root->parent);
}

void test_delete_black_node_with_single_red_child(void)
{
	struct rb_tree_node *root = rb_tree_insert_and_fix_violations(NULL, 10);
	TEST_ASSERT_NOT_NULL(root);

	struct rb_tree_node *n5 = rb_tree_insert_and_fix_violations(root, 5);
	TEST_ASSERT_NOT_NULL(n5);
	root = rb_tree_get_root(n5);

	struct rb_tree_node *n15 = rb_tree_insert_and_fix_violations(root, 15);
	TEST_ASSERT_NOT_NULL(n15);
	root = rb_tree_get_root(n15);

	struct rb_tree_node *n12 = rb_tree_insert_and_fix_violations(root, 12);
	TEST_ASSERT_NOT_NULL(n12);
	root = rb_tree_get_root(n12);

	struct rb_tree_node *target = rb_tree_find(root, 15);
	TEST_ASSERT_NOT_NULL(target);
	TEST_ASSERT_EQUAL(target->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_NOT_NULL(target->left);
	TEST_ASSERT_NULL(target->right);

	rb_tree_delete(root, 15);
	root = rb_tree_get_root(root);

	TEST_ASSERT_NULL(rb_tree_find(root, 15));

	struct rb_tree_node *n12_after = rb_tree_find(root, 12);
	TEST_ASSERT_NOT_NULL(n12_after);
	TEST_ASSERT_EQUAL_PTR(root->right, n12_after);
	TEST_ASSERT_EQUAL_PTR(n12_after->parent, root);
	TEST_ASSERT_NULL(n12_after->left);
	TEST_ASSERT_NULL(n12_after->right);
	TEST_ASSERT_EQUAL(n12_after->color, RB_TREE_COLOR_BLACK);

	struct rb_tree_node *n5_after = rb_tree_find(root, 5);
	TEST_ASSERT_NOT_NULL(n5_after);
	TEST_ASSERT_EQUAL_PTR(root->left, n5_after);
	TEST_ASSERT_EQUAL_PTR(n5_after->parent, root);
	TEST_ASSERT_EQUAL(n5_after->color, RB_TREE_COLOR_BLACK);

	TEST_ASSERT_EQUAL(root->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_NULL(root->parent);
}

void test_delete_node_with_two_children(void)
{
	struct rb_tree_node *root = rb_tree_insert_and_fix_violations(NULL, 10);
	TEST_ASSERT_NOT_NULL(root);

	struct rb_tree_node *n5 = rb_tree_insert_and_fix_violations(root, 5);
	TEST_ASSERT_NOT_NULL(n5);
	root = rb_tree_get_root(n5);

	struct rb_tree_node *n15 = rb_tree_insert_and_fix_violations(root, 15);
	TEST_ASSERT_NOT_NULL(n15);
	root = rb_tree_get_root(n15);

	struct rb_tree_node *n3 = rb_tree_insert_and_fix_violations(root, 3);
	TEST_ASSERT_NOT_NULL(n3);
	root = rb_tree_get_root(n3);

	struct rb_tree_node *n7 = rb_tree_insert_and_fix_violations(root, 7);
	TEST_ASSERT_NOT_NULL(n7);
	root = rb_tree_get_root(n7);

	struct rb_tree_node *target = rb_tree_find(root, 5);
	TEST_ASSERT_NOT_NULL(target);
	TEST_ASSERT_NOT_NULL(target->left);
	TEST_ASSERT_NOT_NULL(target->right);
	TEST_ASSERT_EQUAL(target->color, RB_TREE_COLOR_BLACK);

	rb_tree_delete(root, 5);
	root = rb_tree_get_root(root);

	TEST_ASSERT_NULL(rb_tree_find(root, 5));

	struct rb_tree_node *n7_after = rb_tree_find(root, 7);
	TEST_ASSERT_NOT_NULL(n7_after);
	TEST_ASSERT_EQUAL_PTR(root->left, n7_after);
	TEST_ASSERT_EQUAL_PTR(n7_after->parent, root);
	TEST_ASSERT_EQUAL(n7_after->color, RB_TREE_COLOR_BLACK);

	struct rb_tree_node *n3_after = rb_tree_find(root, 3);
	TEST_ASSERT_NOT_NULL(n3_after);
	TEST_ASSERT_EQUAL_PTR(n7_after->left, n3_after);
	TEST_ASSERT_EQUAL_PTR(n3_after->parent, n7_after);
	TEST_ASSERT_NULL(n3_after->left);
	TEST_ASSERT_NULL(n3_after->right);
	TEST_ASSERT_EQUAL(n3_after->color, RB_TREE_COLOR_RED);

	struct rb_tree_node *n15_after = rb_tree_find(root, 15);
	TEST_ASSERT_NOT_NULL(n15_after);
	TEST_ASSERT_EQUAL_PTR(root->right, n15_after);
	TEST_ASSERT_EQUAL_PTR(n15_after->parent, root);
	TEST_ASSERT_EQUAL(n15_after->color, RB_TREE_COLOR_BLACK);

	TEST_ASSERT_EQUAL(root->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_NULL(root->parent);
}

void test_delete_black_root_with_two_black_children(void)
{
	struct rb_tree_node *root = rb_tree_insert_and_fix_violations(NULL, 20);
	TEST_ASSERT_NOT_NULL(root);
	root = rb_tree_get_root(root);

	const uint32_t values[] = {10, 30, 5, 15, 25, 35};
	int count = sizeof(values) / sizeof(values[0]);
	int i;

	for (i = 0; i < count; i++) {
		struct rb_tree_node *node = rb_tree_insert_and_fix_violations(root, values[i]);
		TEST_ASSERT_NOT_NULL(node);
		root = rb_tree_get_root(node);
	}

	struct rb_tree_node *node20 = rb_tree_find(root, 20);
	struct rb_tree_node *node10 = rb_tree_find(root, 10);
	struct rb_tree_node *node30 = rb_tree_find(root, 30);
	struct rb_tree_node *node5 = rb_tree_find(root, 5);
	struct rb_tree_node *node15 = rb_tree_find(root, 15);
	struct rb_tree_node *node25 = rb_tree_find(root, 25);
	struct rb_tree_node *node35 = rb_tree_find(root, 35);

	TEST_ASSERT_NOT_NULL(node20);
	TEST_ASSERT_NOT_NULL(node10);
	TEST_ASSERT_NOT_NULL(node30);
	TEST_ASSERT_NOT_NULL(node5);
	TEST_ASSERT_NOT_NULL(node15);
	TEST_ASSERT_NOT_NULL(node25);
	TEST_ASSERT_NOT_NULL(node35);

	TEST_ASSERT_EQUAL_PTR(root, node20);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_BLACK, node20->color);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_BLACK, node10->color);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_BLACK, node30->color);

	root = rb_tree_delete(root, 20);
	TEST_ASSERT_NOT_NULL(root);
	root = rb_tree_get_root(root);

	TEST_ASSERT_NULL(rb_tree_find(root, 20));

	node25 = rb_tree_find(root, 25);
	node10 = rb_tree_find(root, 10);
	node30 = rb_tree_find(root, 30);
	node5 = rb_tree_find(root, 5);
	node15 = rb_tree_find(root, 15);
	node35 = rb_tree_find(root, 35);

	TEST_ASSERT_NOT_NULL(node25);
	TEST_ASSERT_NOT_NULL(node10);
	TEST_ASSERT_NOT_NULL(node30);
	TEST_ASSERT_NOT_NULL(node5);
	TEST_ASSERT_NOT_NULL(node15);
	TEST_ASSERT_NOT_NULL(node35);

	TEST_ASSERT_EQUAL_PTR(root, node25);
	TEST_ASSERT_NULL(root->parent);

	TEST_ASSERT_EQUAL_PTR(root->left, node10);
	TEST_ASSERT_EQUAL_PTR(node10->parent, root);

	TEST_ASSERT_EQUAL_PTR(root->right, node30);
	TEST_ASSERT_EQUAL_PTR(node30->parent, root);

	TEST_ASSERT_EQUAL_PTR(node10->left, node5);
	TEST_ASSERT_EQUAL_PTR(node10->right, node15);
	TEST_ASSERT_EQUAL_PTR(node5->parent, node10);
	TEST_ASSERT_EQUAL_PTR(node15->parent, node10);
	TEST_ASSERT_NULL(node5->left);
	TEST_ASSERT_NULL(node5->right);
	TEST_ASSERT_NULL(node15->left);
	TEST_ASSERT_NULL(node15->right);

	TEST_ASSERT_NULL(node30->left);
	TEST_ASSERT_EQUAL_PTR(node30->right, node35);
	TEST_ASSERT_EQUAL_PTR(node35->parent, node30);
	TEST_ASSERT_NULL(node35->left);
	TEST_ASSERT_NULL(node35->right);

	TEST_ASSERT_EQUAL(RB_TREE_COLOR_BLACK, root->color);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_BLACK, node10->color);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_BLACK, node30->color);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_RED, node5->color);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_RED, node15->color);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_RED, node35->color);

	rb_tree_deinit(root);
}

void test_delete_black_node_with_red_successor(void)
{
	struct rb_tree_node *root = rb_tree_insert_and_fix_violations(NULL, 20);
	TEST_ASSERT_NOT_NULL(root);
	root = rb_tree_get_root(root);

	const uint32_t values[] = {10, 30, 25, 27, 35};
	int count = sizeof(values) / sizeof(values[0]);
	int i;

	for (i = 0; i < count; i++) {
		struct rb_tree_node *node = rb_tree_insert_and_fix_violations(root, values[i]);
		TEST_ASSERT_NOT_NULL(node);
		root = rb_tree_get_root(node);
	}

	struct rb_tree_node *node20 = rb_tree_find(root, 20);
	struct rb_tree_node *node10 = rb_tree_find(root, 10);
	struct rb_tree_node *node30 = rb_tree_find(root, 30);
	struct rb_tree_node *node25 = rb_tree_find(root, 25);
	struct rb_tree_node *node27 = rb_tree_find(root, 27);
	struct rb_tree_node *node35 = rb_tree_find(root, 35);

	TEST_ASSERT_NOT_NULL(node20);
	TEST_ASSERT_NOT_NULL(node10);
	TEST_ASSERT_NOT_NULL(node30);
	TEST_ASSERT_NOT_NULL(node25);
	TEST_ASSERT_NOT_NULL(node27);
	TEST_ASSERT_NOT_NULL(node35);

	TEST_ASSERT_EQUAL(RB_TREE_COLOR_BLACK, node30->color);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_RED, node35->color);

	root = rb_tree_delete(root, 30);
	TEST_ASSERT_NOT_NULL(root);
	root = rb_tree_get_root(root);

	TEST_ASSERT_NULL(rb_tree_find(root, 30));

	node20 = rb_tree_find(root, 20);
	node10 = rb_tree_find(root, 10);
	node25 = rb_tree_find(root, 25);
	node27 = rb_tree_find(root, 27);
	node35 = rb_tree_find(root, 35);

	TEST_ASSERT_NOT_NULL(node20);
	TEST_ASSERT_NOT_NULL(node10);
	TEST_ASSERT_NOT_NULL(node25);
	TEST_ASSERT_NOT_NULL(node27);
	TEST_ASSERT_NOT_NULL(node35);

	TEST_ASSERT_EQUAL_PTR(root, node20);
	TEST_ASSERT_NULL(root->parent);

	TEST_ASSERT_EQUAL_PTR(node20->left, node10);
	TEST_ASSERT_EQUAL_PTR(node10->parent, node20);
	TEST_ASSERT_NULL(node10->left);
	TEST_ASSERT_NULL(node10->right);

	TEST_ASSERT_EQUAL_PTR(node20->right, node27);
	TEST_ASSERT_EQUAL_PTR(node27->parent, node20);

	TEST_ASSERT_EQUAL_PTR(node27->left, node25);
	TEST_ASSERT_EQUAL_PTR(node27->right, node35);
	TEST_ASSERT_EQUAL_PTR(node25->parent, node27);
	TEST_ASSERT_EQUAL_PTR(node35->parent, node27);
	TEST_ASSERT_NULL(node25->left);
	TEST_ASSERT_NULL(node25->right);
	TEST_ASSERT_NULL(node35->left);
	TEST_ASSERT_NULL(node35->right);

	TEST_ASSERT_EQUAL(RB_TREE_COLOR_BLACK, node20->color);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_BLACK, node10->color);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_RED, node27->color);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_BLACK, node25->color);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_BLACK, node35->color);

	rb_tree_deinit(root);
}

void test_delete_black_leaf_with_far_red_nephew(void)
{
	struct rb_tree_node *root = rb_tree_insert_and_fix_violations(NULL, 30);
	TEST_ASSERT_NOT_NULL(root);
	root = rb_tree_get_root(root);

	const uint32_t values[] = {15, 45, 10, 20, 40, 50, 60};
	int count = sizeof(values) / sizeof(values[0]);
	int i;

	for (i = 0; i < count; i++) {
		struct rb_tree_node *node = rb_tree_insert_and_fix_violations(root, values[i]);
		TEST_ASSERT_NOT_NULL(node);
		root = rb_tree_get_root(node);
	}

	struct rb_tree_node *node30 = rb_tree_find(root, 30);
	struct rb_tree_node *node15 = rb_tree_find(root, 15);
	struct rb_tree_node *node45 = rb_tree_find(root, 45);
	struct rb_tree_node *node40 = rb_tree_find(root, 40);
	struct rb_tree_node *node50 = rb_tree_find(root, 50);
	struct rb_tree_node *node60 = rb_tree_find(root, 60);
	struct rb_tree_node *node10 = rb_tree_find(root, 10);
	struct rb_tree_node *node20 = rb_tree_find(root, 20);

	TEST_ASSERT_NOT_NULL(node30);
	TEST_ASSERT_NOT_NULL(node15);
	TEST_ASSERT_NOT_NULL(node45);
	TEST_ASSERT_NOT_NULL(node40);
	TEST_ASSERT_NOT_NULL(node50);
	TEST_ASSERT_NOT_NULL(node60);
	TEST_ASSERT_NOT_NULL(node10);
	TEST_ASSERT_NOT_NULL(node20);

	TEST_ASSERT_EQUAL(RB_TREE_COLOR_BLACK, node40->color);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_BLACK, node50->color);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_RED, node60->color);

	root = rb_tree_delete(root, 40);
	TEST_ASSERT_NOT_NULL(root);
	root = rb_tree_get_root(root);

	TEST_ASSERT_NULL(rb_tree_find(root, 40));

	node30 = rb_tree_find(root, 30);
	node15 = rb_tree_find(root, 15);
	node45 = rb_tree_find(root, 45);
	node50 = rb_tree_find(root, 50);
	node60 = rb_tree_find(root, 60);
	node10 = rb_tree_find(root, 10);
	node20 = rb_tree_find(root, 20);

	TEST_ASSERT_NOT_NULL(node30);
	TEST_ASSERT_NOT_NULL(node15);
	TEST_ASSERT_NOT_NULL(node45);
	TEST_ASSERT_NOT_NULL(node50);
	TEST_ASSERT_NOT_NULL(node60);
	TEST_ASSERT_NOT_NULL(node10);
	TEST_ASSERT_NOT_NULL(node20);

	TEST_ASSERT_EQUAL_PTR(root, node30);
	TEST_ASSERT_NULL(root->parent);

	TEST_ASSERT_EQUAL_PTR(root->left, node15);
	TEST_ASSERT_EQUAL_PTR(node15->parent, root);
	TEST_ASSERT_EQUAL_PTR(node15->left, node10);
	TEST_ASSERT_EQUAL_PTR(node15->right, node20);
	TEST_ASSERT_EQUAL_PTR(node10->parent, node15);
	TEST_ASSERT_EQUAL_PTR(node20->parent, node15);
	TEST_ASSERT_NULL(node10->left);
	TEST_ASSERT_NULL(node10->right);
	TEST_ASSERT_NULL(node20->left);
	TEST_ASSERT_NULL(node20->right);

	TEST_ASSERT_EQUAL_PTR(root->right, node50);
	TEST_ASSERT_EQUAL_PTR(node50->parent, root);
	TEST_ASSERT_EQUAL_PTR(node50->left, node45);
	TEST_ASSERT_EQUAL_PTR(node50->right, node60);
	TEST_ASSERT_EQUAL_PTR(node45->parent, node50);
	TEST_ASSERT_EQUAL_PTR(node60->parent, node50);
	TEST_ASSERT_NULL(node45->left);
	TEST_ASSERT_NULL(node45->right);
	TEST_ASSERT_NULL(node60->left);
	TEST_ASSERT_NULL(node60->right);

	TEST_ASSERT_EQUAL(RB_TREE_COLOR_BLACK, node30->color);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_BLACK, node15->color);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_BLACK, node45->color);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_BLACK, node60->color);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_RED, node50->color);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_RED, node10->color);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_RED, node20->color);

	rb_tree_deinit(root);
}

void test_delete_root_with_deep_subtrees(void)
{
	uint32_t values[] = {40, 20, 60, 10, 30, 50, 70, 5, 15, 25, 35, 45, 55, 65, 75};
	size_t value_count = sizeof(values) / sizeof(values[0]);
	struct rb_tree_node *root = NULL;
	size_t i;

	for (i = 0; i < value_count; i++) {
		struct rb_tree_node *node = rb_tree_insert_and_fix_violations(root, values[i]);
		TEST_ASSERT_NOT_NULL(node);
		root = rb_tree_get_root(node);
	}

	assert_tree_properties(root);

	root = rb_tree_delete(root, 40);
	TEST_ASSERT_NOT_NULL(root);
	root = rb_tree_get_root(root);

	uint32_t expected[] = {5, 10, 15, 20, 25, 30, 35, 45, 50, 55, 60, 65, 70, 75};
	size_t expected_count = sizeof(expected) / sizeof(expected[0]);

	assert_tree_properties(root);
	assert_tree_matches(root, expected, expected_count);
	TEST_ASSERT_NULL(rb_tree_find(root, 40));

	struct rb_tree_node *successor = rb_tree_find(root, 45);
	TEST_ASSERT_NOT_NULL(successor);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_BLACK, successor->color);

	rb_tree_deinit(root);
}

void test_delete_black_internal_node_with_red_sibling(void)
{
	uint32_t values[] = {30, 10, 50, 5, 20, 40, 60, 1, 7, 15, 25, 35, 45, 55, 65};
	size_t value_count = sizeof(values) / sizeof(values[0]);
	struct rb_tree_node *root = NULL;
	size_t i;

	for (i = 0; i < value_count; i++) {
		struct rb_tree_node *node = rb_tree_insert_and_fix_violations(root, values[i]);
		TEST_ASSERT_NOT_NULL(node);
		root = rb_tree_get_root(node);
	}

	assert_tree_properties(root);

	root = rb_tree_delete(root, 10);
	TEST_ASSERT_NOT_NULL(root);
	root = rb_tree_get_root(root);

	uint32_t expected[] = {1, 5, 7, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65};
	size_t expected_count = sizeof(expected) / sizeof(expected[0]);

	assert_tree_properties(root);
	assert_tree_matches(root, expected, expected_count);
	TEST_ASSERT_NULL(rb_tree_find(root, 10));

	struct rb_tree_node *parent = rb_tree_find(root, 30);
	struct rb_tree_node *sibling = rb_tree_find(root, 50);
	TEST_ASSERT_NOT_NULL(parent);
	TEST_ASSERT_NOT_NULL(sibling);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_BLACK, parent->color);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_RED, sibling->color);

	rb_tree_deinit(root);
}

void test_delete_black_node_with_cascading_double_black(void)
{
	uint32_t values[] = {25, 10, 40, 5, 15, 30, 45, 27, 35, 50, 60, 55, 65};
	size_t value_count = sizeof(values) / sizeof(values[0]);
	struct rb_tree_node *root = NULL;
	size_t i;

	for (i = 0; i < value_count; i++) {
		struct rb_tree_node *node = rb_tree_insert_and_fix_violations(root, values[i]);
		TEST_ASSERT_NOT_NULL(node);
		root = rb_tree_get_root(node);
	}

	assert_tree_properties(root);

	root = rb_tree_delete(root, 25);
	TEST_ASSERT_NOT_NULL(root);
	root = rb_tree_get_root(root);

	uint32_t expected[] = {5, 10, 15, 27, 30, 35, 40, 45, 50, 55, 60, 65};
	size_t expected_count = sizeof(expected) / sizeof(expected[0]);

	assert_tree_properties(root);
	assert_tree_matches(root, expected, expected_count);
	TEST_ASSERT_NULL(rb_tree_find(root, 25));

	struct rb_tree_node *replacement = rb_tree_find(root, 27);
	TEST_ASSERT_NOT_NULL(replacement);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_RED, replacement->color);

	rb_tree_deinit(root);
}

void test_delete_black_node_requiring_dummy_leaf(void)
{
	uint32_t values[] = {20, 5, 30, 3, 7, 25, 35, 33};
	size_t value_count = sizeof(values) / sizeof(values[0]);
	struct rb_tree_node *root = NULL;
	size_t i;

	for (i = 0; i < value_count; i++) {
		struct rb_tree_node *node = rb_tree_insert_and_fix_violations(root, values[i]);
		TEST_ASSERT_NOT_NULL(node);
		root = rb_tree_get_root(node);
	}

	assert_tree_properties(root);

	root = rb_tree_delete(root, 25);
	TEST_ASSERT_NOT_NULL(root);
	root = rb_tree_get_root(root);

	uint32_t expected[] = {3, 5, 7, 20, 30, 33, 35};
	size_t expected_count = sizeof(expected) / sizeof(expected[0]);

	assert_tree_properties(root);
	assert_tree_matches(root, expected, expected_count);
	TEST_ASSERT_NULL(rb_tree_find(root, 25));

	struct rb_tree_node *node30 = rb_tree_find(root, 30);
	TEST_ASSERT_NOT_NULL(node30);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_BLACK, node30->color);

	rb_tree_deinit(root);
}

void test_delete_black_leaf_with_far_red_nephew_full_fix(void)
{
	uint32_t values[] = {30, 15, 45, 10, 20, 40, 50, 60};
	size_t value_count = sizeof(values) / sizeof(values[0]);
	struct rb_tree_node *root = NULL;
	size_t i;

	for (i = 0; i < value_count; i++) {
		struct rb_tree_node *node = rb_tree_insert_and_fix_violations(root, values[i]);
		TEST_ASSERT_NOT_NULL(node);
		root = rb_tree_get_root(node);
	}

	assert_tree_properties(root);

	root = rb_tree_delete(root, 40);
	TEST_ASSERT_NOT_NULL(root);
	root = rb_tree_get_root(root);

	uint32_t expected[] = {10, 15, 20, 30, 45, 50, 60};
	size_t expected_count = sizeof(expected) / sizeof(expected[0]);

	assert_tree_properties(root);
	assert_tree_matches(root, expected, expected_count);
	TEST_ASSERT_NULL(rb_tree_find(root, 40));

	struct rb_tree_node *node50 = rb_tree_find(root, 50);
	TEST_ASSERT_NOT_NULL(node50);
	TEST_ASSERT_EQUAL(RB_TREE_COLOR_RED, node50->color);

	rb_tree_deinit(root);
}

void test_sequential_deletions_triggering_rotations(void)
{
	uint32_t values[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	size_t value_count = sizeof(values) / sizeof(values[0]);
	struct rb_tree_node *root = NULL;
	size_t i;

	for (i = 0; i < value_count; i++) {
		struct rb_tree_node *node = rb_tree_insert_and_fix_violations(root, values[i]);
		TEST_ASSERT_NOT_NULL(node);
		root = rb_tree_get_root(node);
	}

	assert_tree_properties(root);
	assert_tree_matches(root, values, value_count);

	struct {
		uint32_t key;
		uint32_t expected[10];
		size_t expected_count;
	} cases[] = {
		{2, {1, 3, 4, 5, 6, 7, 8, 9, 10}, 9},
		{8, {1, 3, 4, 5, 6, 7, 9, 10}, 8},
		{4, {1, 3, 5, 6, 7, 9, 10}, 7},
		{6, {1, 3, 5, 7, 9, 10}, 6},
	};
	size_t case_count = sizeof(cases) / sizeof(cases[0]);

	for (i = 0; i < case_count; i++) {
		root = rb_tree_delete(root, cases[i].key);
		TEST_ASSERT_NOT_NULL(root);
		root = rb_tree_get_root(root);
		assert_tree_properties(root);
		assert_tree_matches(root, cases[i].expected, cases[i].expected_count);
		TEST_ASSERT_NULL(rb_tree_find(root, cases[i].key));
	}

	rb_tree_deinit(root);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_find_1);
    RUN_TEST(test_rotation_left_1);
    RUN_TEST(test_rotation_right_1);
    RUN_TEST(test_alloc_1);
    RUN_TEST(test_insert_within_single_range);
    RUN_TEST(test_insert_cross_range_boundary);
    RUN_TEST(test_insert_multiple_ranges_with_rebalancing);
    //RUN_TEST(test_delete_red_leaf);
    //RUN_TEST(test_delete_black_node_with_single_red_child);
    //RUN_TEST(test_delete_node_with_two_children);
    //RUN_TEST(test_delete_black_root_with_two_black_children);
    //RUN_TEST(test_delete_black_node_with_red_successor);
    //RUN_TEST(test_delete_black_leaf_with_far_red_nephew);
    //RUN_TEST(test_delete_root_with_deep_subtrees);
    //RUN_TEST(test_delete_black_internal_node_with_red_sibling);
    //RUN_TEST(test_delete_black_node_with_cascading_double_black);
    //RUN_TEST(test_delete_black_node_requiring_dummy_leaf);
    //RUN_TEST(test_delete_black_leaf_with_far_red_nephew_full_fix);
    //RUN_TEST(test_sequential_deletions_triggering_rotations);
    //RUN_TEST(test_root_remains_black_after_deleting_black_leaf);
    //RUN_TEST(test_mixed_stress_insert_delete);

    return UNITY_END();
}
