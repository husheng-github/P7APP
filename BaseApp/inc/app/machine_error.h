#ifndef __MACHINE_ERROR_H
#define __MACHINE_ERROR_H

typedef enum{
    ERR_OUT_OF_PAPAER,                       //ȱֽ
    ERR_NO_SIM_CARD,                         //sim��δ��⵽
    ERR_SIMCARD_NO_TRAFFIC,                  //sim��û����
    ERR_WIRELESS_REG_NET_DENIED,             //����ע������
    ERR_WIRELESS_REGING,                     //����ע������
    ERR_WIRELESS_ACTIVING_NET,               //���ڼ�����������
    ERR_NO_WIRELESS_MODEL,                   //δ��⵽����ģ��
    ERR_WIFI_AP_DO_NOT_CONFIG,               //wifi�ȵ�δ����
    ERR_CONNECT_BACKEND_FAIL,                //���Ӻ�̨������ʧ��
    ERR_WIFI_CONNECT_AP_FAIL,                //����wifi�ȵ�ʧ��

    ERR_MAX=ERR_WIFI_CONNECT_AP_FAIL+1
}MACHINE_ERROR_CODE_E;

typedef struct{
    MACHINE_ERROR_CODE_E m_error_code;
    s8 m_error_description[128];                //��������
}machine_error_table_t;

typedef struct{
    Queue *m_error_queue;

    s32 (*print_error_msg)(void);
    s32 (*notify_error)(MACHINE_ERROR_CODE_E machine_error_code);
    s32 (*clear_error)(MACHINE_ERROR_CODE_E machine_error_code);
}machine_error_model_t;

void trendit_init_errordata();
machine_error_model_t *error_model_instance(void);

#endif
