#ifndef __PROCESS_TRENBT_CMD_H
#define __PROCESS_TRENBT_CMD_H

#if 1
#define TRENCMD_DEBUG     TRACE_DBG
#define TRENCMD_DEBUGHEX  TRACE_DBG_HEX
#else
#define TRENCMD_DEBUG     
#define TRENCMD_DEBUGHEX   
#endif

#define COMMPROTOCOL_STX1 0x4C
#define COMMPROTOCOL_STX2 0x4B
#define COMMPROTOCOL_EXT  0x03


#define COREMANAGE_MAXSENDBUFSIZE    300
//---------------------------------------------------
//命令定义
//secure manage
#define COMAND_SECUREMANAGE_REQUESTINFO         0x3003       //PC工具向POS终端请求安全处理信息
#define COMAND_SECUREMANAGE_AUTHEN              0x3004       //PC工具向POS终端发送安全数据处理结果
#define COMAND_SECUREMANAGE_GETENCRYPTEDRAND    0x3005       //获取随机数

//pcikey
#define COMMAND_PCIKEY_LOADTMK                 0xF804       //处理下载机身号

//
#define COMMAND_AUTHEN_GETDEVINFO              0xF101
#define COMMAND_MANAGE_DISPFIRMWARENEW         0x1DFE   //提示版本已是最新
#define COMMAND_MANAGE_GETTTSFILECRC           0x1DFD   //获取CRC,在这里要同步MP3 FILE

//---------------------------------------------------
typedef struct _trenbtframe 
{
    u8 m_sn;
	u8 m_res;//响应帧用
	u16 m_status;
	u8 m_type;
    u16 m_cmd;
    u16 m_len;
    u8 *m_data;
}trenbtframe_t;



typedef enum
{
    RESPONSECODE_SUCCESS              = 0x3030,  //处理成功
    RESPONSECODE_CMDNOTSUPPORT        = 0x3031,  //指令码不支持
    RESPONSECODE_PARAMERR             = 0x3032,  //参数错误
    RESPONSECODE_VARFIELDLENERR       = 0x3033,  //可变数据域长度错误
    RESPONSECODE_FRAMEFORMATERR       = 0x3034,  //帧格式错误
    RESPONSECODE_LRCERR               = 0x3035,  //LRC
    RESPONSECODE_OTHERERR             = 0x3036,  //其他
    RESPONSECODE_OVERTIME             = 0x3037,  //超时
    RESPONSECODE_BACKCURRENTSTATE     = 0x3038,   //返回当前状态
    RESPONSECODE_LOWPOWER             = 0x3039,
    RESPONSECODE_UNSECURITY           = 0x303A,
    RESPONSECODE_USERCANCEL           = 0x303B,
    RESPONSECODE_DECRYPTERR           = 0x303C,
    RESPONSECODE_ENCRYPTERR           = 0x303D,
    RESPONSECODE_KEYEXIST             = 0x303E,
    RESPONSECODE_WORKMODEERR          = 0x303F,  //非工作模式
    RESPONSECODE_FAIL                 = 0x3040,
    RESPONSECODE_SNNOTSAME            = 0x3135,
    RESPONSECODE_WAIT                 = 0xffff
}RESPONSECODE_DEF;

//----------------------------------------------
s32 process_trenbtcmd(void);
void trendit_init_trendit_bt_cmd_data(void);


#endif
