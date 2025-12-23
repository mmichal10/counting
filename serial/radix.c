#include <stdint.h>
#include <assert.h>
#include <string.h>

#define RADIX_KEY_LEN 16
struct radix_tree_node {
	char key[RADIX_KEY_LEN];
	uint8_t key_len;
	struct radix_tree_node *children;
	uint8_t children_cnt;
	uint32_t count;
};

void radix_tree_insert(struct radix_tree_node *node, char *key, uint32_t key_len) {
	uint8_t i;

	if (strncmp(node->key, key, RADIX_KEY_LEN) == 0) {
		node->count++;
		return;
	}

	for (i = 0; i < node->children_cnt; i++) {
		if (strncmp(node->key, key, node->key_len) == 0) {
			return radix_tree_insert(node, key + node->key_len, key_len - node->key_len);
		}
	}

	node->children[node->children_cnt] = calloc(1, struct radix_tree_node);
	assert(node->children[node->children_cnt]);

	strcpy(node->children[node->children_cnt]->key, key);
	node->children_cnt++;
}

struct radix_tree_node *radix_tree_find(struct radix_tree_node *node, char *key, uint32_t key_len) {
	uint8_t i;

	if (strncmp(node->key, key, RADIX_KEY_LEN) == 0) {
		
		return node;
	}

	for (i = 0; i < node->children_cnt; i++) {
		if (strncmp(node->key, key, node->key_len) == 0) {
			return radix_tree_find(node, key + node->key_len, key_len - node->key_len);
		}
	}

}
