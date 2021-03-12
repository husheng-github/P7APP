#include "ddiglobal.h"
#include "app_global.h"

static reg_dev_t reg_dev_a[MODEL_MACHINE];
static model_device_t *g_devs[MODEL_MACHINE];

/**
 * @brief 初始化device所有的全局变量
 */
#if 0
void trendit_init_device_data(void)
{
    memset(reg_dev_a, 0, sizeof(reg_dev_t));
    reg_dev_a[0].mod = MODEL_WIRELESS;
    reg_dev_a[0].m_instance = wireless_instance;
    reg_dev_a[1].mod = MODEL_WIFI;
    reg_dev_a[1].m_instance = wifi_instance;
    reg_dev_a[2].mod = MODEL_PRINTER;
    reg_dev_a[2].m_instance = printer_instance;
    reg_dev_a[3].mod = MODEL_AUDIO;
    reg_dev_a[3].m_instance = audio_instance;
    reg_dev_a[4].mod = MODEL_KEY;
    reg_dev_a[4].m_instance = key_instance;
    reg_dev_a[5].mod = MODEL_USBCDC;
    reg_dev_a[5].m_instance = usbcdc_instance;
    reg_dev_a[6].mod = MODEL_PRINTPORT;
    reg_dev_a[6].m_instance = printport_instance;
    reg_dev_a[7].mod = MODEL_TMS;
    reg_dev_a[7].m_instance = tms_instance;
    reg_dev_a[8].mod = MODEL_MACHINE;
    reg_dev_a[8].m_instance = machine_instance;
}
#else
const reg_dev_t g_reg_dev_tab[] = 
{
    {MODEL_WIRELESS, wireless_instance},
    {MODEL_WIFI,    wifi_instance},
    {MODEL_PRINTER, printer_instance},
    {MODEL_AUDIO,   audio_instance},
    {MODEL_KEY,     key_instance},
    {MODEL_USBCDC,  usbcdc_instance},
    {MODEL_PRINTPORT, printport_instance},
    {MODEL_TMS,     tms_instance},
    {MODEL_MACHINE,     machine_instance},
    {MODEL_NONE,    NULL},
};

#endif

/**
 * @brief 调用每个设备的instance函数获得设备节点，并注册
 */
void trendit_reg_dev()
{
    s32 i = 0;

  #if 0
    for(i = 0; i < sizeof(reg_dev_a)/sizeof(reg_dev_a[0]); i++)
    {
        if(reg_dev_a[i].mod > 0)
        {
            g_devs[reg_dev_a[i].mod-1] = reg_dev_a[i].m_instance();
        }

        if(MODEL_MACHINE == reg_dev_a[i].mod)
        {
            break;
        }
    }
  #else
    while(1)
    {
        if(g_reg_dev_tab[i].mod > 0)
        {
            g_devs[g_reg_dev_tab[i].mod-1] = g_reg_dev_tab[i].m_instance();
        }
        else
        {
            break;
        }

        ddi_watchdog_feed();
        i++;
    }
  #endif
}

/**
 * @brief 通过命令字获取设备节点
 * @retval  设备结构体
 */
model_device_t* trendit_get_model(u16 cmd)
{
    u8 model = cmd>>8;
    
    return g_devs[model-1];
}

/**
 * @brief 打印机器当前设备信息
 * @retval  无
 */
