

#ifndef _DDI_COM_H_
#define _DDI_COM_H_


#include "ddi_common.h"



typedef struct _strComAttr
{
    s32 m_baud;     //波特率，9600、19200、38400、115200
    s32 m_databits;     //数据位，取值：5、6、7、8
    s32 m_parity;       //校验方式：0-无校验；1-奇校验；2-偶校验
    s32 m_stopbits;     //停止位：1或2
}strComAttr;


/*串口控制命令*/
#define     DDI_COM_CTL_BAUD            (0) //设置波特率
#define     DDI_COM_CTL_FLWCTL          (1) //设置流控


typedef enum _ComPortNo
{
    PORT_UART0_NO = 0,
    PORT_UART1_NO = 1,
    PORT_UART2_NO = 2,
    PORT_UART3_NO = 3,
    
    PORT_HID_NO   = 4,
    PORT_CDCD_NO  = 5,
    PORT_SPI_NO   = 6,

    PORT_PRINT_NO = 7,
    PORT_NO_MAX   = 8,
    PORT_UART_PHY_MAX = 4,
}ComPortNo_t;



//=====================================================
//对外函数声明
extern s32 ddi_com_open (u32 nCom, const strComAttr *lpstrAttr);
extern s32 ddi_com_close (u32 nCom);
extern s32 ddi_com_clear (u32 nCom);
extern s32 ddi_com_tcdrain (u32 nCom);
extern s32 ddi_com_read (u32 nCom, u8 *lpOut, s32 nLe );
extern s32 ddi_com_write (u32 nCom, u8 *lpIn, s32 nLe );
extern s32 ddi_com_ioctl(u32 nCom, u32 nCmd, u32 lParam, u32 wParam);

//=======================================================
typedef s32 (*core_ddi_com_open) (u32 nCom, const strComAttr *lpstrAttr);
typedef s32 (*core_ddi_com_close) (u32 nCom);
typedef s32 (*core_ddi_com_clear) (u32 nCom);
typedef s32 (*core_ddi_com_read) (u32 nCom, u8 *lpOut, s32 nLe );
typedef s32 (*core_ddi_com_write) (u32 nCom, u8 *lpIn, s32 nLe );
typedef s32 (*core_ddi_com_ioctl)(u32 nCom, u32 nCmd, u32 lParam, u32 wParam);
typedef s32 (*core_ddi_com_tcdrain)(u32 nCom);




#endif

