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
//�����
//secure manage
#define COMAND_SECUREMANAGE_REQUESTINFO         0x3003       //PC������POS�ն�����ȫ������Ϣ
#define COMAND_SECUREMANAGE_AUTHEN              0x3004       //PC������POS�ն˷��Ͱ�ȫ���ݴ�����
#define COMAND_SECUREMANAGE_GETENCRYPTEDRAND    0x3005       //��ȡ�����

//pcikey
#define COMMAND_PCIKEY_LOADTMK                 0xF804       //�������ػ����

//
#define COMMAND_AUTHEN_GETDEVINFO              0xF101
#define COMMAND_MANAGE_DISPFIRMWARENEW         0x1DFE   //��ʾ�汾��������
#define COMMAND_MANAGE_GETTTSFILECRC           0x1DFD   //��ȡCRC,������Ҫͬ��MP3 FILE

//---------------------------------------------------
typedef struct _trenbtframe 
{
    u8 m_sn;
	u8 m_res;//��Ӧ֡��
	u16 m_status;
	u8 m_type;
    u16 m_cmd;
    u16 m_len;
    u8 *m_data;
}trenbtframe_t;



typedef enum
{
    RESPONSECODE_SUCCESS              = 0x3030,  //����ɹ�
    RESPONSECODE_CMDNOTSUPPORT        = 0x3031,  //ָ���벻֧��
    RESPONSECODE_PARAMERR             = 0x3032,  //��������
    RESPONSECODE_VARFIELDLENERR       = 0x3033,  //�ɱ������򳤶ȴ���
    RESPONSECODE_FRAMEFORMATERR       = 0x3034,  //֡��ʽ����
    RESPONSECODE_LRCERR               = 0x3035,  //LRC
    RESPONSECODE_OTHERERR             = 0x3036,  //����
    RESPONSECODE_OVERTIME             = 0x3037,  //��ʱ
    RESPONSECODE_BACKCURRENTSTATE     = 0x3038,   //���ص�ǰ״̬
    RESPONSECODE_LOWPOWER             = 0x3039,
    RESPONSECODE_UNSECURITY           = 0x303A,
    RESPONSECODE_USERCANCEL           = 0x303B,
    RESPONSECODE_DECRYPTERR           = 0x303C,
    RESPONSECODE_ENCRYPTERR           = 0x303D,
    RESPONSECODE_KEYEXIST             = 0x303E,
    RESPONSECODE_WORKMODEERR          = 0x303F,  //�ǹ���ģʽ
    RESPONSECODE_FAIL                 = 0x3040,
    RESPONSECODE_SNNOTSAME            = 0x3135,
    RESPONSECODE_WAIT                 = 0xffff
}RESPONSECODE_DEF;

//----------------------------------------------
s32 process_trenbtcmd(void);
void trendit_init_trendit_bt_cmd_data(void);


#endif
