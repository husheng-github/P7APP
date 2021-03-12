#include "ddiglobal.h"
#include "app_global.h"

static printer_module_t *g_model_printer = NULL;
static u16 g_last_printer_packetno = 0;              //��һ�δ�ӡ�İ����

/**
 * @brief ����ʱ��������ʱ���ã�����ʱ�������������崻����ڴ������Ż�����
 */
void init_last_print_packno(void)
{
    g_last_printer_packetno = 0x01ff;
}

/**
 * @brief ��ʼ�����ļ����е�ȫ�ֱ���
 */
void trendit_init_printer_data(void)
{
    g_model_printer = NULL;
    init_last_print_packno();
}

/**
 * @brief ��ȡ�ı�ĩβ���з��ĸ���
 * @param[in] value_string �ı�����
 * @param[in] value_string_len �ı����ݳ���
 * @retval  �ı�ĩβ���з�����
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
    s32 enter_times = get_conent_enter_times(value_string, value_string_len);   //���з��ĸ���
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

    if(DDI_OK == ret)  //��ӡ�ɹ����ٱ������ţ���ȱֽ�󣬷������ط��Ĵ�ӡ����Ż���֮ǰ��
    {
        g_last_printer_packetno = cur_packno;
    }

    return ret;
}

/**
 * @brief ���ⲿ����ֱ�Ӵ�ӡ����������ֽ�����ԡ�usb������
 * @param[in] print_string escָ������
 * @param[in] print_string_len escָ�����ݳ���
 * @retval  DDI_OK ��ӡ�ɹ�
 * @retval  <0 ʧ��
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
                if(ret == DDI_EACCES) //ȱֽ
                {
                    audio_instance()->audio_play(AUDIO_DEVICE_OUT_PAPER, AUDIO_PLAY_BLOCK);
                    TRACE_DBG("printer no paper");
                }
                else if(ret == DDI_EHIGHTEMP)
                {
                    //trendit_audio_play("��ӡ���¶ȹ���,ֹͣ��ӡ");
                    TRACE_DBG("printer high temp");
                }
                else if(ret == DDI_ETIMEOUT)
                {
                    //trendit_audio_play("��ӡ���¶ȹ���,ֹͣ��ӡ");
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
 * @brief ���ⲿ����ֱ�Ӵ�ӡ����������ֽ�����ԡ�usb������
 * @param[in] print_string escָ������
 * @param[in] print_string_len escָ�����ݳ���
 * @param[in] print_times ��ӡ����
 * @param[in] cur_print_len ����ʱ���ĵ�ǰ��ֽ����
 * @retval  DDI_OK ��ӡ�ɹ�
 * @retval  <0 ʧ��
 */
static s32 direct_print_times(u8 *print_string, u16 print_string_len, s32 print_times, s32 *cur_print_len)
{
    s32 ret = DDI_ERR;
    u32 print_len;
    system_data_instance_t *system_data = system_data_instance();
    s32 i = 0, j = 0;
    s8 tmp_buff[128] = {0};
    s32 enter_times = 0;   //���з��ĸ���
    s32 after_enter_num = 0;

    enter_times = get_conent_enter_times(print_string, print_string_len-1);   //���з��ĸ���
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
                //��ʼǰ��һ����ֽ
                if(0 == i)
                {
                    ddi_thmprn_esc_p("\r\n", strlen("\r\n"));
                }

                if(print_times > 1)
                {
                    snprintf(tmp_buff, sizeof(tmp_buff), 
                                "\x1B\x40\x1B\x61\x01###########����%d����###########\r\n\x1B\x40", i+1);
                    ddi_thmprn_esc_p(tmp_buff, strlen(tmp_buff));
                }

                
                ddi_thmprn_esc_p(print_string, print_string_len);

                //���һ�в��ĸ�����
                if(i+1 == print_times)
                {
                    after_enter_num = 4;
                }
                else
                {
                    after_enter_num = 2;
                }
                
                //�����з�
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
                        if(ret == DDI_EACCES) //ȱֽ
                        {
                            audio_instance()->audio_play(AUDIO_DEVICE_OUT_PAPER, AUDIO_PLAY_BLOCK);
                            TRACE_DBG("printer no paper");
                        }
                        else if(ret == DDI_EHIGHTEMP)
                        {
                            //trendit_audio_play("��ӡ���¶ȹ���,ֹͣ��ӡ");
                            TRACE_DBG("printer high temp");
                        }
                        else if(ret == DDI_ETIMEOUT)
                        {
                            //trendit_audio_play("��ӡ���¶ȹ���,ֹͣ��ӡ");
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
 * @brief ���ⲿ�����ж��Ƿ���ֽ
 * @retval  TRUE ȱֽ
 * @retval  FALSE ��ֽ
 */
static s32 check_paper(void)
{
    s32 status = FALSE;
    
    status = ddi_thmprn_ioctl(DDI_THMPRN_CTL_GET_PAPER_STATUS, 0, 0);

    return DDI_EACCES == status;
}

/**
 * @brief ��������ֽ���Ĳ��ԣ����쳣��ӡ�쳣��Ϣ�����쳣��ֽ
 * @retval ��
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

                //����
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
            ddi_misc_msleep(50);   //�����س���0.125*�ֿ����24*20HZ   ÿ���ֽԼ60mm
        }



        while(1)
        {
            ret = ddi_thmprn_get_status();        
            if((ret == DDI_OK) || (ret == DDI_EACCES) || (ret == DDI_EHIGHTEMP) || ret == DDI_ETIMEOUT)
            {   
                if(ret == DDI_EACCES) //ȱֽ
                {
                    audio_instance()->audio_play(AUDIO_DEVICE_OUT_PAPER, AUDIO_PLAY_BLOCK);
                    TRACE_DBG("printer no paper");
                }
                else if(ret == DDI_EHIGHTEMP)
                {
                    //trendit_audio_play("��ӡ���¶ȹ���,ֹͣ��ӡ");
                    TRACE_DBG("printer high temp");
                }
                else if(ret == DDI_ETIMEOUT)
                {
                    //trendit_audio_play("��ӡ���¶ȹ���,ֹͣ��ӡ");
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
 * @brief ��ӡ�ĳ�ʼ������
 * @retval  ��ӡ���豸�ṹ��
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




