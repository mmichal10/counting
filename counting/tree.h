#include "tree.c"

struct rb_tree_node *rb_tree_alloc_node(uint32_t val, struct rb_tree_node *parent);

struct rb_tree_node *rb_tree_get_root(struct rb_tree_node *node);

#define RB_TREE_NODE_ALREADY_EXISTS 1
#define RB_TREE_INSERT_ENOMEM 2

struct rb_tree_node *rb_tree_insert(struct rb_tree_node *node, uint32_t val);

struct rb_tree_node* rb_tree_insert_and_fix_violations(struct rb_tree_node *node, uint32_t val);

struct rb_tree_node *rb_tree_find(struct rb_tree_node *node, uint32_t val);

void rb_tree_rotate_left(struct rb_tree_node *x);
void rb_tree_rotate_right(struct rb_tree_node *x);
