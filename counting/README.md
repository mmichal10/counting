Compilation:

`gcc main.c`



Usage:

`./a.out <path_to_the_input_file>`



The directory `tools` contains a tool for generating example input files


The macros in `config.h` allow to change the number of workers and number of shards. Modifing the latter slightly increases the memory consumption but it can reduce lock contention.
