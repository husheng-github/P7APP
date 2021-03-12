#include "ddiglobal.h"
#include "app_global.h"

#include "ddiglobal.h"
#include "app_global.h"

static usbcdc_module_t *g_model_usbcdc = NULL;
static frameinfo_t   g_usb_frameinfo;
static u8 g_usbcdc_dl_repeat_flg;        //防止重复下载标志
static u32 g_usbcdc_dl_repeat_timerid;
#define USBCDC_DL_REPEAT_TIME       30000   //下载成功后，30S才能再次下载

/**
 * @brief 初始化本文件所有的全局变量
 */
void trendit_init_usbcdc_data(void)
{
    g_model_usbcdc = NULL;
}

static s32 exec_cfg(u16 cmd, u8 *value_string, u16 value_stringlen)
{
    s32 ret = DDI_ERR;
    
    TRACE_DBG("cmd:%04x, value_string:%s", cmd, value_string);

    return ret;
}

static s32 exec_qry(u16 cmd, u8 *ret_string, u16 ret_stringlen)
{
    s32 ret = DDI_ERR;
    
    TRACE_DBG("cmd:%04x", cmd);
    switch(cmd)
    {
        default:
            ret = DDI_EUNKOWN_CMD;
            break;
    }

    return ret;
}

static s32 exec_cmd(u16 cmd, u8 *value_string, u16 value_stringlen, u8 *ret_string, u16 ret_stringlen)
{
    s32 ret = DDI_ERR;
    
    TRACE_DBG("cmd:%04x, value_string:%s", cmd, value_string);

    return ret;
}
#define USB_TIMEOUTVALUE   100  
s32 usb_frameinfo_free(void)
{
    g_usb_frameinfo.m_commstep = 0; 
    g_usb_frameinfo.m_rxvalid = PARSEPROTOCOL_STEP_IDLE; 
    if(g_usb_frameinfo.m_rxbuf != NULL)
    {
        k_freeapp(g_usb_frameinfo.m_rxbuf);
        g_usb_frameinfo.m_rxbuf = NULL;
    }
    g_usb_frameinfo.m_rxcnt = 0;
    return 0;
}
void *usb_frameinfo_malloc(u32 nBytes)
{
    if(g_usb_frameinfo.m_rxbuf != NULL)
    {
        k_freeapp(g_usb_frameinfo.m_rxbuf);
        g_usb_frameinfo.m_rxbuf = NULL;
    }
    return k_mallocapp(nBytes);
}
#define FRAME_STATUS_IDLE           0        //空闲状态
#define FRAME_STATUS_DL_HEAD_1      1        //程序下载协议头1(0x55)
//#define FRAME_STATUS_DL_HEAD_2      2        //程序下载协议头2(0xAA)
//#define FRAME_STATUS_DL_HEAD_3      3        //程序下载协议头3(0xAA)
#define FRAME_STATUS_DL_DATA        4        //程序下载数据
#define FRAME_STATUS_DL_TAIL_0      5        //程序下载协议尾0(0x55)
//#define FRAME_STATUS_DL_TAIL_1      6        //程序下载协议尾0(0xAA)
//#define FRAME_STATUS_DL_TAIL_2      7        //程序下载协议尾0(0x55)
//#define FRAME_STATUS_DL_TAIL_3      8        //程序下载协议尾0(0xAA)
#define FRAME_STATUS_DL_CHECK       9        //CHECKSUM
const u8 frame_dl_head[4] = {0x55, 0x55, 0xAA, 0xAA};
const u8 frame_dl_tail[4] = {0x55, 0xAA, 0x55, 0xAA};
//------------------------------------------------
#define FRAME_STATUS_TRENDIT_HEAD       20       //协议头(0x02)
#define FRAME_STATUS_TRENDIT_FLG        21       //TRETEI/TRERSP
#define FRAME_STATUS_TRENDIT_SN         22       //TRETEI/TRERSP
#define FRAME_STATUS_TRENDIT_COMH       23       //命令第一个字节
#define FRAME_STATUS_TRENDIT_COML       24       //命令第二个字节
#define FRAME_STATUS_TRENDIT_LENH       25       //长度高字节
#define FRAME_STATUS_TRENDIT_LENL       26       //长度低字节
#define FRAME_STATUS_TRENDIT_DATA       27       //数据
#define FRAME_STATUS_TRENDIT_END        28       //结束
#define FRAME_STATUS_TRENDIT_CRCH       29       //CRC高字节
#define FRAME_STATUS_TRENDIT_CRCL       30       //CRC低字节
const u8 frame_trendit_flg_tei[] = "TRETEI";    //TRETEI  请求斑纹和响应报文
const u8 frame_trendit_flg_rsp[] = "TRERSP";    //TRERSP  应答报文
//------------------------------------------------
#define FRAME_STATUS_TRENBT_STA         40      //TRENDIT蓝牙协议4C
#define FRAME_STATUS_TRENBT_STX         41      //蓝牙协议头4B
#define FRAME_STATUS_TRENBT_LEN         42      //长度
#define FRAME_STATUS_TRENBT_FLG         43      //帧标志
#define FRAME_STATUS_TRENBT_CMD         44      //命令字
#define FRAME_STATUS_TRENBT_SN          45      //SN
#define FRAME_STATUS_TRENBT_DATA        46      //数据
#define FRAME_STATUS_TRENBT_EXT         47      //结束
#define FRAME_STATUS_TRENBT_LRC         48      //校验
//------------------------------------------------
s32 usb_frame_parse(void)
{
    s32 i;
    u8 data;
//    s32 ret;
    u32 timeid;
//    u8 ascflg;
    u16 crc16;
    
//    ascflg = 0;
    while(0 == g_usb_frameinfo.m_rxvalid)
    {
        ddi_watchdog_feed();
        i = ddi_com_read(PORT_CDCD_NO, &data, 1);
        ddi_misc_get_tick(&timeid);
        if(i <= 0)
        {
            if((u32)(timeid-g_usb_frameinfo.m_bwtID) > USB_TIMEOUTVALUE)
            {
                //字节间超时，
                usb_frameinfo_free();
                g_usb_frameinfo.m_bwtID = timeid;
            }
            break;
        }
        else
        {
//TRACE_DBG("step=%d,data=%02X", g_usb_frameinfo.m_commstep, data);            
            g_usb_frameinfo.m_bwtID = timeid;
            switch(g_usb_frameinfo.m_commstep)
            {
            case FRAME_STATUS_IDLE:           //0        //空闲状态
                if(g_usb_frameinfo.m_rxcnt>=(FRAMEINFO_BUF_MAX-1))
                {
                    g_usb_frameinfo.m_rxcnt = 0;
                }
                if(0x55 == data)
                {
                    g_usb_frameinfo.m_commstep  = FRAME_STATUS_DL_HEAD_1;
                    g_usb_frameinfo.m_buf[g_usb_frameinfo.m_rxcnt++] = data;
                    g_usb_frameinfo.m_rxlen = 1;
                }
                else if(0x02 == data)
                {
                    g_usb_frameinfo.m_commstep  = FRAME_STATUS_TRENDIT_FLG;
                    g_usb_frameinfo.m_rxcnt = 0;
                }
                else if(0x4C == data)
                {
                    g_usb_frameinfo.m_commstep  = FRAME_STATUS_TRENBT_STX;
                    g_usb_frameinfo.m_buf[g_usb_frameinfo.m_rxcnt++] = data;
                }
                else if((0x0a == data)|| (0x0d == data))
                {
                    if(0 != g_usb_frameinfo.m_rxcnt)
                    {
                        g_usb_frameinfo.m_buf[g_usb_frameinfo.m_rxcnt] = 0;
                        g_usb_frameinfo.m_rxlen = g_usb_frameinfo.m_rxcnt;
                        g_usb_frameinfo.m_protocoltype = PROTOCOL_TYPE_ASCII;
                        g_usb_frameinfo.m_rxvalid = PARSEPROTOCOL_STEP_RECEIVEDATASUC;  //return 1;  //收到完整的字符串
                        break;
                    }
                }
                else
                {
                    g_usb_frameinfo.m_buf[g_usb_frameinfo.m_rxcnt++] = data;
                }
                break;
            //------------------------------------------
            case FRAME_STATUS_DL_HEAD_1:      //1        //程序下载协议头1(0x55)
                if(g_usb_frameinfo.m_rxcnt>=(FRAMEINFO_BUF_MAX-1))
                {
                    g_usb_frameinfo.m_rxcnt = 0;
                }
                if(frame_dl_head[g_usb_frameinfo.m_rxlen] == data)
                {
                    g_usb_frameinfo.m_buf[g_usb_frameinfo.m_rxcnt++] = data;
                    g_usb_frameinfo.m_rxlen ++;
                    if(g_usb_frameinfo.m_rxlen>=4)
                    {
                        //收完了头
                        g_usb_frameinfo.m_rxcnt = 0;
                        g_usb_frameinfo.m_rxlen = PC_DL_FRAME_SIZE_MAX; 
                        //动态申请
                        g_usb_frameinfo.m_rxbuf = (u8*)usb_frameinfo_malloc(g_usb_frameinfo.m_rxlen); 
                        if(g_usb_frameinfo.m_rxbuf==NULL)
                        {
                            //报错
                            g_usb_frameinfo.m_commstep = FRAME_STATUS_IDLE;
                        }
                        else
                        {
                            g_usb_frameinfo.m_commstep = FRAME_STATUS_DL_DATA;
                            g_usb_frameinfo.m_check = 0;
                        }
                    }
                }
                else if(0x02 == data)
                {
                    g_usb_frameinfo.m_commstep = FRAME_STATUS_TRENDIT_FLG;
                    g_usb_frameinfo.m_rxcnt = 0;
                }
                else if((0x0a == data)|| (0x0d == data))
                {
                    if(0 != g_usb_frameinfo.m_rxcnt)
                    {
                        g_usb_frameinfo.m_buf[g_usb_frameinfo.m_rxcnt] = 0;
                        g_usb_frameinfo.m_rxlen = g_usb_frameinfo.m_rxcnt;
                        g_usb_frameinfo.m_protocoltype = PROTOCOL_TYPE_ASCII;
                        g_usb_frameinfo.m_rxvalid = PARSEPROTOCOL_STEP_RECEIVEDATASUC;  //return 1;  //收到完整的字符串
                        break;
                    }
                }
                else
                {
                    g_usb_frameinfo.m_buf[g_usb_frameinfo.m_rxcnt++] = data;
                }
                break;
            //case FRAME_STATUS_DL_HEAD_2:      //2        //程序下载协议头2(0xAA)
            //    break;
            //case FRAME_STATUS_DL_HEAD_3:      //3        //程序下载协议头3(0xAA)
            //    break;
            case FRAME_STATUS_DL_DATA:        //4        //程序下载数据
                g_usb_frameinfo.m_rxbuf[g_usb_frameinfo.m_rxcnt++] = data;
                g_usb_frameinfo.m_check += data;
                if(g_usb_frameinfo.m_rxcnt>=g_usb_frameinfo.m_rxlen)
                {
                    g_usb_frameinfo.m_commstep = FRAME_STATUS_DL_TAIL_0;
                    g_usb_frameinfo.m_rxcnt = 0;
                }
                break;
            case FRAME_STATUS_DL_TAIL_0:      //5        //程序下载协议尾0(0x55)
                if(data == frame_dl_tail[g_usb_frameinfo.m_rxcnt])
                {
                    g_usb_frameinfo.m_rxcnt++;
                    if(g_usb_frameinfo.m_rxcnt>=4)
                    {
                        g_usb_frameinfo.m_commstep = FRAME_STATUS_DL_CHECK;
                        g_usb_frameinfo.m_rxcnt = 0;
                        //break;
                    }
                }
                else
                {
                    //出错
                    usb_frameinfo_free();
                }
                break;
           #if 0
            case FRAME_STATUS_DL_TAIL_1:      //6        //程序下载协议尾0(0xAA)
                break;
            case FRAME_STATUS_DL_TAIL_2:      //7        //程序下载协议尾0(0x55)
                break;
            case FRAME_STATUS_DL_TAIL_3:      //8        //程序下载协议尾0(0xAA)
                break;
          #endif
            case FRAME_STATUS_DL_CHECK:       //9        //CHECKSUM
                g_usb_frameinfo.m_commstep = 0;
                if(data == ((u8)(g_usb_frameinfo.m_check&0xff)))
                {
                    //收到完整的包
                    g_usb_frameinfo.m_protocoltype = PROTOCOL_TYPE_55AA;
                    g_usb_frameinfo.m_rxvalid = PARSEPROTOCOL_STEP_RECEIVEDATASUC;//1;
                    //return 1;           //收到完整的字符串
                }
                else
                {
                    //接收错误
                    usb_frameinfo_free();
                }
                break;
            //------------------------------------------------
            //case FRAME_STATUS_TRENDIT_HEAD:      // 20       //协议头(0x02)
            //    break;
            case FRAME_STATUS_TRENDIT_FLG:        //21       //TRETEI/TRERSP
                if((data == frame_trendit_flg_tei[g_usb_frameinfo.m_rxcnt])
                 ||(data == frame_trendit_flg_rsp[g_usb_frameinfo.m_rxcnt]))
                {
                    g_usb_frameinfo.m_buf[g_usb_frameinfo.m_rxcnt++] = data;
                    if(6 == g_usb_frameinfo.m_rxcnt)
                    {
                        if(0 == memcmp(g_usb_frameinfo.m_buf, frame_trendit_flg_tei, 6))
                        {
                            g_usb_frameinfo.m_frametype = FRAME_TYPE_TEI_REQ;
                        }
                        else if(0 == memcmp(g_usb_frameinfo.m_buf, frame_trendit_flg_rsp, 6))
                        {
                            g_usb_frameinfo.m_frametype = FRAME_TYPE_TEI_RSP;
                        }
                        else
                        {
                            //出错
                            g_usb_frameinfo.m_commstep = FRAME_STATUS_IDLE;
                            g_usb_frameinfo.m_rxcnt = 0;
                            break;
                        }
                        g_usb_frameinfo.m_commstep = FRAME_STATUS_TRENDIT_SN;
                    }
                }
                else
                {
                    g_usb_frameinfo.m_commstep = FRAME_STATUS_IDLE;
                    g_usb_frameinfo.m_rxcnt = 0;
                }
                break;
            case FRAME_STATUS_TRENDIT_SN:         //22       //TRETEI/TRERSP
                g_usb_frameinfo.m_sn = data;
                g_usb_frameinfo.m_commstep = FRAME_STATUS_TRENDIT_COMH;
                break;
            case FRAME_STATUS_TRENDIT_COMH:       //23       //命令第一个字节
                g_usb_frameinfo.m_cmd = (((u16)data)<<8);
                g_usb_frameinfo.m_buf[0] = data;
                g_usb_frameinfo.m_commstep = FRAME_STATUS_TRENDIT_COML;
                break;
            case FRAME_STATUS_TRENDIT_COML:       //24       //命令第二个字节
                g_usb_frameinfo.m_cmd += data;
                g_usb_frameinfo.m_buf[1] = data;
                g_usb_frameinfo.m_commstep = FRAME_STATUS_TRENDIT_LENH;
                break;
            case FRAME_STATUS_TRENDIT_LENH:       //25       //长度高字节
                g_usb_frameinfo.m_rxlen = (((u16)data)<<8);
                g_usb_frameinfo.m_buf[2] = data;
                g_usb_frameinfo.m_commstep = FRAME_STATUS_TRENDIT_LENL;
                break;
            case FRAME_STATUS_TRENDIT_LENL:       //26       //长度低字节
                g_usb_frameinfo.m_rxlen += data;
                g_usb_frameinfo.m_buf[3] = data;
                g_usb_frameinfo.m_rxcnt = 0;
                if(g_usb_frameinfo.m_rxlen<=4096)    //根据协议，长度大于4096
                {
                    g_usb_frameinfo.m_rxbuf = (u8*)usb_frameinfo_malloc(g_usb_frameinfo.m_rxlen);
                    if(g_usb_frameinfo.m_rxbuf != NULL)
                    {
                        g_usb_frameinfo.m_commstep = FRAME_STATUS_TRENDIT_DATA;
                        break;
                    }
                }
                //出错
                g_usb_frameinfo.m_commstep = FRAME_STATUS_IDLE;
                break;
            case FRAME_STATUS_TRENDIT_DATA:       //27       //数据
                g_usb_frameinfo.m_rxbuf[g_usb_frameinfo.m_rxcnt++] = data;
                if(g_usb_frameinfo.m_rxcnt >= g_usb_frameinfo.m_rxlen)
                {
                    g_usb_frameinfo.m_commstep = FRAME_STATUS_TRENDIT_END;
                }
                break;
            case FRAME_STATUS_TRENDIT_END:        //28       //结束
                if(data != 0x03)
                {
                    usb_frameinfo_free();
                }
                else
                {
                    g_usb_frameinfo.m_commstep = FRAME_STATUS_TRENDIT_CRCH;
                }
                break;
            case FRAME_STATUS_TRENDIT_CRCH:       //29       //CRC高字节
                g_usb_frameinfo.m_check = (((u16)data)<<8);
                g_usb_frameinfo.m_commstep = FRAME_STATUS_TRENDIT_CRCL;
                break;
            case FRAME_STATUS_TRENDIT_CRCL:       //30       //CRC低字节
                g_usb_frameinfo.m_check += data;
                //计算CRC
                crc16 = trendit_Crc16CCITT(0, g_usb_frameinfo.m_buf, 4, NULL);
                crc16 = trendit_Crc16CCITT(crc16, g_usb_frameinfo.m_rxbuf, g_usb_frameinfo.m_rxlen, NULL);
                if(crc16 == g_usb_frameinfo.m_check)
                {
                    //校验成功
                    g_usb_frameinfo.m_rxvalid = PARSEPROTOCOL_STEP_RECEIVEDATASUC;//1
                }
                else
                {
                    g_usb_frameinfo.m_rxvalid = PARSEPROTOCOL_STEP_RECEIVEDATAERR;//2;      //check err;
                }
                g_usb_frameinfo.m_protocoltype = PROTOCOL_TYPE_TEI;
                g_usb_frameinfo.m_commstep = FRAME_STATUS_IDLE;
                //return 1;
                break;
            //----------------------------------------------
            case FRAME_STATUS_TRENBT_STA:         //40      //TRENDIT蓝牙协议4C
                
                break;
            case FRAME_STATUS_TRENBT_STX:         //41      //蓝牙协议头4B
                if(g_usb_frameinfo.m_rxcnt>=(FRAMEINFO_BUF_MAX-1))
                {
                    g_usb_frameinfo.m_rxcnt = 0;
                }
                if(0x4B == data)
                {
                    g_usb_frameinfo.m_buf[g_usb_frameinfo.m_rxcnt++] = data;
                    g_usb_frameinfo.m_rxlen ++;
                    g_usb_frameinfo.m_commstep = FRAME_STATUS_TRENBT_LEN;
                    g_usb_frameinfo.m_rxcnt = 0;
                    g_usb_frameinfo.m_check = 0;
                }
                else if(0x02 == data)
                {
                    g_usb_frameinfo.m_commstep = FRAME_STATUS_TRENDIT_FLG;
                    g_usb_frameinfo.m_rxcnt = 0;
                }
                else if((0x0a == data)|| (0x0d == data))
                {
                    if(0 != g_usb_frameinfo.m_rxcnt)
                    {
                        g_usb_frameinfo.m_buf[g_usb_frameinfo.m_rxcnt] = 0;
                        g_usb_frameinfo.m_rxlen = g_usb_frameinfo.m_rxcnt;
                        g_usb_frameinfo.m_protocoltype = PROTOCOL_TYPE_ASCII;
                        g_usb_frameinfo.m_rxvalid = PARSEPROTOCOL_STEP_RECEIVEDATASUC;  //return 1;  //收到完整的字符串
                        break;
                    }
                }
                else
                {
                    g_usb_frameinfo.m_commstep = FRAME_STATUS_IDLE;
                    g_usb_frameinfo.m_buf[g_usb_frameinfo.m_rxcnt++] = data;
                }
                break;
            case FRAME_STATUS_TRENBT_LEN:         //42      //长度
                g_usb_frameinfo.m_check ^= data;
                if(g_usb_frameinfo.m_rxcnt == 0)
                {
                    g_usb_frameinfo.m_rxlen = ((u16)data)<<8;
                    g_usb_frameinfo.m_rxcnt = 1;
                }
                else
                {
                    g_usb_frameinfo.m_rxlen += data;
                    if((g_usb_frameinfo.m_rxlen>=4)&&(g_usb_frameinfo.m_rxlen<=(PC_DL_BLOCK_SIZE+4)))
                    {
                        g_usb_frameinfo.m_rxlen -= 4;   //去掉指示位、指令号、序列号
                        g_usb_frameinfo.m_rxcnt = 0;
                        if(g_usb_frameinfo.m_rxlen!=0)
                        {
                            g_usb_frameinfo.m_rxbuf = usb_frameinfo_malloc(g_usb_frameinfo.m_rxlen);
                            if(g_usb_frameinfo.m_rxbuf != NULL)
                            {
                                g_usb_frameinfo.m_commstep = FRAME_STATUS_TRENBT_FLG;
                                break;
                            }
                        }
                        else
                        {
                            g_usb_frameinfo.m_commstep = FRAME_STATUS_TRENBT_FLG;
                            break;
                        }
                    }
                    usb_frameinfo_free();
                }
                break;
            case FRAME_STATUS_TRENBT_FLG:         //43      //帧标志
                g_usb_frameinfo.m_check ^= data;
                if((FRAME_TYPE_TRENBT_REQ == data)      //0x2F
                 ||(FRAME_TYPE_TRENBT_NOTIFY == data)   //0x3F
                 ||(FRAME_TYPE_TRENBT_RSP == data)      //0x4F
                 ||(FRAME_TYPE_TRENBT_REPORT == data)   //0x5F
                 ||(FRAME_TYPE_TRENBT_ACK == data))     //0x6F
                {
                    g_usb_frameinfo.m_frametype = data;
                    g_usb_frameinfo.m_commstep = FRAME_STATUS_TRENBT_CMD;
                    g_usb_frameinfo.m_rxcnt = 0;
                }
                else
                {
                    usb_frameinfo_free();
                }
                break;
            case FRAME_STATUS_TRENBT_CMD:         //44      //命令字
                g_usb_frameinfo.m_check ^= data;
                if(0 == g_usb_frameinfo.m_rxcnt)
                {
                    g_usb_frameinfo.m_rxcnt = 1;
                    g_usb_frameinfo.m_cmd = ((u16)data)<<8;
                }
                else
                {
                    g_usb_frameinfo.m_cmd += data;
                    g_usb_frameinfo.m_rxcnt = 0;
                    g_usb_frameinfo.m_commstep = FRAME_STATUS_TRENBT_SN;
                }
                break;
            case FRAME_STATUS_TRENBT_SN:          //45      //SN
                g_usb_frameinfo.m_check ^= data;
                g_usb_frameinfo.m_sn = data;
                if(g_usb_frameinfo.m_rxlen>0)
                {
                    g_usb_frameinfo.m_commstep = FRAME_STATUS_TRENBT_DATA;
                }
                else
                {
                    g_usb_frameinfo.m_commstep = FRAME_STATUS_TRENBT_EXT;
                }
                g_usb_frameinfo.m_rxcnt = 0;
                break;
            case FRAME_STATUS_TRENBT_DATA:        //46      //数据
                g_usb_frameinfo.m_check ^= data;
                g_usb_frameinfo.m_rxbuf[g_usb_frameinfo.m_rxcnt++] = data;
                if(g_usb_frameinfo.m_rxcnt >= g_usb_frameinfo.m_rxlen)
                {
                    g_usb_frameinfo.m_commstep = FRAME_STATUS_TRENBT_EXT;
                }
                break;
            case FRAME_STATUS_TRENBT_EXT:         //47      //结束
                if(0x03 == data)
                {
                    g_usb_frameinfo.m_commstep = FRAME_STATUS_TRENBT_LRC;
                    g_usb_frameinfo.m_check ^= data;
                }
                else
                {
                    usb_frameinfo_free();
                }
                break;
            case FRAME_STATUS_TRENBT_LRC:         //48      //校验
                //TRACE_DBG("lrc:%x, cmd:%x", (u8)(g_usb_frameinfo.m_check&0xFF), g_usb_frameinfo.m_cmd);
                if(((u8)(g_usb_frameinfo.m_check&0xFF))==data)
                {
                    //校验成功
                    g_usb_frameinfo.m_rxvalid = PARSEPROTOCOL_STEP_RECEIVEDATASUC;//1;
                }
                else
                {
                    g_usb_frameinfo.m_rxvalid = PARSEPROTOCOL_STEP_RECEIVEDATAERR;//2;
                }
                g_usb_frameinfo.m_protocoltype = PROTOCOL_TYPE_TRENBT;
                g_usb_frameinfo.m_commstep = FRAME_STATUS_IDLE;
                break;
            //--------------------------------------------
            default:
                usb_frameinfo_free();
                break;
            }
        }
    }
    return g_usb_frameinfo.m_rxvalid;
}
frameinfo_t *usb_frame_parse_getinfo(void)
{
    return &g_usb_frameinfo;
}

