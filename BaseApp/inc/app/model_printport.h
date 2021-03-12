#ifndef __MODEL_PRINTPORT_H
#define __MODEL_PRINTPORT_H

#define PRINT_BUFF_SIZE 10240                       //打印缓存
#define PRINT_RX_BWT_MAX   50                    //字节间超时
#define PRINTPORT_CONFIG_FILE                 "/mtd0/printport_config"
typedef struct{
    model_device_t super;
    u32 m_bwtID;             //记录字节间超时
    u8 *m_print_buff;
    s32 (*write)(u8 *write_buff, s32 len);
    u16 m_rxcnt;             //记录接收的数据长度
    u8 m_ruf[2];    //保留
}printport_module_t;

printport_module_t *printport_instance(void);
void trendit_init_printport_data(void);

#endif