void trendit_print_machine_info(void)
{
    s8 sp_boot_ver[128] = {0};
    s8 sp_core_ver[128] = {0};
    cur_server_info_t cur_server_info;
    s8 tmp_buff[2048] = {0};
    s8 buff[1024] = {0};
    s32 len = 0;
    s32 i = 0;
    s32 ret = DDI_ERR;
    tcp_protocol_module_t *tcp_protocol_module = tcp_protocol_instance();
    hwsn_info_t l_hwsn_info;
    s8 pos_sn[32];
    s8 cfg_code[32];
    s8 sn_key[32];
    strSimInfo sim_info;
    model_device_t *model_device = NULL;
    s8 commu_mode = 0;
    s8 wireless_ver[128] = {0};
    s8 wifi_at_ver[128] = {0};
    s8 wifi_sdk_ver[128] = {0};

    memset(&l_hwsn_info, 0, sizeof(l_hwsn_info));
    ret = ddi_misc_ioctl(MISC_IOCTL_GETHWSNANDSNKEY, 0, (u32)&l_hwsn_info);

    memset(cfg_code, 0, sizeof(cfg_code));
    memset(sn_key, 0, sizeof(sn_key));
    if(l_hwsn_info.m_cfglen <= 32)
    {
        memcpy(cfg_code, l_hwsn_info.m_cfg, l_hwsn_info.m_cfglen);
    }

    if(l_hwsn_info.m_snkeylen <= 32)
    {
        memcpy(sn_key, l_hwsn_info.m_snkey, l_hwsn_info.m_snkeylen);
    }
    
    memset(sp_boot_ver, 0, sizeof(sp_boot_ver));
    memset(sp_core_ver, 0, sizeof(sp_core_ver));
    memset(pos_sn, 0, sizeof(pos_sn));
    ddi_misc_read_dsn(SNTYPE_POSSN, pos_sn);
    ddi_misc_get_firmwareversion(FIRMWARETYPE_APBOOT, sp_boot_ver);
    ddi_misc_get_firmwareversion(FIRMWARETYPE_APCORE, sp_core_ver);
    memset(&cur_server_info, 0, sizeof(cur_server_info));
    if(tcp_protocol_module)
    {
        tcp_protocol_module->get_server_info(&cur_server_info);
    }
    
    memset(tmp_buff, 0, sizeof(tmp_buff));
    memcpy(tmp_buff, "\x1B\x21\x00", 3);//字体设置为24X24
    len += 3;
    
    memcpy(tmp_buff+len, "\x1B\x61\x01", 3);//居中
    len += 3;

    memcpy(tmp_buff+len, "\x1D\x21\x11", 3); //放大一倍
    len += 3;
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", "设备信息\r\n");
    len += strlen("设备信息\r\n");
    
    memcpy(tmp_buff+len, "\x1D\x21\x00", 3); //还原
    len += 3;

    memcpy(tmp_buff+len, "\x1B\x61\x00", 3);//居左
    len += 3;

    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "机身号:%s\r\n密钥:%s\r\n配置码:%s\r\n", 
                                            pos_sn+1, sn_key, cfg_code);
    snprintf(buff+strlen(buff), sizeof(buff)-strlen(buff), "Boot:%s\r\nCore:%s\r\nApp:%s\r\n", 
                                            sp_boot_ver, sp_core_ver, APP_VER);
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", buff);
    len += strlen(buff);

    memset(wireless_ver, 0, sizeof(wireless_ver));
    memset(buff, 0, sizeof(buff));
    ret = ddi_gprs_ioctl(DDI_GPRS_GET_MODULE_VER, (u32)wireless_ver, sizeof(wireless_ver));
    snprintf(buff, sizeof(buff), "无线版本:%s\r\n", wireless_ver);
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", buff);
    len += strlen(buff);

    memset(buff, 0, sizeof(buff));
    memset(wifi_at_ver, 0, sizeof(wifi_at_ver));
    memset(wifi_sdk_ver, 0, sizeof(wifi_sdk_ver));
    ddi_wifi_ioctl(WIFI_IOCTL_GET_FIRMWARE_VER, WIFI_VER_AT, (u32)wifi_at_ver);
    ddi_wifi_ioctl(WIFI_IOCTL_GET_FIRMWARE_VER, WIFI_VER_SDK, (u32)wifi_sdk_ver);
    snprintf(buff, sizeof(buff), "Wifi AT:%s\r\nWifi sdk:%s\r\n", wifi_at_ver, wifi_sdk_ver);
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", buff);
    len += strlen(buff);
                
    model_device = &(machine_instance()->super);
    model_device->qry_from_cfg(model_device, CFG_COMAND_MACHINE_COMMU_STRATEGY, &commu_mode, sizeof(commu_mode));
    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "通讯策略:%d\r\n", commu_mode);
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", buff);
    len += strlen(buff);

    memset(buff, 0, sizeof(buff));
    snprintf(buff, sizeof(buff), "当前通讯方式:%s\r\n", cur_server_info.m_cur_commu_type);
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", buff);
    len += strlen(buff);

    memset(buff, 0, sizeof(buff));
    snprintf(buff+strlen(buff), sizeof(buff)-strlen(buff), "服务器IP地址:%s\r\n服务器端口号:%d\r\n本地ip:%s\r\n", 
                                            trendit_hide_word(cur_server_info.m_serverinfo),
                                            cur_server_info.m_server_port,
                                            cur_server_info.m_local_ip);

    if(COMMU_WIFI == network_strategy_instance()->m_current_type)
    {
        snprintf(buff+strlen(buff), sizeof(buff)-strlen(buff), "wifi名称:%s\r\n", 
                                                wifi_instance()->m_wifi_ssid);
    }
        
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", buff);
    len += strlen(buff);

    ddi_gprs_get_siminfo(&sim_info);
    if(strlen(sim_info.iccid))
    {
        snprintf(buff, sizeof(buff), "手机卡序号:%s\r\n", sim_info.iccid);
        snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", buff);
        len += strlen(buff);
    }
    
    snprintf(buff, sizeof(buff), "打印浓度:%d\r\n", printer_instance()->m_print_gray);
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", buff);
    len += strlen(buff);

    snprintf(buff, sizeof(buff), "声音大小:%d\r\n", audio_instance()->m_vol);
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", buff);
    len += strlen(buff);
    
    //支持的码制
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "\r\n支持码制:\r\n");
    len += strlen("\r\n支持码制:\r\n");

    //二维码
    memcpy(tmp_buff+len, "\x1B\x61\x01", 3);//居中
    len += 3;
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", "Qrcode\r\n");
    len += strlen("Qrcode\r\n");
    memcpy(tmp_buff+len, "\x1D\x71\x0C\x01\x09\x02\x31\x32\x33\x34\x35\x36\x37\x38\x39", 15);
    len += 15;
    memcpy(tmp_buff+len, "\r\n\x1B\x61\x01", 5);//居中
    len += 5;
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", "123456789");
    len += strlen("123456789");

    memcpy(tmp_buff+len, "\r\n\r\n\x1B\x61\x01", 7);//居中
    len += 7;
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", "UPC-A");
    len += strlen("UPC-A");
    memcpy(tmp_buff+len, "\x1d\x48\x02", 3);
    len += 3;
    memcpy(tmp_buff+len, "\r\n\x1d\x6b\x41\x0c\x31\x32\x33\x34\x35\x36\x37\x38\x39\x30\x31\x32", 18);
    len += 18;
    
    memcpy(tmp_buff+len, "\r\n\r\n\x1B\x61\x01", 7);//居中
    len += 7;
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", "UPC-E");
    len += strlen("UPC-E");
    memcpy(tmp_buff+len, "\x1d\x48\x02", 3);
    len += 3;
    memcpy(tmp_buff+len, "\r\n\x1d\x6b\x42\x0c\x30\x32\x33\x34\x35\x36\x30\x30\x30\x30\x38\x39", 18);
    len += 18;

    memcpy(tmp_buff+len, "\r\n\r\n\x1B\x61\x01", 7);//居中
    len += 7;
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", "JAN13");
    len += strlen("JAN13");
    memcpy(tmp_buff+len, "\x1d\x48\x02", 3);
    len += 3;
    memcpy(tmp_buff+len, "\r\n\x1d\x6b\x43\x0c\x30\x32\x33\x34\x35\x36\x30\x30\x30\x30\x38\x39", 18);
    len += 18;

    memcpy(tmp_buff+len, "\r\n\r\n\x1B\x61\x01", 7);//居中
    len += 7;
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", "JAN8");
    len += strlen("JAN8");
    memcpy(tmp_buff+len, "\x1d\x48\x02", 3);
    len += 3;
    memcpy(tmp_buff+len, "\r\n\x1d\x6b\x44\x08\x30\x32\x33\x34\x35\x36\x30\x30", 14);
    len += 14;

    memcpy(tmp_buff+len, "\r\n\r\n\x1B\x61\x01", 7);//居中
    len += 7;
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", "CODE39");
    len += strlen("CODE39");
    memcpy(tmp_buff+len, "\x1d\x48\x02", 3);
    len += 3;
    memcpy(tmp_buff+len, "\r\n\x1d\x6b\x45\x08\x30\x32\x33\x34\x35\x36\x30\x30", 14);
    len += 14;

    memcpy(tmp_buff+len, "\r\n\r\n\x1B\x61\x01", 7);//居中
    len += 7;
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", "ITF");
    len += strlen("ITF");
    memcpy(tmp_buff+len, "\x1d\x48\x02", 3);
    len += 3;
    memcpy(tmp_buff+len, "\r\n\x1d\x6b\x46\x08\x30\x32\x33\x34\x35\x36\x30\x30", 14);
    len += 14;

    memcpy(tmp_buff+len, "\r\n\r\n\x1B\x61\x01", 7);//居中
    len += 7;
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", "CODAB");
    len += strlen("CODAB");
    memcpy(tmp_buff+len, "\x1d\x48\x02", 3);
    len += 3;
    memcpy(tmp_buff+len, "\r\n\x1d\x6b\x47\x08\x41\x32\x33\x34\x35\x36\x30\x41", 14);
    len += 14;

    memcpy(tmp_buff+len, "\r\n\r\n\x1B\x61\x01", 7);//居中
    len += 7;
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", "CODE93");
    len += strlen("CODE93");
    memcpy(tmp_buff+len, "\x1d\x48\x02", 3);
    len += 3;
    memcpy(tmp_buff+len, "\r\n\x1d\x6b\x48\x08\x41\x30\x32\x33\x34\x35\x36\x41", 14);
    len += 14;

    memcpy(tmp_buff+len, "\r\n\r\n\x1B\x61\x01", 7);//居中
    len += 7;
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", "CODE128");
    len += strlen("CODE128");
    memcpy(tmp_buff+len, "\x1d\x48\x02", 3);
    len += 3;
    memcpy(tmp_buff+len, "\r\n\x1d\x6b\x49\x09\x7B\x41\x30\x32\x33\x34\x35\x36\x41", 15);
    len += 15;

    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", "\r\n\r\n\r\n");
    len +=6;

    printer_instance()->direct_print(tmp_buff, len);
}

