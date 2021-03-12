#ifndef __DLIST_H
#define __DLIST_H

/* This file is from Linux Kernel (include/linux/list.h)
* and modified by simply removing hardware prefetching of list items.
* Here by copyright, credits attributed to wherever they belong.
* Kulesh Shanmugasundaram (kulesh [squiggly] isis.poly.edu)
*/

/*
* Simple doubly linked list implementation.
*
* Some of the internal functions (“__xxx”) are useful when
* manipulating whole lists rather than single entries, as
* sometimes we already know the next/prev entries and we can
* generate better code by using them directly rather than
* using the generic single-entry routines.
*/
/**
 * container_of - cast a member of a structure out to the containing structure
 *
 * @ptr:    the pointer to the member.
 * @type:    the type of the container struct this is embedded in.
 * @member:    the name of the member within the struct.
 *
 */
 #if 0
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({            \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

#endif
/*
 * These are non-NULL pointers that will result in page faults
 * under normal circumstances, used to verify that nobody uses
 * non-initialized list entries.
 */
#define LIST_POISON1  ((void *) 0x00100100)
#define LIST_POISON2  ((void *) 0x00200100)

//小结构体声明
struct list_head {
    struct list_head *next, *prev;
};

//以下两个宏函数，完成对小结构体变量的初始化，使其自己指向自己，用来实现头结点初始化
#define LIST_HEAD_INIT(name) { &(name), &(name) }


#define LIST_HEAD(name) \
struct list_head name = LIST_HEAD_INIT(name)

//ptr为小结构体地址
#define INIT_LIST_HEAD(ptr) do { \
    (ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

/*
* Insert a new entry between two known consecutive entries.
*
* This is only for internal list manipulation where we know
* the prev/next entries already!
*/

/**
* list_entry – get the struct for this entry
* @ptr:    the &struct list_head pointer.
* @type:    the type of the struct this is embedded in.
* @member:    the name of the list_struct within the struct.
*/
//使用小结构体的实际地址减去小结构体相对于大结构体的偏移量来求出大结构的实际地址
//type是大结构体类型，ptr为小结构体实际地址，member为小结构体变量
//将大结构体放在0地址处，那么小结构体的地址就是相对于大结构体的偏移量
#define list_entry(ptr, type, member) ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))


/**
* list_for_each    -    iterate over a list
* @pos:    the &struct list_head to use as a loop counter.
* @head:    the head for your list.
*/
//对小结构体组成的链表的遍历（从前到后），该宏函数只是一个for语句，需要用户添加循环体
#define list_for_each(pos, head)\
for (pos = (head)->next; pos != (head);pos = pos->next)
/**
* list_for_each_prev    -    iterate over a list backwards
* @pos:    the &struct list_head to use as a loop counter.
* @head:    the head for your list.
*/
//对小结构体组成的链表的遍历（从后到前），该宏函数只是一个for语句，需要用户添加循环体
#define list_for_each_prev(pos, head) \
for (pos = (head)->prev; pos != (head); \
pos = pos->prev)

/**
* list_for_each_safe    -    iterate over a list safe against removal of list entry
* @pos:    the &struct list_head to use as a loop counter.
* @n:        another &struct list_head to use as temporary storage
* @head:    the head for your list.
*/
//对小结构链表的遍历，当访问当前节点时，记住下一个节点的地址，避免断链。
#define list_for_each_safe(pos, n, head) \
for (pos = (head)->next, n = pos->next; pos != (head); \
pos = n, n = pos->next)

/**
* list_for_each_entry    -    iterate over list of given type
* @pos:    the type * to use as a loop counter.
* @head:    the head for your list.
* @member:    the name of the list_struct within the struct.
*/
//对大结构体的遍历，pos是大结构体类型的地址，member是小结构体的变量，
//typeof用来获取参数的类型
// int a; =>  typeof(a) b;
 #define list_for_each_entry(pos, head, member)                \
for (pos = list_entry((head)->next, typeof(*pos), member);    \
&pos->member != (head);                     \
pos = list_entry(pos->member.next, typeof(*pos), member))


/**
 * list_for_each_entry_reverse - iterate backwards over list of given type.
 * @pos:    the type * to use as a loop cursor.
 * @head:    the head for your list.
 * @member:    the name of the list_struct within the struct.
 */
#define list_for_each_entry_reverse(pos, head, member)            \
    for (pos = list_entry((head)->prev, typeof(*pos), member);    \
         &pos->member != (head);     \
         pos = list_entry(pos->member.prev, typeof(*pos), member))

/**
 * list_for_each_entry_continue - continue iteration over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:    the head for your list.
 * @member:    the name of the list_struct within the struct.
 *
 * Continue to iterate over list of given type, continuing after
 * the current position.
 */
#define list_for_each_entry_continue(pos, head, member)         \
    for (pos = list_entry(pos->member.next, typeof(*pos), member);    \
         &pos->member != (head);    \
         pos = list_entry(pos->member.next, typeof(*pos), member))



/**
 * list_for_each_entry_continue_reverse - iterate backwards from the given point
 * @pos:    the type * to use as a loop cursor.
 * @head:    the head for your list.
 * @member:    the name of the list_struct within the struct.
 *
 * Start to iterate over list of given type backwards, continuing after
 * the current position.
 */
#define list_for_each_entry_continue_reverse(pos, head, member)        \
    for (pos = list_entry(pos->member.prev, typeof(*pos), member);    \
         &pos->member != (head);    \
         pos = list_entry(pos->member.prev, typeof(*pos), member))

/**
* list_for_each_entry_safe – iterate over list of given type safe against removal of list entry
* @pos:    the type * to use as a loop counter.
* @n:        another type * to use as temporary storage
* @head:    the head for your list.
* @member:    the name of the list_struct within the struct.
*/

#define list_for_each_entry_safe(pos, n, head, member)            \
for (pos = list_entry((head)->next, typeof(*pos), member),    \
n = list_entry(pos->member.next, typeof(*pos), member);    \
&pos->member != (head);                     \
pos = n, n = list_entry(n->member.next, typeof(*n), member))



#endif

