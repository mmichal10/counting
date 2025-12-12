#ifndef __TREE_C__
#define __TREE_C__
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#define RB_TREE_COLOR_BLACK 4
#define RB_TREE_COLOR_RED 5

struct rb_tree_node {
	uint32_t range_min;
	uint32_t range_max;

	struct rb_tree_node *parent;
	struct rb_tree_node *left;
	struct rb_tree_node *right;

	uint64_t exists;
	uint64_t visited;

	uint8_t color;
};

#define RB_TREE_GET_MIN_NODE_RANGE(__value) (__value & 0xffffffc0)
#define RB_TREE_GET_MAX_NODE_RANGE(__value) (__value | 0x0000003f)
#define RB_TREE_GET_ID_IN_NODE_RANGE(__value) (__value & 0x0000003f)

#define RB_TREE_VALUE_EXISTS(__node, __value) \
	(__node->exists & (uint64_t)((uint64_t)1 << RB_TREE_GET_ID_IN_NODE_RANGE(__value)))

#define RB_TREE_VALUE_WAS_VISITED(__node, __value) \
	(__node->visited & (uint64_t)((uint64_t)1 << RB_TREE_GET_ID_IN_NODE_RANGE(__value)))

#define RB_TREE_SET_EXISTS(__node, __value) \
	(__node->exists |= (uint64_t)((uint64_t)1 << RB_TREE_GET_ID_IN_NODE_RANGE(__value)))

#define RB_TREE_SET_WAS_VISITED(__node, __value) \
	(__node->visited |= (uint64_t)((uint64_t)1 << RB_TREE_GET_ID_IN_NODE_RANGE(__value)))

struct rb_tree_node *rb_tree_alloc_node(uint32_t val, struct rb_tree_node *parent) {
	struct rb_tree_node *ret = calloc(1, sizeof(struct rb_tree_node));
	if (ret == NULL)
		return NULL;

	ret->range_min = RB_TREE_GET_MIN_NODE_RANGE(val);
	ret->range_max = RB_TREE_GET_MAX_NODE_RANGE(val);
	ret->parent = parent;
	ret->color = RB_TREE_COLOR_RED;

	RB_TREE_SET_EXISTS(ret, val);
	ret->visited = 0;

	return ret;
}

void rb_tree_deinit(struct rb_tree_node *root) {
	if (root == NULL)
		return;

	if (root->left != NULL) {
		rb_tree_deinit(root->left);
		root->left = NULL;
	} 

	if (root->right != NULL) {
		rb_tree_deinit(root->right);
		root->right = NULL;
	}

	free(root);
}

struct rb_tree_node *rb_tree_get_root(struct rb_tree_node *node) {

		if (node == NULL)
			return NULL;
	
		while (node->parent)
			node = node->parent;

		return node;
}

struct rb_tree_node *rb_tree_insert(struct rb_tree_node *node, uint32_t val) {

	struct rb_tree_node *new_node;
	
	if (node == NULL)
		return rb_tree_alloc_node(val, NULL);

	if (node->range_min > val && node->left != NULL)
		return rb_tree_insert(node->left, val);
	else if (node->range_max < val && node->right != NULL)
		return rb_tree_insert(node->right, val);

	if (val >= node->range_min && val <= node->range_max) {
		// I assume that the code doesn't depend on checking the if the value
		// already exsists in the cache
		RB_TREE_SET_EXISTS(node, val);
		return node;
	}

	new_node = rb_tree_alloc_node(val, node);
	if (new_node == NULL)
		return NULL;

	if (node->range_min > val)
		node->left = new_node;
	else if (node->range_min < val)
		node->right = new_node;
	else
		assert(0);

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

struct rb_tree_node* rb_tree_insert_and_fix_violations(struct rb_tree_node *node, uint32_t val) {

	struct rb_tree_node *new_node = rb_tree_insert(node, val);

	if (new_node == NULL)
		return NULL;

	rb_tree_fix_violations(new_node);

	return new_node;
}

struct rb_tree_node *rb_tree_find(struct rb_tree_node *node, uint32_t val) {
	if (node == NULL)
		return NULL;

	if (val >= node->range_min && val <= node->range_max) {
		if (RB_TREE_VALUE_EXISTS(node, val))
			return node;

		return NULL;
	}

