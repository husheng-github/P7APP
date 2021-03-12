#include "ddiglobal.h"
#include "app_global.h"

static wireless_module_t *g_model_wireless = NULL;

/**
 * @brief 初始化本文件所有的全局变量
 */
void trendit_init_wireless_data(void)
{
    g_model_wireless = NULL;
}

static s32 qry_command_wire_info(u8 *ret_string, u16 ret_stringlen)
{
    s32 ret = DDI_ERR;
    s8 imei[32] = {0};
    strSimInfo sim_info;
    strBasestationInfo basestation_info;
    u8 prssi;
    u16 prxfull;
    u32 cid = 0;
    
    do{
        if(NULL == ret_string || 0 == ret_stringlen)
        {
            ret = DDI_EINVAL;
            break;
        }

        memset(imei, 0, sizeof(imei));
        ddi_gprs_get_imei(imei, sizeof(imei));

        memset(&sim_info, 0, sizeof(sim_info));
        ddi_gprs_get_siminfo(&sim_info);

        memset(&basestation_info, 0, sizeof(basestation_info));
        ddi_gprs_get_basestation_info(&basestation_info);

        ddi_gprs_get_signalquality(&prssi, &prxfull);
        if(MACHINE_P6_HEX == trendit_get_machine_code())
        {
            cid = trendit_asc_to_u32(basestation_info.asCi, strlen(basestation_info.asCi), MODULUS_DEC);
        }
        else
        {
            cid = trendit_asc_to_u32(basestation_info.asCi, strlen(basestation_info.asCi), MODULUS_HEX);
        }
        snprintf(ret_string, ret_stringlen, "%s,%s,%s,%s,%s,%s,%d,%d", GET_STR(imei),
                                                                GET_STR(sim_info.iccid),
                                                                GET_STR(sim_info.imsi),
                                                                GET_STR(basestation_info.asMcc),
                                                                GET_STR(basestation_info.asMnc),
                                                                GET_STR(basestation_info.asLac),
                                                                cid,
                                                                prssi);

        ret = DDI_OK;
    }while(0);

    return ret;
}

static s32 qry_command_wire_model_ver(u8 *ret_string, u16 ret_stringlen)
{
    s32 ret = DDI_ERR;
    
    do{
        if(NULL == ret_string || 0 == ret_stringlen)
        {
            ret = DDI_EINVAL;
            break;
        }

        ret = ddi_gprs_ioctl(DDI_GPRS_GET_MODULE_VER, (u32)ret_string, ret_stringlen);
        if(0 == strlen(ret_string))
        {
            snprintf(ret_string, ret_stringlen, "%s", "null");
        }
    }while(0);

    return ret;
}


static s32 qry_command_wire_state(u8 *ret_string, u16 ret_stringlen)
{
    s32 ret = DDI_ERR;
    
    do{
        if(NULL == ret_string || 0 == ret_stringlen)
        {
            ret = DDI_EINVAL;
            break;
        }
        
        snprintf(ret_string, ret_stringlen, "%d,%d", ddi_gprs_get_pdpstatus(WS_GET_WIRELESS_STATE),
                                                     ddi_gprs_get_pdpstatus(WS_GET_REG_NETWORK_FAIL_REASON));

        ret = DDI_OK;
    }while(0);

    return ret;
}

static s32 cfg_command_csq_min(u8 *value_string, u16 value_stringlen)
{
    s32 ret = DDI_ERR;
    s8 dst[2][SPLIT_PER_MAX] = {0};
        
    do{
        if(NULL == value_string || 0 == value_stringlen)
        {
            ret = DDI_EINVAL;
            break;
        }

        memset(dst, 0, sizeof(dst));
        ret = trendit_split(dst, sizeof(dst)/sizeof(dst[0]), value_string, ",", SPLIT_IGNOR_LEN0);
        if(ret != sizeof(dst)/sizeof(dst[0]))
        {
            ret = DDI_EINVAL;
            break;
        }

        trendit_trim(dst[0]);
        trendit_trim(dst[1]);
        ret = trendit_atoi(dst[0]);
        if(ret < 0 || ret > 31)
        {
            TRACE_ERR("value is invalid:%d", ret);
            ret = DDI_EINVAL;
            break;
        }
        
        ret = trendit_atoi(dst[1]);
        if(ret < 0 || ret > 31)
        {
            TRACE_ERR("value is invalid:%d", ret);
            ret = DDI_EINVAL;
            break;
        }

        wireless_instance()->m_dail_min_csq = trendit_atoi(dst[0]);
        wireless_instance()->m_download_min_csq = trendit_atoi(dst[1]);
        ret = DDI_OK;
    }while(0);

    return ret;
}


