#include "ddiglobal.h"
#include "app_global.h"

static APP_PARSEDATA gAppParseData;
static data_parser_t *g_data_parser;

/**
 * @brief 初始化本文件所有的全局变量
 */
void trendit_init_parsedata()
{
    g_data_parser = NULL;
    gAppParseData.m_parse_step = PARSE_STEP1_STX;
    gAppParseData.m_data = (u8 *)k_mallocapp(PARSEBUFFERMAXSIZE);
    if(!gAppParseData.m_data)
    {
        gAppParseData.m_data = (u8 *)k_mallocapp(PARSEBUFFERMAXSIZE_BAK);
        if(gAppParseData.m_data)
        {
            TRACE_INFO("malloc failed, use bak size");
            gAppParseData.m_data_buff_size = PARSEBUFFERMAXSIZE_BAK;
        }
    }
    else
    {
        gAppParseData.m_data_buff_size = PARSEBUFFERMAXSIZE;
    }

    TRACE_INFO("parse buff size:%d", gAppParseData.m_data_buff_size);
}

static s32 parse_data(u8 *rec_data, s32 rec_data_len, s32 continue_flag)
{
    s32 ret = DDI_ERR;
    u8 data = 0;
    s32 tmplen = 0;
    u8 checkcrc[2];
    s32 i = 0;
    s32 crclen = 0;

    if(!continue_flag)
    {
        if(trendit_query_timer(gAppParseData.m_beg_ticks, PARSE_OVERTIME))
        {
            TRACE_DBG("overtime, then retry parse");
            gAppParseData.m_parse_step = PARSE_STEP1_STX;
        }
    }

    while(i<rec_data_len)
    {
        data = rec_data[i++];
        //TRACE_DBG("data:%0x, step:%d", data, gAppParseData.m_parse_step);
        switch(gAppParseData.m_parse_step)
        {
            case PARSE_STEP1_STX:
                if(data == STX)
                {
                    gAppParseData.m_rx_cnt = 0;
                    gAppParseData.m_parse_step++;
                    gAppParseData.m_tmplen = 0;
                }
                break;
                
             case PARSE_STEP2_KW:
                gAppParseData.m_tmplen++;
                gAppParseData.m_data[gAppParseData.m_rx_cnt++] = data;
                if(gAppParseData.m_tmplen >= strlen(KEY_WORDS))
                {
                    if(memcmp(gAppParseData.m_data, KEY_WORDS, strlen(KEY_WORDS)) == 0)
                    {
                        gAppParseData.m_beg_ticks = trendit_get_ticks();
                        gAppParseData.m_parse_step++;
                        gAppParseData.m_tmplen = 0;
                    }
                    else
                    {
                        gAppParseData.m_parse_step = PARSE_STEP1_STX;
                    }
                }
                break;
                
            case PARSE_STEP3_PACKAGE_NO:
                gAppParseData.m_tmplen++;
                gAppParseData.m_data[gAppParseData.m_rx_cnt++] = data;
                if(gAppParseData.m_tmplen >= PARSE_STEP3_LEN)
                {
                    trendit_unpack_u16(&gAppParseData.m_package_no, gAppParseData.m_data+gAppParseData.m_rx_cnt-PARSE_STEP3_LEN);
                    gAppParseData.m_parse_step++;
                    gAppParseData.m_tmplen = 0;
                }
                break;

            case PARSE_STEP4_SENTIME:
                gAppParseData.m_tmplen++;
                gAppParseData.m_data[gAppParseData.m_rx_cnt++] = data;
                
                if(gAppParseData.m_tmplen >= PARSE_STEP4_LEN)
                {
                    gAppParseData.m_tmplen = 0;
                    gAppParseData.m_parse_step++;
                }
                break;

            case PARSE_STEP5_0_PROTOCOL:
                gAppParseData.m_data[gAppParseData.m_rx_cnt++] = data;
                gAppParseData.m_parse_step++;
                if(PROTOCOL_VERSION != data)
                {
                    TRACE_ERR("version code is invalid:%d", data);
                    gAppParseData.m_parse_step = PARSE_STEP1_STX;
                }
                break;
                
            case PARSE_STEP5_CMD_TYPE:
                gAppParseData.m_data[gAppParseData.m_rx_cnt++] = data;
                gAppParseData.m_cmd_type = data;
                gAppParseData.m_parse_step++;
                gAppParseData.m_tmplen = 0;
                break;

            case PARSE_STEP6_DATALEN:
                gAppParseData.m_tmplen++;
                gAppParseData.m_data[gAppParseData.m_rx_cnt++] = data;
                if(gAppParseData.m_tmplen >= PARSE_STEP6_LEN)
                {
                    trendit_unpack_u16(&gAppParseData.m_data_len, gAppParseData.m_data+gAppParseData.m_rx_cnt-PARSE_STEP6_LEN);
                    if(gAppParseData.m_data_len > (gAppParseData.m_data_buff_size-PROTOCOL_DATA_LEN_EXCEPT_CONTENT))
                    {
                        TRACE_ERR("m_data_len:%d is overflow", gAppParseData.m_data_len);
                        gAppParseData.m_parse_step = PARSE_STEP1_STX;
                    }
                    else if(gAppParseData.m_data_len > 0)
                    {
                        gAppParseData.m_parse_step++;
                    }

                    gAppParseData.m_tmplen = 0;
                }
                break;
                
            case PARSE_STEP7_DATA:
                gAppParseData.m_data[gAppParseData.m_rx_cnt++] = data;
                gAppParseData.m_tmplen++;
                if(gAppParseData.m_tmplen >= gAppParseData.m_data_len)
                {
                    gAppParseData.m_parse_step++;
                }
                break;
                
            case PARSE_STEP8_ETX:
                if(data == ETX)
                {
                    gAppParseData.m_data[gAppParseData.m_rx_cnt++] = data;
                    gAppParseData.m_parse_step++;
                    gAppParseData.m_tmplen = 0;
                }
                else
                {
                    TRACE_ERR("ETX is invalid");
                    gAppParseData.m_parse_step = PARSE_STEP1_STX;
                }
                break;
                
            case PARSE_STEP9_CRC:
                gAppParseData.m_tmplen++;
                gAppParseData.m_data[gAppParseData.m_rx_cnt++] = data;
                if(gAppParseData.m_tmplen >= PARSE_STEP9_LEN)
                {
                    tmplen = PARSE_STEP1_LEN+PARSE_STEP2_LEN+PARSE_STEP3_LEN+PARSE_STEP4_LEN;
                    gAppParseData.m_parse_step = PARSE_STEP1_STX;
                    crclen = gAppParseData.m_data_len+PARSE_STEP5_0_PROTOCOL_LEN + PARSE_STEP5_LEN + PARSE_STEP6_LEN;
                    #ifdef IBOX
                    trendit_Crc16CCITT_stand(0, &gAppParseData.m_data[tmplen-1], crclen, checkcrc);
                    #else
                    trendit_Crc16CCITT(0, &gAppParseData.m_data[tmplen-1], crclen, checkcrc);
                    #endif
                    TRACE_DBG("CRC:%02x %02x", checkcrc[0], checkcrc[1]);
                    TRACE_DBG("send CRC:%02x %02x", gAppParseData.m_data[gAppParseData.m_rx_cnt-2], gAppParseData.m_data[gAppParseData.m_rx_cnt-1]);
                    if(0 == memcmp(&gAppParseData.m_data[gAppParseData.m_rx_cnt-2], checkcrc, 2))
                    {
                        ret = i;
                        goto _out;
                    }
                }
                break;
                
            default:
                gAppParseData.m_parse_step = PARSE_STEP1_STX;
                break;
        }

        ddi_watchdog_feed();
    }

_out:
    return ret;
}

