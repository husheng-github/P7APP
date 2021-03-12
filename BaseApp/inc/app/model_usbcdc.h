#ifndef __MODEL_USBCDC_H
#define __MODEL_USBCDC_H

typedef enum _PROTOCOL_TYPE
{
    PROTOCOL_TYPE_55AA = 0,
    PROTOCOL_TYPE_TEI,        //smart pos TEI protocol
    PROTOCOL_TYPE_TRENBT,   //��������ˢ����ָ��淶
    PROTOCOL_TYPE_ASCII,     //�ַ�������0x0D/0x0A����

}PROTOCOL_TYPE_t;

typedef enum _FRAME_TYPE
{
    FRAME_TYPE_NONE    = 0,
    //---------------------
    FRAME_TYPE_TEI_REQ = 1,         //�����,��Ӧ���
    FRAME_TYPE_TEI_RSP = 2,         //��Ӧ��
    //------------------------
    FRAME_TYPE_TRENBT_REQ = 0x2F,   //������
    FRAME_TYPE_TRENBT_NOTIFY = 0x3F,  //֪ͨ����
    FRAME_TYPE_TRENBT_RSP = 0x4F,   //��Ӧ��
    FRAME_TYPE_TRENBT_REPORT = 0x5F, //�������ͱ���
    FRAME_TYPE_TRENBT_ACK = 0x6F,   //Ӧ����
    
}FRAME_TYPE_t;

typedef enum
{
    PARSEPROTOCOL_STEP_IDLE = 0,  //���Խ�������
    PARSEPROTOCOL_STEP_RECEIVEDATASUC,
    PARSEPROTOCOL_STEP_RECEIVEDATAERR,//����Э�������
}PARSEPROTOCOL_STEP;

#define DOWNLOADBLOCKOFFSET       12
#define PC_DL_BLOCK_SIZE    (4096)
#define PC_DL_FRAME_SIZE_MAX    (68+PC_DL_BLOCK_SIZE)

#define FRAMEINFO_BUF_MAX   128
typedef struct __FRAMEINFO
{
    u32 m_bwtID;    //��¼�ֽڼ䳬ʱ
//    u32 m_rectimer;
    u8 *m_rxbuf;    //����BUF
    u16 m_cmd;      //����
    u16 m_rxlen;    //�������ݳ���
    u16 m_rxcnt;    //�������ݼ���
    u16 m_check;    //CHECKSUM
    u8 m_rxvalid;
    u8 m_commstep;
    u8 m_synheadcnt;
    u8 m_protocoltype;  //Э�����ͣ���0x55����Э���(PROTOCOL_TYPE_t)
    u8 m_frametype; //֡����,������֡��Ӧ��֡����Ӧ֡
    u8 m_sn;        //������ˮ��
    u8 m_buf[FRAMEINFO_BUF_MAX];  //��������ָ��
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

