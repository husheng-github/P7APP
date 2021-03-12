#ifndef __DEVICE_H
#define __DEVICE_H

#define INIT_SUPER_METHOD(self, model_no, model_name)                              \
do{                                                          \
    self->super.m_no                 = model_no;             \
    self->super.exec_cmd             = exec_cmd;             \
    self->super.exec_qry             = exec_qry;             \
    self->super.exec_cfg             = exec_cfg;             \
    self->super.load_cfg             = trendit_load_device_cfg;             \
    self->super.save_cfg              = trendit_save_device_cfg;            \
    self->super.restore_cfg           = trendit_restore_device_cfg;            \
    self->super.qry_from_cfg          = trendit_query_from_device_cfg;      \
    snprintf(self->super.m_model_name, sizeof(self->super.m_model_name), "%s", model_name);             \
}while(0)

typedef s32 (*exec_cfg_func)(u16 cmd, u8 *value_string, u16 value_stringlen);

typedef struct _model_device {
    s32 m_no;                                      //index of device
    s8 m_model_name[32];
    s8 m_cfg_file_name[64];
    s32 m_self_check_res;                          //true:打开ok   false:设备故障
    queue_t *m_cfg_list;
    s32 (*exec_cmd)(u16 cmd, u8 *value_string, u16 value_stringlen, u8 *ret_string, u16 ret_stringlen);   //inner function,execute AT+CMD report
    s32 (*exec_qry)(u16 cmd, u8 *ret_string, u16 ret_stringlen);
    exec_cfg_func exec_cfg;
    s32 (*load_cfg)(struct _model_device *device, s8 *cfg_file_name, exec_cfg_func cfg_func);
    s32 (*save_cfg)(struct _model_device *device, u16 cmd, u8 *valuestring, u16 valuestring_len);
    s32 (*remove_cfg_file)(struct _model_device *device);
    s32 (*restore_cfg)(struct _model_device *device);
    s32 (*qry_from_cfg)(struct _model_device *device, u16 cmd, u8 *ret_string, u16 ret_stringlen);
    int (*destruct)();
}model_device_t;

typedef model_device_t* (*model_instance)();

typedef enum
{
    MODEL_NONE    = 0,
    MODEL_WIRELESS = 1,
    MODEL_WIFI,
    MODEL_PRINTER,
    MODEL_AUDIO,
    MODEL_KEY,
    MODEL_USBCDC,
    MODEL_PRINTPORT,
    MODEL_TMS,
    MODEL_MACHINE=0x10,
} AP_DEV_E;

typedef struct __reg_dev_s
{
    AP_DEV_E mod;
    model_instance m_instance;
}reg_dev_t;

model_device_t* trendit_get_model(u16 cmd);
void trendit_reg_dev(void);
void trendit_init_device_data(void);
void trendit_print_machine_info(void);
void trenit_remove_all_device_cfg_file(void);



#endif
