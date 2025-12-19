#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint16_t rewrite_little_16(FILE* file) {
    uint16_t x;
    fread(&x, sizeof(x), 1, file);
    return (x << 8) |
           (x >> 8);
}

uint32_t rewrite_little_32(FILE* file) {
    uint32_t x;
    fread(&x, sizeof(x), 1, file);
    return ((x & 0x000000FF) << 24) |
           ((x & 0x0000FF00) << 8) |
           ((x & 0x00FF0000) >> 8) |
           ((x & 0xFF000000) >> 24);
}

void rewrite_big(FILE* file, uint64_t n) {
    uint8_t byte[8];
    for (int k = 0; k < 8; k++) {
        byte[7 - k] = n & 0xFF;
        n >>= 8;
    }
    fwrite(byte, sizeof(byte), 1, file);
}

int main(void) {
    char filename[] = "matrix.in";

    FILE* fh = fopen(filename, "rb");

    uint16_t size = rewrite_little_16(fh);
    int64_t trace_sum = 0;

    for (uint16_t row = 0; row < size; row++) {
        for (uint16_t col = 0; col < size; col++) {
            uint32_t elem = rewrite_little_32(fh);
            if (row == col) {
                trace_sum += (int64_t)(int32_t)elem;
            }
        }
    }

    char filename2[] = "trace.out";

    FILE* fh2 = fopen(filename2, "wb");
    rewrite_big(fh2, (uint64_t)trace_sum);

    fclose(fh);
    fclose(fh2);

    return 0;
}