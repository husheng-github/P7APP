#ifndef __DEVICE_DEFAULT_CFG_H
#define __DEVICE_DEFAULT_CFG_H

#define  SYSTEM_DATA_FILE                 "/mtd0/system_data"
#define  BACKEND_DEFAULT_CFG              "47.106.157.121,8008"
#define  BACKEND_DEFAULT_CFG_WITH_CMD     "1003:"##BACKEND_DEFAULT_CFG

typedef enum{
    RESEASON_COMMAND_POWER_OFF=1,         //ָ��ػ�
    RESEASON_COMMAND_REBOOT,            //ָ������
    RESEASON_REBOOT_UPDATE,             //��������
    RESEASON_KEY_POWER_OFF,             //�����ػ�
    RESEASON_WATCHDOG,                  //���Ź���λ
    RESEASON_HARD_FAULT,                //hardfault ����λ
    RESEASON_OTHER = 10                 //����ԭ��
}LAST_POWEROFF_RESEASON_E;

//�����������壬����device��m_cfg_list��
typedef struct{
    u16 m_cmd_key;
    s8 *m_value_string;
}cfg_cmd_key_value_t;

typedef struct{
    LAST_POWEROFF_RESEASON_E m_power_reason;          //����ԭ��
    u32 m_print_len;             //��ӡ����
    s8 m_wifi_ssid[32];
    s8 m_wifi_password[32];
    s8 m_serverinfor[64];                      //������������ip
    s32 m_serverport;                          //�������˿�
    s32 m_vol;                                 //������С
    s8 m_reserve[512];
    u8 m_crc[2];
}system_data_t;

typedef struct{
    system_data_t m_system_data;
    s32 (*load_data)(void);
    s32 (*save_data)(void);
    s32 (*get_wifi_data)(s8 *ssid, s32 ssid_len, s8 *password, s32 password_len);
    u32 (*get_printlen)(void);
    s32 (*get_power_reason)(void);
    s32 (*set_wifi_data)(s8 *ssid, s8 *password);
    s32 (*set_printlen)(u32 printlen);
    s32 (*set_poweroff_reason)(LAST_POWEROFF_RESEASON_E poweroff_reason);
}system_data_instance_t;

s32 trendit_load_device_cfg(model_device_t *device, s8 *cfg_file_name, exec_cfg_func cfg_func);
void trendit_init_system_data(void);
system_data_instance_t *system_data_instance(void);
s32 trendit_query_from_device_cfg(model_device_t *device           , u16 cmd, u8 *ret_string, u16 ret_stringlen);
s32 trendit_save_device_cfg(model_device_t *device, u16 cmd, u8 *valuestring, u16 valuestring_len);
s32 trendit_restore_device_cfg(model_device_t *device);



#endif
