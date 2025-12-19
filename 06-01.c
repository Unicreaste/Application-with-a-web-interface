#include <stdio.h>
#include <string.h>

int empty(char* s) {
    for (int i = 0; s[i]; i++) {
        if (s[i] != ' ' && s[i] != '\n') {
            return 0;
        }
    }

    return 1;
}

int main(void) {
    char filename[] = "input.txt";

    FILE* fh = fopen(filename, "r");

    char filename2[] = "output.txt";

    FILE* fh2 = fopen(filename2, "w");

    char line[20001];
    int w = 0, s = 0, p = 0;

    int parag = 0;
    int word = 0;

    while (fgets(line, 20001, fh)) {
        int len = strlen(line);

        if (empty(line)) {
            parag = 0;
            if (word) {
                w++;
                word = 0;
            }
            continue;
        }

        if (!parag) {
            p++;
            parag = 1;
        }

        for (int i = 0; i < len; i++) {
            char c = line[i];

            if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')) {
                if (!word)
                    word = 1;
            } else if (c == '.') {
                s++;
                if (word) {
                    w++;
                    word = 0;
                }
            } else if (c == ' ' || c == '\n') {
                if (c == '\n') {
                    int j = i - 1;
                    while (j >= 0 && line[j] == ' ') {
                        j--;
                    }
                    if (j >= 0 && line[j] == '-') {
                        continue;
                    }
                }
                if (word) {
                    w++;
                    word = 0;
                }
            }
        }
    }

    if (word) {
        w++;
    }

    fprintf(fh2, "%d %d %d", w, s, p);

    fclose(fh);
    fclose(fh2);
    return 0;
}