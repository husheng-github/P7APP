#include "ddiglobal.h"
#include "app_global.h"

static printer_module_t *g_model_printer = NULL;
static u16 g_last_printer_packetno = 0;              //上一次打印的包序号

/**
 * @brief 开机时或者上线时调用，上线时调用是因服务器宕机后内存里的序号会清零
 */
void init_last_print_packno(void)
{
    g_last_printer_packetno = 0x01ff;
}

/**
 * @brief 初始化本文件所有的全局变量
 */
void trendit_init_printer_data(void)
{
    g_model_printer = NULL;
    init_last_print_packno();
}

/**
 * @brief 获取文本末尾换行符的个数
 * @param[in] value_string 文本内容
 * @param[in] value_string_len 文本内容长度
 * @retval  文本末尾换行符个数
 */
static s32 get_conent_enter_times(u8 *value_string, u16 value_string_len)
{
    s8 *pstr = value_string+value_string_len;
    s32 len = 0;
    s32 times = 0;

    if(value_string_len < 2)
    {
        return 0;
    }

    while(1)
    {
        pstr = pstr - 2;
        if(0 != memcmp(pstr, "\r\n", 2))
        {
            break;
        }

        len += strlen("\r\n");
        times++;
        if(len >= value_string_len)
        {
            break;
        }
    }

    return times;
}


static s32 cmd_command_print(u8 *value_string, u16 value_string_len, u8 *ret_string, u16 ret_stringlen)
{
    s32 ret = DDI_ERR;
    s8 dst[2][SPLIT_PER_MAX];
    s32 print_times = 0;
    s32 enter_times = get_conent_enter_times(value_string, value_string_len);   //换行符的个数
    s32 cur_print_len = 0;

    do{
        memset(dst, 0, sizeof(dst));
        ret = trendit_split(dst, sizeof(dst)/sizeof(dst[0]), value_string, ",", SPLIT_NOIGNOR);
        if(sizeof(dst)/sizeof(dst[0]) != ret)
        {
            TRACE_ERR("need two args");
            ret = DDI_EINVAL;
            break;
        }

        print_times = trendit_atoi(dst[0]);
        if(print_times > 5 || print_times < 1)
        {
            TRACE_ERR("print times is invalid:%d", print_times);
            ret = DDI_EINVAL;
            break;
        }

        ret = printer_instance()->direct_print_times(value_string+2, value_string_len-2, print_times, &cur_print_len);
        if(DDI_OK == ret)
        {
            snprintf(ret_string, ret_stringlen, "%d", cur_print_len);
        }
    }while(0);

    return ret;
}


static s32 cmd_command_esc_pos(u8 *value_string, u16 value_string_len, u8 *ret_string, u16 ret_stringlen)
{
    s32 ret = DDI_ERR;
    s8 *pstr = NULL;
    s32 i = 0;
    u32 print_len;

    //TRACE_DBG_HEX(value_string, value_string_len);
    do{
        if(0 == value_string_len || value_string == NULL)
        {
            ret = DDI_EINVAL;
            break;
        }

        ret = printer_instance()->direct_print(value_string,value_string_len);
    }while(0);

    return ret;
}


static s32 exec_cfg(u16 cmd, u8 *value_string, u16 value_stringlen)
{
    s32 ret = DDI_ERR;
    s32 val = 0;
    
    TRACE_DBG("cmd:%04x, value_string[0]:%02x, value_string[1]:%02x", cmd, value_string[0], value_string[1]);
    switch(cmd)
    {
        case CFG_COMAND_PRINT_GRAY:
            val = trendit_atoi(value_string);
            ret =ddi_thmprn_ioctl(DDI_THMPRN_CTL_GRAY, val, 0);
            if(DDI_OK == ret)
            {
                printer_instance()->m_print_gray = val;
            }

            if(printer_instance()->m_load_cfg)
            {
                if(ret == DDI_OK)
                {
                    audio_instance()->audio_play(AUDIO_CFG_SUCCESS, AUDIO_PLAY_BLOCK);
                }
                else
                {
                    audio_instance()->audio_play(AUDIO_CFG_FAIL, AUDIO_PLAY_BLOCK);
                }
            }
            break;

        default:
            break;
    }

    return ret;
}

