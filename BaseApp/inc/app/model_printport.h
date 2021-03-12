#ifndef __MODEL_PRINTPORT_H
#define __MODEL_PRINTPORT_H

#define PRINT_BUFF_SIZE 10240                       //��ӡ����
#define PRINT_RX_BWT_MAX   50                    //�ֽڼ䳬ʱ
#define PRINTPORT_CONFIG_FILE                 "/mtd0/printport_config"
typedef struct{
    model_device_t super;
    u32 m_bwtID;             //��¼�ֽڼ䳬ʱ
    u8 *m_print_buff;
    s32 (*write)(u8 *write_buff, s32 len);
    u16 m_rxcnt;             //��¼���յ����ݳ���
    u8 m_ruf[2];    //����
}printport_module_t;

printport_module_t *printport_instance(void);
void trendit_init_printport_data(void);

#endif