static s32 _handle_data(DATA_SOURCE_E data_source)
{
    u16 cmd = 0;
    u16 length = 0;
    s32 len = DATA_BEGIN_INDEX;
    u8 *p_data = NULL;
    model_device_t *model_device = NULL;
    s32 ret = DDI_ERR;
    u8 ret_buff[RETURN_CONTENT_MAXSIZE/2] = {0};          //指令返回内容
    u8 ret_content_buff[RETURN_CONTENT_MAXSIZE] = {0};
    u16 ret_content_len = 0;
    s8 *pstr = NULL;
    s8 sn[64] = {0};
    app_answer_data_t *app_answer_data;
    u16 value_string_len = 0;
    u8 bak_value = 0;      //用于备份下一个TLV的第一个字节
    u16 last_cmd = 0;
    s32 last_cmd_ret = DDI_ERR;

    memset(ret_content_buff, 0, sizeof(ret_content_buff));
    //ret_content_buff[ret_content_len++] = PROTOCOL_VERSION;
    ret_content_buff[ret_content_len++] = trendit_get_machine_code();
    
    memset(sn, 0, sizeof(sn));
    ddi_misc_read_dsn(SNTYPE_POSSN, sn);
    ret_content_buff[ret_content_len++] = sn[0];
    snprintf(ret_content_buff+ret_content_len, sizeof(ret_content_buff), "%s", sn+1);
    ret_content_len += strlen(sn+1);
    
    while((len-DATA_BEGIN_INDEX) < gAppParseData.m_data_len)
    {
        memset(ret_buff, 0, sizeof(ret_buff));
        trendit_unpack_u16(&cmd, gAppParseData.m_data+len);
        len += 2;
        trendit_unpack_u16(&length, gAppParseData.m_data+len);
        len += 2;

        //tlv里的长度非法
        if(length > gAppParseData.m_data_len)
        {
            TRACE_ERR("The len in content is invalid");
            ret_content_buff[ret_content_len++] = -DDI_EINVAL;
            trendit_pack_u16(ret_content_buff+ret_content_len, cmd);
            ret_content_len += 2;
            trendit_pack_u16(ret_content_buff+ret_content_len, 0);  //len
            ret_content_len += 2;
            ret = DDI_EINVAL;
            break;
        }
        
        p_data = gAppParseData.m_data+len;
        value_string_len = length+1;

        //备份下一个命令的第一个字节数据，然后将它置为0，这样调用函数可使用strlen等函数
        bak_value = p_data[length];
        p_data[length] = 0;

        //TRACE_ERR("cmd:%x, %d, %d", cmd, length, value_string_len);
        //TRACE_DBG_HEX(p_data, value_string_len);
        model_device = (model_device_t *)trendit_get_model(cmd);
        if(model_device)
        {
            gAppParseData.m_cmd = cmd;
            switch(gAppParseData.m_cmd_type)
            {
                case CMD_TYPE_QRY:
                    ret = model_device->exec_qry(cmd, ret_buff, sizeof(ret_buff));
                    pstr = ret_buff;

                    //指令不存在时，从配置文件读取
                    if(DDI_EUNKOWN_CMD == ret)
                    {
                        ret = model_device->qry_from_cfg(model_device, cmd, ret_buff, sizeof(ret_buff));
                    }
                    break;

                case CMD_TYPE_CFG:
                    ret = model_device->exec_cfg(cmd, p_data, value_string_len);
                    pstr = p_data;

                    //保存到文件
                    if(DDI_OK == ret)
                    {
                        model_device->save_cfg(model_device, cmd, p_data, value_string_len);
                    }
                    break;

                case CMD_TYPE_CMD:
                    if(http_download_routine_status())   //下载时直接返回正在下载
                    {
                        ret = DDI_EUPGRADING;
                        pstr = ret_buff;
                        break;
                    }
                    
                    if(CMD_COMAND_PRINT == last_cmd 
                        && DDI_OK != last_cmd_ret
                        && CMD_COMAND_AUDIO_PLAY == cmd)
                    {
                        TRACE_INFO("last cmd print exec error, then ignore audio play");
                        ret = DDI_ERR;
                        pstr = ret_buff;
                    }
                    else
                    {
                        ret = model_device->exec_cmd(cmd, p_data, value_string_len, ret_buff, sizeof(ret_buff));
                        pstr = ret_buff;
                    }
                    break;
            }

            last_cmd = cmd;
            last_cmd_ret = ret;
        }
        else
        {
            pstr = p_data;
            TRACE_ERR("unknow cmd:%04x", cmd);
            ret = DDI_EUNKOWN_CMD;
        }

        //构建回复正文区
        ret_content_buff[ret_content_len++] = -ret;
        trendit_pack_u16(ret_content_buff+ret_content_len, cmd);
        ret_content_len += 2;
        trendit_pack_u16(ret_content_buff+ret_content_len, strlen(pstr));  //len
        ret_content_len += 2;
        memcpy(ret_content_buff+ret_content_len, pstr, strlen(pstr));
        ret_content_len += strlen(pstr);

        //恢复下个TLV的第一个字节数据
        p_data[length] = bak_value;
        
        //todo  长度超过
        
        len += length;

        ddi_watchdog_feed();
    }

    do{
        app_answer_data = k_mallocapp(sizeof(app_answer_data_t));
        if(NULL == app_answer_data)
        {
            ret = DDI_EOVERFLOW;
            break;
        }
        
        app_answer_data->m_cmd_type = gAppParseData.m_cmd_type;
        app_answer_data->m_package_no = gAppParseData.m_package_no;
        app_answer_data->m_data = k_mallocapp(20+RETURN_CONTENT_MAXSIZE);
        if(NULL == app_answer_data->m_data)
        {
            k_freeapp(app_answer_data);
            app_answer_data = NULL;
            ret = DDI_EOVERFLOW;
            break;
        }

        memset(app_answer_data->m_data, 0, 20+RETURN_CONTENT_MAXSIZE);
        len = trendit_pack_answer_data(app_answer_data->m_data, ret_content_buff, ret_content_len, gAppParseData.m_package_no, CMD_TYPE_ANSWER);
        ret = DDI_OK;
    }while(0);

    if(DATA_SOURCE_TCP == data_source)
    {
        tcp_protocol_instance()->send(app_answer_data->m_data, len);
    }
    else
    {
        ddi_com_write(PORT_CDCD_NO, app_answer_data->m_data, len);
    }

    if(app_answer_data->m_data)
    {
        k_freeapp(app_answer_data->m_data);
        app_answer_data->m_data = NULL;
    }

    if(app_answer_data)
    {
        k_freeapp(app_answer_data);
        app_answer_data = NULL;
    }

    return ret;
}

