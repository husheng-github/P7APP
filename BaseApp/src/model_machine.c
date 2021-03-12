#include "ddiglobal.h"
#include "app_global.h"

static machine_module_t *g_model_machine = NULL;

/**
 * @brief 初始化本文件所有的全局变量
 */
void trendit_init_machine_data(void)
{
    g_model_machine = NULL;
}

static s32 cfg_command_machine_commu_strategy(u8 *value_string)
{
    s32 ret = DDI_ERR;

    do{
        if(NULL == value_string)
        {
            ret = DDI_EINVAL;
            break;
        }

        ret = trendit_atoi(value_string);
        switch(ret)
        {
            case WIFI_PRIORITY:
            case WIRELESS_2G_PRIORITY:
            case WIFI_ONLY:
            case WIRELESS_2G_ONLY:
                if(ret == WIFI_ONLY && MSG_TERMINAL == wifi_instance()->m_msg_status)
                {
                    TRACE_ERR("wifi module don't exist");
                    ret = DDI_EUN_SUPPORT;
                    break;
                }
                
                if(network_strategy_instance()->m_network_strategy_type != ret)
                {
                    network_strategy_instance()->m_network_strategy_type = ret;
                    network_strategy_instance()->restart();
                }
                ret = DDI_OK;
                break;

            default:
                ret = DDI_EINVAL;
                break;
        }
    }while(0);

    return ret;
}


static s32 cfg_command_machine_backend_con(u8 *value_string)
{
    s8 dst[2][SPLIT_PER_MAX];
    s32 ret = DDI_ERR;

    do{
        if(NULL == value_string)
        {
            ret = DDI_EINVAL;
            break;
        }

        ret = trendit_split(dst, sizeof(dst)/sizeof(dst[0]), value_string, ",", SPLIT_IGNOR_LEN0);
        if(sizeof(dst)/sizeof(dst[0]) != ret)
        {
            TRACE_ERR("don't have two arg:%s", value_string);
            ret = DDI_EINVAL;
            break;
        }

        trendit_trim(dst[0]);
        trendit_trim(dst[1]);
        ret = tcp_protocol_instance()->cfg_server_info(dst[0], trendit_atoi(dst[1]));
    }while(0);

    return ret;
}

static s32 cfg_command_machine_hbt_interval(u8 *value_string)
{
    s32 ret = DDI_ERR;

    do{
        if(NULL == value_string)
        {
            ret = DDI_EINVAL;
            break;
        }

        ret = trendit_atoi(value_string);
        if(ret > HBT_INTERVAL_MAX || ret < HBT_INTERVAL_MIN)
        {
            ret = DDI_EINVAL;
            break;
        }

        machine_instance()->m_heart_beat_interval = ret;
        ret = DDI_OK;
    }while(0);

    return ret;
}

static s32 cfg_command_machine_net_strategy_con_time(u8 *value_string)
{
    s32 ret = DDI_ERR;
    s8 dst[4][SPLIT_PER_MAX];

    do{
        if(NULL == value_string)
        {
            ret = DDI_EINVAL;
            break;
        }

        memset(dst, 0, sizeof(dst));
        ret = trendit_split(dst, sizeof(dst)/sizeof(dst[0]), value_string, ",", SPLIT_NOIGNOR);
        if(sizeof(dst)/sizeof(dst[0]) != ret)
        {
            TRACE_ERR("The param is invalid:%d", ret);
            ret = DDI_EINVAL;
            break;
        }

        trendit_trim(dst[0]);
        trendit_trim(dst[1]);
        trendit_trim(dst[2]);
        trendit_trim(dst[3]);

        ret = trendit_atoi(dst[0]);
        if(ret < WIFI_REG_MIN_TIME || ret > WIFI_REG_MAX_TIME)
        {
            TRACE_ERR("wifi reg param is invalid:%d", ret);
            ret = DDI_EINVAL;
            break;
        }

        ret = trendit_atoi(dst[1]);
        if(ret < WIFI_CON_BACKEND_MIN_TIME || ret > WIFI_CON_BACKEND_MAX_TIME)
        {
            TRACE_ERR("wifi connect backend param is invalid:%d", ret);
            ret = DDI_EINVAL;
            break;
        }

        ret = trendit_atoi(dst[2]);
        if(ret < WIRELESS_REG_MIN_TIME || ret > WIRELESS_REG_MAX_TIME)
        {
            TRACE_ERR("wireless reg param is invalid:%d", ret);
            ret = DDI_EINVAL;
            break;
        }

        ret = trendit_atoi(dst[3]);
        if(ret < WIRELESS_CON_BACKEND_MIN_TIME || ret > WIRELESS_CON_BACKEND_MAX_TIME)
        {
            TRACE_ERR("wireless connect backend param is invalid:%d", ret);
            ret = DDI_EINVAL;
            break;
        }

        TRACE_INFO("%d,%d,%d,%d", trendit_atoi(dst[0]), trendit_atoi(dst[1]), trendit_atoi(dst[2]), trendit_atoi(dst[3]));
        network_strategy_instance()->m_wifi_reg_net_try_overtime = trendit_atoi(dst[0])*1000;
        network_strategy_instance()->m_wifi_tcp_con_try_overtime = trendit_atoi(dst[1])*1000;
        network_strategy_instance()->m_wireless_reg_net_try_overtime = trendit_atoi(dst[2])*1000;
        network_strategy_instance()->m_wireless_tcp_con_try_overtime = trendit_atoi(dst[3])*1000;
        ret = DDI_OK;
    }while(0);

    return ret;
}

