#ifndef __MODEL_WIFI_H
#define __MODEL_WIFI_H

#define WIFI_CONFIG_FILE                   "/mtd0/wifi_config"
#define WIFI_CHECK_NET_SLEEP_TIME          1000              //���פ�����߳�����ʱ��
#define WIFI_PING_INTERVAL_SLEEP_TIME      (60*1000*10)      //����ping�����ʱ��
#define WIFI_CONNECT_OVERTIME                  20000
#define WIFI_SCANAP_OVERTIME                   20000
#define WIFI_SSID  "Trendit"
#define WIFI_PWD   "trendit123"
#define CFG_WIFI_OVERTIME                    (180*1000)          //������ʱʱ��  120��


#define CMD_QRY_WIFI_INFO                    0x0280

#define CMD_COMAND_WIFI_SCANAP               0x02C0
#define CMD_COMAND_WIFI_CONNECTAP            0x02C1
#define CMD_COMAND_WIFI_DISCONNECTAP         0x02C2

typedef enum{
    WIFI_CHECK_MODEL,
    WIFI_AP_NAME_CHECK
}WIFI_CHECK_STEP_E;

typedef enum{
    WIFI_CONNECT_AP,
    WIFI_CFG_NTP,       //����ntp������
    WIFI_GET_NTP,       //��ȡntp
    WIFI_PING
}WIFI_NETWORK_STEP_E;

//wifi��ҳ��������
typedef enum{
    LISTEN_CLIENT,
    READ_REQUEST,
    CHECK_CONNECT_AP
}WIFI_WEB_CONFIG_STEP_E;


typedef struct{
    model_device_t super;
    msg_t *m_msg;
    s32 m_network_state;
    u32 m_msg_status;                          //��Ϣ��״̬
    s32 m_beg_con_ticks;                       //�����ж����ӳ�ʱ��ticks
    u32 m_thread_sleep_ticks;                  //�����������ߵı���
    s32 m_open_fail_try_times;                 //wifi�豸��ʧ�ܵĳ��Դ���
    s32 m_have_sync_time;                      //�Ƿ�ͬ��rtc     1:��ͬ��
    s32 m_use_ntp_no_times;                    //ʹ�õ�ntp��Ŷ�Ӧ�Ĵ���
    s8 m_wifi_ssid[32];
    s8 m_wifi_password[32];
    s32 (*check_network_connect_success)(void);
    s32 (*post_wifi_msg)(void);
    void (*terminal_wifi_msg)(void);
    void (*cfg_wifi_ssid)(s8 *ssid, s8 *pwd);
    void (*airkiss_config)(void);              //΢�Ź��ں�����
    void (*web_config)(void);                  //��ҳ����
}wifi_module_t;

wifi_module_t *wifi_instance(void);
void trendit_init_wifi_data(void);


#endif

