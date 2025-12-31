Compilation:

`make`



Usage:

`./count_models <path_to_the_input_file>`

`verify.py` parases JSON file and prints how many occurances of each model are in the input file


Design:

The program accepts as a paramter a path to a JSON file with the following format:
```
[{"id":0,"model":"RDV2","serial":"HD92731454"},{"id":1,"model":"DRV1","serial":"HD78784271"}]
```
The program counts how many models there are and how many times each one is present.

To track the models the program uses a hashmap. The model strings are hashed using FNV hasing function (implemented in `hash.c`) and inserted into the hashmap. The hashmap in divided into shards, and for a given hash the shard is choosen by calculating `hash % shards_count`. The number of max possible hashes is controlled by `MAX_HASHES` macro in `main.c`

The hashtable implements open addressing with linear probing. The max key len is 16 characters but it can be modified by changing `MAX_KEY_LEN` in `hashtable.c`. The number of occurances of a key is tracked by `count` atomic variable in `struct hash_table_entry` defined in `hashtable.c`.

If the number of elements in a hashtable shard reaches a threshold of 75% of max number of elements (defined with `get_resize_threshold()` macro), it attempts to double the size of the allocated memory. If the allocation fails, the program continues to operate normally and repeates the allocation attempt upon inserting next new element and continues to do so until reaching ~95% of capacity. If the allocation fails at that point, the program returns an error to the user.

To enable processing in parallel, the program creates two pthreads (configurable with `THREAD_COUNT` in `main.c`). As the input file is in json format, the file can't be simply divided into chunks by calculating the offsets because the workers would end up with corrupted first and the last entries. To prevent corrupting the input data, the shard sizes are adjusted with `json_shard_the_file()` in `parse_json.c`. Simliar adjustments are done in the worker loops (`sharded_counting` in `main.c`) so no entry is lost.

The concurrent access to the hashtable is protected by locks defined in `struct counting_ctx` in `counting.c`. Each of the shards has its own lock. Inserting new entries into a hash table happens in `counting_insert_model` in `counting.c`. One important details is that even though the `entry->count` is an atomic variable, it needs to be accessed under a read lock, because otherwise, another worker could resize the shard concurrently.
