#ifndef __MODEL_USBCDC_H
#define __MODEL_USBCDC_H

typedef enum _PROTOCOL_TYPE
{
    PROTOCOL_TYPE_55AA = 0,
    PROTOCOL_TYPE_TEI,        //smart pos TEI protocol
    PROTOCOL_TYPE_TRENBT,   //大趋蓝牙刷卡器指令规范
    PROTOCOL_TYPE_ASCII,     //字符串，以0x0D/0x0A结束

}PROTOCOL_TYPE_t;

typedef enum _FRAME_TYPE
{
    FRAME_TYPE_NONE    = 0,
    //---------------------
    FRAME_TYPE_TEI_REQ = 1,         //请求包,或应答包
    FRAME_TYPE_TEI_RSP = 2,         //响应包
    //------------------------
    FRAME_TYPE_TRENBT_REQ = 0x2F,   //请求报文
    FRAME_TYPE_TRENBT_NOTIFY = 0x3F,  //通知报文
    FRAME_TYPE_TRENBT_RSP = 0x4F,   //响应包
    FRAME_TYPE_TRENBT_REPORT = 0x5F, //主动上送报文
    FRAME_TYPE_TRENBT_ACK = 0x6F,   //应答报文
    
}FRAME_TYPE_t;

typedef enum
{
    PARSEPROTOCOL_STEP_IDLE = 0,  //可以接收数据
    PARSEPROTOCOL_STEP_RECEIVEDATASUC,
    PARSEPROTOCOL_STEP_RECEIVEDATAERR,//接收协议包错误
}PARSEPROTOCOL_STEP;

#define DOWNLOADBLOCKOFFSET       12
#define PC_DL_BLOCK_SIZE    (4096)
#define PC_DL_FRAME_SIZE_MAX    (68+PC_DL_BLOCK_SIZE)

#define FRAMEINFO_BUF_MAX   128
typedef struct __FRAMEINFO
{
    u32 m_bwtID;    //记录字节间超时
//    u32 m_rectimer;
    u8 *m_rxbuf;    //接收BUF
    u16 m_cmd;      //命令
    u16 m_rxlen;    //接收数据长度
    u16 m_rxcnt;    //接收数据计数
    u16 m_check;    //CHECKSUM
    u8 m_rxvalid;
    u8 m_commstep;
    u8 m_synheadcnt;
    u8 m_protocoltype;  //协议类型，如0x55下载协议等(PROTOCOL_TYPE_t)
    u8 m_frametype; //帧类型,如请求帧、应答帧、响应帧
    u8 m_sn;        //接收流水号
    u8 m_buf[FRAMEINFO_BUF_MAX];  //缓冲其他指令
}frameinfo_t;
typedef struct{
    model_device_t super;
    u32 m_promptnewversiontimer;
}usbcdc_module_t;

usbcdc_module_t *usbcdc_instance(void);
void trendit_init_usbcdc_data(void);
s32 usb_frameinfo_free(void);
//void *usb_frameinfo_malloc(u32 nBytes);
s32 usb_frame_parse(void);
frameinfo_t *usb_frame_parse_getinfo(void);


#endif