/**
 * @brief 删除所有设备的配置文件，在本地或tms升级成功时执行
 * @param[in] 无
 * @retval  无
 */
void trenit_remove_all_device_cfg_file(void)
{
    model_device_t *model_device = NULL;
    s32 i = 0;
    system_data_instance_t *system_data = system_data_instance();
    tcp_protocol_module_t *tcp_instance = tcp_protocol_instance();
    audio_module_t *audio_model = audio_instance();

    //备份服务器ip、端口、声音大小
    snprintf(system_data->m_system_data.m_serverinfor, sizeof(system_data->m_system_data.m_serverinfor), "%s", tcp_instance->m_serverinfor);
    system_data->m_system_data.m_serverport = tcp_instance->m_serverport;
    system_data->m_system_data.m_vol = audio_model->m_vol;
    system_data->save_data();
    while(1)
    {
        if(g_reg_dev_tab[i].mod > 0)
        {
            model_device = g_devs[g_reg_dev_tab[i].mod-1];
            if(model_device)
            {
                if(DDI_OK == trendit_file_access(model_device->m_cfg_file_name))
                {
                    ddi_vfs_deletefile(model_device->m_cfg_file_name);
                }
            }
        }
        else
        {
            break;
        }

        ddi_watchdog_feed();
        i++;
    }
}

