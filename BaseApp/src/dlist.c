#include "dlist.h"

/*
* Insert a new entry between two known consecutive entries.
*
* This is only for internal list manipulation where we know
* the prev/next entries already!
*/

//将节点new插入到prev和next所指的节点之间
     void __list_add(struct list_head *new,
				struct list_head *prev,
				struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/**
* list_add – add a new entry
* @new: new entry to be added
* @head: list head to add it after
*
* Insert a new entry after the specified head.
* This is good for implementing stacks.
*/

//内核链表的头插入
     void list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}

/**
* list_add_tail – add a new entry
* @new: new entry to be added
* @head: list head to add it before
*
* Insert a new entry before the specified head.
* This is useful for implementing queues.
*/
//内核链表的尾插入
     void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
}

/*
* Delete a list entry by making the prev/next entries
* point to each other.
*
* This is only for internal list manipulation where we know
* the prev/next entries already!
*/
//删除next和prev之间的节点
     void __list_del(struct list_head *prev, struct list_head *next)
{
	next->prev = prev;
	prev->next = next;
}

/**
* list_del – deletes entry from list.
* @entry: the element to delete from the list.
* Note: list_empty on entry does not return true after this, the entry is in an undefined state.
*/
//删除entry指向的节点，删除之后对指针域进行设NULL，linux内核不作free操作，由用户来执行free
     void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = (void *) 0;
	entry->prev = (void *) 0;
}

/**
* list_del_init – deletes entry from list and reinitialize it.
* @entry: the element to delete from the list.
*/
//删除节点之后，让其指针域分别指向自己
     void list_del_init(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry);
}

/**
* list_move – delete from one list and add as another’s head
* @list: the entry to move
* @head: the head that will precede our entry
*/

//链表节点的移动：将list指向的节点删除后插入到另一个链表头结点的后面（头插入）
     void list_move(struct list_head *list,
				struct list_head *head)
{
	__list_del(list->prev, list->next);
	list_add(list, head);
}

/**
* list_move_tail – delete from one list and add as another’s tail
* @list: the entry to move
* @head: the head that will follow our entry
*/
//链表节点的移动：将list指向的节点删除后插入到另一个链表头结点的前面（尾插入）
     void list_move_tail(struct list_head *list,
					struct list_head *head)
{
	__list_del(list->prev, list->next);
	list_add_tail(list, head);
}

/**
* list_empty – tests whether a list is empty
* @head: the list to test.
*/
//判断一个链表是否为空
     int list_empty(struct list_head *head)
{
	return head->next == head;
}

//实现分别以list和head为头结点的两个链表的合并，并将list头结点去除
     void __list_splice(struct list_head *list,
					struct list_head *head)
{
	struct list_head *first = list->next;
	struct list_head *last = list->prev;
	struct list_head *at = head->next;

	first->prev = head;
	head->next = first;

	last->next = at;
	at->prev = last;
}

/**
* list_splice – join two lists
* @list: the new list to add.
* @head: the place to add it in the first list.
*/
//如果list指向的链表不为空，则合并
void list_splice(struct list_head *list, struct list_head *head)
{
if (!list_empty(list))
__list_splice(list, head);
}

/**
* list_splice_init – join two lists and reinitialise the emptied list.
* @list: the new list to add.
* @head: the place to add it in the first list.
*
* The list at @list is reinitialised
*/
//将两个链表合并之后，将除去的头结点的指针域自己指向自己
void list_splice_init(struct list_head *list,
struct list_head *head)
{
if (!list_empty(list)) {
__list_splice(list, head);
INIT_LIST_HEAD(list);
}
}