	if (val < node->range_min)
		return rb_tree_find(node->left, val);
	else if (val > node->range_max)
		return rb_tree_find(node->right, val);
	else
		assert(0);
}

static inline void clear_pointers(struct rb_tree_node *node) {
	if (node == NULL)
		return;
	
	node->parent = NULL;
	node->left = NULL;
	node->right = NULL;
}

static inline void replace_node(struct rb_tree_node *old_node, struct rb_tree_node *new_node) {
	struct rb_tree_node *parent = NULL;
	struct rb_tree_node *left_child = NULL;
	struct rb_tree_node *right_child = NULL;

	if (old_node != NULL) {
		parent = old_node->parent;
		left_child = old_node->left;
		right_child = old_node->right;
	}

	if (parent != NULL && old_node == parent->left) {
		parent->left = new_node;
		if (new_node != NULL)
			new_node->parent = parent;
	} else if (parent != NULL && old_node == parent->right) {
		parent->right = new_node;
		if (new_node != NULL)
			new_node->parent = parent;
	} else if (parent == NULL && new_node != NULL) {
			new_node->parent = parent;
	} else {
		assert(parent == NULL);
	}

	if (left_child != NULL && left_child != new_node) {
		left_child->parent = new_node;
		if (new_node != NULL)
			new_node->left = left_child;
	}

	if (right_child != NULL && right_child != new_node) {
		right_child->parent = new_node;
		if (new_node != NULL)
			new_node->right = right_child;
	}
}

void rb_tree_dealloc_node(struct rb_tree_node *node) {
	free(node);
}

static inline struct rb_tree_node* _get_root_somehow(struct rb_tree_node *parent,
		struct rb_tree_node *child_1, struct rb_tree_node *child_2) {
	if (parent != NULL)
		return rb_tree_get_root(parent);
	if (child_1 != NULL)
		return rb_tree_get_root(child_2);
	if (child_2 != NULL)
		return rb_tree_get_root(child_1);

	return NULL;
}

static inline struct rb_tree_node* _get_replacing_node(struct rb_tree_node *node) {
	struct rb_tree_node *replacing_node;

	if (node->right != NULL) {
		replacing_node = node->right;

		while (replacing_node->left != NULL)
			replacing_node = replacing_node->left;
	} else if (node->left != NULL) {
		replacing_node = node->left;

		while (replacing_node->right != NULL)
			replacing_node = replacing_node->right;
	} else {
		replacing_node = NULL;
	}

	return replacing_node;
}

struct rb_tree_node *rb_tree_delete(struct rb_tree_node *root, uint32_t val) {
	struct rb_tree_node *parent;
	struct rb_tree_node *left_child;
	struct rb_tree_node *right_child;
	struct rb_tree_node *target_node;
	struct rb_tree_node *replacing_node;
	struct rb_tree_node *node_to_fix = NULL;
	uint8_t target_node_color;
	uint8_t replacing_node_color;
	struct rb_tree_node dummy_node = {};
	uint8_t target_is_left_child;

	if (root == NULL)
		return NULL;

	target_node = rb_tree_find(root, val);
	if (target_node == NULL)
		return rb_tree_get_root(root);

	parent = target_node->parent;
	left_child = target_node->left;
	right_child = target_node->right;
	target_node_color = target_node->color;

	if (left_child == NULL && right_child == NULL && target_node_color == RB_TREE_COLOR_RED) {
		replace_node(target_node, NULL);

		rb_tree_dealloc_node(target_node);

		return rb_tree_get_root(parent);
	}

	dummy_node.left = (void*)0xdeafbeef;
	dummy_node.right = (void*)0xdeafbeef;
	dummy_node.color = RB_TREE_COLOR_BLACK;

	if (target_node->left == NULL) {
		replacing_node = target_node->right;

		if (replacing_node != NULL)
			node_to_fix = replacing_node->right;

		if (node_to_fix == NULL) {
			if (replacing_node == NULL) {
				assert(parent != NULL);
				dummy_node.parent = parent;
			} else {
				dummy_node.parent = replacing_node;
			}

			target_is_left_child = (dummy_node.parent->left == target_node);
		}
		replace_node(target_node, replacing_node);

		if (node_to_fix == NULL) {
			node_to_fix = &dummy_node;
			if (target_is_left_child)
				dummy_node.parent->left = &dummy_node;
			else
				dummy_node.parent->right = &dummy_node;
		}

		if (replacing_node != NULL) {
			replacing_node_color = replacing_node->color;
			replacing_node->color = target_node_color;
		} else {
			replacing_node_color = RB_TREE_COLOR_BLACK;
		}
		rb_tree_dealloc_node(target_node);

	} else if (target_node->right == NULL) {
		replacing_node = target_node->left;

		if (replacing_node != NULL)
			node_to_fix = replacing_node->right;

		if (node_to_fix == NULL) {
			if (replacing_node == NULL) {
				assert(parent != NULL);
				dummy_node.parent = parent;
			} else {
				dummy_node.parent = replacing_node;
			}

			target_is_left_child = (dummy_node.parent->left == target_node);
		}

		replace_node(target_node, replacing_node);

		if (node_to_fix == NULL) {
			node_to_fix = &dummy_node;
			if (target_is_left_child)
				dummy_node.parent->left = &dummy_node;
			else
				dummy_node.parent->right = &dummy_node;
		}

		if (replacing_node != NULL) {
			replacing_node_color = replacing_node->color;
			replacing_node->color = target_node_color;
		} else {
			replacing_node_color = RB_TREE_COLOR_BLACK;
		}

		rb_tree_dealloc_node(target_node);
	} else if (target_node->right != NULL && target_node->left != NULL) {
		replacing_node = _get_replacing_node(target_node);
		assert(replacing_node != NULL);

		node_to_fix = replacing_node->right;

		if (node_to_fix == NULL)
			node_to_fix = &dummy_node;

		replace_node(replacing_node, node_to_fix);

		replace_node(target_node, replacing_node);

		replacing_node_color = replacing_node->color;
		replacing_node->color = target_node_color;

		rb_tree_dealloc_node(target_node);
	}

