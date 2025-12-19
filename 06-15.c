#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 200001
#define MAX_NAME 101

typedef struct Node {
    char name[MAX_NAME];
    unsigned int ip;
    struct Node* next;
} Node;

Node* table[TABLE_SIZE] = {0};

unsigned int hash(char* s) {
    unsigned int h = 5381;
    while (*s) {
        h = ((h << 5) + h) + (unsigned char)(*s);
        s++;
    }
    return h % TABLE_SIZE;
}

void add(char* name, unsigned int ip) {
    unsigned int h = hash(name);

    Node* node = malloc(sizeof(Node));
    strcpy(node->name, name);
    node->ip = ip;
    node->next = table[h];
    table[h] = node;
}

int find(char* name, unsigned int* ip) {
    unsigned int h = hash(name);
    Node* cur = table[h];

    while (cur) {
        if (!strcmp(cur->name, name)) {
            *ip = cur->ip;
            return 1;
        }
        cur = cur->next;
    }
    return 0;
}

int main(void) {
    FILE* fh = fopen("input.txt", "r");

    int n;
    fscanf(fh, "%d", &n);

    for (int i = 0; i < n; i++) {
        char name[MAX_NAME];
        unsigned int ip;
        fscanf(fh, "%100s %u", name, &ip);
        add(name, ip);
    }

    int m;
    fscanf(fh, "%d", &m);

    FILE* fh2 = fopen("output.txt", "w");

    for (int i = 0; i < m; i++) {
        char name[MAX_NAME];
        fscanf(fh, "%100s", name);

        unsigned int ip;
        if (find(name, &ip)) {
            fprintf(fh2, "%u\n", ip);
        } else {
            fprintf(fh2, "-1\n");
        }
    }

    fclose(fh);
    fclose(fh2);
    return 0;
}