static u16 get_current_packno(void)
{
    return gAppParseData.m_package_no;
}

static u16 get_current_cmd(void)
{
    return gAppParseData.m_cmd;
}

static void sync_time(void)
{
    u8 sys_rtc[12] = {0};

    memset(sys_rtc, 0, sizeof(sys_rtc));
    ddi_misc_get_time(sys_rtc);

    //年月日不一致
    if(sys_rtc[0] != gAppParseData.m_data[gAppParseData.m_rx_cnt-6]
        || sys_rtc[1] != gAppParseData.m_data[gAppParseData.m_rx_cnt-5]
        || sys_rtc[2] != gAppParseData.m_data[gAppParseData.m_rx_cnt-4])
    {
        TRACE_INFO("sync backend time:%02x/%02x/%02x %02x:%02x:%02x",
                            gAppParseData.m_data[gAppParseData.m_rx_cnt-6],
                            gAppParseData.m_data[gAppParseData.m_rx_cnt-5],
                            gAppParseData.m_data[gAppParseData.m_rx_cnt-4],
                            gAppParseData.m_data[gAppParseData.m_rx_cnt-3],
                            gAppParseData.m_data[gAppParseData.m_rx_cnt-2],
                            gAppParseData.m_data[gAppParseData.m_rx_cnt-1]);
        ddi_misc_set_time(gAppParseData.m_data+gAppParseData.m_rx_cnt-6);
    }

    //年月日一致，分钟不一致
    if(
        sys_rtc[0] == gAppParseData.m_data[gAppParseData.m_rx_cnt-6]
        && sys_rtc[1] == gAppParseData.m_data[gAppParseData.m_rx_cnt-5]
        && sys_rtc[2] == gAppParseData.m_data[gAppParseData.m_rx_cnt-4]
        && sys_rtc[3] == gAppParseData.m_data[gAppParseData.m_rx_cnt-3])
    {
        if(sys_rtc[4] != gAppParseData.m_data[gAppParseData.m_rx_cnt-2])
        {
            TRACE_INFO("sync backend time:%02x/%02x/%02x %02x:%02x:%02x",
                            gAppParseData.m_data[gAppParseData.m_rx_cnt-6],
                            gAppParseData.m_data[gAppParseData.m_rx_cnt-5],
                            gAppParseData.m_data[gAppParseData.m_rx_cnt-4],
                            gAppParseData.m_data[gAppParseData.m_rx_cnt-3],
                            gAppParseData.m_data[gAppParseData.m_rx_cnt-2],
                            gAppParseData.m_data[gAppParseData.m_rx_cnt-1]);
            ddi_misc_set_time(gAppParseData.m_data+gAppParseData.m_rx_cnt-6);
        }
    }
}