static s32 cfg_command_machine_poweroff_key_delay_time(u8 *value_string)
{
    s32 ret = DDI_ERR;

    do{
        if(NULL == value_string)
        {
            ret = DDI_EINVAL;
            break;
        }

        ret = trendit_atoi(value_string);
        if(ret < POWER_KEY_DELAY_MIN_TIME || ret > POWER_KEY_DELAY_MAX_TIME)
        {
            TRACE_ERR("param is invalid:%d", ret);
            ret = DDI_EINVAL;
            break;
        }

        TRACE_INFO("val:%d", ret);
        machine_instance()->m_power_key_delay_time = ret;
        ret = DDI_OK;
    }while(0);

    return ret;
}

static s32 cfg_command_machine_cfg_ntp(u8 *value_string, s8 value_string_len)
{
    s32 ret = DDI_ERR;
    s8 dst[3][SPLIT_PER_MAX];
    s32 len = 0;

    do{
        if(NULL == value_string)
        {
            ret = DDI_EINVAL;
            break;
        }

        ret = trendit_split(dst, sizeof(dst)/sizeof(dst[0]), value_string, ",", SPLIT_NOIGNOR);
        if(ret != sizeof(dst)/sizeof(dst[0]))
        {
            TRACE_ERR("param is invalid:%s", value_string);
            ret = DDI_EINVAL;
            break;
        }

        len = sizeof(machine_instance()->m_ntp_info[0]);
        snprintf(machine_instance()->m_ntp_info[0], len, "%s", dst[0]);
        snprintf(machine_instance()->m_ntp_info[1], len, "%s", dst[1]);
        snprintf(machine_instance()->m_ntp_info[2], len, "%s", dst[2]);
        ret = DDI_OK;
    }while(0);

    return ret;
}

static s32 cfg_comand_machine_tms_cfg(u8 *value_string, s8 value_string_len)
{
    machine_module_t *machine_module = machine_instance();
    s8 dst[2][SPLIT_PER_MAX] = {0};
    s32 ret = DDI_ERR;

    do{
        memset(dst, 0, sizeof(dst));
        ret = trendit_split(dst, sizeof(dst)/sizeof(dst[0]), value_string, ",", SPLIT_IGNOR_LEN0);
        if(ret != sizeof(dst)/sizeof(dst[0]))
        {
            TRACE_ERR("invalid param");
            ret = DDI_EINVAL;
            break;
        }

        ret = trendit_atoi(dst[0]);
        if(0 != ret && 1 != ret)
        {
            TRACE_ERR("switch is invalid");
            ret = DDI_EINVAL;
            break;
        }

        if(strlen(dst[1]) != 6 || DDI_OK != trendit_isdigit(dst[1]))
        {
            TRACE_ERR("time is invalid");
            ret = DDI_EINVAL;
            break;
        }
        
        machine_module->m_tms_auto_switch = trendit_atoi(dst[0]);
        snprintf(machine_module->m_tms_auto_detect_time, sizeof(machine_module->m_tms_auto_detect_time), "%s", dst[1]);
        ret = DDI_OK;
    }while(0);

    return ret;
}

static s32 cfg_command_machine_wifi_airkill_url(u8 *value_string, s8 value_string_len)
{
    machine_module_t *machine_module = machine_instance();
    
    snprintf(machine_module->m_wifi_airkiss_url, sizeof(machine_module->m_wifi_airkiss_url), "%s", value_string);

    return DDI_OK;
}
    
static s32 qry_command_memory_info(u8 *ret_string, u16 ret_stringlen)
{

    mem_info_t mem_info;
    u32 free_space = 0;

    memset(&mem_info, 0, sizeof(mem_info));
    ddi_misc_ioctl(MISC_IOCTL_GET_SYSTEM_MEM, (u32)&mem_info, 0);
    ddi_vfs_free_space(&free_space);
    snprintf(ret_string, ret_stringlen, "%d,%d", mem_info.m_remaining_memory,free_space);
    TRACE_DBG("memory:%d,%d",mem_info.m_remaining_memory,free_space);
    return DDI_OK;
}

