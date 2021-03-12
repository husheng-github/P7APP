#ifndef __POSTMESSAGE_H
#define __POSTMESSAGE_H

typedef enum{
    MSG_NORMAL,        //普通优先级(缺省)
    MSG_HIGH           //高优先级
}msg_priority_t;

typedef enum{
    MSG_INIT,          //MSG初始化状态
    MSG_SERVICE,       //MSG正常服务状态
    MSG_TERMINAL       //MSG终止状态
}msg_status_t;

typedef enum{
    MSG_OUT,           //本消息处理完毕后从对列清除
    MSG_KEEP          //本消息处理完毕后移动到对列尾部
}msg_handle_rst_t;

typedef struct _msg                                           //消息机制节点
{
    msg_priority_t m_priority;                                //消息优先级
    msg_status_t   m_status;                                  //消息状态 
    s32 (*m_func)(struct _msg *m_msg);                         //消息入口函数
    u32 m_wparam;                                             //消息高参数
    u32 m_lParam;                                             //消息低参数
    s8 m_msgname[32];                                         //消息名称
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