static s32 usb_frame_deal_ASCIIcom(u8* wbuf, u32 wlen)
{
    s8 dst[2][SPLIT_PER_MAX] = {0};
    str_audio_tts audio_tts;
    s32 ret;

    if(NULL != strstr(wbuf, "test"))
    {
        TRACE_INFO("test enter:%s,%d", wbuf, wlen);
        testmain();
    }
    else if(NULL != strstr(wbuf, "serverip:"))
    {
        trendit_split(dst, 2, wbuf+strlen("serverip:"), ",", SPLIT_NOIGNOR);
        //trendit_cfg_server_info(dst[0], trendit_atoi(dst[1]));
        tcp_protocol_instance()->cfg_server_info(dst[0], trendit_atoi(dst[1]));
                
    }
    else if(NULL != strstr(wbuf, "opendebug"))
    {
        trendit_cdc_output_switch(TRUE);
        TRACE_INFO("open debug output");
    }
    else if(NULL != strstr(wbuf, "closedebug"))
    {
        TRACE_INFO("close debug output");
        trendit_cdc_output_switch(FALSE);
    }
    else if(NULL != strstr(wbuf,"print:"))
    {
        printer_instance()->direct_print(wbuf+strlen("print:"), wlen-strlen("print:"));
    }
    usb_frameinfo_free();
    return 0;
}

