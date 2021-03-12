#include "ddiglobal.h"
#include "app_global.h"

static queue_t* g_message_queue = NULL;

/**
 * @brief ��ʼ��ȫ�ֱ���
 */
void trendit_init_msg_data(void)
{
    g_message_queue = NULL;
}

/**
 * @brief ����msg queue
 * @retval  DDI_OK   �ɹ�
 * @retval  DDI_EOVERFLOW   û�ڴ�
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
 * @brief ����Ϣ����������Ϣ
 * @param[in] msg ��Ϣ���
 * @retval  DDI_OK   �ɹ�
 * @retval  DDI_EINVAL   ��������
 * @retval  DDI_EOVERFLOW   û�ڴ�
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
 * @brief ����Ϣ���л�ȡ��Ϣ���
 * @param[in] m_msg ����������Ϣ���
 * @retval  DDI_OK   �ɹ�
 * @retval  DDI_ERR  ��Ϣ����Ϊ��
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
 * @brief ��ӡ��Ϣ���н��
 * @retval  ��
 */
void trendit_print_allmsg()
{
    trendit_queue_traverse(g_message_queue, print_allmsg);
}
