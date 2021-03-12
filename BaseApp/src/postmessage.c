#include "ddiglobal.h"
#include "app_global.h"

static queue_t* g_message_queue = NULL;

/**
 * @brief 初始化全局变量
 */
void trendit_init_msg_data(void)
{
    g_message_queue = NULL;
}

/**
 * @brief 创建msg queue
 * @retval  DDI_OK   成功
 * @retval  DDI_EOVERFLOW   没内存
 */
s32 trendit_init_msg()
{
    s32 ret = DDI_EOVERFLOW;

    if(NULL == g_message_queue)
    {
        g_message_queue = trendit_queue_create();
    }

    if(NULL != g_message_queue)
    {
        ret = DDI_OK;
    }

    return ret;
}

/**
 * @brief 往消息队列推送消息
 * @param[in] msg 消息结点
 * @retval  DDI_OK   成功
 * @retval  DDI_EINVAL   参数错误
 * @retval  DDI_EOVERFLOW   没内存
 */
s32 trendit_postmsg(msg_t *msg)
{
    s32 ret = DDI_ERR;
    void *node = NULL;

    do{
        if(NULL == msg)
        {
            ret = DDI_EINVAL;
            break;
        }

        if(MSG_HIGH == msg->m_priority)
        {

        }
        else
        {
            node = trendit_queue_entry(g_message_queue, (void *)msg);
            if(NULL == node)
            {
                TRACE_ERR("enter queue failed");
                ret = DDI_EOVERFLOW;
                break;
            }
        }
        ret = DDI_OK;
    }while(0);

    return ret;
}

s32 trendit_sendmsg(msg_t *m_msg)
{
    
}

/**
 * @brief 从消息队列获取消息结点
 * @param[in] m_msg 用来填充的消息结点
 * @retval  DDI_OK   成功
 * @retval  DDI_ERR  消息队列为空
 */
s32 trendit_querymsg(msg_t **m_msg)
{
    s32 ret = DDI_ERR;

    do{
        if(trendit_queue_is_empty(g_message_queue))
        {
            break;
        }

        trendit_queue_delete(g_message_queue, (void **)m_msg);
        if(NULL != *m_msg)
        {
            ret = DDI_OK;
        }
    }while(0);

    return ret;
}

static void print_allmsg(pnode_t node)
{
    msg_t *m_msg = NULL;

    m_msg = (msg_t *)node->data;
    TRACE_DBG("msg:%x, name:%s, prior:%d, status:%d", m_msg, m_msg->m_msgname, m_msg->m_priority, m_msg->m_status);
}

/**
 * @brief 打印消息所有结点
 * @retval  无
 */
void trendit_print_allmsg()
{
    trendit_queue_traverse(g_message_queue, print_allmsg);
}