static s32 qry_command_machine_version(u8 *ret_string, u16 ret_stringlen)
{
    s32 ret = DDI_ERR;
    s8 pos_sn[32];
    s8 cfg_code[32];
    s8 boot_version[32];
    s8 core_version[32];
    s32 hard_version;
    s8 sn_key[32];

    hwsn_info_t l_hwsn_info;
    do{
        if(NULL == ret_string || 0 == ret_stringlen)
        {
            ret = DDI_EINVAL;
            break;
        }


        memset(&l_hwsn_info, 0, sizeof(l_hwsn_info));
        ret = ddi_misc_ioctl(MISC_IOCTL_GETHWSNANDSNKEY, 0, (u32)&l_hwsn_info);

        memset(cfg_code, 0, sizeof(cfg_code));
        memset(sn_key, 0, sizeof(sn_key));
        memset(pos_sn, 0, sizeof(pos_sn));
        memset(core_version, 0, sizeof(core_version));
        memset(boot_version, 0, sizeof(boot_version));
        if(l_hwsn_info.m_cfglen <= 32)
        {
            memcpy(cfg_code, l_hwsn_info.m_cfg, l_hwsn_info.m_cfglen);
        }

        if(l_hwsn_info.m_snkeylen <= 32)
        {
            memcpy(sn_key, l_hwsn_info.m_snkey, l_hwsn_info.m_snkeylen);
        }

        ddi_misc_read_dsn(SNTYPE_POSSN, pos_sn);
        ddi_misc_get_firmwareversion(FIRMWARETYPE_APBOOT, boot_version);
        ddi_misc_get_firmwareversion(FIRMWARETYPE_APCORE, core_version);
        hard_version = ddi_misc_ioctl(MISC_IOCTL_GETHWVER, 0, 0);
        snprintf(ret_string, ret_stringlen, "%s,%s,%s,%d,%s,%s,%s", GET_STR(pos_sn+1),
                                                              GET_STR(cfg_code),
                                                              GET_STR(sn_key),
                                                              hard_version,
                                                              GET_STR(boot_version),
                                                              GET_STR(core_version),
                                                              APP_VER);
        ret = DDI_OK;
    }while(0);

    return ret;
}

static s32 qry_command_machine_key(u8 *ret_string, u16 ret_stringlen)
{
    s32 ret = DDI_ERR;
    s8 sn_key[32];

    hwsn_info_t l_hwsn_info;
    do{
        if(NULL == ret_string || 0 == ret_stringlen)
        {
            ret = DDI_EINVAL;
            break;
        }

        memset(&l_hwsn_info, 0, sizeof(l_hwsn_info));
        ret = ddi_misc_ioctl(MISC_IOCTL_GETHWSNANDSNKEY, 0, (u32)&l_hwsn_info);

        memset(sn_key, 0, sizeof(sn_key));
        if(l_hwsn_info.m_snkeylen <= 32)
        {
            memcpy(sn_key, l_hwsn_info.m_snkey, l_hwsn_info.m_snkeylen);
        }

        snprintf(ret_string, ret_stringlen, "%s", GET_STR(sn_key));
        ret = DDI_OK;
    }while(0);

    return ret;
}

static s32 qry_command_machine_pwo_check_res(u8 *ret_string, u16 ret_stringlen)
{
    s32 ret = DDI_ERR;
    wifi_module_t *wifi_module = wifi_instance();
    wireless_module_t *wireless_module = wireless_instance();

    do{
        if(NULL == ret_string || 0 == ret_stringlen)
        {
            ret = DDI_EINVAL;
            break;
        }

        if(wifi_module && wireless_module)
        {
            if(wifi_module->super.m_self_check_res && wireless_module->super.m_self_check_res)
            {
                snprintf(ret_string, ret_stringlen, "[0]");
            }
            else if(!wifi_module->super.m_self_check_res && !wireless_module->super.m_self_check_res)
            {
                snprintf(ret_string, ret_stringlen, "[1,2]");
            }
            else if(!wifi_module->super.m_self_check_res)
            {
                snprintf(ret_string, ret_stringlen, "[2]");
            }
            else if(!wireless_module->super.m_self_check_res)
            {
                snprintf(ret_string, ret_stringlen, "[1]");
            }
        }

        ret = DDI_OK;
    }while(0);

    return ret;
}


static s32 qry_command_current_commu_channel(u8 *ret_string, u16 ret_stringlen)
{
    s32 ret = DDI_ERR;
    s32 csq = 0;
    u8 rssi = 0;
    u16 rxfull = 0;

    do{
        if(NULL == ret_string || 0 == ret_stringlen)
        {
            ret = DDI_EINVAL;
            break;
        }

        switch(network_strategy_instance()->m_current_type)
        {
            case COMMU_WIRELESS:
                ddi_gprs_get_signalquality(&rssi, &rxfull);
                csq = rssi;
                break;

            case COMMU_WIFI:
                ddi_wifi_get_signal(&csq);
                break;
        }
        
        snprintf(ret_string, ret_stringlen, "%d,%d", network_strategy_instance()->m_current_type, csq);
        ret = DDI_OK;
    }while(0);

    return ret;
}

static s32 qry_command_last_drop_time(u8 *ret_string, u16 ret_stringlen)
{
    s32 ret = DDI_ERR;

    do{
        if(NULL == ret_string || 0 == ret_stringlen)
        {
            ret = DDI_EINVAL;
            break;
        }

        snprintf(ret_string, ret_stringlen, "%s", GET_STR(network_strategy_instance()->m_last_drop_time));
        ret = DDI_OK;
    }while(0);

    return ret;
}

