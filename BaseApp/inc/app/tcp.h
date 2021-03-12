#ifndef __TCP_H
#define __TCP_H

#define TCP_CREATE_SOCKET_SLEEP_TIME       1000
#define TCP_CHECK_CONNECT_SLEEP_TIME       1000


typedef enum{
    SOCKET_CLOSE,
    SOCKET_CONNECTING,     //socket��������
    SOCKET_CONNECTED       //socket���ӳɹ�
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
    u32 m_thread_sleep_ticks;                  //�����������ߵı���
    u32 m_beg_con_time;                        //��ʼ����ʱ�䣬�����ж��Ƿ����ӳ�ʱ
    s32 m_state;
    u32 m_socketid;
    s8 m_serverinfor[64];                      //������������ip
    s32 m_serverport;                          //�������˿�
    s8 m_local_ip[32];                         //����ip
    u32 m_wifi_tcp_connect_overtime;           //wifi �������ֳ�ʱʱ��
    u32 m_wireless_tcp_connect_overtime;       //wireless �������ֳ�ʱʱ��
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

