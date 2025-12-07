#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#define COUNT 1000000000
#define LIMIT (1024*1024*1024)

int main() {
	char *inputfile_name = "random_numbers";
	FILE *f = fopen(inputfile_name, "wb");
	uint32_t i, num;
	int ret;
	//uint8_t array[LIMIT] = {};
	srand((unsigned)time(NULL));
	uint32_t seen_only_once = 0;
	uint32_t unique_numbers = 0;

	if (f == NULL) {
		printf("Failed to open file\n");
		return 1;
	}


	for (i = 0; i < COUNT; i++) {
		num = rand() % LIMIT;

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

end:
	fclose(f);
}
