#ifndef _DDI_INNERKEY_H_
#define _DDI_INNERKEY_H_


#include "ddi_common.h"




#define INNERKEY_IOCTL_INITIAL     0
#define INNERKEY_IOCTL_MACX919     1
#define INNERKEY_IOCTL_MACX9_9     2  //X9.9�㷨��

typedef enum _WKARITH
{
    WKTYPE_TMKENCRYPTED_WITHCHECKVLAUE = 0,
    WKTYPE_TMKENCRYPTED_NOCHECKVALUE   = 1,
    WKTYPE_PLAIN
    
}WKARITH;

typedef struct _macencrymod
{
    u32 m_keytype;
    u32 m_index;
    u8  *m_buf;
    u32 m_len;
}macencrymod_t;

/*������Կ��������*/
typedef struct __str_INJECTCERT
{
    u8 *m_data;         //����
    u16 m_filelen;      //�ļ��ܳ���
    u16 m_len;          //���ݳ���
    u16 m_offset;       //��ǰƫ�Ƶ�ַ
    u8 m_type;          //����
    u8 m_rfu[1];        //����
}str_injectcert_t;

typedef struct __INNERKEY_CRYPTOGRAPHY
{
    u8 *m_datain;
    u8 *m_dataout;      
    u8 *m_key;          //���ݲ�ͬ���ܶ�,�����Ƕ���RSA,���key==NULL,�����m_type���Ͳ��ù̶���Կ
    u16 m_datainlen;
    u16 m_dataoutlen;   
    u16 m_keylen;
    u8  m_mod;          //bit7: 0:����   1:����
                        //bit6~4: 0:��ͨ
                        //        ��������          
                        //bit3~0: 0:SM4   1: DES   2:RSA��Կ  3:RSA˽Կ
    u8  m_group;        //�����
    u8  m_type;         //����,��TMK��TPK��TAK�ȣ�
    u8  m_index;        //������
    u8  m_rfu[2];       //����
}INNERKEY_CRYPTOGRAPHY_t;

#define TRENDIT_GROUPINDEX    0


#define PCI_RECKEYTYPE_MANAGEKEY      0x14
#define PCI_RECKEYTYPE_APPKEY     0x15
#define PCI_RECKEYTYPE_AUTHKEY    0x16



#define TYPE_TMK            0x04  //TMK
#define TYPE_PINK           0x05  //PIN key
#define TYPE_MACK           0x06  //MAC key
#define TYPE_TDK            0x07  //TDK key
#define TYPE_FIXEDK         0x08  //fixed key
#define TYPE_BPK            0x09  //authentication key of session key(PINK and MACK TDK)
#define TYPE_DUKPTK         0x0A
#define TYPE_KEYRESERVED    0x10

#define TYPE_NULL           0xff


#define PCIKEYTYPE_TMK       TYPE_TMK
#define PCIKEYTYPE_TPK       TYPE_PINK
#define PCIKEYTYPE_TDK       TYPE_TDK
#define PCIKEYTYPE_TAK       TYPE_MACK
#define PCIKEYTYPE_MANAGE    TYPE_FIXEDK
#define PCIKEYTYPE_APP       TYPE_FIXEDK
#define PCIKEYTYPE_AUTH      TYPE_FIXEDK
#define PCIKEYTYPE_SNKEY     TYPE_FIXEDK


#define PCIKEYTYPE_MANAGE_INDEX 7
#define PCIKEYTYPE_AUTH_INDEX   8
#define PCIKEYTYPE_APP_INDEX    9
#define PCIKEYTYPE_SNKEY_INDEX  10

#define PCIKEYTYPE_WK_MAXINDEX        10

#define PCIKEYTYPE_TPK_MAXINDEX       PCIKEYTYPE_WK_MAXINDEX
#define PCIKEYTYPE_TMK_MAXINDEX       PCIKEYTYPE_WK_MAXINDEX
#define PCIKEYTYPE_TDK_MAXINDEX       PCIKEYTYPE_WK_MAXINDEX
#define PCIKEYTYPE_TAK_MAXINDEX       PCIKEYTYPE_WK_MAXINDEX
#define PCIKEYTYPE_MANAGE_MAXINDEX    1
#define PCIKEYTYPE_APP_MAXINDEX       1
#define PCIKEYTYPE_AUTH_MAXINDEX      1

#define PUBLICKEY_VERSION_OFFSET   0
#define PUBLICKEY_VERSION_SIZE     8


