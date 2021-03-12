#ifndef __POSTMESSAGE_H
#define __POSTMESSAGE_H

typedef enum{
    MSG_NORMAL,        //��ͨ���ȼ�(ȱʡ)
    MSG_HIGH           //�����ȼ�
}msg_priority_t;

typedef enum{
    MSG_INIT,          //MSG��ʼ��״̬
    MSG_SERVICE,       //MSG��������״̬
    MSG_TERMINAL       //MSG��ֹ״̬
}msg_status_t;

typedef enum{
    MSG_OUT,           //����Ϣ������Ϻ�Ӷ������
    MSG_KEEP          //����Ϣ������Ϻ��ƶ�������β��
}msg_handle_rst_t;

typedef struct _msg                                           //��Ϣ���ƽڵ�
{
    msg_priority_t m_priority;                                //��Ϣ���ȼ�
    msg_status_t   m_status;                                  //��Ϣ״̬ 
    s32 (*m_func)(struct _msg *m_msg);                         //��Ϣ��ں���
    u32 m_wparam;                                             //��Ϣ�߲���
    u32 m_lParam;                                             //��Ϣ�Ͳ���
    s8 m_msgname[32];                                         //��Ϣ����
}msg_t;

typedef struct __message_node
{
    msg_t  m_msg_node;
    struct list_head list;
}message_node_t;


void trendit_init_msg_data(void);
s32 trendit_init_msg(void);
s32 trendit_postmsg(msg_t *msg);
s32 trendit_sendmsg(msg_t *m_msg);
s32 trendit_querymsg(msg_t **m_msg);
void trendit_print_allmsg(void);


#endif