static void handle_data(DATA_SOURCE_E data_source, u8 *rec_data, s32 rec_data_len, s32 socket_id)
{
    s32 have_handle_data_len = 0;
    s32 i = 0;
    s32 ret = DDI_ERR;
    s32 continue_flag = 0;

    while(1)
    {
        ret = parse_data(rec_data+have_handle_data_len, rec_data_len-have_handle_data_len, continue_flag);
        TRACE_DBG("ret:%d", ret);
        if(ret < DDI_OK)
        {
            break;
        }
        else
        {
            sync_time();      //检测是否需同步时间
            have_handle_data_len += ret;
            TRACE_DBG("package no:%04x", gAppParseData.m_package_no);
            TRACE_DBG("m_cmd_type:%d", gAppParseData.m_cmd_type);
            TRACE_DBG("m_data_len:%d", gAppParseData.m_data_len);
            _handle_data(data_source);
        }

        if(have_handle_data_len >= rec_data_len)
        {
            TRACE_INFO("hand done");
            break;
        }
        else
        {
            continue_flag = 1;
        }
    }
}

/**
 * @brief audio的初始化函数
 * @retval  audio的设备结构体
 */
data_parser_t *data_parser_instance(void)
{
    if(g_data_parser)
    {
        return g_data_parser;
    }
    
    g_data_parser = k_mallocapp(sizeof(data_parser_t));
    if(NULL == g_data_parser)
    {
        TRACE_ERR("malloc fail");
        return NULL;
    }
    
    memset(g_data_parser, 0, sizeof(data_parser_t));
    g_data_parser->handle_data = handle_data;
    g_data_parser->get_current_cmd = get_current_cmd;
    g_data_parser->get_current_packno = get_current_packno;

    return g_data_parser;
}



