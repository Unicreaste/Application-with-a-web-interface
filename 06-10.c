#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int val;
    struct Node* next;
} Node;

Node* arr[100001];
Node* prev[100001];

Node* add(Node* head, Node** last, int i) {
    Node* cur = malloc(sizeof(Node));
    cur->val = i;
    cur->next = NULL;
    arr[i] = cur;
    prev[i] = *last;

    if (*last)
        (*last)->next = cur;
    else
        head = cur;

    (*last) = cur;

    return head;
}

int main(void) {
    char filename[] = "input.txt";

    FILE* fh = fopen(filename, "r");

    int N, M;
    fscanf(fh, "%d %d", &N, &M);

    Node* head = NULL;
    Node* last = NULL;

    for (int i = 1; i <= N; i++) {
        head = add(head, &last, i);
    }

for (int i = 0; i < M; i++) {
    int b, c;
    fscanf(fh, "%d %d", &b, &c);

    Node* B = arr[b];
    Node* C = arr[c];

    Node* beforeB = prev[b];
    Node* afterC = C->next;

    if (beforeB) {
        beforeB->next = afterC;
        if (afterC)
            prev[afterC->val] = beforeB;
    } else {
        head = afterC;
        if (afterC)
            prev[afterC->val] = NULL;
    }

    C->next = head;
    if (head)
        prev[head->val] = C;

    head = B;
    prev[b] = NULL;
}


    char filename2[] = "output.txt";

    FILE* fh2 = fopen(filename2, "w");

    Node* cur = head;
    while (cur) {
        fprintf(fh2, "%d ", cur->val);
        cur = cur->next;
    }

    fclose(fh);
    fclose(fh2);
    return 0;
}