static s32 exec_cfg(u16 cmd, u8 *value_string, u16 value_stringlen)
{
    s32 ret = DDI_ERR;
    
    TRACE_DBG("cmd:%04x, value_string:%s", cmd, value_string);
    switch(cmd)
    {
        case CFG_COMAND_WIREDIAL:
            ret = cfg_command_csq_min(value_string, value_stringlen);
            break;
    }

    return ret;
}

static s32 exec_qry(u16 cmd, u8 *ret_string, u16 ret_stringlen)
{
    s32 ret = DDI_ERR;
    
    TRACE_DBG("cmd:%04x", cmd);
    switch(cmd)
    {
        case QRY_COMAND_WIRE_INFO:
            ret = qry_command_wire_info(ret_string, ret_stringlen);
            break;

        case QRY_COMAND_WIRE_MODEL_VER:
            ret = qry_command_wire_model_ver(ret_string, ret_stringlen);
            break;
            
        case QRY_COMAND_WIRE_STATE:
            ret = qry_command_wire_state(ret_string, ret_stringlen);
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
    
    TRACE_DBG("cmd:%04x, value_string:%s", cmd, value_string);

    return ret;
}

static s32 format_time(s8 *time_info, u8 *out_time_buff, s32 out_time_buff_len)
{
    //+CCLK: "17/07/05, 00:22:48 +00"
    s32 ret = DDI_ERR;
    s8 dst[5][SPLIT_PER_MAX];
    s8 tmp_buff[64] = {0};
    s8 tmp_buff2[64] = {0};

    do{
        if(NULL == time_info || NULL == out_time_buff || out_time_buff_len < 6)
        {
            ret = DDI_EINVAL;
            break;
        }
        
        memset(dst, 0, sizeof(dst));
        ret = trendit_split(dst, sizeof(dst)/sizeof(dst[0]), time_info, ",", SPLIT_NOIGNOR);
        if(ret != 2)
        {
            TRACE_ERR("time don't have ':%s", time_info);
            ret = DDI_EINVAL;
            break;
        }

        memset(tmp_buff2, 0, sizeof(tmp_buff2));
        snprintf(tmp_buff2, sizeof(tmp_buff2), "%s", dst[1]);
        //取年月日
        memset(tmp_buff, 0, sizeof(tmp_buff));
        snprintf(tmp_buff, sizeof(tmp_buff), "%s", dst[0]);
        memset(dst, 0, sizeof(dst));
        ret = trendit_split(dst, sizeof(dst)/sizeof(dst[0]), tmp_buff, "/", SPLIT_NOIGNOR);
        if(ret != 3)
        {
            TRACE_ERR("date is invalid:%s", tmp_buff);
            ret = DDI_EINVAL;
            break;
        }
        
        if(trendit_atoi(dst[0]) < 19)
        {
            TRACE_ERR("year is invalid:%s", time_info);
            ret = DDI_ERR;
            break;
        }

        out_time_buff[0] = trendit_asc_to_u32(dst[0], strlen(dst[0]), MODULUS_HEX);
        out_time_buff[1] = trendit_asc_to_u32(dst[1], strlen(dst[1]), MODULUS_HEX);
        out_time_buff[2] = trendit_asc_to_u32(dst[2], strlen(dst[2]), MODULUS_HEX);

        memset(dst, 0, sizeof(dst));
        trendit_trim(tmp_buff2);
        ret = trendit_split(dst, sizeof(dst)/sizeof(dst[0]), tmp_buff2, ":", SPLIT_NOIGNOR);
        if(ret != 3)
        {
            TRACE_ERR("time is invalid:%s,%d", tmp_buff2, ret);
            ret = DDI_EINVAL;
            break;
        }

        out_time_buff[3] = trendit_asc_to_u32(dst[0], strlen(dst[0]), MODULUS_HEX);
        out_time_buff[4] = trendit_asc_to_u32(dst[1], strlen(dst[1]), MODULUS_HEX);
        out_time_buff[5] = trendit_asc_to_u32(dst[2], 2, MODULUS_HEX);

        TRACE_INFO("%02x%02x%02x %02x:%02x:%02x", out_time_buff[0],
                                                  out_time_buff[1],
                                                  out_time_buff[2],
                                                  out_time_buff[3],
                                                  out_time_buff[4],
                                                  out_time_buff[5]);
        ret = DDI_OK;
    }while(0);

    return ret;
}

static s32 wireless_msg_handle(struct _msg *m_msg)
{
    s32 ret = MSG_KEEP;
    wireless_module_t *wireless_module = wireless_instance();
    s32 use_ntp_no;
    s32 use_ntp_no_times;
    s8 rtc[64];
    u8 sys_rtc[6];

    do{
        if(MSG_TERMINAL == m_msg->m_status)
        {
            ret = MSG_OUT;
            break;
        }
        else if(MSG_INIT == m_msg->m_status)
        {
            ddi_gprs_open();

            //设置apn   todo
            
            m_msg->m_status = MSG_SERVICE;
            ret = MSG_KEEP;
            wireless_module->m_thread_sleep_ticks = trendit_get_ticks();
            m_msg->m_lParam = WIRELESS_REG_NETWORK;
            break;
        }
        else
        {
            if(WIRELESS_REG_NETWORK == m_msg->m_lParam)
            {
                //休眠时间还未到
                if(!trendit_query_timer(wireless_module->m_thread_sleep_ticks, WIRELESS_CHECK_REG_SLEEP_TIME))
                {
                    ret = MSG_KEEP;
                    break;
                }
                else
                {
                    wireless_module->m_thread_sleep_ticks = trendit_get_ticks();
                }
                
                switch(ddi_gprs_get_pdpstatus(WS_GET_WIRELESS_STATE))
                {
                    case WS_SEARCHING_NETWORK:
                        if(!wireless_module->m_have_play_poweron_audio)
                        {
                            wireless_module->m_have_play_poweron_audio = TRUE;
                            audio_instance()->play_poweron_audio();
                        }
                        wireless_module->super.m_self_check_res = TRUE;
                        error_model_instance()->notify_error(ERR_WIRELESS_REGING);
                        break;

                    case WS_ACTIVE_PDP:
                        if(!wireless_module->m_have_play_poweron_audio)
                        {
                            wireless_module->m_have_play_poweron_audio = TRUE;
                            audio_instance()->play_poweron_audio();
                        }
                        error_model_instance()->clear_error(ERR_WIRELESS_REGING);
                        error_model_instance()->notify_error(ERR_WIRELESS_ACTIVING_NET);
                        break;

                    case WS_ACTIVE_PDP_SUCCESS:
                        if(!wireless_module->m_have_play_poweron_audio)
                        {
                            wireless_module->m_have_play_poweron_audio = TRUE;
                            audio_instance()->play_poweron_audio();
                        }
                        TRACE_INFO("wireless network active success");
                        m_msg->m_lParam = WIRELESS_CFG_NTP;
                        ret = MSG_KEEP;
                        error_model_instance()->clear_error(ERR_WIRELESS_ACTIVING_NET);
                        break;
                }

                switch(ddi_gprs_get_pdpstatus(WS_GET_REG_NETWORK_FAIL_REASON))
                {
                    case WS_NO_WIRELESS_MODULE:
                        TRACE_ERR("don't detect wireless module");
                        ret = MSG_OUT;
                        wireless_module->m_msg_status = MSG_TERMINAL;
                        error_model_instance()->notify_error(ERR_NO_WIRELESS_MODEL);
                        break;
                        
                    case WS_REGISTRATION_DENIED:
                        error_model_instance()->notify_error(ERR_WIRELESS_REG_NET_DENIED);
                        break;

                    case WS_NO_INSERT_SIM:
                        TRACE_ERR("don't detect simcard");
                        ret = MSG_OUT;
                        wireless_module->m_msg_status = MSG_TERMINAL;
                        error_model_instance()->notify_error(ERR_NO_SIM_CARD);
                        break;
                }
            }
            else if(WIRELESS_CFG_NTP == m_msg->m_lParam)
            {
                if(wifi_instance()->m_have_sync_time)
                {
                    TRACE_INFO("wifi have sync success, then ignore");
                    m_msg->m_lParam = WIRELESS_PING;
                    ret = MSG_KEEP;
                    break;
                }
                
                use_ntp_no = wireless_instance()->m_use_ntp_no;
                use_ntp_no_times = wireless_instance()->m_use_ntp_no_times;
                ret = ddi_gprs_ioctl(DDI_GPRS_CTL_CFG_NTP_SERVER, (u32)machine_instance()->m_ntp_info[use_ntp_no], 123);
                if(DDI_OK == ret)
                {
                    m_msg->m_lParam = WIRELESS_GET_NTP;
                    wireless_module->m_thread_sleep_ticks = trendit_get_ticks();
                }
                else
                {
                    wireless_instance()->m_use_ntp_no_times++;
                    if(wireless_instance()->m_use_ntp_no_times > 10)
                    {
                        TRACE_ERR("get failed");
                        m_msg->m_lParam = WIRELESS_PING;
                        wireless_instance()->m_have_sync_time = TRUE;
                    }
                }
                ret = MSG_KEEP;
            }
            else if(WIRELESS_GET_NTP == m_msg->m_lParam)
            {
                memset(sys_rtc, 0, sizeof(sys_rtc));
                ddi_misc_get_time(sys_rtc);
                if(wifi_instance()->m_have_sync_time)
                {
                    TRACE_INFO("wifi have sync success, then ignore");
                    m_msg->m_lParam = WIRELESS_PING;
                    ret = MSG_KEEP;
                    break;
                }
                
                if(WS_ACTIVE_PDP_SUCCESS != ddi_gprs_get_pdpstatus(WS_GET_WIRELESS_STATE))
                {
                    TRACE_ERR("wireless network dropped, then retry");
                    m_msg->m_lParam = WIRELESS_REG_NETWORK;
                    ret = MSG_KEEP;
                    break;
                }
                
                //休眠时间还未到
                if(!trendit_query_timer(wireless_module->m_thread_sleep_ticks, WIRELESS_CHECK_REG_SLEEP_TIME))
                {
                    ret = MSG_KEEP;
                    break;
                }
                else
                {
                    wireless_module->m_thread_sleep_ticks = trendit_get_ticks();
                }
                
                memset(rtc, 0, sizeof(rtc));
                memset(sys_rtc, 0, sizeof(sys_rtc));
                ddi_gprs_ioctl(DDI_GPRS_CTL_QRY_NTP, (u32)rtc, sizeof(rtc));

                if(DDI_OK == format_time(rtc, sys_rtc, sizeof(sys_rtc)))
                {
                    wireless_instance()->m_have_sync_time = TRUE;
                    ddi_misc_set_time(sys_rtc);
                    m_msg->m_lParam = WIRELESS_PING;
                }
                else
                {
                    wireless_instance()->m_use_ntp_no_times++;
                    if(wireless_instance()->m_use_ntp_no_times > 5)
                    {
                        if(wireless_instance()->m_use_ntp_no >= 2)
                        {
                            TRACE_ERR("get failed");
                            m_msg->m_lParam = WIRELESS_PING;
                            wireless_instance()->m_have_sync_time = TRUE;
                        }
                        else
                        {
                            wireless_instance()->m_use_ntp_no++;
                            TRACE_INFO("try next ntp:%s", machine_instance()->m_ntp_info[wireless_instance()->m_use_ntp_no]);
                            m_msg->m_lParam = WIRELESS_CFG_NTP;
                        }
                    }
                }
                ret = MSG_KEEP;
            }
            else
            {
                if(WS_ACTIVE_PDP_SUCCESS != ddi_gprs_get_pdpstatus(WS_GET_WIRELESS_STATE))
                {
                    TRACE_ERR("wireless network dropped, then retry");
                    m_msg->m_lParam = WIRELESS_REG_NETWORK;
                    ret = MSG_KEEP;
                    break;
                }
                
                //休眠时间还未到
                if(!trendit_query_timer(wireless_module->m_thread_sleep_ticks, WIRELESS_PING_INTERVAL_SLEEP_TIME))
                {
                    ret = MSG_KEEP;
                    break;
                }
                else
                {
                    wireless_module->m_thread_sleep_ticks = trendit_get_ticks();
                }

                //todo  发送ping包

                ret = MSG_KEEP;
            }
        }
    }while(0);

    return ret;
}

static s32 post_wireless_msg()
{
    msg_t *wireless_connect_msg = NULL;
    s32 ret = DDI_ERR;
    wireless_module_t *wireless_module = wireless_instance();

    wireless_connect_msg = (msg_t *)k_mallocapp(sizeof(msg_t));
    if(NULL == wireless_connect_msg)
    {
        wireless_module->m_msg_status = MSG_TERMINAL;
        TRACE_ERR("malloc failed");
    }
    else
    {
        TRACE_DBG("wireless:%x", wireless_connect_msg);
        memset(wireless_connect_msg, 0, sizeof(msg_t));
        wireless_connect_msg->m_func = wireless_msg_handle;
        wireless_connect_msg->m_priority = MSG_NORMAL;
        wireless_connect_msg->m_lParam = 0;
        wireless_connect_msg->m_wparam = 0;
        wireless_connect_msg->m_status = MSG_INIT;
        wireless_module->m_msg = wireless_connect_msg;
        wireless_module->m_msg_status = MSG_SERVICE;
        snprintf(wireless_connect_msg->m_msgname, sizeof(wireless_connect_msg->m_msgname), "%s", "wireless");
        trendit_postmsg(wireless_connect_msg);
    }

    return ret;
}

static void terminal_wireless_msg()
{
    wireless_module_t *wireless_module = wireless_instance();

    if(wireless_module->m_msg)
    {
        wireless_module->m_msg->m_status = MSG_TERMINAL;
        wireless_module->m_msg = NULL;
    }
}


static s32 check_network_active_success(void)
{
    return WS_ACTIVE_PDP_SUCCESS == ddi_gprs_get_pdpstatus(WS_GET_WIRELESS_STATE);
}

/**
 * @brief 无线的初始化函数
 * @retval  无线的设备结构体
 */
wireless_module_t *wireless_instance(void)
{
    if(g_model_wireless)
    {
        return g_model_wireless;
    }

    g_model_wireless = k_mallocapp(sizeof(wireless_module_t));
    if(NULL == g_model_wireless)
    {
        TRACE_ERR("malloc fail");
        return NULL;
    }
    memset(g_model_wireless, 0, sizeof(wireless_module_t));
    INIT_SUPER_METHOD(g_model_wireless, MODEL_WIRELESS, "wireless");
    g_model_wireless->post_wireless_msg = post_wireless_msg;
    g_model_wireless->terminal_wireless_msg = terminal_wireless_msg;
    g_model_wireless->check_network_active_success = check_network_active_success;

    g_model_wireless->super.load_cfg(&(g_model_wireless->super), WIRELESS_CONFIG_FILE, exec_cfg);
    
    return g_model_wireless;
}

