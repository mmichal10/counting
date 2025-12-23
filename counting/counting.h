#include "counting.c"

int count_numbers(uint32_t *arr, int count, struct tree_owner ctx[]);

void prepare_shards(struct tree_owner ctx[], uint64_t shards_count, uint64_t shard_size);
void destroy_shards(struct tree_owner ctx[], uint64_t shards_count);

uint64_t seen_only_once(struct tree_owner *owner);

uint64_t aggregate_unique_numbers(struct tree_owner ctx[], uint64_t shards);

uint64_t aggregate_seen_only_once(struct tree_owner ctx[], uint64_t shards);
