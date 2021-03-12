#ifndef __MODEL_WIFI_H
#define __MODEL_WIFI_H

#define WIFI_CONFIG_FILE                   "/mtd0/wifi_config"
#define WIFI_CHECK_NET_SLEEP_TIME          1000              //检测驻网的线程休眠时间
#define WIFI_PING_INTERVAL_SLEEP_TIME      (60*1000*10)      //发送ping包间隔时间
#define WIFI_CONNECT_OVERTIME                  20000
#define WIFI_SCANAP_OVERTIME                   20000
#define WIFI_SSID  "Trendit"
#define WIFI_PWD   "trendit123"
#define CFG_WIFI_OVERTIME                    (180*1000)          //配网超时时间  120秒


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
    WIFI_CFG_NTP,       //配置ntp服务器
    WIFI_GET_NTP,       //获取ntp
    WIFI_PING
}WIFI_NETWORK_STEP_E;

//wifi网页配网步骤
typedef enum{
    LISTEN_CLIENT,
    READ_REQUEST,
    CHECK_CONNECT_AP
}WIFI_WEB_CONFIG_STEP_E;


typedef struct{
    model_device_t super;
    msg_t *m_msg;
    s32 m_network_state;
    u32 m_msg_status;                          //消息的状态
    s32 m_beg_con_ticks;                       //用于判断连接超时的ticks
    u32 m_thread_sleep_ticks;                  //用于任务休眠的变量
    s32 m_open_fail_try_times;                 //wifi设备打开失败的尝试次数
    s32 m_have_sync_time;                      //是否同步rtc     1:已同步
    s32 m_use_ntp_no_times;                    //使用的ntp序号对应的次数
    s8 m_wifi_ssid[32];
    s8 m_wifi_password[32];
    s32 (*check_network_connect_success)(void);
    s32 (*post_wifi_msg)(void);
    void (*terminal_wifi_msg)(void);
    void (*cfg_wifi_ssid)(s8 *ssid, s8 *pwd);
    void (*airkiss_config)(void);              //微信公众号配网
    void (*web_config)(void);                  //网页配网
}wifi_module_t;

wifi_module_t *wifi_instance(void);
void trendit_init_wifi_data(void);


#endif

