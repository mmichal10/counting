Compilation:

`gcc main.c`



Usage:

`./a.out <path_to_the_input_file>`



The directory `tools` contains a tool for generating example input files


The macros in `config.h` allow to change the number of workers and number of shards. Modifing the latter slightly increases the memory consumption but it can reduce lock contention.


Design:

The program accepts as a parameter a path to a binary file containing numbers from 0x0 to 0xffffffff and checks:
a. how many distinct numbers are in the file
b. how many numbers in the file are unique numbers.
After processing the whole file, the program prints these information to stdout.

The program creates `THREAD_COUNT` (defined in `config.h`) threads. Each of the the threads processes its own chunk of the file. The threads read the numbers from the input file and put them in the global data structure descirbled below. During the processing each of the workers logs the progress to stdout. The logic of wokers is implemented in `sharded_counting()` in main.c. 

The numbers are tracked by two global bitmaps defined in `counting.c`:
```
struct tree_owner {
	...
	uint64_t *added_once;
	uint64_t *added_twice;
	...
};
```

Each bit in the bitmaps represent a number from the range 0 - 0xffffffff. Numbers that have been seen only once have the bit set in `addeded_once` bitmap. If a number have been seen twice or more, the program also sets the bit in `added_twice`. The number of elements seen once and more times is cached in:
```
struct tree_owner {
	...
	uint64_t elements_in_map;
	uint64_t repeated_elements; 
	...
};
```

To enabled multiple workers work with the bitmap concurrently, the data structure is divided into `SHARDS` buckets (defined in `config.h`). Each of the shards tracks a separate range of numbers.

For each shard each bitmap is protected by its own lock defined in `struct tree_owner`. `pthread_rwlock_t lock;` protects `added_once` bitamp and `pthread_rwlock_t visited_lock;` protects `added_twice` bitmap.

The logic of adding new values to the bitmap is implemented in `count_numbers()` in `counting.c`. The function accepts an array of `uint32_t` numbers and inserts each number into its target bucket.
