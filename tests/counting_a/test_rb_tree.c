#include "tree.c"
#include "unity.h"

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

void test_find_2(void)
{
	struct rb_tree_node n1 = {};
	n1.range_min = 1;
	n1.range_max = 1;
	struct rb_tree_node n2 = {};;
	n2.range_min = 2;
	n2.range_max = 2;
	struct rb_tree_node root = {};
	root.range_min = 3;
	root.range_max = 3;
	struct rb_tree_node n4 = {};
	n4.range_min = 4;
	n4.range_max = 4;
	struct rb_tree_node n5 = {};
	n5.range_min = 5;
	n5.range_max = 5;

	root.left = &n2;
	root.right = &n4;

	n2.left = &n1;
	n4.right = &n5;

	TEST_ASSERT_EQUAL_PTR(rb_tree_find(&root, 1), &n1);
	TEST_ASSERT_EQUAL_PTR(rb_tree_find(&root, 2), &n2);
	TEST_ASSERT_EQUAL_PTR(rb_tree_find(&root, 3), &root);
	TEST_ASSERT_EQUAL_PTR(rb_tree_find(&root, 4), &n4);
	TEST_ASSERT_EQUAL_PTR(rb_tree_find(&root, 5), &n5);
}

void test_find_3(void)
{
	struct rb_tree_node n1 = {};
	n1.range_min = 1;
	n1.range_max = 1;
	struct rb_tree_node n2 = {};;
	n2.range_min = 2;
	n2.range_max = 2;
	struct rb_tree_node root = {};
	root.range_min = 3;
	root.range_max = 3;
	struct rb_tree_node n4 = {};
	n4.range_min = 4;
	n4.range_max = 4;
	struct rb_tree_node n5 = {};
	n5.range_min = 5;
	n5.range_max = 5;

	root.left = &n2;
	root.right = &n4;

	n2.left = &n1;
	n4.right = &n5;

	TEST_ASSERT_NULL(rb_tree_find(&root, 11));
	TEST_ASSERT_NULL(rb_tree_find(&root, 12));
	TEST_ASSERT_NULL(rb_tree_find(&root, 13));
	TEST_ASSERT_NULL(rb_tree_find(&root, 14));
	TEST_ASSERT_NULL(rb_tree_find(&root, 15));
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
	struct rb_tree_node n7 = {};;
	n7.range_min = 7;
	struct rb_tree_node n9 = {};
	n9.range_min = 9;
	struct rb_tree_node n11 = {};
	n11.range_min = 11;

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

void test_insert_1(void)
{
/*

		  11
		 /
		7
	   / \
	  5   9

*/
	struct rb_tree_node n5 = {};
	n5.range_min = 5;
	struct rb_tree_node n7 = {};;
	n7.range_min = 7;
	struct rb_tree_node n9 = {};
	n9.range_min = 9;
	struct rb_tree_node n11 = {};
	n11.range_min = 11;

	n11.left = &n7;

	n7.left = &n5;
	n7.right = &n9;

	struct rb_tree_node *node12 = rb_tree_insert(&n11, 12);
	TEST_ASSERT_NOT_NULL(node12);
	TEST_ASSERT_EQUAL(n11.right->range_min, 12);
	TEST_ASSERT_EQUAL_PTR(node12->parent, &n11);


	struct rb_tree_node *node6 = rb_tree_insert(&n11, 6);
	TEST_ASSERT_NOT_NULL(node6);
	TEST_ASSERT_EQUAL(n5.right->range_min, 6);
	TEST_ASSERT_EQUAL_PTR(node6->parent, &n5);

	free(node12);
	free(node6);
}

void test_alloc_1(void)
{
	struct rb_tree_node *root = rb_tree_insert(NULL, 12);
	TEST_ASSERT_NOT_NULL(root);
	TEST_ASSERT_EQUAL(root->range_min, 12);
	TEST_ASSERT_NULL(root->parent);

	struct rb_tree_node *child = rb_tree_insert(root, 15);
	
	TEST_ASSERT_EQUAL_PTR(root->right, child);
	TEST_ASSERT_EQUAL_PTR(child->parent, root);

	free(root);
	free(child);
}

void test_insert_and_fix_1(void)
{
	struct rb_tree_node *n3 = rb_tree_insert_and_fix_violations(NULL, 3);
	TEST_ASSERT_NOT_NULL(n3);
	TEST_ASSERT_EQUAL(n3->color, RB_TREE_COLOR_BLACK);
	
	struct rb_tree_node *n21 = rb_tree_insert_and_fix_violations(n3, 21);
	TEST_ASSERT_NOT_NULL(n21);

	TEST_ASSERT_EQUAL(n3->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n21->color, RB_TREE_COLOR_RED);

	TEST_ASSERT_EQUAL_PTR(n3->right, n21);
	TEST_ASSERT_EQUAL_PTR(n21->parent, n3);

	struct rb_tree_node *n32 = rb_tree_insert_and_fix_violations(n3, 32);
	TEST_ASSERT_NOT_NULL(n32);

	TEST_ASSERT_EQUAL_PTR(n21->left, n3);
	TEST_ASSERT_EQUAL_PTR(n21->right, n32);

	TEST_ASSERT_EQUAL_PTR(n3->parent, n21);
	TEST_ASSERT_EQUAL_PTR(n32->parent, n21);

	TEST_ASSERT_NULL(n21->parent);
	TEST_ASSERT_NULL(n3->left);
	TEST_ASSERT_NULL(n3->right);
	TEST_ASSERT_NULL(n32->left);
	TEST_ASSERT_NULL(n32->right);

	TEST_ASSERT_EQUAL(n21->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n3->color, RB_TREE_COLOR_RED);
	TEST_ASSERT_EQUAL(n32->color, RB_TREE_COLOR_RED);

	struct rb_tree_node *n15 = rb_tree_insert_and_fix_violations(n21, 15);
	TEST_ASSERT_NOT_NULL(n15);

	TEST_ASSERT_EQUAL_PTR(n21->left, n3);
	TEST_ASSERT_EQUAL_PTR(n21->right, n32);

	TEST_ASSERT_EQUAL_PTR(n3->parent, n21);
	TEST_ASSERT_EQUAL_PTR(n32->parent, n21);

	TEST_ASSERT_EQUAL_PTR(n3->right, n15);
	TEST_ASSERT_EQUAL_PTR(n15->parent, n3);

	TEST_ASSERT_NULL(n21->parent);
	TEST_ASSERT_NULL(n3->left);
	TEST_ASSERT_NULL(n32->left);
	TEST_ASSERT_NULL(n32->right);
	TEST_ASSERT_NULL(n15->left);
	TEST_ASSERT_NULL(n15->right);

	TEST_ASSERT_EQUAL(n21->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n3->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n32->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n15->color, RB_TREE_COLOR_RED);
	
	free(n3);
	free(n21);
	free(n32);
	free(n15);
}

void test_insert_and_fix_2(void)
{
	// LL rotation
	struct rb_tree_node *n30 = rb_tree_insert_and_fix_violations(NULL, 30);
	struct rb_tree_node *n20 = rb_tree_insert_and_fix_violations(n30, 20);
	struct rb_tree_node *n10 = rb_tree_insert_and_fix_violations(n30, 10);

	TEST_ASSERT_EQUAL_PTR(n20->left, n10);
	TEST_ASSERT_EQUAL_PTR(n20->right, n30);
	TEST_ASSERT_NULL(n20->parent);

	TEST_ASSERT_EQUAL_PTR(n10->parent, n20);
	TEST_ASSERT_EQUAL_PTR(n30->parent, n20);

	TEST_ASSERT_NULL(n10->right);
	TEST_ASSERT_NULL(n10->left);
	TEST_ASSERT_NULL(n30->right);
	TEST_ASSERT_NULL(n30->left);

	TEST_ASSERT_EQUAL(n10->color, RB_TREE_COLOR_RED);
	TEST_ASSERT_EQUAL(n20->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n30->color, RB_TREE_COLOR_RED);
}

void test_insert_and_fix_3(void)
{
	//LR rotation
	struct rb_tree_node *n30 = rb_tree_insert_and_fix_violations(NULL, 30);
	struct rb_tree_node *n10 = rb_tree_insert_and_fix_violations(n30, 10);
	struct rb_tree_node *n20 = rb_tree_insert_and_fix_violations(n30, 20);

	TEST_ASSERT_EQUAL_PTR(n20->left, n10);
	TEST_ASSERT_EQUAL_PTR(n20->right, n30);
	TEST_ASSERT_NULL(n20->parent);

	TEST_ASSERT_EQUAL_PTR(n10->parent, n20);
	TEST_ASSERT_EQUAL_PTR(n30->parent, n20);

	TEST_ASSERT_NULL(n10->right);
	TEST_ASSERT_NULL(n10->left);
	TEST_ASSERT_NULL(n30->right);
	TEST_ASSERT_NULL(n30->left);

	TEST_ASSERT_EQUAL(n10->color, RB_TREE_COLOR_RED);
	TEST_ASSERT_EQUAL(n20->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n30->color, RB_TREE_COLOR_RED);
	
}

void test_insert_and_fix_4(void)
{
	//RL rotation
	struct rb_tree_node *n10 = rb_tree_insert_and_fix_violations(NULL, 10);
	struct rb_tree_node *n30 = rb_tree_insert_and_fix_violations(n10, 30);
	struct rb_tree_node *n20 = rb_tree_insert_and_fix_violations(n10, 20);

	TEST_ASSERT_EQUAL_PTR(n20->left, n10);
	TEST_ASSERT_EQUAL_PTR(n20->right, n30);
	TEST_ASSERT_NULL(n20->parent);

	TEST_ASSERT_EQUAL_PTR(n10->parent, n20);
	TEST_ASSERT_EQUAL_PTR(n30->parent, n20);

	TEST_ASSERT_NULL(n10->right);
	TEST_ASSERT_NULL(n10->left);
	TEST_ASSERT_NULL(n30->right);
	TEST_ASSERT_NULL(n30->left);

	TEST_ASSERT_EQUAL(n20->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n10->color, RB_TREE_COLOR_RED);
	TEST_ASSERT_EQUAL(n30->color, RB_TREE_COLOR_RED);
	
}

void test_insert_and_fix_5(void)
{
	//Recoloring propagation upward
	struct rb_tree_node *n10 = rb_tree_insert_and_fix_violations(NULL, 10);
	struct rb_tree_node *root = rb_tree_get_root(n10);
	struct rb_tree_node *n5 = rb_tree_insert_and_fix_violations(root, 5);
	root = rb_tree_get_root(n5);
	struct rb_tree_node *n15 = rb_tree_insert_and_fix_violations(root, 15);
	root = rb_tree_get_root(n15);
	struct rb_tree_node *n1 = rb_tree_insert_and_fix_violations(root, 1);
	root = rb_tree_get_root(n1);
	struct rb_tree_node *n6 = rb_tree_insert_and_fix_violations(root, 6);
	root = rb_tree_get_root(n6);
	struct rb_tree_node *n12 = rb_tree_insert_and_fix_violations(root, 12);
	root = rb_tree_get_root(n12);
	struct rb_tree_node *n17 = rb_tree_insert_and_fix_violations(root, 17);
	root = rb_tree_get_root(n17);

	TEST_ASSERT_EQUAL_PTR(n10->left, n5);
	TEST_ASSERT_EQUAL_PTR(n10->right, n15);
	TEST_ASSERT_NULL(n10->parent);

	TEST_ASSERT_EQUAL_PTR(n5->left, n1);
	TEST_ASSERT_EQUAL_PTR(n5->right, n6);
	TEST_ASSERT_EQUAL_PTR(n5->parent, n10);

	TEST_ASSERT_EQUAL_PTR(n15->left, n12);
	TEST_ASSERT_EQUAL_PTR(n15->right, n17);
	TEST_ASSERT_EQUAL_PTR(n15->parent, n10);

	TEST_ASSERT_NULL(n1->right);
	TEST_ASSERT_NULL(n1->left);
	TEST_ASSERT_EQUAL_PTR(n1->parent, n5);

	TEST_ASSERT_NULL(n6->right);
	TEST_ASSERT_NULL(n6->left);
	TEST_ASSERT_EQUAL_PTR(n6->parent, n5);

	TEST_ASSERT_NULL(n6->right);
	TEST_ASSERT_NULL(n6->left);
	TEST_ASSERT_EQUAL_PTR(n6->parent, n5);

	TEST_ASSERT_NULL(n12->right);
	TEST_ASSERT_NULL(n12->left);
	TEST_ASSERT_EQUAL_PTR(n12->parent, n15);

	TEST_ASSERT_NULL(n17->right);
	TEST_ASSERT_NULL(n17->left);
	TEST_ASSERT_EQUAL_PTR(n17->parent, n15);

	TEST_ASSERT_EQUAL(n10->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n5->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n15->color, RB_TREE_COLOR_BLACK);

	TEST_ASSERT_EQUAL(n1->color, RB_TREE_COLOR_RED);
	TEST_ASSERT_EQUAL(n6->color, RB_TREE_COLOR_RED);
	TEST_ASSERT_EQUAL(n12->color, RB_TREE_COLOR_RED);
	TEST_ASSERT_EQUAL(n17->color, RB_TREE_COLOR_RED);
}

void test_insert_and_fix_6(void)
{
	//Recoloring propagation upward
	struct rb_tree_node *n10 = rb_tree_insert_and_fix_violations(NULL, 10);
	struct rb_tree_node *root = rb_tree_get_root(n10);
	struct rb_tree_node *n5 = rb_tree_insert_and_fix_violations(root, 5);
	root = rb_tree_get_root(n5);
	struct rb_tree_node *n1 = rb_tree_insert_and_fix_violations(root, 1);
	root = rb_tree_get_root(n1);
	struct rb_tree_node *n15 = rb_tree_insert_and_fix_violations(root, 15);
	root = rb_tree_get_root(n15);
	struct rb_tree_node *n20 = rb_tree_insert_and_fix_violations(root, 20);
	root = rb_tree_get_root(n20);
	struct rb_tree_node *n25 = rb_tree_insert_and_fix_violations(root, 25);
	root = rb_tree_get_root(n25);
	struct rb_tree_node *n30 = rb_tree_insert_and_fix_violations(root, 30);
	root = rb_tree_get_root(n30);

	TEST_ASSERT_EQUAL_PTR(n5->left, n1);
	TEST_ASSERT_EQUAL_PTR(n5->right, n15);
	TEST_ASSERT_NULL(n5->parent);

	TEST_ASSERT_NULL(n1->left);
	TEST_ASSERT_NULL(n1->right);
	TEST_ASSERT_EQUAL_PTR(n1->parent, n5);

	TEST_ASSERT_EQUAL_PTR(n15->left, n10);
	TEST_ASSERT_EQUAL_PTR(n15->right, n25);
	TEST_ASSERT_EQUAL_PTR(n15->parent, n5);

	TEST_ASSERT_NULL(n10->left);
	TEST_ASSERT_NULL(n10->right);
	TEST_ASSERT_EQUAL_PTR(n10->parent, n15);

	TEST_ASSERT_EQUAL_PTR(n25->left, n20);
	TEST_ASSERT_EQUAL_PTR(n25->right, n30);
	TEST_ASSERT_EQUAL_PTR(n25->parent, n15);

	TEST_ASSERT_EQUAL_PTR(n25->left, n20);
	TEST_ASSERT_EQUAL_PTR(n25->right, n30);
	TEST_ASSERT_EQUAL_PTR(n25->parent, n15);

	TEST_ASSERT_NULL(n20->left);
	TEST_ASSERT_NULL(n20->right);
	TEST_ASSERT_EQUAL_PTR(n20->parent, n25);

	TEST_ASSERT_NULL(n30->left);
	TEST_ASSERT_NULL(n30->right);
	TEST_ASSERT_EQUAL_PTR(n30->parent, n25);


	TEST_ASSERT_EQUAL(n5->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n1->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n10->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n25->color, RB_TREE_COLOR_BLACK);

	TEST_ASSERT_EQUAL(n15->color, RB_TREE_COLOR_RED);
	TEST_ASSERT_EQUAL(n20->color, RB_TREE_COLOR_RED);
	TEST_ASSERT_EQUAL(n30->color, RB_TREE_COLOR_RED);
}

void test_insert_and_fix_7(void)
{
	//Recoloring propagation upward
	struct rb_tree_node *n7 = rb_tree_insert_and_fix_violations(NULL, 7);
	struct rb_tree_node *root = rb_tree_get_root(n7);
	struct rb_tree_node *n3 = rb_tree_insert_and_fix_violations(root, 3);
	root = rb_tree_get_root(n3);
	struct rb_tree_node *n18 = rb_tree_insert_and_fix_violations(root, 18);
	root = rb_tree_get_root(n18);
	struct rb_tree_node *n10 = rb_tree_insert_and_fix_violations(root, 10);
	root = rb_tree_get_root(n10);
	struct rb_tree_node *n22 = rb_tree_insert_and_fix_violations(root, 22);
	root = rb_tree_get_root(n22);
	struct rb_tree_node *n8 = rb_tree_insert_and_fix_violations(root, 8);
	root = rb_tree_get_root(n8);
	struct rb_tree_node *n11 = rb_tree_insert_and_fix_violations(root, 11);
	root = rb_tree_get_root(n11);

	TEST_ASSERT_EQUAL_PTR(n7->left, n3);
	TEST_ASSERT_EQUAL_PTR(n7->right, n18);
	TEST_ASSERT_NULL(n7->parent);

	TEST_ASSERT_NULL(n3->left);
	TEST_ASSERT_NULL(n3->right);
	TEST_ASSERT_EQUAL_PTR(n3->parent, n7);

	TEST_ASSERT_EQUAL_PTR(n18->left, n10);
	TEST_ASSERT_EQUAL_PTR(n18->right, n22);
	TEST_ASSERT_EQUAL_PTR(n18->parent, n7);

	TEST_ASSERT_EQUAL_PTR(n10->left, n8);
	TEST_ASSERT_EQUAL_PTR(n10->right, n11);
	TEST_ASSERT_EQUAL_PTR(n10->parent, n18);

	TEST_ASSERT_NULL(n22->left);
	TEST_ASSERT_NULL(n22->right);
	TEST_ASSERT_EQUAL_PTR(n22->parent, n18);

	TEST_ASSERT_NULL(n8->left);
	TEST_ASSERT_NULL(n8->right);
	TEST_ASSERT_EQUAL_PTR(n8->parent, n10);

	TEST_ASSERT_NULL(n11->left);
	TEST_ASSERT_NULL(n11->right);
	TEST_ASSERT_EQUAL_PTR(n8->parent, n10);

	TEST_ASSERT_EQUAL(n7->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n3->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n10->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n22->color, RB_TREE_COLOR_BLACK);

	TEST_ASSERT_EQUAL(n18->color, RB_TREE_COLOR_RED);
	TEST_ASSERT_EQUAL(n8->color, RB_TREE_COLOR_RED);
	TEST_ASSERT_EQUAL(n11->color, RB_TREE_COLOR_RED);
}

void test_insert_and_fix_8(void)
{
	//Recoloring propagation upward
	struct rb_tree_node *n41 = rb_tree_insert_and_fix_violations(NULL, 41);
	struct rb_tree_node *root = rb_tree_get_root(n41);
	struct rb_tree_node *n38 = rb_tree_insert_and_fix_violations(root, 38);
	root = rb_tree_get_root(n38);
	struct rb_tree_node *n31 = rb_tree_insert_and_fix_violations(root, 31);
	root = rb_tree_get_root(n31);
	struct rb_tree_node *n12 = rb_tree_insert_and_fix_violations(root, 12);
	root = rb_tree_get_root(n12);
	struct rb_tree_node *n19 = rb_tree_insert_and_fix_violations(root, 19);
	root = rb_tree_get_root(n19);
	struct rb_tree_node *n8 = rb_tree_insert_and_fix_violations(root, 8);
	root = rb_tree_get_root(n8);

	TEST_ASSERT_EQUAL_PTR(n38->left, n19);
	TEST_ASSERT_EQUAL_PTR(n38->right, n41);
	TEST_ASSERT_NULL(n38->parent);

	TEST_ASSERT_EQUAL_PTR(n19->left, n12);
	TEST_ASSERT_EQUAL_PTR(n19->right, n31);
	TEST_ASSERT_EQUAL_PTR(n19->parent, n38);

	TEST_ASSERT_NULL(n41->left);
	TEST_ASSERT_NULL(n41->right);
	TEST_ASSERT_EQUAL_PTR(n41->parent, n38);

	TEST_ASSERT_EQUAL_PTR(n12->left, n8);
	TEST_ASSERT_NULL(n12->right);
	TEST_ASSERT_EQUAL_PTR(n12->parent, n19);

	TEST_ASSERT_NULL(n31->left);
	TEST_ASSERT_NULL(n31->right);
	TEST_ASSERT_EQUAL_PTR(n31->parent, n19);

	TEST_ASSERT_NULL(n8->left);
	TEST_ASSERT_NULL(n8->right);
	TEST_ASSERT_EQUAL_PTR(n8->parent, n12);

	TEST_ASSERT_EQUAL(n38->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n41->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n12->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n31->color, RB_TREE_COLOR_BLACK);

	TEST_ASSERT_EQUAL(n19->color, RB_TREE_COLOR_RED);
	TEST_ASSERT_EQUAL(n8->color, RB_TREE_COLOR_RED);
}

void test_insert_and_fix_9(void)
{
	// Stress ascending
	struct rb_tree_node *n1 = rb_tree_insert_and_fix_violations(NULL, 1);
	struct rb_tree_node *root = rb_tree_get_root(n1);
	struct rb_tree_node *n2 = rb_tree_insert_and_fix_violations(root, 2);
	root = rb_tree_get_root(n2);
	struct rb_tree_node *n3 = rb_tree_insert_and_fix_violations(root, 3);
	root = rb_tree_get_root(n3);
	struct rb_tree_node *n4 = rb_tree_insert_and_fix_violations(root, 4);
	root = rb_tree_get_root(n4);
	struct rb_tree_node *n5 = rb_tree_insert_and_fix_violations(root, 5);
	root = rb_tree_get_root(n5);
	struct rb_tree_node *n6 = rb_tree_insert_and_fix_violations(root, 6);
	root = rb_tree_get_root(n6);
	struct rb_tree_node *n7 = rb_tree_insert_and_fix_violations(root, 7);
	root = rb_tree_get_root(n7);
	struct rb_tree_node *n8 = rb_tree_insert_and_fix_violations(root, 8);
	root = rb_tree_get_root(n8);
	struct rb_tree_node *n9 = rb_tree_insert_and_fix_violations(root, 9);
	root = rb_tree_get_root(n9);
	struct rb_tree_node *n10 = rb_tree_insert_and_fix_violations(root, 10);
	root = rb_tree_get_root(n10);

	TEST_ASSERT_EQUAL_PTR(n4->left, n2);
	TEST_ASSERT_EQUAL_PTR(n4->right, n6);
	TEST_ASSERT_NULL(n4->parent);

	TEST_ASSERT_EQUAL_PTR(n2->left, n1);
	TEST_ASSERT_EQUAL_PTR(n2->right, n3);
	TEST_ASSERT_EQUAL_PTR(n2->parent, n4);

	TEST_ASSERT_EQUAL_PTR(n6->left, n5);
	TEST_ASSERT_EQUAL_PTR(n6->right, n8);
	TEST_ASSERT_EQUAL_PTR(n6->parent, n4);

	TEST_ASSERT_NULL(n1->left);
	TEST_ASSERT_NULL(n1->right);
	TEST_ASSERT_EQUAL_PTR(n1->parent, n2);

	TEST_ASSERT_NULL(n3->left);
	TEST_ASSERT_NULL(n3->right);
	TEST_ASSERT_EQUAL_PTR(n3->parent, n2);

	TEST_ASSERT_NULL(n5->left);
	TEST_ASSERT_NULL(n5->right);
	TEST_ASSERT_EQUAL_PTR(n5->parent, n6);

	TEST_ASSERT_EQUAL_PTR(n8->left, n7);
	TEST_ASSERT_EQUAL_PTR(n8->right, n9);
	TEST_ASSERT_EQUAL_PTR(n8->parent, n6);

	TEST_ASSERT_NULL(n7->left);
	TEST_ASSERT_NULL(n7->right);
	TEST_ASSERT_EQUAL_PTR(n7->parent, n8);

	TEST_ASSERT_NULL(n9->left);
	TEST_ASSERT_EQUAL_PTR(n9->right, n10);
	TEST_ASSERT_EQUAL_PTR(n9->parent, n8);

	TEST_ASSERT_NULL(n10->left);
	TEST_ASSERT_NULL(n10->right);
	TEST_ASSERT_EQUAL_PTR(n10->parent, n9);

	TEST_ASSERT_EQUAL(n4->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n2->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n6->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n1->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n3->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n5->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n7->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n9->color, RB_TREE_COLOR_BLACK);

	TEST_ASSERT_EQUAL(n8->color, RB_TREE_COLOR_RED);
	TEST_ASSERT_EQUAL(n10->color, RB_TREE_COLOR_RED);
}

void test_insert_and_fix_10(void)
{
	// Stress descending
	struct rb_tree_node *n10 = rb_tree_insert_and_fix_violations(NULL, 10);
	struct rb_tree_node *root = rb_tree_get_root(n10);
	struct rb_tree_node *n9 = rb_tree_insert_and_fix_violations(root, 9);
	root = rb_tree_get_root(n9);
	struct rb_tree_node *n8 = rb_tree_insert_and_fix_violations(root, 8);
	root = rb_tree_get_root(n8);
	struct rb_tree_node *n7 = rb_tree_insert_and_fix_violations(root, 7);
	root = rb_tree_get_root(n7);
	struct rb_tree_node *n6 = rb_tree_insert_and_fix_violations(root, 6);
	root = rb_tree_get_root(n6);
	struct rb_tree_node *n5 = rb_tree_insert_and_fix_violations(root, 5);
	root = rb_tree_get_root(n5);
	struct rb_tree_node *n4 = rb_tree_insert_and_fix_violations(root, 4);
	root = rb_tree_get_root(n4);
	struct rb_tree_node *n3 = rb_tree_insert_and_fix_violations(root, 3);
	root = rb_tree_get_root(n3);
	struct rb_tree_node *n2 = rb_tree_insert_and_fix_violations(root, 2);
	root = rb_tree_get_root(n2);
	struct rb_tree_node *n1 = rb_tree_insert_and_fix_violations(root, 1);
	root = rb_tree_get_root(n1);

	TEST_ASSERT_EQUAL_PTR(n7->left, n5);
	TEST_ASSERT_EQUAL_PTR(n7->right, n9);
	TEST_ASSERT_NULL(n7->parent);

	TEST_ASSERT_EQUAL_PTR(n5->left, n3);
	TEST_ASSERT_EQUAL_PTR(n5->right, n6);
	TEST_ASSERT_EQUAL_PTR(n5->parent, n7);

	TEST_ASSERT_EQUAL_PTR(n9->left, n8);
	TEST_ASSERT_EQUAL_PTR(n9->right, n10);
	TEST_ASSERT_EQUAL_PTR(n9->parent, n7);

	TEST_ASSERT_EQUAL_PTR(n3->left, n2);
	TEST_ASSERT_EQUAL_PTR(n3->right, n4);
	TEST_ASSERT_EQUAL_PTR(n3->parent, n5);

	TEST_ASSERT_NULL(n6->left);
	TEST_ASSERT_NULL(n6->right);
	TEST_ASSERT_EQUAL_PTR(n6->parent, n5);

	TEST_ASSERT_NULL(n8->left);
	TEST_ASSERT_NULL(n8->right);
	TEST_ASSERT_EQUAL_PTR(n8->parent, n9);

	TEST_ASSERT_NULL(n10->left);
	TEST_ASSERT_NULL(n10->right);
	TEST_ASSERT_EQUAL_PTR(n10->parent, n9);

	TEST_ASSERT_EQUAL_PTR(n2->left, n1);
	TEST_ASSERT_NULL(n2->right);
	TEST_ASSERT_EQUAL_PTR(n2->parent, n3);

	TEST_ASSERT_NULL(n4->left);
	TEST_ASSERT_NULL(n4->right);
	TEST_ASSERT_EQUAL_PTR(n4->parent, n3);

	TEST_ASSERT_NULL(n1->left);
	TEST_ASSERT_NULL(n1->right);
	TEST_ASSERT_EQUAL_PTR(n1->parent, n2);

	TEST_ASSERT_EQUAL(n4->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n2->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n6->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n10->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n8->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n5->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n7->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n9->color, RB_TREE_COLOR_BLACK);

	TEST_ASSERT_EQUAL(n3->color, RB_TREE_COLOR_RED);
	TEST_ASSERT_EQUAL(n1->color, RB_TREE_COLOR_RED);
}

void test_insert_and_fix_11(void)
{
	// Stress random insert
	struct rb_tree_node *n5 = rb_tree_insert_and_fix_violations(NULL, 5);
	struct rb_tree_node *root = rb_tree_get_root(n5);
	struct rb_tree_node *n1 = rb_tree_insert_and_fix_violations(root, 1);
	root = rb_tree_get_root(n1);
	struct rb_tree_node *n9 = rb_tree_insert_and_fix_violations(root, 9);
	root = rb_tree_get_root(n9);
	struct rb_tree_node *n3 = rb_tree_insert_and_fix_violations(root, 3);
	root = rb_tree_get_root(n3);
	struct rb_tree_node *n7 = rb_tree_insert_and_fix_violations(root, 7);
	root = rb_tree_get_root(n7);
	struct rb_tree_node *n2 = rb_tree_insert_and_fix_violations(root, 2);
	root = rb_tree_get_root(n2);
	struct rb_tree_node *n8 = rb_tree_insert_and_fix_violations(root, 8);
	root = rb_tree_get_root(n8);
	struct rb_tree_node *n6 = rb_tree_insert_and_fix_violations(root, 6);
	root = rb_tree_get_root(n6);
	struct rb_tree_node *n4 = rb_tree_insert_and_fix_violations(root, 4);
	root = rb_tree_get_root(n4);

	TEST_ASSERT_EQUAL_PTR(n5->left, n2);
	TEST_ASSERT_EQUAL_PTR(n5->right, n8);
	TEST_ASSERT_NULL(n5->parent);

	TEST_ASSERT_EQUAL_PTR(n2->left, n1);
	TEST_ASSERT_EQUAL_PTR(n2->right, n3);
	TEST_ASSERT_EQUAL_PTR(n2->parent, n5);

	TEST_ASSERT_EQUAL_PTR(n8->left, n7);
	TEST_ASSERT_EQUAL_PTR(n8->right, n9);
	TEST_ASSERT_EQUAL_PTR(n8->parent, n5);

	TEST_ASSERT_NULL(n1->left);
	TEST_ASSERT_NULL(n1->right);
	TEST_ASSERT_EQUAL_PTR(n1->parent, n2);

	TEST_ASSERT_NULL(n3->left);
	TEST_ASSERT_EQUAL_PTR(n3->right, n4);
	TEST_ASSERT_EQUAL_PTR(n3->parent, n2);

	TEST_ASSERT_EQUAL_PTR(n7->left, n6);
	TEST_ASSERT_NULL(n7->right);
	TEST_ASSERT_EQUAL_PTR(n7->parent, n8);

	TEST_ASSERT_NULL(n9->left);
	TEST_ASSERT_NULL(n9->right);
	TEST_ASSERT_EQUAL_PTR(n9->parent, n8);

	TEST_ASSERT_NULL(n4->left);
	TEST_ASSERT_NULL(n4->right);
	TEST_ASSERT_EQUAL_PTR(n4->parent, n3);

	TEST_ASSERT_NULL(n6->left);
	TEST_ASSERT_NULL(n6->right);
	TEST_ASSERT_EQUAL_PTR(n6->parent, n7);

	TEST_ASSERT_EQUAL(n5->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n1->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n3->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n7->color, RB_TREE_COLOR_BLACK);
	TEST_ASSERT_EQUAL(n9->color, RB_TREE_COLOR_BLACK);

	TEST_ASSERT_EQUAL(n2->color, RB_TREE_COLOR_RED);
	TEST_ASSERT_EQUAL(n8->color, RB_TREE_COLOR_RED);
	TEST_ASSERT_EQUAL(n4->color, RB_TREE_COLOR_RED);
	TEST_ASSERT_EQUAL(n6->color, RB_TREE_COLOR_RED);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_find_1);
    RUN_TEST(test_find_2);
    RUN_TEST(test_find_3);
    RUN_TEST(test_rotation_left_1);
    RUN_TEST(test_rotation_right_1);
    RUN_TEST(test_insert_1);
    RUN_TEST(test_alloc_1);
    RUN_TEST(test_insert_and_fix_1);
    RUN_TEST(test_insert_and_fix_2);
    RUN_TEST(test_insert_and_fix_3);
    RUN_TEST(test_insert_and_fix_4);
    RUN_TEST(test_insert_and_fix_5);
    RUN_TEST(test_insert_and_fix_6);
    RUN_TEST(test_insert_and_fix_7);
    RUN_TEST(test_insert_and_fix_8);
    RUN_TEST(test_insert_and_fix_9);
    RUN_TEST(test_insert_and_fix_10);
    RUN_TEST(test_insert_and_fix_11);

    return UNITY_END();
}
