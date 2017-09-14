/**
 *  Copyright (C) 2011
 *  University of Rochester Department of Computer Science
 *    and
 *  Lehigh University Department of Computer Science and Engineering
 *
 * License: Modified BSD
 *          Please see the file LICENSE.RSTM for licensing information
 */

#ifndef LIST_HPP__
#define LIST_HPP__

#include "../tm/rand_r_32.h"
#include "../tm/tm_thread.hpp"

// We construct other data structures from the List. In order to do their
// sanity checks correctly, we might need to pass in a validation function of
// this type
typedef bool (*verifier)(uint32_t, uint32_t, uint32_t);

// Set of LLNodes represented as a linked list in sorted order
class List
{

  // Node in a List
  struct Node
  {
      int m_val;
      Node* m_next;

      // ctors
      Node(int val = -1) : m_val(val), m_next() { }

      Node(int val, Node* next) : m_val(val), m_next(next) { }
  };

  public:

    Node* sentinel;
    int numa_zone;

    List(int numa_zone);

    // true iff val is in the data structure
    bool lookup(int val TM_ARG) const;

    // standard IntSet methods
    void insert(int val TM_ARG);
    void insert_NoTM(int val);

    // remove a node if its value = val
    void remove(int val TM_ARG);

    // make sure the list is in sorted order
    bool isSane() const;

    // make sure the list is in sorted order and for each node x,
    // v(x, verifier_param) is true
    bool extendedSanityCheck(verifier v, uint32_t param1, uint32_t param2) const;

    // find max and min
    int findmax(TM_ARG_ALONE) const;

    int findmin(TM_ARG_ALONE) const;

    // overwrite all elements up to val
    void overwrite(int val TM_ARG);
};


// constructor just makes a sentinel for the data structure
List::List(int numa_zonep) : sentinel(new Node()), numa_zone(numa_zonep) { }

// simple sanity check: make sure all elements of the list are in sorted order
bool List::isSane(void) const
{
    const Node* prev(sentinel);
    const Node* curr((prev->m_next));

    while (curr != NULL) {
        if ((prev->m_val) >= (curr->m_val))
            return false;
        prev = curr;
        curr = curr->m_next;
    }
    return true;
}

// extended sanity check, does the same as the above method, but also calls v()
// on every item in the list
bool List::extendedSanityCheck(verifier v, uint32_t v_param1, uint32_t v_param2) const
{
    const Node* prev(sentinel);
    const Node* curr((prev->m_next));
    while (curr != NULL) {
        if (!v((curr->m_val), v_param1, v_param2) || ((prev->m_val) >= (curr->m_val)))
            return false;
        prev = curr;
        curr = prev->m_next;
    }
    return true;
}

// insert method; find the right place in the list, add val so that it is in
// sorted order; if val is already in the list, exit without inserting
void List::insert(int val TM_ARG)
{
    // traverse the list to find the insertion point
     Node* prev(sentinel);
     Node* curr(TM_READ_Z(prev->m_next, numa_zone));

    while (curr != NULL) {
        if (TM_READ_Z(curr->m_val, numa_zone) >= val)
            break;
        prev = curr;
        curr = TM_READ_Z(prev->m_next, numa_zone);
    }

    // now insert new_node between prev and curr
    if (!curr || (TM_READ_Z(curr->m_val, numa_zone) > val)) {
        Node* insert_point = const_cast<Node*>(prev);

        // create the new node
        Node* i = (Node*)TM_ALLOC(sizeof(Node), numa_zone);
        i->m_val = val;
        i->m_next = const_cast<Node*>(curr);
        TM_WRITE_Z(insert_point->m_next, i, numa_zone);
    }
}

void List::insert_NoTM(int val) {
    // traverse the list to find the insertion point
     Node* prev(sentinel);
     Node* curr(prev->m_next);

    while (curr != NULL) {
        if (curr->m_val >= val)
            break;
        prev = curr;
        curr = prev->m_next;
    }

    // now insert new_node between prev and curr
    if (!curr || (curr->m_val > val)) {
        Node* insert_point = const_cast<Node*>(prev);

        // create the new node
        Node* i = (Node*)TM_ALLOC(sizeof(Node), numa_zone);
        i->m_val = val;
        i->m_next = const_cast<Node*>(curr);
        insert_point->m_next = i;
    }
}

// search function
bool List::lookup(int val TM_ARG) const
{
    bool found = false;
     Node* curr(sentinel);
    curr = TM_READ_Z(curr->m_next, numa_zone);

    while (curr != NULL) {
        if (TM_READ_Z(curr->m_val, numa_zone) >= val)
            break;
        curr = TM_READ_Z(curr->m_next, numa_zone);
    }

    found = ((curr != NULL) && (TM_READ_Z(curr->m_val, numa_zone) == val));
    return found;
}

// findmax function
int List::findmax(TM_ARG_ALONE) const
{
    int max = -1;
     Node* curr(sentinel);
    while (curr != NULL) {
        max = TM_READ_Z(curr->m_val, numa_zone);
        curr = TM_READ_Z(curr->m_next, numa_zone);
    }
    return max;
}

// findmin function
int List::findmin(TM_ARG_ALONE) const
{
    int min = -1;
     Node* curr(sentinel);
    curr = TM_READ_Z(curr->m_next, numa_zone);
    if (curr != NULL)
        min = TM_READ_Z(curr->m_val, numa_zone);
    return min;
}

// remove a node if its value == val
void List::remove(int val TM_ARG)
{
    // find the node whose val matches the request
     Node* prev(sentinel);
     Node* curr(TM_READ_Z(prev->m_next, numa_zone));
    while (curr != NULL) {
        // if we find the node, disconnect it and end the search
        if (TM_READ_Z(curr->m_val, numa_zone) == val) {
            Node* mod_point = const_cast<Node*>(prev);
            TM_WRITE_Z(mod_point->m_next, TM_READ_Z(curr->m_next, numa_zone), numa_zone);

            // delete curr...
            TM_FREE(const_cast<Node*>(curr));
            break;
        }
        else if (TM_READ_Z(curr->m_val, numa_zone) > val) {
            // this means the search failed
            break;
        }
        prev = curr;
        curr = TM_READ_Z(prev->m_next, numa_zone);
    }
}

// search function
void List::overwrite(int val TM_ARG)
{
     Node* curr(sentinel);
    curr = TM_READ_Z(curr->m_next, numa_zone);

    while (curr != NULL) {
        if (TM_READ_Z(curr->m_val, numa_zone) >= val)
            break;
        Node* wcurr = const_cast<Node*>(curr);
        TM_WRITE_Z(wcurr->m_val, TM_READ_Z(wcurr->m_val, numa_zone), numa_zone);
        curr = TM_READ_Z(wcurr->m_next, numa_zone);
    }
}

#endif // LIST_HPP__
