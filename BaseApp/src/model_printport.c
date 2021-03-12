#include "ddiglobal.h"
#include "app_global.h"

#include "ddiglobal.h"
#include "app_global.h"

static printport_module_t *g_model_printport = NULL;

/**
 * @brief 初始化本文件所有的全局变量
 */
void trendit_init_printport_data(void)
{
    g_model_printport = NULL;
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
s32 printport_deal(void)
{
    s32 i;
    u8 tmp[4];
    s32 status;
    printer_module_t *model_printer = printer_instance();


    if(g_model_printport->m_rxcnt >= PRINT_BUFF_SIZE)
    {
        g_model_printport->m_rxcnt = 0;
    }
    
    i = ddi_com_read(PORT_PRINT_NO, &g_model_printport->m_print_buff[g_model_printport->m_rxcnt], PRINT_BUFF_SIZE-g_model_printport->m_rxcnt);
    if(i>0)
    {
        g_model_printport->m_rxcnt += i;
        g_model_printport->m_bwtID = trendit_get_ticks();
    }
    if((g_model_printport->m_rxcnt >= PRINT_BUFF_SIZE)
     ||((g_model_printport->m_rxcnt!=0)&&(1 == trendit_query_timer(g_model_printport->m_bwtID, PRINT_RX_BWT_MAX))))
    {
       // tcp_protocol_instance()->send(g_model_printport->m_print_buff, g_model_printport->m_rxcnt);
       // model_printer->direct_print(g_model_printport->m_print_buff, g_model_printport->m_rxcnt);
       
        #if 1
        //解析数据
        if((3 == g_model_printport->m_rxcnt) 
         &&(0 == memcmp(g_model_printport->m_print_buff, "\x1d\x72\x01",3)))
        {   //连接测试
            tmp[0] = 0;
            g_model_printport->write(tmp, 1);
        }
        else if((3 == g_model_printport->m_rxcnt) 
         &&(0 == memcmp(g_model_printport->m_print_buff, "\x1d\x72\x02",3)))
        {   //打开钱箱
            tmp[0] = 0;
            g_model_printport->write(tmp, 1);
         }
        else if((2 == g_model_printport->m_rxcnt) 
         &&(0 == memcmp(g_model_printport->m_print_buff, "\x12\x57",2)))
        {
            //配置信息打印
            
        }
        else if((0 == memcmp(g_model_printport->m_print_buff, "\x1B\x53",2)))
        {
            //打印文本
            
            model_printer->direct_print(g_model_printport->m_print_buff + 2, g_model_printport->m_rxcnt - 2);
        }
        else if((3 == g_model_printport->m_rxcnt) 
         &&(0 == memcmp(g_model_printport->m_print_buff, "\x10\x04\x04",3)))
        {   //打印机状态
            status = model_printer->check_paper();
            if(status == TRUE)
            {
                tmp[0] = 0x72;
            }
            else
            {
                tmp[0] = 0x12;
            }
            g_model_printport->write(tmp, 1);
         }
        else if((3 == g_model_printport->m_rxcnt) 
         &&(0 == memcmp(g_model_printport->m_print_buff, "\x10\x04\x01",3)))
        {   
            /*TODO 这个地方是否还需要判断？*/
            status = model_printer->check_paper();  
            if(status == TRUE)
            {
                tmp[0] = 0x1E;
            }
            else
            {
                tmp[0] = 0x16;
            }
            g_model_printport->write(tmp, 1);
         }
        else if((3 == g_model_printport->m_rxcnt) 
         &&(0 == memcmp(g_model_printport->m_print_buff, "\x10\x04\x02",3)))
        {   //
            tmp[0] = 0x32;
            g_model_printport->write(tmp, 1);
         }
        else if((3 == g_model_printport->m_rxcnt) 
         &&(0 == memcmp(g_model_printport->m_print_buff, "\x10\x04\x03",3)))
        {   //
            tmp[0] = 0x12;
            g_model_printport->write(tmp, 1);
         }
        else if((0 == memcmp(g_model_printport->m_print_buff, "\x1c\x71\x01",3)))
        {   //下载图片
            tmp[0] = 0x31;
            g_model_printport->write(tmp, 1);
         }
        else if(g_model_printport->m_print_buff[0] == 0x33)
        {
            //测试dev_usbd_print_write函数
            g_model_printport->write(g_model_printport->m_print_buff, g_model_printport->m_rxcnt);
        }
        else  //钱豹收银系统 格式
        {
            model_printer->direct_print(g_model_printport->m_print_buff, g_model_printport->m_rxcnt);
        }
        //dev_com_write(TESTPORT0, g_printbuf, g_printcnt); 
         #endif
        //if(i!=0)
        {
            TRACE_DBG("readlen=%d\r\n", g_model_printport->m_rxcnt);
            TRACE_DBG_HEX(g_model_printport->m_print_buff, g_model_printport->m_rxcnt);
        }
       
        g_model_printport->m_rxcnt = 0;
     }
     return 0;
}
static s32 printport_handle(struct _msg *m_msg)
{
    s32 ret = MSG_KEEP;
    s32 len = 0;
    strComAttr lpstrAttr;
    u32 cur_ticks = 0;

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
            ret = ddi_com_open(PORT_PRINT_NO, &lpstrAttr);
            if(DDI_OK != ret)
            {
                TRACE_ERR("open fail:%d", ret);
                ret = MSG_OUT;
                break;
            }
            
            TRACE_INFO("open print port success");
            m_msg->m_status = MSG_SERVICE;
            ret = MSG_KEEP;
            break;
        }
        else
        {
          #if 0
            memset(g_model_printport->m_print_buff, 0, PRINT_BUFF_SIZE);
            len = 0;
            ret = ddi_com_read(PORT_CDCD_NO, g_model_printport->m_print_buff, PRINT_BUFF_SIZE);
            if(ret > 0)
            {
                cur_ticks = trendit_get_ticks();
                len = ret;
                while(1)
                {
                    ret = ddi_com_read(PORT_CDCD_NO, g_model_printport->m_print_buff+len, PRINT_BUFF_SIZE-len);
                    if(ret > 0)
                    {
                        cur_ticks = trendit_get_ticks();
                        len += ret;
                    }
                    else if(trendit_query_timer(cur_ticks, 500))
                    {
                        TRACE_DBG("len:%d", len);
                        printer_instance()->direct_print(g_model_printport->m_print_buff, len);
                    }
                }
            }
            #endif
            printport_deal();
            
            ret = MSG_KEEP;
        }
    }while(0);

    return ret;
}

