#include <stdio.h>

int main(void) {

    char filename[] = "input.txt";

    FILE* fh = fopen(filename, "r");

    char filename2[] = "output.txt";

    FILE* fh2 = fopen(filename2, "w");

    int N, M;
    fscanf(fh, "%d %d", &N, &M);

    int b;
    for (int i = 0; i < M; i++) {
        fscanf(fh, "%d", &b);
    }

    for (int i = b; i <= N; i++) {
        fprintf(fh2, "%d ", i);
    }
    for (int i = 1; i < b; i++) {
        fprintf(fh2, "%d ", i);
    }

    return 0;
}
