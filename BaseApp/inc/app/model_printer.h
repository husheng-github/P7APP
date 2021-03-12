#ifndef __MODEL_PRINTER_H
#define __MODEL_PRINTER_H

#define PRINTER_CONFIG_FILE                 "/mtd0/printer_config"

#define CFG_COMAND_PRINT_GRAY               0x0301   //打印机浓度，数值越大越浓

#define QRY_COMMAN_PRINT_PAPER_DISTANCE     0x380
#define QRY_COMMAN_PRINT_STATUS             0x381

#define CMD_COMAND_PRINT                    0x03C0


typedef enum{
    CHECK_ESC_LOOP=1,
    CHECK_PRINT_RES
}PRINTER_CHECK_STEP;

typedef struct{
    model_device_t super;
    s32 m_print_gray;                              //打印浓度，0-11，越大越弄
    u32 m_print_len;
    s32 m_load_cfg;                                //FALSE没加载完  TRUE 加载完
    s32 (*direct_print)(u8 *print_string, u16 print_string_len);
    s32 (*direct_print_times)(u8 *print_string, u16 print_string_len, s32 print_times, s32 *cur_print_len);
    s32 (*check_paper)(void);
    void (*handle_paper_key_event)(void);
    void (*init_last_print_packno)(void);
}printer_module_t;


printer_module_t *printer_instance(void);
void trendit_init_printer_data(void);
s32 trendit_post_printer_message(u8 *print_content, u32 len);



#endif

