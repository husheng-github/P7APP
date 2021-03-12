#ifndef __MODEL_WIRELESS_H
#define __MODEL_WIRELESS_H

#define WIRELESS_CONFIG_FILE                   "/mtd0/wireless_config"
#define WIRELESS_CHECK_REG_SLEEP_TIME          1000              //���פ�����߳�����ʱ��
#define WIRELESS_PING_INTERVAL_SLEEP_TIME      (60*1000*10)      //����ping�����ʱ��

#define CFG_COMAND_WIREDIAL                    0x0101
#define QRY_COMAND_WIRE_INFO                   0x0180
#define QRY_COMAND_WIRE_MODEL_VER              0x0181
#define QRY_COMAND_WIRE_STATE                  0x0185

typedef enum{
    WIRELESS_REG_NETWORK,
    WIRELESS_CFG_NTP,       //����ntp������
    WIRELESS_GET_NTP,       //��ȡntp
    WIRELESS_PING
}WIRELESS_REG_NETWORK_STEP_E;

typedef struct{
    model_device_t super;
    msg_t *m_msg;
    u32 m_msg_status;                          //��Ϣ��״̬
    u32 m_thread_sleep_ticks;                  //�����������ߵı���
    s32 m_use_ntp_no;                          //ʹ�õ�ntp���
    s32 m_use_ntp_no_times;                    //ʹ�õ�ntp��Ŷ�Ӧ�Ĵ���
    s32 m_have_play_poweron_audio;             //�Ƿ��Ѳ��ſ�������

    s32 m_dail_min_csq;                        //����ܲ����ź�ǿ��
    s32 m_download_min_csq;                    //����������ź�ǿ��
    s32 m_have_sync_time;                      //�Ƿ�ͬ��rtc     1:��ͬ��
    s32 (*check_network_active_success)(void);
    s32 (*post_wireless_msg)(void);
    void (*terminal_wireless_msg)(void);
}wireless_module_t;

wireless_module_t *wireless_instance(void);
void trendit_init_wireless_data(void);


#endif

