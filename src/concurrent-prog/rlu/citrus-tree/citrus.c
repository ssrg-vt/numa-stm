/**
 * Copyright 2015
 * Maya Arbel (mayaarl [at] cs [dot] technion [dot] ac [dot] il).
 * Adam Morrison (mad [at] cs [dot] technion [dot] ac [dot] il).
 *
 * This file is part of Predicate RCU.
 *
 * Predicate RCU is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors Maya Arbel and Adam Morrison
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include "citrus.h"
#include "prcu.h"

struct node_t {
        int key;
        struct node_t *child[2] __attribute__((aligned(16)));
        pthread_mutex_t lock;
        bool marked;
        int tag[2];
        int value;
};

static int max_key_in_range;
int pred_hash(hash_info info, int key);

struct predicate_info_t {
        int min_key;
        int max_key;
        int max;
};

bool
pred(predicate_info info, int value)
{
        return (info->min_key < value && info->max_key >= value);
}

int
pred_next(predicate_info info, int curr_bucket)
{
        int max_bucket = pred_hash(NULL, info->max_key);
        if (curr_bucket < max_bucket) {
                return curr_bucket + 1;
        } else {
                return -1;
        }
}

int
pred_hash(hash_info info, int key)
{
        int num_buckets = prcu_get_size();
        assert(num_buckets > 0);
        if (num_buckets > max_key_in_range) {
                return key;		//put each value in it's own bucket
        }
        int result;
        int num_elemets_per_bucket = max_key_in_range / num_buckets;
        assert(num_elemets_per_bucket > 0);
        int overflow = max_key_in_range - (num_buckets * num_elemets_per_bucket);
        if (overflow == 0) {
                result = (key / num_elemets_per_bucket);
        } else {
                //The first |overflow| buckets should have an extra key
                int threshold = overflow * (num_elemets_per_bucket + 1);
                if (key < threshold) {
                        result = (key / (num_elemets_per_bucket + 1));
                } else {
                        result = overflow + ((key - (threshold)) / num_elemets_per_bucket);
                }
        }
        assert(result < num_buckets);
        return result;
}

static node
newNode(int key, int value)
{
        node new = (node) malloc(sizeof(struct node_t));
        if (new == NULL) {
                printf("out of memory\n");
                exit(1);
        }
        new->key = key;
        new->marked = false;
        new->child[0] = NULL;
        new->child[1] = NULL;
        new->tag[0] = 0;
        new->tag[1] = 0;
        new->value = value;
        if (pthread_mutex_init(&(new->lock), NULL) != 0) {
                printf("\n mutex init failed\n");
        }
        return new;
}

node
init(int max_key)
{
        //finish initializing RCU
        if (max_key < 0) {
                max_key_in_range = infinity;
        } else {
                max_key_in_range = max_key + 1;
        }
        prcu_set_hash(pred_hash, NULL);
        node root = newNode(infinity, 0);
        root->child[0] = newNode(infinity, 0);
        return root;
}

int
contains(node root, int key)
{
        prcu_enter(key);
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
        prcu_exit(key);
        if (curr == NULL)
                return -1;
        return curr->value;
}

bool
validate(node prev, int tag, node curr, int direction)
{
        bool result;
        if (curr == NULL) {
                result = (!(prev->marked) && (prev->child[direction] == curr)
                          && (prev->tag[direction] == tag));
        } else {
                result = (!(prev->marked) && !(curr->marked)
                          && prev->child[direction] == curr);
        }
        return result;
}

bool
insert(node root, int key, int value)
{
        while (true) {
                prcu_enter(key);
                node prev = root;
                node curr = root->child[0];
                int direction = 0;
                int ckey = curr->key;
                int tag;
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
                tag = prev->tag[direction];
                prcu_exit(key);
                if (curr != NULL)
                        return false;
                pthread_mutex_lock(&(prev->lock));
                if (validate(prev, tag, curr, direction)) {
                        node new = newNode(key, value);
                        prev->child[direction] = new;

                        pthread_mutex_unlock(&(prev->lock));
                        return true;
                }
                pthread_mutex_unlock(&(prev->lock));
        }
}

bool
delete(node root, int key)
{
        while (true) {
                prcu_enter(key);
                node prev = root;
                node curr = root->child[0];
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
                if (curr == NULL) {
                        prcu_exit(key);
                        return false;
                }

                prcu_exit(key);
                pthread_mutex_lock(&(prev->lock));
                pthread_mutex_lock(&(curr->lock));
                if (!validate(prev, 0, curr, direction)) {
                        pthread_mutex_unlock(&(prev->lock));
                        pthread_mutex_unlock(&(curr->lock));
                        continue;
                }
                if (curr->child[0] == NULL) {
                        curr->marked = true;
                        prev->child[direction] = curr->child[1];
                        if (prev->child[direction] == NULL) {
                                prev->tag[direction]++;
                        }
                        pthread_mutex_unlock(&(prev->lock));
                        pthread_mutex_unlock(&(curr->lock));
                        return true;
                }
                if (curr->child[1] == NULL) {
                        curr->marked = true;
                        prev->child[direction] = curr->child[0];
                        if (prev->child[direction] == NULL) {
                                prev->tag[direction]++;
                        }
                        pthread_mutex_unlock(&(prev->lock));
                        pthread_mutex_unlock(&(curr->lock));
                        return true;
                }
                node prevSucc = curr;
                node succ = curr->child[1];
                node next = succ->child[0];
                while (next != NULL) {
                        prevSucc = succ;
                        succ = next;
                        next = next->child[0];
                }
                int succDirection = 1;
                if (prevSucc != curr) {
                        pthread_mutex_lock(&(prevSucc->lock));
                        succDirection = 0;
                }
                pthread_mutex_lock(&(succ->lock));
                if (validate(prevSucc, 0, succ, succDirection)
                                && validate(succ, succ->tag[0], NULL, 0)) {
                        curr->marked = true;
                        node new = newNode(succ->key, succ->value);
                        new->child[0] = curr->child[0];
                        new->child[1] = curr->child[1];
                        pthread_mutex_lock(&(new->lock));
                        prev->child[direction] = new;
                        struct predicate_info_t pred_info = {.min_key = key,.max_key =
                                                        succ->key,.max = max_key_in_range
                                                                 };
                        int min_bucket = pred_hash(NULL, key);
                        int max_bucket = pred_hash(NULL, succ->key);
                        assert(min_bucket <= max_bucket);
                        (void)max_bucket;
                        prcu_wait_for_readers(pred, min_bucket, pred_next, &pred_info);

                        succ->marked = true;
                        if (prevSucc == curr) {
                                new->child[1] = succ->child[1];
                                if (new->child[1] == NULL) {
                                        new->tag[1]++;
                                }
                        } else {
                                prevSucc->child[0] = succ->child[1];
                                if (prevSucc->child[0] == NULL) {
                                        prevSucc->tag[0]++;
                                }
                        }
                        pthread_mutex_unlock(&(prev->lock));
                        pthread_mutex_unlock(&(new->lock));
                        pthread_mutex_unlock(&(curr->lock));
                        if (prevSucc != curr)
                                pthread_mutex_unlock(&(prevSucc->lock));
                        pthread_mutex_unlock(&(succ->lock));
                        return true;
                }
                pthread_mutex_unlock(&(prev->lock));
                pthread_mutex_unlock(&(curr->lock));
                if (prevSucc != curr)
                        pthread_mutex_unlock(&(prevSucc->lock));
                pthread_mutex_unlock(&(succ->lock));
        }
}
