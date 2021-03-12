#include "ddiglobal.h"
#include "app_global.h"


Queue *trendit_intqueue_create(void)
{
    Queue *pqueue = (Queue *)k_mallocapp(sizeof(Queue));
    if(pqueue != NULL)
    {
        pqueue->front = NULL;
        pqueue->rear = NULL;
        pqueue->size = 0;
    }

    return pqueue;
}

void trendit_intqueue_destroy(Queue *pqueue)
{
    if(trendit_intqueue_is_empty(pqueue) != 1)
    {
        trendit_intqueue_clear(pqueue);
    }

    k_freeapp(pqueue);
}

void trendit_intqueue_clear(Queue *pqueue)
{
    while(trendit_intqueue_is_empty(pqueue) != 1)
    {
        trendit_intqueue_delqueue(pqueue, NULL);
    }
}

int trendit_intqueue_is_empty(Queue *pqueue)
{
    if(pqueue->front == NULL && pqueue->rear == NULL && pqueue->size == 0)
        return 1;
    else
        return 0;
}

int trendit_intqueue_getsize(Queue *pqueue)
{
    return pqueue->size;
}

PNode trendit_intqueue_getfront(Queue *pqueue, Item *pitem)
{
    if(trendit_intqueue_is_empty(pqueue) != 1 && pitem != NULL)
        *pitem = pqueue->front->data;
    return pqueue->front;
}

PNode trendit_intqueue_getrear(Queue *pqueue, Item *pitem)
{
    if(trendit_intqueue_is_empty(pqueue) != 1 && pitem != NULL)
    {
        *pitem = pqueue->rear->data;
    }
    return pqueue->rear;
}

PNode trendit_intqueue_enqueue(Queue *pqueue, Item item)
{
    PNode pnode = (PNode)k_mallocapp(sizeof(Node));

    if(pnode != NULL)
    {
        pnode->data = item;
        pnode->next = NULL;

        if(trendit_intqueue_is_empty(pqueue))
        {
            pqueue->front = pnode;
        }
        else
        {
            pqueue->rear->next = pnode;
        }

        pqueue->rear = pnode;
        pqueue->size++;
    }

    return pnode;
}

PNode trendit_intqueue_delqueue(Queue *pqueue, Item *pitem)
{
    PNode pnode = pqueue->front;
    if(trendit_intqueue_is_empty(pqueue) != 1 && pnode != NULL)
    {
        if(pitem != NULL)
            *pitem = pnode->data;
        pqueue->size--;
        pqueue->front = pnode->next;
        k_freeapp(pnode);
        if(pqueue->size == 0)
            pqueue->rear = NULL;
    }

    return pqueue->front;
}

/**
 * @brief 删除队列中的某个结点
 * @param[in] pqueue 队列头结点
 * @param[in] pitem 用来匹配结点的元素
 * @retval  DDI_OK 成功
 * @retval  DDI_ERR 失败
 */
s32 trendit_intqueue_delnode(Queue *pqueue, Item pitem)
{
    s32 ret = DDI_ERR;
    PNode pnode;
    PNode tmp_pnode = NULL;
    s32 i = 0;
    s32 size = 0;

    tmp_pnode = NULL;
    if(pqueue)
    {        
        pnode = pqueue->front;
        size = pqueue->size;

        for(i=0; i<size; i++)
        {
            if(pnode->data == pitem)
            {
                //结点在中间或尾部
                if(tmp_pnode)
                {
                    //结点在尾部
                    if(pnode == pqueue->rear)
                    {
                        k_freeapp(pnode);
                        pnode = NULL;
                        pqueue->size--;
                        tmp_pnode->next = NULL;
                        pqueue->rear = tmp_pnode;
                    }
                    else
                    {
                        tmp_pnode->next = pnode->next;
                        k_freeapp(pnode);
                        pnode = NULL;
                        pqueue->size--;
                    }
                }
                else
                {
                    pqueue->front = pnode->next;
                    k_freeapp(pnode);
                    pnode = NULL;
                    pqueue->size--;
                }
                ret = DDI_OK;
                break;
            }

            tmp_pnode = pnode;
            pnode = pnode->next;
        }

        if(pqueue->size == 0)
        {
            pqueue->rear = NULL;
        }
    }

    return ret;
}


void trendit_intqueue_queue_traverse(Queue *pqueue, void (*visit)(PNode))
{
    PNode pnode = pqueue->front;
    int i = pqueue->size;

    while(i--)
    {
        visit(pnode);
        pnode = pnode->next;
    }
}

// normal queue.
queue_t *trendit_queue_create(void)
{
    queue_t *pqueue = (queue_t *)k_mallocapp(sizeof(queue_t));
    if(pqueue != NULL)
    {
        pqueue->front = NULL;
        pqueue->rear = NULL;
        pqueue->size = 0;
    }

    return pqueue;
}

void trendit_queue_destroy(queue_t *pqueue)
{
    if(trendit_queue_is_empty(pqueue) != 1)
    {
        trendit_queue_clear(pqueue);
        k_freeapp(pqueue);
    }
}

void trendit_queue_clear(queue_t *pqueue)
{
    while(trendit_queue_is_empty(pqueue) != 1)
    {
        trendit_queue_delete(pqueue, NULL);
    }
}

int trendit_queue_is_empty(queue_t *pqueue)
{
    if(pqueue->front == NULL && pqueue->rear == NULL && pqueue->size == 0)
        return 1;
    else
        return 0;
}

int trendit_queue_get_size(queue_t *pqueue)
{
    return pqueue->size;
}

pnode_t trendit_queue_get_front(queue_t *pqueue, void **pitem)
{
    if(trendit_queue_is_empty(pqueue) != 1 && pitem != NULL)
        *pitem = pqueue->front->data;
    return pqueue->front;
}

pnode_t trendit_queue_get_rear(queue_t *pqueue, void **pitem)

{
    if(trendit_queue_is_empty(pqueue) != 1 && pitem != NULL)
    {
        *pitem = pqueue->rear->data;
    }
    return pqueue->rear;
}

pnode_t trendit_queue_entry(queue_t *pqueue, void *item)
{
    pnode_t pnode = (pnode_t)k_mallocapp(sizeof(node_t));

    if(pnode != NULL)
    {
        pnode->data = item;
        pnode->next = NULL;

        if(trendit_queue_is_empty(pqueue))
        {
            pqueue->front = pnode;
        }
        else
        {
            pqueue->rear->next = pnode;
        }

        pqueue->rear = pnode;
        pqueue->size++;
    }

    return pnode;
}

pnode_t trendit_queue_delete(queue_t *pqueue, void **pitem)
{
    pnode_t pnode = pqueue->front;
    if(trendit_queue_is_empty(pqueue) != 1 && pnode != NULL)
    {
        if(pitem != NULL)
            *pitem = pnode->data;
        pqueue->size--;
        pqueue->front = pnode->next;
        k_freeapp(pnode);
        if(pqueue->size == 0)
            pqueue->rear = NULL;
    }

    return pqueue->front;
}

void trendit_queue_traverse(queue_t *pqueue, void (*visit)(pnode_t))
{
    pnode_t pnode = pqueue->front;
    int i = pqueue->size;

    while(i--)
    {
        visit(pnode);
        pnode = pnode->next;
    }
}

