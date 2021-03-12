#ifndef __MODEL_WIRELESS_H
#define __MODEL_WIRELESS_H

#define WIRELESS_CONFIG_FILE                   "/mtd0/wireless_config"
#define WIRELESS_CHECK_REG_SLEEP_TIME          1000              //检测驻网的线程休眠时间
#define WIRELESS_PING_INTERVAL_SLEEP_TIME      (60*1000*10)      //发送ping包间隔时间

#define CFG_COMAND_WIREDIAL                    0x0101
#define QRY_COMAND_WIRE_INFO                   0x0180
#define QRY_COMAND_WIRE_MODEL_VER              0x0181
#define QRY_COMAND_WIRE_STATE                  0x0185

typedef enum{
    WIRELESS_REG_NETWORK,
    WIRELESS_CFG_NTP,       //配置ntp服务器
    WIRELESS_GET_NTP,       //获取ntp
    WIRELESS_PING
}WIRELESS_REG_NETWORK_STEP_E;

typedef struct{
    model_device_t super;
    msg_t *m_msg;
    u32 m_msg_status;                          //消息的状态
    u32 m_thread_sleep_ticks;                  //用于任务休眠的变量
    s32 m_use_ntp_no;                          //使用的ntp序号
    s32 m_use_ntp_no_times;                    //使用的ntp序号对应的次数
    s32 m_have_play_poweron_audio;             //是否已播放开机声音

    s32 m_dail_min_csq;                        //最低能拨号信号强度
    s32 m_download_min_csq;                    //最低能下载信号强度
    s32 m_have_sync_time;                      //是否同步rtc     1:已同步
    s32 (*check_network_active_success)(void);
    s32 (*post_wireless_msg)(void);
    void (*terminal_wireless_msg)(void);
}wireless_module_t;

wireless_module_t *wireless_instance(void);
void trendit_init_wireless_data(void);


#endif

