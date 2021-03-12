#include "dlist.h"

/*
* Insert a new entry between two known consecutive entries.
*
* This is only for internal list manipulation where we know
* the prev/next entries already!
*/

//���ڵ�new���뵽prev��next��ָ�Ľڵ�֮��
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
* list_add �C add a new entry
* @new: new entry to be added
* @head: list head to add it after
*
* Insert a new entry after the specified head.
* This is good for implementing stacks.
*/

//�ں������ͷ����
     void list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}

/**
* list_add_tail �C add a new entry
* @new: new entry to be added
* @head: list head to add it before
*
* Insert a new entry before the specified head.
* This is useful for implementing queues.
*/
//�ں������β����
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
//ɾ��next��prev֮��Ľڵ�
     void __list_del(struct list_head *prev, struct list_head *next)
{
	next->prev = prev;
	prev->next = next;
}

/**
* list_del �C deletes entry from list.
* @entry: the element to delete from the list.
* Note: list_empty on entry does not return true after this, the entry is in an undefined state.
*/
//ɾ��entryָ��Ľڵ㣬ɾ��֮���ָ���������NULL��linux�ں˲���free���������û���ִ��free
     void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = (void *) 0;
	entry->prev = (void *) 0;
}

/**
* list_del_init �C deletes entry from list and reinitialize it.
* @entry: the element to delete from the list.
*/
//ɾ���ڵ�֮������ָ����ֱ�ָ���Լ�
     void list_del_init(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry);
}

/**
* list_move �C delete from one list and add as another��s head
* @list: the entry to move
* @head: the head that will precede our entry
*/

//����ڵ���ƶ�����listָ��Ľڵ�ɾ������뵽��һ������ͷ���ĺ��棨ͷ���룩
     void list_move(struct list_head *list,
				struct list_head *head)
{
	__list_del(list->prev, list->next);
	list_add(list, head);
}

/**
* list_move_tail �C delete from one list and add as another��s tail
* @list: the entry to move
* @head: the head that will follow our entry
*/
//����ڵ���ƶ�����listָ��Ľڵ�ɾ������뵽��һ������ͷ����ǰ�棨β���룩
     void list_move_tail(struct list_head *list,
					struct list_head *head)
{
	__list_del(list->prev, list->next);
	list_add_tail(list, head);
}

/**
* list_empty �C tests whether a list is empty
* @head: the list to test.
*/
//�ж�һ�������Ƿ�Ϊ��
     int list_empty(struct list_head *head)
{
	return head->next == head;
}

//ʵ�ֱַ���list��headΪͷ������������ĺϲ�������listͷ���ȥ��
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
* list_splice �C join two lists
* @list: the new list to add.
* @head: the place to add it in the first list.
*/
//���listָ�������Ϊ�գ���ϲ�
void list_splice(struct list_head *list, struct list_head *head)
{
if (!list_empty(list))
__list_splice(list, head);
}

/**
* list_splice_init �C join two lists and reinitialise the emptied list.
* @list: the new list to add.
* @head: the place to add it in the first list.
*
* The list at @list is reinitialised
*/
//����������ϲ�֮�󣬽���ȥ��ͷ����ָ�����Լ�ָ���Լ�
void list_splice_init(struct list_head *list,
struct list_head *head)
{
if (!list_empty(list)) {
__list_splice(list, head);
INIT_LIST_HEAD(list);
}
}
