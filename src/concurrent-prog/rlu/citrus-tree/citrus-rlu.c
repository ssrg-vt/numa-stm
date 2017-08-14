#include "citrus-rlu.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct node_t {
        int key;
        struct node_t *child[2] __attribute__((aligned(16)));
        pthread_mutex_t lock;
        bool marked;
        int tag[2];
        int value;
};

int rlu_tree_contains(rlu_thread_data_t *self, node root, int key)
{
    RLU_READER_LOCK(self);
    node curr = root->child[0];
    int ckey = curr->key;
    while (curr != NULL && ckey != key) {
        if (ckey > key)
            curr = curr->child[0];
        if (ckey < key)
            curr = curr->child[1];
        if (curr != NULL)
            ckey = curr->key;
    }
    RLU_READER_UNLOCK(self);
    if (curr == NULL)
        return -1;
    return curr->value;
}

inline struct node_t *rlu_new_node(void)
{
    node new_node = (struct node_t *)RLU_ALLOC(sizeof(struct node_t));
    if (new_node == NULL) {
        printf("out of memory\n");
        exit(1);
    }
    return new_node;
}

bool rlu_tree_add(rlu_thread_data_t *self, node root, int key, int value)
{
    restart:
        RLU_READER_LOCK(self);
        node prev = (struct node_t *)RLU_DEREF(self, (root));
        node curr = (struct node_t *)RLU_DEREF(self, (root->child[0]));

        int direction = 0;
        int ckey = curr->key;

        while (curr != NULL && ckey != key) {
            prev = curr;
            if (ckey > key) {
                curr = curr->child[0];
                direction = 0;
            }
            if (ckey < key) {
                curr = curr->child[1];
                direction = 1;
            }
            if (curr != NULL)
                ckey = curr->key;
        }

        if (!RLU_TRY_LOCK(self, &prev)) {
            RLU_ABORT(self);
            goto restart;
        }

        if (curr == NULL) {
            node new_node = rlu_new_node();
            new_node->key = key;
            new_node->value =value;
            new_node->child[0] = NULL;
            new_node->child[1] = NULL;
            RLU_ASSIGN_PTR(self, &(prev->child[direction]), new_node);
        }

        RLU_READER_UNLOCK(self);
        return true;
}


bool rlu_tree_remove(rlu_thread_data_t *self, node root, int key)
{
    restart:
        RLU_READER_LOCK(self);
        node prev = (struct node_t *)RLU_DEREF(self, (root));
        node curr = (struct node_t *)RLU_DEREF(self, (prev->child[0]));
        int direction = 0;
        int ckey = curr->key;
        (void)(direction);
        while (curr != NULL && ckey != key) {
            prev = curr;
            if (ckey > key) {
                curr = curr->child[0];
                direction = 0;
            }
            if (ckey < key) {
                curr = curr->child[1];
                direction = 1;
            }
            if (curr != NULL)
                ckey = curr->key;
        }
        if (curr == NULL) {
            RLU_READER_UNLOCK(self);
            return false;
        }
        node parent = curr;
        node succ = (struct node_t *)RLU_DEREF(self, (curr->child[1]));
        node next = (struct node_t *)RLU_DEREF(self, (succ->child[0]));
        while (next != NULL) {
            parent = succ;
            succ = next;
            next = (struct node_t *)RLU_DEREF(self, (next->child[0]));
        }
        if (parent == NULL) {
            /* RLU_LOCK(self, &succ); */
            if (!RLU_TRY_LOCK(self, &succ)) {
                RLU_ABORT(self);
                goto restart;
            }
            RLU_ASSIGN_PTR(self, &(succ->child[0]), curr->child[0]);
        } else {
            /* RLU_LOCK(self, &parent); */
            if (!RLU_TRY_LOCK(self, &parent)) {
                RLU_ABORT(self);
                goto restart;
            }
            RLU_ASSIGN_PTR(self, &(parent->child[0]), succ->child[1]);
            /* RLU_LOCK(self, &succ); */
            if (!RLU_TRY_LOCK(self, &succ)) {
                RLU_ABORT(self);
                goto restart;
            }
            RLU_ASSIGN_PTR(self, &(succ->child[0]), curr->child[0]);
            RLU_ASSIGN_PTR(self, &(succ->child[1]), curr->child[1]);
        }
        /* RLU_LOCK(self, &prev); */
        if (!RLU_TRY_LOCK(self, &prev)) {
            RLU_ABORT(self);
            goto restart;
        }
        RLU_ASSIGN_PTR(self, &(prev->child[direction]), succ);

        RLU_FREE(self, curr);

        RLU_READER_UNLOCK(self);
        return true;
}
