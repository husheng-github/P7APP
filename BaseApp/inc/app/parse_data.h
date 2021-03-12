#ifndef __PARSE_DATA_H
#define __PARSE_DATA_H

#define PARSE_OVERTIME                              2000
#define RETURN_CONTENT_MAXSIZE                      2048
#define PARSEBUFFERMAXSIZE                          (10*1024+PROTOCOL_DATA_LEN_EXCEPT_CONTENT)
#define PARSEBUFFERMAXSIZE_BAK                      (2048+PROTOCOL_DATA_LEN_EXCEPT_CONTENT)
#define MAX_PRINT_LEN                               (10*1024)
#define STX                                         02
#define KEY_WORDS                                   "trendit"
#define ETX                                         03
#define PARSE_STEP1_LEN                             1
#define PARSE_STEP2_LEN                             strlen(KEY_WORDS)
#define PARSE_STEP3_LEN                             2
#define PARSE_STEP4_LEN                             6
#define PARSE_STEP5_0_PROTOCOL_LEN                  1
#define PARSE_STEP5_LEN                             1
#define PARSE_STEP6_LEN                             2
#define PARSE_STEP8_LEN                             1
#define PARSE_STEP9_LEN                             2

#define DATA_BEGIN_INDEX                            (PARSE_STEP2_LEN+PARSE_STEP3_LEN+PARSE_STEP4_LEN+PARSE_STEP5_LEN+PARSE_STEP5_0_PROTOCOL_LEN+PARSE_STEP6_LEN)
#define PROTOCOL_DATA_LEN_EXCEPT_CONTENT            (PARSE_STEP1_LEN+PARSE_STEP2_LEN+PARSE_STEP3_LEN+PARSE_STEP4_LEN+PARSE_STEP5_LEN+PARSE_STEP5_0_PROTOCOL_LEN+PARSE_STEP6_LEN+PARSE_STEP8_LEN+PARSE_STEP9_LEN)


#define PROTOCOL_VERSION                            0      //后台通讯协议版本


typedef enum{
    PARSE_STEP1_STX = 0,
    PARSE_STEP2_KW,
    PARSE_STEP3_PACKAGE_NO,
    PARSE_STEP4_SENTIME,
    PARSE_STEP5_0_PROTOCOL,
    PARSE_STEP5_CMD_TYPE,
    PARSE_STEP6_DATALEN,
    PARSE_STEP7_DATA,
    PARSE_STEP8_ETX,
    PARSE_STEP9_CRC
}PARSE_STEP;




typedef enum{
    CMD_TYPE_QRY = 1,
    CMD_TYPE_CFG,
    CMD_TYPE_CMD,
    CMD_TYPE_EVENT,         //事件
    CMD_TYPE_ACK,           //应答报文
    CMD_TYPE_ANSWER         //回复报文
}CMD_TYPE;

typedef struct _APP_PARSEDATA
{
    u16 m_rx_cnt;                      //已收的数据长度
    u16 m_data_len;                    //正文区长度
    u8 *m_data;                        //正文内容
    s32 m_tmplen;                      //用来判断各部分内容是否收满的变量
    s32 m_data_buff_size;              //缓存大小
    u8 m_cmd_type;                     //命令类型
    u8 m_parse_step;
    u16 m_package_no;
    u16 m_cmd;                         //命令
    u32 m_beg_ticks;                   //接收长度的时间，用来判断指令是否超时了
}APP_PARSEDATA;

typedef struct _app_answer_data
{
    u16 m_package_no;
    u8 m_cmd_type;                     //命令类型
    u8 *m_data;
    u16 m_data_len;                    //报文长度
}app_answer_data_t;

typedef enum{
    DATA_SOURCE_TCP,
    DATA_SOURCE_USB
}DATA_SOURCE_E;

typedef struct{
    void (*handle_data)(DATA_SOURCE_E data_source, u8 *rec_data, s32 rec_data_len, s32 socket_id);
    u16 (*get_current_packno)(void);
    u16 (*get_current_cmd)(void);
}data_parser_t;

void trendit_init_parsedata(void);
data_parser_t *data_parser_instance(void);


#endif

