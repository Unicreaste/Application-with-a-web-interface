#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int key;
    struct Node* next;
} Node;

Node* append(Node* tail, int n) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->key = n;
    node->next = NULL;
    if (tail) {
        tail->next = node;
    }

    return node;
}

Node* merge(Node* a, Node* b) {
    Node cur;
    Node* tail = &cur;
    cur.next = NULL;

    while (a && b) {
        if (a->key <= b->key) {
            tail->next = a;
            a = a->next;
        } else {
            tail->next = b;
            b = b->next;
        }
        tail = tail->next;
    }

    tail->next = (a) ? a : b;
    return cur.next;
}

void split(Node* head, Node** front, Node** back) {
    Node* slow = head;
    Node* fast = head->next;

    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }

    *front = head;
    *back = slow->next;
    slow->next = NULL;
}

Node* merge_sort(Node* head) {
    if (!head || !head->next) {
        return head;
    }

    Node *a, *b;
    split(head, &a, &b);
    a = merge_sort(a);
    b = merge_sort(b);
    return merge(a, b);
}

int main(void) {
    char filename[] = "input.txt";

    FILE* fh = fopen(filename, "r");

    Node *head = NULL, *tail = NULL;
    int x;
    while (fscanf(fh, "%d", &x) == 1) {
        if (!head) {
            head = tail = append(NULL, x);
        } else {
            tail = append(tail, x);
        }
    }

    head = merge_sort(head);

    Node* cur = head;
    int first = 1;

    char filename2[] = "output.txt";

    FILE* fh2 = fopen(filename2, "w");

    while (cur) {
        if (!first) {
            fprintf(fh2, " ");
        }
        fprintf(fh2, "%d", cur->key);
        first = 0;
        cur = cur->next;
    }
    fprintf(fh2, "\n");

    while (head) {
        Node* tmp = head;
        head = head->next;
        free(tmp);
    }

    fclose(fh);
    fclose(fh2);
    return 0;
}