static s32 qry_command_last_online_time(u8 *ret_string, u16 ret_stringlen)
{
    s32 ret = DDI_ERR;

    do{
        if(NULL == ret_string || 0 == ret_stringlen)
        {
            ret = DDI_EINVAL;
            break;
        }

        snprintf(ret_string, ret_stringlen, "%s", GET_STR(network_strategy_instance()->m_last_online_time));
        ret = DDI_OK;
    }while(0);

    return ret;
}

static s32 qry_command_online_reason(u8 *ret_string, u16 ret_stringlen)
{
    s32 ret = DDI_ERR;

    do{
        if(NULL == ret_string || 0 == ret_stringlen)
        {
            ret = DDI_EINVAL;
            break;
        }

        snprintf(ret_string, ret_stringlen, "%d", network_strategy_instance()->m_online_reason);
        ret = DDI_OK;
    }while(0);

    return ret;
}

static s32 cmd_command_machine_reboot(u8 *ret_string, u16 ret_stringlen)
{
    s32 ret = DDI_ERR;

    TRACE_INFO("ready to reboot machine");
    system_data_instance()->set_poweroff_reason(RESEASON_COMMAND_REBOOT);
    audio_instance()->audio_play(AUDIO_POWER_OFF, AUDIO_PLAY_BLOCK);
    ddi_misc_reboot();

    while(1)
    {
        TRACE_INFO("wait reboot");
        ddi_watchdog_feed();
        ddi_misc_msleep(5000);
    }

    return DDI_OK;
}

const char bitmap_path[5][32] = {"/mtd0/nvimage0.bin",
                         "/mtd0/nvimage1.bin",
                         "/mtd0/nvimage2.bin",
                         "/mtd0/nvimage3.bin",
                         "/mtd0/nvimage4.bin"};

static s32 bitmap_download_callback(s32 status)
{
    TRACE_DBG("status:%d\r\n", status);
    return 0;
}

static s32 cmd_command_machine_download_bitmap(u8 *ret_string, u16 ret_stringlen)
{
    s32 ret = DDI_ERR;
    char buf[8] = {0};
    s32 num = 0;

    if(ret_string == NULL)
    {
        TRACE_INFO("bitmap download url NULL\r\n");
        return DDI_EINVAL;
    }
    TRACE_INFO("bitmap download url：%s\r\n", ret_string);
    memset(buf, 0, sizeof(buf));
    buf[0] = ret_string[0];
    num = trendit_atoi(buf);
    TRACE_INFO("bitmap download serial num:%d, url:%s\r\n", num, &ret_string[2]);

    if(num < 1 || num > 5)
    {
        TRACE_INFO("bitmap download serial num err!\r\n");
        return DDI_EDATA;
    }

    //ddi_file_delete(bitmap_path[num-1]);

    tms_url_content_download((u8*)bitmap_path[num -1], &ret_string[2], 1, bitmap_download_callback); /*TODO 是否需要做成同步？*/

    return DDI_OK;
}

s32 cmd_command_machine_parse_bitmap_url(u8 *data, u16 len, upgrade_info_t *info)
{
    s32 ret = DDI_OK;
    char buf[8] = {0};
    s32 num = 0;
    u8 *bmp_path = NULL;

    if (NULL == data)
    {
        ret = DDI_EINVAL;
        TRACE_INFO("\t.bitmap download url is NULL.\r\n");
    }
    else
    {
        TRACE_INFO("bitmap url: %s\r\n", data);
        memset(buf, 0, sizeof(buf));
        buf[0] = data[0];
        num    = trendit_atoi(buf);
        TRACE_INFO("bitmap download serial num: %d, url: %s\r\n", num, &data[2]);
        if (num < 1 || num > 5)
        {
            ret = DDI_EDATA;
            TRACE_INFO("bitmap download serial num err!\r\n");
        }
        else
        {
            memcpy(info->m_download_url, &data[2], len - 2);
            strcpy(info->m_store_filepath, (u8 *)bitmap_path[num -1]);
        }
    }

    return ret;
}

static s32 cmd_command_machine_delete_bitmap(u8 *ret_string, u16 ret_stringlen)
{
    s32 ret = DDI_ERR;
    u16 num = 0;

    TRACE_INFO("bitmap download url：%s\r\n", ret_string);
    do
    {
        if(ret_string == NULL)
        {
            ret = DDI_EINVAL;
            break;
        }

        num = trendit_atoi(ret_string);
        if(num < 1 || num > 5)
        {
            ret = DDI_EDATA;
            break;
        }
        TRACE_INFO("num：%d, delete bitmap_path:%s\r\n", num, (u8*)bitmap_path[num-1]);
        ddi_file_delete((u8*)bitmap_path[num-1]);

        ret = DDI_OK;
    }while(0);

    return ret;
}