static s32 exec_qry(u16 cmd, u8 *ret_string, u16 ret_stringlen)
{
    s32 ret = DDI_ERR;
    s32 status;
    u32 print_len;
    
    TRACE_DBG("cmd:%04x", cmd);
    switch(cmd)
    {
        case QRY_COMMAN_PRINT_PAPER_DISTANCE:            
            print_len = printer_instance()->m_print_len;
            sprintf(ret_string,"%u",print_len);
            ret = DDI_OK;
            break;

        case QRY_COMMAN_PRINT_STATUS:
            status = ddi_thmprn_ioctl(DDI_THMPRN_CTL_GET_PAPER_STATUS, 0, 0);
            if(DDI_EACCES == status)
            {
                //ret_string[0] = '2';
                strcpy(ret_string,"2");
                ret_stringlen = 1;
            }
            else if(DDI_EBUSY == status)
            {
                //ret_string[0] = '1';
                strcpy(ret_string,"1");
                ret_stringlen = 1;
            } 
            else if(DDI_EHIGHTEMP == status)
            {
                strcpy(ret_string,"3");
                ret_stringlen = 1;
            }
            else
            {
                //ret_string[0] = '0';
                strcpy(ret_string,"0");
                ret_stringlen = 1;
            }
            TRACE_DBG("print status:%s", ret_string);
            ret = DDI_OK;
            break;

        default:
            ret = DDI_EUNKOWN_CMD;
            break;
    }

    return ret;
}

static s32 exec_cmd(u16 cmd, u8 *value_string, u16 value_stringlen, u8 *ret_string, u16 ret_stringlen)
{
    s32 ret = DDI_ERR;
    u16 cur_packno = data_parser_instance()->get_current_packno();

    TRACE_DBG("cmd:%04x, value_string:%s", cmd, value_string);
    if(0x1ff == g_last_printer_packetno || cur_packno > g_last_printer_packetno || g_last_printer_packetno - cur_packno > 0x6f)
    {
        
    }
    else
    {
        TRACE_ERR("same exist no, last no:%x, cur no:%x", g_last_printer_packetno, cur_packno);
        ret = DDI_EHAVE_DONE;
        return ret;
    }
    
    switch(cmd)
    {
        case CMD_COMAND_PRINT:
            ret = cmd_command_print(value_string, value_stringlen, ret_string, ret_stringlen);
            break;

        default:
            break;
    }

    if(DDI_OK == ret)  //打印成功后再保存包序号，因缺纸后，服务器重发的打印包序号还是之前的
    {
        g_last_printer_packetno = cur_packno;
    }

    return ret;
}

/**
 * @brief 给外部调用直接打印函数，如走纸键策略、usb口数据
 * @param[in] print_string esc指令数据
 * @param[in] print_string_len esc指令数据长度
 * @retval  DDI_OK 打印成功
 * @retval  <0 失败
 */
static s32 direct_print(u8 *print_string, u16 print_string_len)
{
    s32 ret = DDI_ERR;
    u32 print_len;
    system_data_instance_t *system_data = system_data_instance();

    do{
        if(NULL == print_string || 0 == print_string_len)
        {
            ret = DDI_EINVAL;
            break;
        }
        
        ddi_thmprn_open();
        ddi_thmprn_esc_p(print_string, print_string_len);
    
        while(1)
        {
            if(ddi_thmprn_esc_loop() != DDI_OK)
            {
                break;
            }

            ddi_watchdog_feed();
        }
        
        while(1)
        {
            ret = ddi_thmprn_get_status();        
            if((ret == DDI_OK) || (ret == DDI_EACCES) || (ret == DDI_EHIGHTEMP) || ret == DDI_ETIMEOUT)
            {   
                if(ret == DDI_EACCES) //缺纸
                {
                    audio_instance()->audio_play(AUDIO_DEVICE_OUT_PAPER, AUDIO_PLAY_BLOCK);
                    TRACE_DBG("printer no paper");
                }
                else if(ret == DDI_EHIGHTEMP)
                {
                    //trendit_audio_play("打印机温度过高,停止打印");
                    TRACE_DBG("printer high temp");
                }
                else if(ret == DDI_ETIMEOUT)
                {
                    //trendit_audio_play("打印机温度过高,停止打印");
                    TRACE_DBG("printer overtime");
                }

                ddi_thmprn_close();
                ddi_thmprn_ioctl(DDI_THMPRN_GET_PRINT_LEN, (u32)&print_len, 0);
                printer_instance()->m_print_len += print_len;
                system_data->set_printlen(printer_instance()->m_print_len);
                break;
            }

            ddi_watchdog_feed();
            ddi_misc_msleep(200);
        }           
    }while(0);

    return ret;
}


