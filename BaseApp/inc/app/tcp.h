#ifndef __TCP_H
#define __TCP_H

#define TCP_CREATE_SOCKET_SLEEP_TIME       1000
#define TCP_CHECK_CONNECT_SLEEP_TIME       1000


typedef enum{
    SOCKET_CLOSE,
    SOCKET_CONNECTING,     //socket正在连接
    SOCKET_CONNECTED       //socket连接成功
}SOCKET_STATUS;

typedef enum{
    COMMU_WIRELESS = 1,
    COMMU_WIFI
}COMMU_TYPE;

typedef struct{
    u8 m_serverinfo[64];
    s32 m_server_port;
    s8 m_local_ip[32];
    s8 m_cur_commu_type[16];
}cur_server_info_t;

typedef struct{
    msg_t *m_msg;
    u32 m_thread_sleep_ticks;                  //用于任务休眠的变量
    u32 m_beg_con_time;                        //开始建联时间，用于判断是否连接超时
    s32 m_state;
    u32 m_socketid;
    s8 m_serverinfor[64];                      //服务器域名或ip
    s32 m_serverport;                          //服务器端口
    s8 m_local_ip[32];                         //本地ip
    u32 m_wifi_tcp_connect_overtime;           //wifi 三次握手超时时间
    u32 m_wireless_tcp_connect_overtime;       //wireless 三次握手超时时间
    s32 (*start)(void);
    void (*stop)(void);
    void (*restart)(void);
    s32 (*send)(u8 *send_data, u32 len);
    s32 (*check_tcp_connect_success)(void);
    s32 (*cfg_server_info)(s8 *ipaddr, s32 port);
    s32 (*get_server_info)(cur_server_info_t *cur_server_info);
    u32 (*get_tcp_connect_overtime)(void);
}tcp_protocol_module_t;

void trendit_init_tcpdata(void);
s32 trendit_get_server_info(s8 *server_ip, s32 server_ip_len, s32 *server_port);
tcp_protocol_module_t *tcp_protocol_instance();


#endif