static s32 cmd_command_machine_modify_bitmap(u8 *ret_string, u16 ret_stringlen)
{
    s32 ret = DDI_ERR;
    u8 buf[8] = {0};
    s32 num = 0;

    if(ret_string == NULL)
    {
        TRACE_INFO("bitmap download url NULL\r\n");
        return DDI_EINVAL;
    }
    //TRACE_INFO("bitmap download url：%s\r\n", ret_string);
    memset(buf, 0, sizeof(buf));
    buf[0] = ret_string[0];
    num = trendit_atoi(buf);
    TRACE_INFO("bitmap download serial num:%d, url:%s\r\n", num, &ret_string[2]);

    if(num < 1 || num > 5)
    {
        TRACE_INFO("bitmap download serial num err!\r\n");
        return DDI_EDATA;
    }

    ddi_file_delete(bitmap_path[num-1]);

    tms_url_content_download((u8*)bitmap_path[num-1], &ret_string[2], 1, bitmap_download_callback); /*TODO 是否需要做成同步？*/

    return DDI_OK;
}

s32 cmd_command_machine_modify_apikey(s8 *value_string, s32 value_string_len)
{
    hwsn_info_t l_hwsn_info;
    s32 ret = DDI_ERR;

    do{
        if(NULL == value_string || 0 == value_string_len)
        {
            ret = DDI_EINVAL;
            break;
        }

        memset(&l_hwsn_info, 0, sizeof(l_hwsn_info));
        ret = ddi_misc_ioctl(MISC_IOCTL_GETHWSNANDSNKEY, 0, (u32)&l_hwsn_info);

        snprintf(l_hwsn_info.m_snkey, sizeof(l_hwsn_info.m_snkey), "%s", value_string);
        l_hwsn_info.m_snkeylen = strlen(l_hwsn_info.m_snkey);

        ret = ddi_misc_ioctl(MISC_IOCTL_SETHWSNANDSNKEY,(u32)&l_hwsn_info, 0);  //设置SN
    }while(0);

    return ret;
}

static s32 cmd_command_machine_audio_stream(u8 *data, u32 data_len, DOWNLOAD_STEP_E download_step,
                                            upgrade_callback_info_t *upgrade_callback_info)
{
    s32 i = 0;
    u8 *down_file_data;
    //TRACE_INFO("******************data_len %d\r\n",data_len);

    trendit_save_file_data(data,data_len);
    //down_file_data = audiofile_instance();
    //memcpy(down_file_data+, data, data_len);

    if(download_step == DOWNLOADED)
    {

        trendit_download_curr_audio_file();

        TRACE_INFO("******************download success\r\n");

    }
#if 0
    for(i = 0; i< 100; i++)
    {
        TRACE_INFO("%x,",data[i]);
    }
#endif
}

static void cmd_command_machine_stream_schedule(u8 *url)
{
    s32 ret;
    upgrade_info_t upgrade_info;
    upgrade_callback_info_t upgrade_callback_info;

    memset(&upgrade_info, 0, sizeof(upgrade_info_t));
    upgrade_callback_info.m_backend_cmd = data_parser_instance()->get_current_cmd();
    upgrade_callback_info.m_packno = data_parser_instance()->get_current_packno();
    upgrade_callback_info.m_lparam = NULL;
    //
    upgrade_info.m_download_file_type = FT_AUDIO;
    upgrade_info.m_store_filepath = NULL;
    upgrade_info.self_save_cb= cmd_command_machine_audio_stream;
    upgrade_info.m_download_url = url;
    upgrade_info.m_upgrade_callback_info = &upgrade_callback_info;
    upgrade_info.upgrade_result_callback = upgrade_result_callback;
    strcpy(upgrade_info.m_default, "audio download");

    ret = trendit_http_schedule_add(&upgrade_info, 0);
    TRACE_INFO("\t.add schedule to download audio with %d.\r\n", ret);
}

static s32 cmd_command_machine_download_audio(s8 *ret_string, s32 ret_stringlen)
{
    s32 ret = DDI_ERR;
    char filename[128] = {0};
    s8 dst[2][SPLIT_PER_MAX];
    s32 num = 0;

    if(ret_string == NULL)
    {
        TRACE_INFO("audio download url NULL\r\n");
        return DDI_EINVAL;
    }

    memset(dst, 0, sizeof(dst));
    ret = trendit_split(dst, sizeof(dst)/sizeof(dst[0]), ret_string, ",", SPLIT_NOIGNOR);
    if(ret != sizeof(dst)/sizeof(dst[0]))
    {
        TRACE_ERR("param is invalid:%s", ret_string);
        return DDI_EINVAL;
    }

    TRACE_INFO("file name:%s,url:%s\r\n",dst[0],dst[1]);

    #if 1

    //文件存在，则返回成功
    if(trendit_search_file_status(dst[0]) == DDI_OK)
    {
        TRACE_ERR("file exist\r\n");
        return DDI_OK;
    }
    //搜索可下载位置
    if(trendit_search_download_status(dst[0]) != DDI_OK)
    {
        TRACE_ERR("file full\r\n");
        return DDI_EINVAL;
    }
    //trendit_audio_vfs_close();
    #endif
    TRACE_INFO("start download audio\r\n");

    if(audiofile_instance() == NULL)
    {

        return DDI_ERR;
    }
    //ddi_misc_msleep(200);
    cmd_command_machine_stream_schedule(dst[1]);

    //memset(filename, 0, sizeof(filename));

    //snprintf(filename, sizeof(filename), "%s", dst[0]);


    //ddi_file_delete(bitmap_path[num-1]);

    //tms_url_content_download((u8*)bitmap_path[num -1], &ret_string[1], 1, bitmap_download_callback); /*TODO 是否需要做成同步？*/
    //trendit_audio_vfs_close();//放到下载成功后关闭
    return DDI_OK;

}

