#ifndef __CRLU__
#define __CRLU__

#include <stdbool.h>
#include "rlu.h"
#define infinity 2147483647

typedef struct node_t *node;
int rlu_tree_contains(rlu_thread_data_t *self, node root, int key);
bool rlu_tree_add(rlu_thread_data_t *self, node root, int key, int value);
bool rlu_tree_remove(rlu_thread_data_t *self, node root, int key);

#endif /* __CRLU__ */