static s32 post_printport_message()
{
    msg_t *printport_msg = NULL;
    s32 ret = DDI_ERR;

    printport_msg = (msg_t *)k_mallocapp(sizeof(msg_t));
    if(NULL == printport_msg)
    {
        TRACE_ERR("malloc failed");
    }
    else
    {
        TRACE_DBG("printport:%x", printport_msg);
        memset(printport_msg, 0, sizeof(msg_t));
        printport_msg->m_func = printport_handle;
        printport_msg->m_priority = MSG_NORMAL;
        printport_msg->m_lParam = 0;
        printport_msg->m_wparam = 0;
        printport_msg->m_status = MSG_INIT;
        snprintf(printport_msg->m_msgname, sizeof(printport_msg->m_msgname), "%s", "printport");
        trendit_postmsg(printport_msg);
    }

    return ret;
}


/**
 * @brief 打印机端口写数据
 * @param[in] write_buff:需写的数据buff
 * @param[in] write_buff_len:需写的数据buff长度
 * @retval  写成功的长度
 * @DDI_EINVAL 参数错误
 */
s32 printport_write(u8 *write_buff, s32 write_buff_len)
{
    s32 ret = DDI_ERR;
    
    do{
        if(NULL == write_buff || 0 == write_buff_len)
        {
            ret = DDI_EINVAL;
            break;
        }

        ret = ddi_com_write(PORT_PRINT_NO, write_buff, write_buff_len);
    }while(0);

    return ret;
}

/**
 * @brief printport的初始化函数
 * @retval  printport的设备结构体
 */
printport_module_t *printport_instance(void)
{
    if(g_model_printport)
    {
        return g_model_printport;
    }

    g_model_printport = k_mallocapp(sizeof(printport_module_t));
    if(NULL == g_model_printport)
    {
        TRACE_ERR("malloc fail");
        return NULL;
    }
    memset(g_model_printport, 0, sizeof(printport_module_t));

    g_model_printport->m_print_buff = (u8 *)k_mallocapp(PRINT_BUFF_SIZE);
    if(NULL == g_model_printport->m_print_buff)
    {
        TRACE_ERR("malloc failed");
        k_freeapp(g_model_printport);
        return NULL;
    }
    memset(g_model_printport->m_print_buff, 0, PRINT_BUFF_SIZE);
    
    INIT_SUPER_METHOD(g_model_printport, MODEL_PRINTPORT, "printport");
    g_model_printport->write = printport_write;

    post_printport_message();
    g_model_printport->super.load_cfg(&(g_model_printport->super), PRINTPORT_CONFIG_FILE, exec_cfg);
    
    return g_model_printport;
}


