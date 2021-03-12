#ifndef __NETWORK_CONTROL_STRATEGY_H
#define __NETWORK_CONTROL_STRATEGY_H

typedef enum{
    REG_NETWORK,       //���ݲ����ж����������Ƿ�ok
    CHECK_RTC,         //���ʱ��
    TCP_CONNECT,       //�жϵ�ǰ���Ե���������tcp�Ƿ�ok
    CHECK_NETWORK      //�������������������
}NETWORK_CTROL_STEP_E;

typedef enum{
    WIFI_PRIORITY,                    //WIFI����
    WIRELESS_2G_PRIORITY,             //2G����
    WIRELESS_2G_ONLY,                 //��2G
    WIFI_ONLY                //��wifi
}network_strategy_type;

typedef enum{
    REASON_POWER_ON,           //��������
    REASON_NET_DROP,           //�����������
    REASON_BACKEND_DENIED,     //���������ߵ�
    REASON_NET_CHANGE          //�����л�
}NET_ONLINE_REASON_E;

typedef void (*p_func_device_online_callback)(COMMU_TYPE commu_type);             //��������ok��Ļص�֪ͨ
typedef void (*p_func_device_dropline_callback)(COMMU_TYPE commu_type);               //����������ߺ�Ļص�֪ͨ
typedef void (*p_func_online_callback)(void);                    //���̨����ok��Ļص�֪ͨ
typedef void (*p_func_network_drop_callback)(void);   //���̨���ӶϿ���Ļص�֪ͨ

typedef enum{
    DEVICE_NET_ERR,    //���������쳣
    DEVICE_NET_OK,     //��������ok
    TCP_NET_OK,        //��������ok
    TCP_NET_ERR        //���������쳣
}NET_STATE_E;

typedef struct{
    network_strategy_type m_network_strategy_type;
    u32 m_beg_reg_net_ticks;                       //�����ж������������ӳ�ʱ��ticks
    u32 m_beg_connect_ticks;                       //�����ж�tcp���ӳ�ʱ��ticks
    COMMU_TYPE m_current_type;
    msg_t *m_msg;
    Queue *m_device_online_callback_func_list;
    Queue *m_device_dropline_callback_func_list;
    Queue *m_online_callback_func_list;
    Queue *m_drop_callback_func_list;
    s32 m_tcp_type;
    s32 m_connect_play;                            //ֻ����ʱ����һ�������ɹ�
    u32 m_wireless_reg_net_try_overtime;           //2G�������ߵ�ע�����Գ�ʱʱ��
    u32 m_wireless_tcp_con_try_overtime;           //2G�������ߵ�tcp���Գ�ʱʱ��
    u32 m_wifi_reg_net_try_overtime;               //wifi����wifi��ע�����Գ�ʱʱ��
    u32 m_wifi_tcp_con_try_overtime;               //wifi����wifi��tcp���Գ�ʱʱ��
    u8 m_last_online_time[16];
    u8 m_last_drop_time[16];
    NET_STATE_E m_wifi_net_state;                 //���ȵ�������������������----������������������ʹ�ô�ͨѶ��ʽ
    NET_STATE_E m_wireless_net_state;             //���ȵ�������������������----������������������ʹ�ô�ͨѶ��ʽ
    NET_ONLINE_REASON_E m_online_reason;
    
    void (*start)(void);
    void (*restart)(void);
    void (*stop)(void);
    void (*reg_device_online_callback)(p_func_online_callback p_func);
    void (*notify_device_online)(COMMU_TYPE commu_type);                                             //ע�������������ߺ��֪ͨ��Ϣ
    void (*reg_device_dropline_callback)(p_func_device_dropline_callback p_func);   //ע�������������ߺ��֪ͨ��Ϣ
    void (*notify_device_dropline)(COMMU_TYPE commu_type);
    void (*reg_online_callback)(p_func_online_callback p_func);
    void (*notify_online)(void);
    void (*reg_network_drop_callback)(p_func_network_drop_callback p_func);
    void (*notify_network_drop)(void);
}network_strategy_t;

void trendit_init_network_strategy_data(void);
network_strategy_t *network_strategy_instance(void);

#endif
