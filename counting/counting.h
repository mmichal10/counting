#include "counting.c"

int count_numbers(uint32_t *arr, int count, struct tree_owner ctx[]);

void prepare_shards(struct tree_owner ctx[], uint32_t shards_count, uint32_t shard_size);
void destroy_shards(struct tree_owner ctx[], uint32_t shards_count, uint32_t shard_size);

uint32_t seen_only_once(struct tree_owner *owner);

uint32_t aggregate_unique_numbers(struct tree_owner ctx[], uint32_t shards);

uint32_t aggregate_seen_only_once(struct tree_owner ctx[], uint32_t shards);
