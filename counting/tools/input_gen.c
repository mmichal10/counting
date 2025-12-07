#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#define COUNT 10000

int main() {
	char *inputfile_name = "random_numbers";
	FILE *f = fopen(inputfile_name, "wb");
	uint32_t i, num;
	int ret;

	if (f == NULL) {
		printf("Failed to open file\n");
		return 1;
	}

	for (i = 0; i < COUNT + 500; i++) {
		num = i % COUNT;
		// I know it's slow to write every single number separately, but it's just helper code, so it doesn't matter
		ret = fwrite(&num, sizeof(num), 1, f);		
		if (ret != 1) {
			printf("Failed to generate input file.\n");
			goto end;
		}
	}

end:
	fclose(f);
}