static s32 cmd_command_machine_update_audio(s8 *ret_string, s32 ret_stringlen)
{
    s32 ret = DDI_ERR;
    char filename[128] = {0};
    s8 dst[2][SPLIT_PER_MAX];
    s32 num = 0;

    if(ret_string == NULL)
    {
        TRACE_INFO("audio update string NULL\r\n");
        return DDI_EINVAL;
    }

    memset(dst, 0, sizeof(dst));
    ret = trendit_split(dst, sizeof(dst)/sizeof(dst[0]), ret_string, ",", SPLIT_NOIGNOR);
    if(ret != sizeof(dst)/sizeof(dst[0]))
    {
        TRACE_ERR("param is invalid:%s", ret_string);
        return DDI_EINVAL;
    }

    TRACE_INFO("file name:%s,url:%s\r\n",dst[0],dst[1]);
    //文件存在，则更新
    if(trendit_search_file_status(dst[0]) != DDI_OK)
    {
        return DDI_ENOFILE;
    }
    if(audiofile_instance() == NULL)
    {
        TRACE_ERR("audiofile instance err\r\n");
        return DDI_ERR;
    }

    //ddi_misc_msleep(200);
    //TRACE_INFO("start updater111\n");
    cmd_command_machine_stream_schedule(dst[1]);

    //trendit_audio_vfs_close();//放到更新成功后关闭
    return DDI_OK;
}


static s32 cmd_command_machine_delete_audio(s8 *ret_string, s32 ret_stringlen)
{
    s32 ret = DDI_ERR;
    u16 num = 0;

    if(ret_string == NULL)
    {
        TRACE_INFO("audio delete name NULL\r\n");
        return DDI_EINVAL;
    }


    ret = trendit_delete_audio_file(ret_string);

    return ret;

}

s32 cmd_command_machine_modify_log_level(s8 *value_string, s32 value_string_len)
{
    s8 dst[2][SPLIT_PER_MAX];
    s32 ret = DDI_ERR;
    u16 model_value = 0;

    do{
        memset(dst, 0, sizeof(dst));
        ret = trendit_split(dst, sizeof(dst)/sizeof(dst[0]), value_string, ",", SPLIT_NOIGNOR);
        if(ret != sizeof(dst)/sizeof(dst[0]))
        {
            ret = DDI_EINVAL;
            break;
        }

        trendit_trim(dst[0]);
        model_value = trendit_asc_to_u32(dst[0], strlen(dst[0]), MODULUS_HEX);
        if(0xff == model_value && DEBUG == trendit_atoi(dst[1]))
        {
            trendit_cdc_output_switch(TRUE);
            TRACE_INFO("open cdc log");
            ret = DDI_OK;
        }
    }while(0);

    return ret;
}

