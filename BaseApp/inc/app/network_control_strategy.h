#ifndef __NETWORK_CONTROL_STRATEGY_H
#define __NETWORK_CONTROL_STRATEGY_H

typedef enum{
    REG_NETWORK,       //依据策略判断物理网络是否ok
    CHECK_RTC,         //检测时钟
    TCP_CONNECT,       //判断当前策略的物理网络tcp是否ok
    CHECK_NETWORK      //检查物理网络和连接情况
}NETWORK_CTROL_STEP_E;

typedef enum{
    WIFI_PRIORITY,                    //WIFI优先
    WIRELESS_2G_PRIORITY,             //2G优先
    WIRELESS_2G_ONLY,                 //仅2G
    WIFI_ONLY                //仅wifi
}network_strategy_type;

typedef enum{
    REASON_POWER_ON,           //开机导致
    REASON_NET_DROP,           //物理网络掉网
    REASON_BACKEND_DENIED,     //被服务器踢掉
    REASON_NET_CHANGE          //网络切换
}NET_ONLINE_REASON_E;

typedef void (*p_func_device_online_callback)(COMMU_TYPE commu_type);             //物理网络ok后的回调通知
typedef void (*p_func_device_dropline_callback)(COMMU_TYPE commu_type);               //物理网络掉线后的回调通知
typedef void (*p_func_online_callback)(void);                    //与后台连接ok后的回调通知
typedef void (*p_func_network_drop_callback)(void);   //与后台连接断开后的回调通知

typedef enum{
    DEVICE_NET_ERR,    //物理网络异常
    DEVICE_NET_OK,     //物理网络ok
    TCP_NET_OK,        //数据网络ok
    TCP_NET_ERR        //数据网络异常
}NET_STATE_E;

typedef struct{
    network_strategy_type m_network_strategy_type;
    u32 m_beg_reg_net_ticks;                       //用于判断物理网络连接超时的ticks
    u32 m_beg_connect_ticks;                       //用于判断tcp连接超时的ticks
    COMMU_TYPE m_current_type;
    msg_t *m_msg;
    Queue *m_device_online_callback_func_list;
    Queue *m_device_dropline_callback_func_list;
    Queue *m_online_callback_func_list;
    Queue *m_drop_callback_func_list;
    s32 m_tcp_type;
    s32 m_connect_play;                            //只开机时播报一次联网成功
    u32 m_wireless_reg_net_try_overtime;           //2G优先无线的注网尝试超时时间
    u32 m_wireless_tcp_con_try_overtime;           //2G优先无线的tcp尝试超时时间
    u32 m_wifi_reg_net_try_overtime;               //wifi优先wifi的注网尝试超时时间
    u32 m_wifi_tcp_con_try_overtime;               //wifi优先wifi的tcp尝试超时时间
    u8 m_last_online_time[16];
    u8 m_last_drop_time[16];
    NET_STATE_E m_wifi_net_state;                 //优先的网络物理网络连不上----后面连上做策略重新使用此通讯方式
    NET_STATE_E m_wireless_net_state;             //优先的网络物理网络连不上----后面连上做策略重新使用此通讯方式
    NET_ONLINE_REASON_E m_online_reason;
    
    void (*start)(void);
    void (*restart)(void);
    void (*stop)(void);
    void (*reg_device_online_callback)(p_func_online_callback p_func);
    void (*notify_device_online)(COMMU_TYPE commu_type);                                             //注册物理网络上线后的通知消息
    void (*reg_device_dropline_callback)(p_func_device_dropline_callback p_func);   //注册物理网络上线后的通知消息
    void (*notify_device_dropline)(COMMU_TYPE commu_type);
    void (*reg_online_callback)(p_func_online_callback p_func);
    void (*notify_online)(void);
    void (*reg_network_drop_callback)(p_func_network_drop_callback p_func);
    void (*notify_network_drop)(void);
}network_strategy_t;

void trendit_init_network_strategy_data(void);
network_strategy_t *network_strategy_instance(void);

#endif