/**
 * @brief 下载结果的返回
 * @param[in] ret:下载结果
 * @retval  返回长度
 */
s32 bootdownload_sendresponse(s32 ret)
{
    
    u8 senddata[20];
    u8 senddatalen;
    
    memcpy(senddata,"\x55\x55\xaa\x03",4);
    senddata[4] = ret;
    senddata[5] = trendit_get_machine_code();
    senddata[6] = ddi_misc_ioctl(MISC_IOCTL_GETHWVER,0,0);
    memcpy(&senddata[7],"\x55\xaa\x55\xaa",4);

    TRACE_DBG_HEX(senddata, 11);
    return ddi_com_write(PORT_CDCD_NO, senddata, 11);
}

static s32 usb_handle(struct _msg *m_msg)
{
    s32 ret = MSG_KEEP;
    strComAttr lpstrAttr;
    s8 dst[2][SPLIT_PER_MAX] = {0};
    str_audio_tts audio_tts;
#if 0 	
    u32 print_start_tick = 0;
    u32 print_end_tick = 0;
    u32 print_len = 0;
#endif
    do{
        if(MSG_TERMINAL == m_msg->m_status)
        {
            TRACE_INFO("terminal this message");
            ret = MSG_OUT;
            break;
        }
        else if(MSG_INIT == m_msg->m_status)
        {
            memset(&lpstrAttr,0,sizeof(strComAttr));
            lpstrAttr.m_baud = 115200;
            lpstrAttr.m_databits = 8;
            lpstrAttr.m_parity = 0;
            lpstrAttr.m_stopbits = 1;
            ret = ddi_com_open(PORT_CDCD_NO, &lpstrAttr);

            if(DDI_OK != ret)
            {
                TRACE_ERR("open fail:%d", ret);
                ret = MSG_OUT;
                break;
            }

            TRACE_INFO("open cdc port success");
            m_msg->m_status = MSG_SERVICE;
            ret = MSG_KEEP;
            break;
        }
        else
        {
            ret = usb_frame_parse();
            if(ret)
            {
                switch(g_usb_frameinfo.m_protocoltype)
                {
                case PROTOCOL_TYPE_55AA:
                    if(trendit_query_timer(g_usbcdc_dl_repeat_timerid, USBCDC_DL_REPEAT_TIME))
                    {
                        g_usbcdc_dl_repeat_flg = 0;
                    }
                    if(g_usbcdc_dl_repeat_flg == 0)   
                    {
                        ret  = dl_deal_main();
                        if(ret == 0)
                        {
                            //文件下载成功，防止再次下载，必须等USBCDC_DL_REPEAT_TIME才能下
                            g_usbcdc_dl_repeat_flg  = 1;
                            g_usbcdc_dl_repeat_timerid = trendit_get_ticks();
                        }
                    }
                    break;
                case PROTOCOL_TYPE_TEI:
                    break;
                case PROTOCOL_TYPE_TRENBT:
                    process_trenbtcmd();
                    break;
                case PROTOCOL_TYPE_ASCII:
                    usb_frame_deal_ASCIIcom(g_usb_frameinfo.m_buf, g_usb_frameinfo.m_rxlen);
                    break;
                }
                usb_frameinfo_free();
            }
            ret = MSG_KEEP;
        }
    }while(0);

    return ret;
}