static s32 exec_cfg(u16 cmd, u8 *value_string, u16 value_stringlen)
{
    s32 ret = DDI_ERR;
    event_model_t *event_model = NULL;

    TRACE_DBG("cmd:%04x, value_string:%s", cmd, value_string);
    switch(cmd)
    {
        case CFG_COMAND_MACHINE_BACKEND_CON:
            ret = cfg_command_machine_backend_con(value_string);
            break;

        case CFG_COMAND_MACHINE_HBT_INTERVAL:
            ret = cfg_command_machine_hbt_interval(value_string);
            break;

        case CFG_COMAND_MACHINE_COMMU_STRATEGY:
            ret = cfg_command_machine_commu_strategy(value_string);
            break;

        case CFG_COMAND_MACHINE_NET_STRATEGY_CON_TIME:
            ret = cfg_command_machine_net_strategy_con_time(value_string);
            break;

        case CFG_COMAND_MACHINE_POWEROFF_KEY_DELAY_TIME:
            ret = cfg_command_machine_poweroff_key_delay_time(value_string);
            break;

        case CFG_COMAND_MACHINE_CFG_NTP:
            ret = cfg_command_machine_cfg_ntp(value_string, value_stringlen);
            break;

        case CFG_COMAND_MACHINE_TMS_CFG:
            ret = cfg_comand_machine_tms_cfg(value_string, value_stringlen);
            break;
            
        case CFG_COMAND_MACHINE_WIFI_AIRKISS_URL:
            ret = cfg_command_machine_wifi_airkill_url(value_string, value_stringlen);
            break;

        case EVENT_HBT:
        case EVENT_ONLINE:
        case EVENT_CHECK_DEVICE:
        case EVENT_POWERON:
        case EVENT_VER:
        case EVENT_PAPER_CHANGE:
            event_model = event_instance();
            if(event_model)
            {
                ret = event_model->cfg_event(cmd, value_string, value_stringlen);
            }
            break;

        default:
            ret = DDI_EUNKOWN_CMD;
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
        case CFG_QRY_MACHINE_MEM_INFO:
            ret = qry_command_memory_info(ret_string, ret_stringlen);
            break;

        case CFG_QRY_MACHINE_ALL_VERSION:
            ret = qry_command_machine_version(ret_string, ret_stringlen);
            break;

        case CFG_QRY_MACHINE_SN_KEY:
            ret = qry_command_machine_key(ret_string, ret_stringlen);
            break;
        
        case CFG_QRY_MACHINE_PWO_CHECK_RES:
            ret = qry_command_machine_pwo_check_res(ret_string, ret_stringlen);
            break;

        case CFG_QRY_MACHINE_CURRENT_COMMU_CHANNEL:
            ret = qry_command_current_commu_channel(ret_string, ret_stringlen);
            break;

        case CFG_QRY_MACHINE_LAST_DROP_TIME:
            ret = qry_command_last_drop_time(ret_string, ret_stringlen);
            break;

        case CFG_QRY_MACHINE_LAST_ONLINE_TIME:
            ret = qry_command_last_online_time(ret_string, ret_stringlen);
            break;

        case CFG_QRY_MACHINE_ONLINE_REASON:
            ret = qry_command_online_reason(ret_string, ret_stringlen);
            break;

        case CFG_QRY_MACHINE_LAST_POWEROFF_REASON:
            snprintf(ret_string, ret_stringlen, "%d", machine_instance()->m_power_on_reason);
            ret = DDI_OK;
            break;

        case CFG_QRY_MACHINE_RUN_TIME:
            snprintf(ret_string, ret_stringlen, "%u", trendit_get_ticks()/1000);
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

    TRACE_DBG("cmd:%04x, value_string:%s", cmd, value_string);

    switch(cmd)
    {
        case CMD_COMMAND_MACHINE_UPGRADE_APP:
            ret = tms_machine_command_parse(cmd, value_string, value_stringlen, ret_string, ret_stringlen);
            break;
        
        case CMD_COMMAND_MACHINE_UPGRADE_CORE:
            ret = tms_machine_command_parse(cmd, value_string, value_stringlen, ret_string, ret_stringlen);
            break;
        
        case CMD_COMMAND_MACHINE_REBOOT:
            ret = cmd_command_machine_reboot(ret_string, ret_stringlen);
            break;

        case CMD_COMMAND_MACHINE_DOWNLOAD_BITMAP:
            ret = tms_machine_command_parse(cmd, value_string, value_stringlen, ret_string, ret_stringlen);
            break;

        case CMD_COMMAND_MACHINE_MODIFY_BITMAP:
            ret = cmd_command_machine_modify_bitmap(value_string, value_stringlen);
            break;

        case CMD_COMMAND_MACHINE_DELETE_BITMAP:
            ret = cmd_command_machine_delete_bitmap(value_string, value_stringlen);
            break;

        case CMD_COMMAND_MACHINE_MODIFY_APIKEY:
            ret = cmd_command_machine_modify_apikey(value_string, value_stringlen);
            break;
        case CMD_COMMAND_MACHINE_DOWNLOAD_AUDIO:
            ret = cmd_command_machine_download_audio(value_string, value_stringlen);
            break;
        case CMD_COMMAND_MACHINE_UPDATE_AUDIO:
            ret = cmd_command_machine_update_audio(value_string, value_stringlen);
            break;
        case CMD_COMMAND_MACHINE_DELETE_AUDIO:
            ret = cmd_command_machine_delete_audio(value_string, value_stringlen);
            break;

        case CMD_COMMAND_MACHINE_MODIFY_LOG_LEVEL:
            ret = cmd_command_machine_modify_log_level(value_string, value_stringlen);
            break;

        default:
            ret = DDI_EUN_SUPPORT;
            break;
    }

    return ret;
}

static void online_callback(void)
{
    machine_module_t *machine_module = machine_instance();
    event_model_t *event_model = event_instance();

    if(!machine_module->m_have_send_poweron_msg)
    {
        if(event_model)
        {
            event_model->send_event(EVENT_POWERON, DDI_OK, NULL, 0);
            event_model->send_event(EVENT_VER, DDI_OK, NULL, 0);
            event_model->send_event(EVENT_CHECK_DEVICE, DDI_OK, NULL, 0);
        }

        machine_module->m_have_send_poweron_msg = TRUE;
    }
}

static s32 machine_msg_handle(struct _msg *m_msg)
{
    s32 ret = MSG_KEEP;
    u8 key;
    printer_module_t *printer_module = printer_instance();
    machine_module_t *machine_module = machine_instance();
    event_model_t *event_model = event_instance();
    machine_error_model_t *error_model = error_model_instance();

    do{
        if(MSG_TERMINAL == m_msg->m_status)
        {
            ret = MSG_OUT;
            break;
        }
        else if(MSG_INIT == m_msg->m_status)
        {
            machine_module->m_heart_beat_beg_ticks = trendit_get_ticks();
            m_msg->m_status = MSG_SERVICE;
            ret = MSG_KEEP;
            break;
        }
        else
        {
            if(trendit_query_timer(machine_module->m_thread_sleep_ticks, CHECK_PAPER_INTERVAL_TIME))
            {
                machine_module->m_thread_sleep_ticks = trendit_get_ticks();
                ret = printer_module->check_paper();
                if(ret != machine_module->m_paper_status)
                {
                    machine_module->m_paper_status = ret;
                    TRACE_INFO("parper change to %s", ret?"no paper":"have paper");
                    if(event_model && tcp_protocol_instance()->check_tcp_connect_success())
                    {
                        event_model->send_event(EVENT_PAPER_CHANGE, DDI_OK, NULL, 0);
                    }

                    if(error_model)
                    {
                        //缺纸
                        if(ret)
                        {
                            TRACE_INFO("parper don't detect");
                            audio_instance()->audio_play(AUDIO_DEVICE_OUT_PAPER, AUDIO_PLAY_BLOCK);
                            error_model->notify_error(ERR_OUT_OF_PAPAER);
                        }
                        else
                        {
                            error_model->clear_error(ERR_OUT_OF_PAPAER);
                        }
                    }
                }
            }
            
            if(!tcp_protocol_instance()->check_tcp_connect_success())
            {
                m_msg->m_status = MSG_INIT;
                ret = MSG_KEEP;
                break;
            }

            if(http_download_routine_status())
            {
                m_msg->m_status = MSG_INIT;
                ret = MSG_KEEP;
                break;
            }
            
            if(trendit_query_timer(machine_module->m_heart_beat_beg_ticks, machine_module->m_heart_beat_interval*1000))
            {
                TRACE_INFO("overtime, trigger hbt event");
                machine_module->m_heart_beat_beg_ticks = trendit_get_ticks();
                if(event_model)
                {
                    event_model->send_event(EVENT_HBT, DDI_OK, NULL, 0);
                }
            }

            ret = MSG_KEEP;
        }
    }while(0);

    return ret;
}

static s32 post_machine_msg()
{
    msg_t *loop_msg = NULL;
    s32 ret = DDI_ERR;

    loop_msg = (msg_t *)k_mallocapp(sizeof(msg_t));
    if(NULL == loop_msg)
    {
        TRACE_ERR("malloc failed");
    }
    else
    {
        TRACE_DBG("machine_msg:%x", loop_msg);
        memset(loop_msg, 0, sizeof(msg_t));
        loop_msg->m_func = machine_msg_handle;
        loop_msg->m_priority = MSG_NORMAL;
        loop_msg->m_lParam = 0;
        loop_msg->m_wparam = 0;
        loop_msg->m_status = MSG_INIT;
        snprintf(loop_msg->m_msgname, sizeof(loop_msg->m_msgname), "%s", "machine");
        trendit_postmsg(loop_msg);
    }

    return ret;
}

static s32 get_power_key_delay_time(void)
{
    return machine_instance()->m_power_key_delay_time;
}

/**
 * @brief 整机的初始化函数
 * @retval  整机的设备结构体
 */
machine_module_t *machine_instance(void)
{
    s32 len = 0;
	tms_schedule_module_t *tms_schedule;

    if(g_model_machine)
    {
        return g_model_machine;
    }

    g_model_machine = k_mallocapp(sizeof(machine_module_t));
    if(NULL == g_model_machine)
    {
        TRACE_ERR("malloc fail");
        return NULL;
    }

    memset(g_model_machine, 0, sizeof(machine_module_t));
    INIT_SUPER_METHOD(g_model_machine, MODEL_MACHINE, "machine");
    g_model_machine->get_power_key_delay_time = get_power_key_delay_time;

    g_model_machine->super.load_cfg(&(g_model_machine->super), MACHINE_CONFIG_FILE, exec_cfg);
    g_model_machine->m_paper_status = FALSE;     //默认有纸，这样开机后，若没纸张，会报缺纸
    g_model_machine->m_power_on_reason = system_data_instance()->get_power_reason();
    //启动网络策略
    g_model_machine->m_network_strategy = network_strategy_instance();
    if(g_model_machine->m_network_strategy)
    {
        g_model_machine->m_network_strategy->start();
        g_model_machine->m_network_strategy->reg_online_callback(online_callback);
    }

	tms_schedule = tms_schedule_instance();
	 if (tms_schedule && tms_schedule->base.init)
	 {
        tms_schedule->base.init();
     }

    if(0 == strlen(g_model_machine->m_ntp_info[0]))
    {
        len = sizeof(g_model_machine->m_ntp_info[0]);
        snprintf(g_model_machine->m_ntp_info[0], len, "%s", "cn.ntp.org.cn");
        snprintf(g_model_machine->m_ntp_info[1], len, "%s", "ntp.sjtu.edu.cn");
        snprintf(g_model_machine->m_ntp_info[2], len, "%s", "us.pool.ntp.org");
    }
    post_machine_msg();

    return g_model_machine;
}



