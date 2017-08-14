#ifndef __KLIST_H
#define __KLIST_H

/*
 * Simple doubly linked klist implementation.
 */

struct klist_head {
	struct klist_head *next, *prev;
};

#define KLIST_HEAD_INIT(name) { &(name), &(name) }

#define KLIST_HEAD(name) \
	struct klist_head name = KLIST_HEAD_INIT(name)

#define INIT_KLIST_HEAD(ptr) do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

/*
 * Insert a new entry between two known consecutive entries. 
 *
 * This is only for internal klist manipulation where we know
 * the prev/next entries already!
 */
static inline void __klist_add(struct klist_head *new,
			      struct klist_head *prev,
			      struct klist_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/**
 * klist_add - add a new entry
 * @new: new entry to be added
 * @head: klist head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void klist_add(struct klist_head *new, struct klist_head *head)
{
	__klist_add(new, head, head->next);
}

/**
 * klist_add_tail - add a new entry
 * @new: new entry to be added
 * @head: klist head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void klist_add_tail(struct klist_head *new, struct klist_head *head)
{
	__klist_add(new, head->prev, head);
}

/*
 * Delete a klist entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal klist manipulation where we know
 * the prev/next entries already!
 */
static inline void __klist_del(struct klist_head *prev, struct klist_head *next)
{
	next->prev = prev;
	prev->next = next;
}

/**
 * klist_del - deletes entry from klist.
 * @entry: the element to delete from the klist.
 * Note: klist_empty on entry does not return true after this, the entry is in an undefined state.
 */
static inline void klist_del(struct klist_head *entry)
{
	__klist_del(entry->prev, entry->next);
	entry->next = (void *) 0;
	entry->prev = (void *) 0;
}

/**
 * klist_del_init - deletes entry from klist and reinitialize it.
 * @entry: the element to delete from the klist.
 */
static inline void klist_del_init(struct klist_head *entry)
{
	__klist_del(entry->prev, entry->next);
	INIT_KLIST_HEAD(entry);
}

/**
 * klist_move - delete from one klist and add as another's head
 * @klist: the entry to move
 * @head: the head that will precede our entry
 */
static inline void klist_move(struct klist_head *klist, struct klist_head *head)
{
        __klist_del(klist->prev, klist->next);
        klist_add(klist, head);
}

/**
 * klist_move_tail - delete from one klist and add as another's tail
 * @klist: the entry to move
 * @head: the head that will follow our entry
 */
static inline void klist_move_tail(struct klist_head *klist,
				  struct klist_head *head)
{
        __klist_del(klist->prev, klist->next);
        klist_add_tail(klist, head);
}

/**
 * klist_empty - tests whether a klist is empty
 * @head: the klist to test.
 */
static inline int klist_empty(struct klist_head *head)
{
	return head->next == head;
}

/**
 * klist_entry - get the struct for this entry
 * @ptr:	the &struct klist_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the klist_struct within the struct.
 */
#define klist_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
 * klist_for_each	-	iterate over a klist
 * @pos:	the &struct klist_head to use as a loop counter.
 * @head:	the head for your klist.
 */
#define klist_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); \
        	pos = pos->next)
/**
 * klist_for_each_prev	-	iterate over a klist backwards
 * @pos:	the &struct klist_head to use as a loop counter.
 * @head:	the head for your klist.
 */
#define klist_for_each_prev(pos, head) \
	for (pos = (head)->prev; pos != (head); \
        	pos = pos->prev)

/**
 * klist_for_each_safe	-	iterate over a klist safe against removal of klist entry
 * @pos:	the &struct klist_head to use as a loop counter.
 * @n:		another &struct klist_head to use as temporary storage
 * @head:	the head for your klist.
 */
#define klist_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/**
 * klist_for_each_entry	-	iterate over klist of given type
 * @pos:	the type * to use as a loop counter.
 * @head:	the head for your klist.
 * @member:	the name of the klist_struct within the struct.
 */
#define klist_for_each_entry(pos, head, member)				\
	for (pos = klist_entry((head)->next, typeof(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = klist_entry(pos->member.next, typeof(*pos), member))

/**
 * klist_for_each_entry_safe - iterate over klist of given type safe against removal of klist entry
 * @pos:	the type * to use as a loop counter.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your klist.
 * @member:	the name of the klist_struct within the struct.
 */
#define klist_for_each_entry_safe(pos, n, head, member)			\
	for (pos = klist_entry((head)->next, typeof(*pos), member),	\
		n = klist_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = n, n = klist_entry(n->member.next, typeof(*n), member))

#endif
