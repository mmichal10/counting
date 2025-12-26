#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#define CHUNK_BYTES (64 * 1024 * 1024)  /* 64 MB, must be multiple of 4 */

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <binary_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    /* Ensure chunk size is aligned to uint32_t */
    const size_t elems_per_chunk = CHUNK_BYTES / sizeof(uint32_t);
    uint32_t *buffer = malloc(elems_per_chunk * sizeof(uint32_t));
    if (!buffer) {
        perror("malloc");
        fclose(fp);
        return EXIT_FAILURE;
    }

    while (1) {
        size_t n = fread(buffer, sizeof(uint32_t), elems_per_chunk, fp);
        if (n == 0) {
            if (ferror(fp)) {
                perror("fread");
                free(buffer);
                fclose(fp);
                return EXIT_FAILURE;
            }
            break; /* EOF */
        }

        for (size_t i = 0; i < n; i++) {
            printf("%u\n", buffer[i]);
        }
    }

    free(buffer);
    fclose(fp);
    return EXIT_SUCCESS;
}