typedef enum _TERMINAL_PK_TYPE_
{
    PK_ACQUIRER,
    PK_ACQUIRERBAK,
    PK_PREVENT = 20,
    PK_TRENDIT_ROOT,
    PK_ACQUIRER_ROOT,
    PK_USBBOOT

}TERMINAL_PK_TYPE;

typedef struct _INNERKEY_VERITYSIGNATURE
{
    u8 *m_filename;
    u32 m_addr;
    u32 m_len;
    u8  m_mod;
    u8  m_type;
    u8  m_ruf[2];
}INNERKEY_VERITYSIGNATURE_t;

#define     DDI_INNERKEY_CTL_VER                (0) //��ȡ������Կ�豸�汾
#define     DDI_INNERKEY_CTL_TKEY_INJECT        (1) //����ʱ��Կ���浽ָ����Կ��
#define     DDI_INNERKEY_CTL_TKEY_ENCRYPT       (2) //��ʱ��Կ����
#define     DDI_INNERKEY_CTL_TKEY_DECRYPT       (3) //��ʱ��Կ����
#define     DDI_INNERKEY_CTL_HKEY_ENCRYPT       (4) //˫������8�ֽڼ���
#define     DDI_INNERKEY_CTL_HKEY_DECRYPT       (5) //˫������8�ֽڽ���
#define     DDI_INNERKEY_CTL_KEY_CHECK          (6) //����Ƿ������Կ
#define     DDI_INNERKEY_CTL_KEY_TAK_ENCRYPT_TAK    (7) //ʹ�ù�����Կ���ܹ�����Կ
#define     DDI_INNERKEY_CTL_INJECTCERT         (8)
#define     DDI_INNERKEY_CTL_GETCERT            (9)
#define     DDI_INNERKEY_CTL_CRYPTOGRAPHY       (10) //�ӽ���
#define     DDI_INNERKEY_CTL_REQ_SECURE_DATA    (11) //���밲ȫ����
#define     DDI_INNERKEY_CTL_SET_SECURE_DATA    (12) //���ð�ȫ����
#define     DDI_INNERKEY_CTL_VERITYSIGNATURE    (13) //����У���ļ�ǩ��

//=====================================================
//���⺯������
extern s32 ddi_innerkey_open (void);
extern s32 ddi_innerkey_close(void);
extern s32 ddi_innerkey_delete(u32 nKeygroupindex,u32 nKeytype,u32 nIndex);
extern s32 ddi_innerkey_inject (u32 nKeygroupindex,u32 nKeytype,u32 nIndex, const u8* lpKeyData, u32 lpKeyDatalen,u32 ArithType);
extern s32 ddi_innerkey_encrypt(u32 nKeygroupindex,u32 nKeytype,u32 nIndex, u8* lpKeyData, u32 lpKeyDatalen,u32 ArithType);
extern s32 ddi_innerkey_decrypt(u32 nKeygroupindex,u32 nKeytype,u32 nIndex, u8* lpKeyData, u32 lpKeyDatalen,u32 ArithType);
extern s32 ddi_innerkey_ioctl(u32 nCmd, u32 lParam, u32 wParam);
extern s32 ddi_innerkey_getHW21SNinfo(u8 *randfact,u32 randfactlen,u8 *HW21SNinfo,u32 *HW21SNinfolen);

extern void ddi_DES(u8 *key,u8 keylen,u8 *dat,u32 datalen,u8 mode);
//=================================================
typedef s32 (*core_ddi_innerkey_open) (void);
typedef s32 (*core_ddi_innerkey_close)(void);
typedef s32 (*core_ddi_innerkey_inject )(u32 nKeygroupindex,u32 nKeytype,u32 nIndex, const u8* lpKeyData, u32 lpKeyDatalen,u32 ArithType);
typedef s32 (*core_ddi_innerkey_delete )(u32 nKeygroupindex,u32 nKeytype,u32 nIndex);
typedef s32 (*core_ddi_innerkey_encrypt)(u32 nKeygroupindex,u32 nKeytype,u32 nIndex, u8* lpKeyData, u32 lpKeyDatalen,u32 ArithType);
typedef s32 (*core_ddi_innerkey_decrypt)(u32 nKeygroupindex,u32 nKeytype,u32 nIndex, u8* lpKeyData, u32 lpKeyDatalen,u32 ArithType);
typedef s32 (*core_ddi_innerkey_getHW21SNinfo)(u8 *randfact,u32 randfactlen,u8 *HW21SNinfo,u32 *HW21SNinfolen);
typedef s32 (*core_ddi_innerkey_ioctl)(u32 nCmd, u32 lParam, u32 wParam);

#endif