/**
 * @brief 给外部调用直接打印函数，如走纸键策略、usb口数据
 * @param[in] print_string esc指令数据
 * @param[in] print_string_len esc指令数据长度
 * @param[in] print_times 打印联数
 * @param[in] cur_print_len 返回时填充的当前走纸距离
 * @retval  DDI_OK 打印成功
 * @retval  <0 失败
 */
static s32 direct_print_times(u8 *print_string, u16 print_string_len, s32 print_times, s32 *cur_print_len)
{
    s32 ret = DDI_ERR;
    u32 print_len;
    system_data_instance_t *system_data = system_data_instance();
    s32 i = 0, j = 0;
    s8 tmp_buff[128] = {0};
    s32 enter_times = 0;   //换行符的个数
    s32 after_enter_num = 0;

    enter_times = get_conent_enter_times(print_string, print_string_len-1);   //换行符的个数
    TRACE_DBG("enter_times:%d", enter_times);
    ddi_thmprn_open();
    if(DDI_EHIGHTEMP == ddi_thmprn_get_status())
    {
        TRACE_DBG("temp is high");
        ret = DDI_EHIGHTEMP;
    }
    else
    {
        for(i=0; i<print_times; i++)
        {
            do{
                //开始前加一行走纸
                if(0 == i)
                {
                    ddi_thmprn_esc_p("\r\n", strlen("\r\n"));
                }

                if(print_times > 1)
                {
                    snprintf(tmp_buff, sizeof(tmp_buff), 
                                "\x1B\x40\x1B\x61\x01###########【第%d联】###########\r\n\x1B\x40", i+1);
                    ddi_thmprn_esc_p(tmp_buff, strlen(tmp_buff));
                }

                
                ddi_thmprn_esc_p(print_string, print_string_len);

                //最后一行补四个换行
                if(i+1 == print_times)
                {
                    after_enter_num = 4;
                }
                else
                {
                    after_enter_num = 2;
                }
                
                //补换行符
                if(enter_times < after_enter_num)
                {
                    memset(tmp_buff, 0, sizeof(tmp_buff));
                    for(j=0; j<after_enter_num-enter_times; j++)
                    {
                        snprintf(tmp_buff+strlen(tmp_buff), sizeof(tmp_buff)-strlen(tmp_buff), "\r\n");
                    }
                    if(strlen(tmp_buff))
                    {
                        ddi_thmprn_esc_p(tmp_buff, strlen(tmp_buff));
                    }
                }

                while(1)
                {
                    if(ddi_thmprn_esc_loop() != DDI_OK)
                    {
                        break;
                    }

                    ddi_watchdog_feed();
                }
                
                while(1)
                {
                    ret = ddi_thmprn_get_status();        
                    if((ret == DDI_OK) || (ret == DDI_EACCES) || (ret == DDI_EHIGHTEMP) || (ret == DDI_ETIMEOUT))
                    {   
                        if(ret == DDI_EACCES) //缺纸
                        {
                            audio_instance()->audio_play(AUDIO_DEVICE_OUT_PAPER, AUDIO_PLAY_BLOCK);
                            TRACE_DBG("printer no paper");
                        }
                        else if(ret == DDI_EHIGHTEMP)
                        {
                            //trendit_audio_play("打印机温度过高,停止打印");
                            TRACE_DBG("printer high temp");
                        }
                        else if(ret == DDI_ETIMEOUT)
                        {
                            //trendit_audio_play("打印机温度过高,停止打印");
                            TRACE_DBG("printer overtime");
                        }

                        break;
                    }

                    ddi_watchdog_feed();
                    ddi_misc_msleep(200);
                }           

                TRACE_DBG("print times:%d end:%d", i+1, ret);
                if(DDI_OK != ret)
                {
                    break;
                }
            }while(0);
        }
    }

    ddi_thmprn_close();
    ddi_thmprn_ioctl(DDI_THMPRN_GET_PRINT_LEN, (u32)&print_len, 0);
    printer_instance()->m_print_len += print_len;
    system_data->set_printlen(printer_instance()->m_print_len);
    *cur_print_len = print_len;

    return ret;
}


/**
 * @brief 给外部调用判断是否有纸
 * @retval  TRUE 缺纸
 * @retval  FALSE 有纸
 */
static s32 check_paper(void)
{
    s32 status = FALSE;
    
    status = ddi_thmprn_ioctl(DDI_THMPRN_CTL_GET_PAPER_STATUS, 0, 0);

    return DDI_EACCES == status;
}

