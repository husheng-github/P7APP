#ifndef __QUEUE_H__
#define __QUEUE_H__

#ifdef __cplusplus
extern "C"{
#endif

typedef unsigned int Item;

typedef struct node
{
    Item data;
    struct node* next;
}Node,*PNode;

typedef struct 
{
    PNode front;
    PNode rear;
    int size;
}Queue;

typedef struct noden
{
    void* data;
    struct noden* next;
}node_t,*pnode_t;

typedef struct queue
{
    pnode_t front;
    pnode_t rear;
    int size;
}queue_t;


/****************************************************************************
 * \|/front
 *  +----------------+
 *  | data  | next   |
 *  +----------------+
 * /|\ rear(1)       |
 *                  \|/
 *                   +----------------+
 *                   |  data  | next  |
 *                   +----------------+
 *                  /|\ rear(2)       |
 *                                   \|/
 *                                    +------------------+
 *                                    |  data |  next    |
 *                                    +------------------+
 *                                   /|\ rear(3)
 ****************************************************************************/


/* building a empty queue */
Queue* trendit_intqueue_create(void);

/* destroy a build queue */
void trendit_intqueue_destroy(Queue* pqueue);

/* clear a queue */
void trendit_intqueue_clear(Queue* pqueue);

/* check queue empty or not*/
int trendit_intqueue_is_empty(Queue* pqueue);

/* get queue size */
int trendit_intqueue_getsize(Queue* pqueue);

/* get the front node */
PNode trendit_intqueue_getfront(Queue* pqueue,Item *pitem);

/* get the rear node */
PNode trendit_intqueue_getrear(Queue* pqueue,Item *pitem);

/* insert a new node to queue */
PNode trendit_intqueue_enqueue(Queue* pqueue,Item item);

s32 trendit_intqueue_delnode(Queue *pqueue, Item pitem);

/* remove a node from queue */
PNode trendit_intqueue_delqueue(Queue* pqueue,Item *pitem);

/* iterate queue and call visit with the node */
void trendit_intqueue_queue_traverse(Queue* pqueue,void (*visit)(PNode));


/* building a empty queue */
queue_t* trendit_queue_create(void);

/* destroy a build queue */
void trendit_queue_destroy(queue_t* pqueue);

/* clear a queue */
void trendit_queue_clear(queue_t* pqueue);

/* check queue empty or not*/
int trendit_queue_is_empty(queue_t* pqueue);

/* get queue size */
int trendit_queue_get_size(queue_t* pqueue);

/* get the front node */
pnode_t trendit_queue_get_front(queue_t* pqueue, void **pitem);

/* get the rear node */
pnode_t trendit_queue_get_rear(queue_t* pqueue, void* *pitem);

/* insert a new node to queue */
pnode_t trendit_queue_entry(queue_t* pqueue, void* item);

/* remove a node from queue */
pnode_t trendit_queue_delete(queue_t* pqueue, void* *pitem);

/* iterate queue and call visit with the node */
void trendit_queue_traverse(queue_t* pqueue,void (*visit)(pnode_t));


#ifdef __cplusplus
}
#endif

#endif

