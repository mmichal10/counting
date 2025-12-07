#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#define RB_TREE_COLOR_BLACK 4
#define RB_TREE_COLOR_RED 5

struct rb_tree_node {
	int32_t val;
	struct rb_tree_node *parent;
	struct rb_tree_node *left;
	struct rb_tree_node *right;
	uint8_t color;
};

struct rb_tree_node *rb_tree_alloc_node(int32_t val, struct rb_tree_node *parent) {
	struct rb_tree_node *ret = calloc(1, sizeof(struct rb_tree_node));
	if (ret == NULL)
		return NULL;

	ret->val = val;
	ret->parent = parent;
	ret->color = RB_TREE_COLOR_RED;

	return ret;
}

struct rb_tree_node *rb_tree_get_root(struct rb_tree_node *node) {

		if (node == NULL)
			return NULL;
	
		while (node->parent)
			node = node->parent;

		return node;
}

struct rb_tree_node *rb_tree_insert(struct rb_tree_node *node, int32_t val) {

	struct rb_tree_node *new_node;
	
	// Allocate the root
	if (node == NULL) // TODO Is there unlikely() in the userpsace?
		return rb_tree_alloc_node(val, NULL);

	if (node->val > val && node->left != NULL)
		return rb_tree_insert(node->left, val);
	else if (node->val < val && node->right != NULL)
		return rb_tree_insert(node->right, val);

	new_node = rb_tree_alloc_node(val, node);
	if (new_node == NULL)
		return NULL;

	if (node->val > val)
		node->left = new_node;
	else
		node->right = new_node;

	return new_node;
}

void rb_tree_rotate_left(struct rb_tree_node *x) {
	//1. y is the right child of x
	assert(x->right != NULL);

	struct rb_tree_node *y = x->right;
	struct rb_tree_node *parent = x->parent;

	//2. Move y’s left subtree to x’s right subtree.
	x->right = y->left;
	if (x->right)
		x->right->parent = x;

  	//3. Update the parent of y to parent of x
	y->parent = x->parent;

	//4. Update x’s parent to point to y instead of x.
	if (parent != NULL && x == parent->left)
		parent->left = y;
	else if (parent != NULL && x == parent->right)
		parent->right = y;

	//5. Set y’s left child to x.
	y->left = x;

	//6. Update x’s parent to y.
	x->parent = y;
}

void rb_tree_rotate_right(struct rb_tree_node *x) {
	//1. y is the left child of x
	assert(x->left != NULL);

	struct rb_tree_node *y = x->left;
	struct rb_tree_node *parent = x->parent;

	//2. Move y’s right subtree to x’s left subtree.
	x->left = y->right;
	if (x->left)
		x->left->parent = x;
	
  	//3. Update the parent of y to parent of x
	y->parent = x->parent;

	//4. Update x’s parent to point to y instead of x.
	if (parent != NULL && x == parent->left)
		parent->left = y;
	if (parent != NULL && x == parent->right)
		parent->right = y;

	//5. Set y’s right child to x.
	y->right = x;

	//6. Update x’s parent to y.
	x->parent = y;
}

static inline struct rb_tree_node *rb_tree_get_uncle(struct rb_tree_node *node) {

	struct rb_tree_node *parent = node->parent;
	struct rb_tree_node *grandparent = parent->parent;
	
	if (grandparent == NULL)
		return NULL;

	if (parent == grandparent->left)
		return grandparent->right;
	else
		return grandparent->left;
}

void rb_tree_fix_violations(struct rb_tree_node *node) {
	struct rb_tree_node *grandparent;
	struct rb_tree_node *parent;
	struct rb_tree_node *uncle;
	uint8_t tmp_color;

	parent = node->parent;

	if (parent == NULL) {
		node->color = RB_TREE_COLOR_BLACK;
		return;
	}

	if (parent->color == RB_TREE_COLOR_BLACK)
		return;

	uncle = rb_tree_get_uncle(node);
	grandparent = parent->parent;

	if (uncle != NULL && uncle->color == RB_TREE_COLOR_RED) {

		uncle->color = RB_TREE_COLOR_BLACK;
		parent->color = RB_TREE_COLOR_BLACK;

		if (grandparent->parent != NULL) {
			grandparent->color = RB_TREE_COLOR_RED;
			return rb_tree_fix_violations(grandparent);
		}
	} else {
		if (parent == grandparent->left && node == parent->left) {

			rb_tree_rotate_right(grandparent);

			tmp_color = grandparent->color;
			grandparent->color = parent->color;
			parent->color = tmp_color;

		} else if (parent == grandparent->right && node == parent->right) {
			
			rb_tree_rotate_left(grandparent);

			tmp_color = grandparent->color;
			grandparent->color = parent->color;
			parent->color = tmp_color;

		} else if (parent == grandparent->left && node == parent->right) {
		
			rb_tree_rotate_left(parent);

			rb_tree_rotate_right(grandparent);

			tmp_color = grandparent->color;
			grandparent->color = node->color;
			node->color = tmp_color;

		} else if (parent == grandparent->right && node == parent->left) {

			rb_tree_rotate_right(parent);

			rb_tree_rotate_left(grandparent);

			tmp_color = grandparent->color;
			grandparent->color = node->color;
			node->color = tmp_color;

		} else {
			assert(0);
		}
	}
}

struct rb_tree_node* rb_tree_insert_and_fix_violations(struct rb_tree_node *node, int32_t val) {

	struct rb_tree_node *new_node = rb_tree_insert(node, val);

	if (new_node == NULL)
		return NULL;

	rb_tree_fix_violations(new_node);

	return new_node;
}

struct rb_tree_node *rb_tree_find(struct rb_tree_node *node, int32_t val) {
	if (node == NULL)
		return NULL;

	if (node->val == val)
		return node;

	if (node->val > val)
		return rb_tree_find(node->left, val);
	else
		return rb_tree_find(node->right, val);
}