	if (target_node_color == RB_TREE_COLOR_RED || replacing_node_color == RB_TREE_COLOR_RED)
		goto remove_dummy_node;

	// Here comes the fixing

	struct rb_tree_node *sybling;

	while (node_to_fix != NULL && node_to_fix->parent != NULL && node_to_fix->color == RB_TREE_COLOR_BLACK) {
		if (node_to_fix == node_to_fix->parent->left) {
			sybling = node_to_fix->parent->right;
			if (sybling != NULL && sybling->color == RB_TREE_COLOR_RED) {
				sybling->color = RB_TREE_COLOR_BLACK;
				node_to_fix->parent->color = RB_TREE_COLOR_RED;
				rb_tree_rotate_left(node_to_fix->parent);

				// After the shift left node_to_fix has a new parent, so sybling changed as well
				sybling = node_to_fix->parent->right;
			}

			if (sybling &&
				(sybling->left == NULL || sybling->left->color == RB_TREE_COLOR_BLACK) &&
				(sybling->right == NULL || sybling->right->color == RB_TREE_COLOR_BLACK)) {

					sybling->color = RB_TREE_COLOR_RED;
					node_to_fix = node_to_fix->parent;
					continue;

			} else {
				if (sybling != NULL &&
						(sybling->right == NULL || sybling->right->color == RB_TREE_COLOR_BLACK) &&
						sybling->left != NULL) {
					sybling->left->color = RB_TREE_COLOR_BLACK;
					sybling->color = RB_TREE_COLOR_RED;
					rb_tree_rotate_right(sybling);
					sybling = node_to_fix->parent->right;
				}
				
				if (sybling != NULL)
					sybling->color = node_to_fix->parent->color;

				node_to_fix->parent->color = RB_TREE_COLOR_BLACK;
				if (sybling != NULL) {
					assert(sybling->right != NULL);
					sybling->right->color = RB_TREE_COLOR_BLACK;
					rb_tree_rotate_left(node_to_fix->parent);
				}
				node_to_fix = rb_tree_get_root(node_to_fix);
			}

		} else if (node_to_fix == node_to_fix->parent->right) {

			sybling = node_to_fix->parent->left;
			if (sybling != NULL && sybling->color == RB_TREE_COLOR_RED) {
				sybling->color = RB_TREE_COLOR_BLACK;
				node_to_fix->parent->color = RB_TREE_COLOR_RED;
				rb_tree_rotate_right(node_to_fix->parent);

				// After the shift right node_to_fix has a new parent, so sybling changed as well
				sybling = node_to_fix->parent->left;
			}

			if (sybling != NULL &&
				(sybling->right == NULL || sybling->right->color == RB_TREE_COLOR_BLACK) &&
				(sybling->left == NULL || sybling->left->color == RB_TREE_COLOR_BLACK)) {

					sybling->color = RB_TREE_COLOR_RED;
					node_to_fix = node_to_fix->parent;
					continue;

			} else {
				if (sybling != NULL &&
						(sybling->left == NULL || sybling->left->color == RB_TREE_COLOR_BLACK) &&
						sybling->right != NULL) {
					sybling->right->color = RB_TREE_COLOR_BLACK;
					sybling->color = RB_TREE_COLOR_RED;
					rb_tree_rotate_left(sybling);
					sybling = node_to_fix->parent->left;
				}
				
				if (sybling != NULL)
					sybling->color = node_to_fix->parent->color;

				node_to_fix->parent->color = RB_TREE_COLOR_BLACK;

				if (sybling != NULL) {
					assert(sybling->left != NULL);
					sybling->left->color = RB_TREE_COLOR_BLACK;
					rb_tree_rotate_right(node_to_fix->parent);
				}
				node_to_fix = rb_tree_get_root(node_to_fix);
			}

			
		} else {
			assert(0);
		}
	}
	if (node_to_fix != NULL)
		node_to_fix->color = RB_TREE_COLOR_BLACK;

remove_dummy_node:
	if (dummy_node.parent != NULL) {
		if (dummy_node.parent->left == &dummy_node)
			dummy_node.parent->left = NULL;
		else if (dummy_node.parent->right == &dummy_node)
			dummy_node.parent->right = NULL;
		else
			assert(0);
	}

	return _get_root_somehow(node_to_fix, NULL, NULL);
	

	
}

#endif