static s32 post_usbcdc_message()
{
    msg_t *usb_msg = NULL;
    s32 ret = DDI_ERR;

    usb_msg = (msg_t *)k_mallocapp(sizeof(msg_t));
    if(NULL == usb_msg)
    {
        TRACE_ERR("malloc failed");
    }
    else
    {
        TRACE_DBG("usbcdc:%x", usb_msg);
        memset(usb_msg, 0, sizeof(msg_t));
        usb_msg->m_func = usb_handle;
        usb_msg->m_priority = MSG_NORMAL;
        usb_msg->m_lParam = 0;
        usb_msg->m_wparam = 0;
        usb_msg->m_status = MSG_INIT;
        snprintf(usb_msg->m_msgname, sizeof(usb_msg->m_msgname), "%s", "usbcdc");
        trendit_postmsg(usb_msg);
    }

    return ret;
}

/**
 * @brief usbcdc的初始化函数
 * @retval  usbcdc的设备结构体
 */
usbcdc_module_t *usbcdc_instance(void)
{
    if(g_model_usbcdc)
    {
        return g_model_usbcdc;
    }

    g_model_usbcdc = k_mallocapp(sizeof(usbcdc_module_t));
    if(NULL == g_model_usbcdc)
    {
        TRACE_ERR("malloc fail");
        return NULL;
    }
    
    memset(g_model_usbcdc, 0, sizeof(usbcdc_module_t));
    INIT_SUPER_METHOD(g_model_usbcdc, MODEL_USBCDC, "usbcdc");

    post_usbcdc_message();
    ddi_misc_get_tick(&g_usb_frameinfo.m_bwtID);
    g_usb_frameinfo.m_rxvalid = 0;
    g_usb_frameinfo.m_commstep = 0;
    g_usb_frameinfo.m_rxcnt = 0;
    g_usbcdc_dl_repeat_flg = 0;
    return g_model_usbcdc;
}


