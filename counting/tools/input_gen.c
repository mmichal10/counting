#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#define COUNT 1000000000
#define LIMIT (1024*1024*1024)

uint32_t gen32(void) {
	return ((uint32_t)rand() << 16) ^ (uint32_t)rand();
}

int main() {
	char *inputfile_name = "random_numbers";
	FILE *f = fopen(inputfile_name, "wb");
	uint32_t i, num;
	int ret;
	//uint8_t array[LIMIT] = {};
	srand((unsigned)time(NULL));
	uint32_t seen_only_once = 0;
	uint32_t unique_numbers = 0;
	uint32_t curr_min = UINT_MAX;
	uint32_t curr_max = 0;

	if (f == NULL) {
		printf("Failed to open file\n");
		return 1;
	}


	for (i = 0; i < COUNT; i++) {
		num = gen32(); //% LIMIT;
		if (num > curr_max)
			curr_max = num;
		if (num < curr_min)
			curr_min = num;

		/*
		if (array[num] == 0) {
			unique_numbers += 1;
			seen_only_once += 1;
			array[num] = 1;
		} else if (array[num] == 1) {
			seen_only_once -= 1;
			array[num] = 2;
		}
		*/

		// I know it's slow to write every single number separately, but it's just helper code, so it doesn't matter
		ret = fwrite(&num, sizeof(num), 1, f);		
		if (ret != 1) {
			printf("Failed to generate input file.\n");
			goto end;
		}
	}

	printf("Unique numbers %u\n", unique_numbers);
	printf("seen only once %u\n", seen_only_once);
	printf("Curr min %u curr max %u\n", curr_min, curr_max);

end:
	fclose(f);
}
