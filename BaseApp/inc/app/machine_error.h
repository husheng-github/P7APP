#ifndef __MACHINE_ERROR_H
#define __MACHINE_ERROR_H

typedef enum{
    ERR_OUT_OF_PAPAER,                       //缺纸
    ERR_NO_SIM_CARD,                         //sim卡未检测到
    ERR_SIMCARD_NO_TRAFFIC,                  //sim卡没流量
    ERR_WIRELESS_REG_NET_DENIED,             //无线注网被拒
    ERR_WIRELESS_REGING,                     //正在注册网络
    ERR_WIRELESS_ACTIVING_NET,               //正在激活数据连接
    ERR_NO_WIRELESS_MODEL,                   //未检测到无线模块
    ERR_WIFI_AP_DO_NOT_CONFIG,               //wifi热点未配置
    ERR_CONNECT_BACKEND_FAIL,                //连接后台服务器失败
    ERR_WIFI_CONNECT_AP_FAIL,                //连接wifi热点失败

    ERR_MAX=ERR_WIFI_CONNECT_AP_FAIL+1
}MACHINE_ERROR_CODE_E;

typedef struct{
    MACHINE_ERROR_CODE_E m_error_code;
    s8 m_error_description[128];                //错误描述
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
