#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ROWS 10000
#define MAX_COLS 20
#define MAX_FIELD_LEN 102
#define MAX_LINE_LEN 2500

typedef enum {
    FIELD_INT,
    FIELD_STR
} FieldType;

typedef struct {
    int pase;
    char strok[MAX_FIELD_LEN];
    FieldType type;
} Field;

typedef struct {
    Field fields[MAX_COLS];
    int num;
} Row;

int n_rows = 0;
Row rows[MAX_ROWS];
int N = -1;

char* skip_spaces(char* s) {
    while (*s && isspace((unsigned char)*s))
        s++;
    return s;
}

char* parse_field(char* s, Field* field) {
    s = skip_spaces(s);
    if (*s == '"') {
        s++;
        char* dst = field->strok;
        while (*s && *s != '"') {
            *dst++ = *s++;
        }
        *dst = '\0';
        if (*s == '"')
            s++;
        field->type = FIELD_STR;
    } else if (isdigit((unsigned char)*s) || *s == '-' || *s == '+') {
        char* endptr;
        long val = strtol(s, &endptr, 10);
        field->pase = (int)val;
        s = endptr;
        field->type = FIELD_INT;
    } else {
        field->pase = 0;
        field->strok[0] = '\0';
        field->type = FIELD_INT;
    }
    return skip_spaces(s);
}

int parse_row(char* arr, Row* row) {
    char* s = arr;
    int i = 0;
    while (*s && i < MAX_COLS) {
        s = parse_field(s, &row->fields[i]);
        i++;
        if (*s == ';') {
            s++;
        } else {
            break;
        }
    }
    row->num = i;
    return i;
}

int compare_rows(const void* a, const void* b) {
    const Row* ra = (const Row*)a;
    const Row* rb = (const Row*)b;
    const Field* fa = &ra->fields[N];
    const Field* fb = &rb->fields[N];

    if (fa->type == FIELD_INT) {
        int ia = fa->pase;
        int ib = fb->pase;
        return (ia > ib) - (ia < ib);
    } else {
        return strcmp(fa->strok, fb->strok);
    }
}

void print_row(FILE* f, const Row* row) {
    for (int i = 0; i < row->num; i++) {
        if (i > 0) {
            fputc(';', f);
        }
        if (row->fields[i].type == FIELD_STR) {
            fputc('"', f);
            fputs(row->fields[i].strok, f);
            fputc('"', f);
        } else {
            fprintf(f, "%d", row->fields[i].pase);
        }
    }
    fputc('\n', f);
}

int main(void) {
    char filename[] = "input.txt";

    FILE* in = fopen(filename, "r");

    fscanf(in, "%d\n", &N);

    char arr[MAX_LINE_LEN];
    while (fgets(arr, sizeof(arr), in)) {
        int len = strlen(arr);
        if (len > 0 && arr[len - 1] == '\n') {
            arr[len - 1] = '\0';
        }

        if (strlen(skip_spaces(arr)) == 0) {
            continue;
        }

        if (n_rows >= MAX_ROWS) {
            break;
        }

        parse_row(arr, &rows[n_rows]);
        n_rows++;
    }

    fclose(in);

    if (n_rows > 0) {
        qsort(rows, n_rows, sizeof(Row), compare_rows);
    }

    char filename2[] = "output.txt";

    FILE* fh2 = fopen(filename2, "w");

    for (int i = 0; i < n_rows; i++) {
        print_row(fh2, &rows[i]);
    }

    fclose(fh2);

    return 0;
}