/**
 * @brief 处理按下走纸键的策略，有异常打印异常信息，无异常走纸
 * @retval 无
 */
void handle_paper_key_event(void)
{
    machine_error_model_t *error_model = error_model_instance();
    u32 key;
    u8 key_code = 0;
    u8 key_event = 0;
    s32 have_err = TRUE;
    u32 print_len = 0;
    s32 ret = DDI_ERR;
    //u32 print_len;

    if(error_model)
    {
        if(DDI_OK != error_model->print_error_msg())
        {
            have_err = FALSE;
        }
    }
    else
    {
        have_err = FALSE;
    }

    if(!have_err)
    {
        ddi_thmprn_open();
        while(1)
        {
            if(ddi_key_read(&key) > 0)
            {
                key_code = key&0xff;
                key_event = (key&0xff00)>>8;
                if(key_code != KEY_PAPER)
                {
                    TRACE_INFO("change to other key:%x", key_code);
                    break;
                }

                //结束
                if(key_event == KEY_EVENT_UP)
                {
                    TRACE_INFO("release paper key");
                    break;
                }
            }

            ddi_thmprn_esc_p("\r\n", strlen("\r\n"));
            while(1)
            {
                if(ddi_thmprn_esc_loop() != DDI_OK)
                {
                    break;
                }

                ddi_watchdog_feed();
            }

            if(DDI_EACCES == ddi_thmprn_get_status())
            {
                audio_instance()->audio_play(AUDIO_DEVICE_OUT_PAPER, AUDIO_PLAY_BLOCK);
                break;
            }

            ddi_watchdog_feed();
            ddi_misc_msleep(50);   //点像素长度0.125*字库点数24*20HZ   每秒出纸约60mm
        }



        while(1)
        {
            ret = ddi_thmprn_get_status();        
            if((ret == DDI_OK) || (ret == DDI_EACCES) || (ret == DDI_EHIGHTEMP) || ret == DDI_ETIMEOUT)
            {   
                if(ret == DDI_EACCES) //缺纸
                {
                    audio_instance()->audio_play(AUDIO_DEVICE_OUT_PAPER, AUDIO_PLAY_BLOCK);
                    TRACE_DBG("printer no paper");
                }
                else if(ret == DDI_EHIGHTEMP)
                {
                    //trendit_audio_play("打印机温度过高,停止打印");
                    TRACE_DBG("printer high temp");
                }
                else if(ret == DDI_ETIMEOUT)
                {
                    //trendit_audio_play("打印机温度过高,停止打印");
                    TRACE_DBG("printer overtime");
                }

               // ddi_thmprn_close();
               // ddi_thmprn_ioctl(DDI_THMPRN_GET_PRINT_LEN, (u32)&print_len, 0);
               // printer_instance()->m_print_len += print_len;
               // system_data->set_printlen(printer_instance()->m_print_len);
                break;
            }

            ddi_watchdog_feed();
            ddi_misc_msleep(50);
        }           


        ddi_thmprn_close();
        ddi_thmprn_ioctl(DDI_THMPRN_GET_PRINT_LEN, (u32)&print_len, 0);
        printer_instance()->m_print_len += print_len;
        system_data_instance()->set_printlen(printer_instance()->m_print_len);
    }
}

/**
 * @brief 打印的初始化函数
 * @retval  打印的设备结构体
 */
printer_module_t *printer_instance(void)
{
    system_data_instance_t *system_data = system_data_instance();
    
    if(g_model_printer)
    {
        return g_model_printer;
    }

    g_model_printer = k_mallocapp(sizeof(printer_module_t));
    if(NULL == g_model_printer)
    {
        TRACE_ERR("malloc fail");
        return NULL;
    }

    memset(g_model_printer, 0, sizeof(printer_module_t));
    g_model_printer->m_print_len = system_data->get_printlen();
    INIT_SUPER_METHOD(g_model_printer, MODEL_PRINTER, "printer");
    
    ddi_thmprn_esc_init();
    g_model_printer->direct_print = direct_print;
    g_model_printer->direct_print_times = direct_print_times;
    g_model_printer->check_paper = check_paper;
    g_model_printer->handle_paper_key_event = handle_paper_key_event;
    g_model_printer->init_last_print_packno = init_last_print_packno;

    g_model_printer->m_load_cfg = FALSE;
    g_model_printer->super.load_cfg(&(g_model_printer->super), PRINTER_CONFIG_FILE, exec_cfg);
    g_model_printer->m_load_cfg = TRUE;

    return g_model_printer;
}




