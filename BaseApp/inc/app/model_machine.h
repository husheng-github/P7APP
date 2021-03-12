#ifndef __MODEL_MACHINE_H
#define __MODEL_MACHINE_H

#define MACHINE_CONFIG_FILE            "/mtd0/machine_config"

#define CHECK_PAPER_INTERVAL_TIME                      (500)    //500毫秒检测一次纸

#define CFG_COMAND_MACHINE_BACKEND_CON                 0x1003
#define CFG_COMAND_MACHINE_HBT_INTERVAL                0x1004
#define CFG_COMAND_MACHINE_COMMU_STRATEGY              0x1005        //通信策略选择
#define CFG_COMAND_MACHINE_NET_STRATEGY_CON_TIME       0x1009
#define CFG_COMAND_MACHINE_POWEROFF_KEY_DELAY_TIME     0x100A        //关机键持续的时间
#define CFG_COMAND_MACHINE_CFG_NTP                     0x100B
#define CFG_COMAND_MACHINE_TMS_CFG                     0x100C
#define CFG_COMAND_MACHINE_WIFI_AIRKISS_URL            0x100D





#define CFG_QRY_MACHINE_MEM_INFO                       0x1080
#define CFG_QRY_MACHINE_ALL_VERSION                    0x1081
#define CFG_QRY_MACHINE_PWO_CHECK_RES                  0x1082
#define CFG_QRY_MACHINE_CURRENT_COMMU_CHANNEL          0x1083
#define CFG_QRY_MACHINE_LAST_DROP_TIME                 0x1084
#define CFG_QRY_MACHINE_LAST_ONLINE_TIME               0x1085
#define CFG_QRY_MACHINE_ONLINE_REASON                  0x1086
#define CFG_QRY_MACHINE_LAST_POWEROFF_REASON           0x1087
#define CFG_QRY_MACHINE_SN_KEY                         0x1088
#define CFG_QRY_MACHINE_RUN_TIME                       0x1089




#define CMD_COMMAND_MACHINE_UPGRADE_APP                0x10C0
#define CMD_COMMAND_MACHINE_UPGRADE_CORE               0x10C1

#define CMD_COMMAND_MACHINE_REBOOT                     0x10c2

#define CMD_COMMAND_MACHINE_DOWNLOAD_BITMAP            0x10c4
#define CMD_COMMAND_MACHINE_DELETE_BITMAP              0x10c5
#define CMD_COMMAND_MACHINE_MODIFY_BITMAP              0x10c6
#define CMD_COMMAND_MACHINE_MODIFY_APIKEY              0x10CA

#define CMD_COMMAND_MACHINE_DOWNLOAD_AUDIO             0x10C7
#define CMD_COMMAND_MACHINE_UPDATE_AUDIO               0x10C8
#define CMD_COMMAND_MACHINE_DELETE_AUDIO               0x10C9
#define CMD_COMMAND_MACHINE_MODIFY_LOG_LEVEL           0x10CB






#define HBT_INTERVAL_MIN                               10
#define HBT_INTERVAL_MAX                               3600

#define WIFI_REG_MIN_TIME                              20
#define WIFI_REG_MAX_TIME                              300
#define WIRELESS_REG_MIN_TIME                          20
#define WIRELESS_REG_MAX_TIME                          300
#define WIFI_CON_BACKEND_MIN_TIME                      20
#define WIFI_CON_BACKEND_MAX_TIME                      300
#define WIRELESS_CON_BACKEND_MIN_TIME                  20
#define WIRELESS_CON_BACKEND_MAX_TIME                  300
#define POWER_KEY_DELAY_MIN_TIME                       0
#define POWER_KEY_DELAY_MAX_TIME                       5000



typedef struct{
    model_device_t super;
    u32 m_thread_sleep_ticks;
    s32 m_paper_status;                         //true缺纸    false有纸
    u32 m_heart_beat_beg_ticks;                //判断心跳
    s32 m_heart_beat_interval;                 //心跳间隔，单位秒
    network_strategy_t *m_network_strategy;
    s32 m_have_send_poweron_msg;               //是否已发送开机报完
    s32 m_power_on_reason;                     //上一次关机原因
    s32 m_power_key_delay_time;                //关机键响应的时间
    s8 m_ntp_info[3][64];                      //ntp服务器
    s8 m_wifi_airkiss_url[64];
    s32 m_tms_auto_switch;                     //tms自动检测开关
    s8 m_tms_auto_detect_time[7];              //tms每天自动检测的时间

    s32 (*get_power_key_delay_time)(void);
}machine_module_t;

machine_module_t *machine_instance(void);
void trendit_init_machine_data(void);


#endif


