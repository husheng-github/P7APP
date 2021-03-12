#include "tms_upgrade.h"

#define TMS_MACHINE_PLATFORM_NAME           "MTK"
#define TMS_MACHINE_FIRMWARE_CLASS          "TREN_5703"
#define TMS_REMOTE_PRIMARY_PORT             80
#define TMS_REMOTE_STANDBY_PORT             TMS_REMOTE_PRIMARY_PORT
#define TMS_REMOTE_DEBUGING_PORT            3021


/********************************************************************************
 *
 *
 *       升级文件下载并升级
 *
 */
static s32 tms_firmware_download_stream(u8 *data, u32 data_len, DOWNLOAD_STEP_E download_step, upgrade_callback_info_t *info)
{
    s32 ret = -1;

    STD_PRINT("data: %s, len: %d, step: %d.\r\n", data, data_len, download_step);
    if (!info)
    {
         STD_PRINT("illeage para.\r\n");
    }
    else
    {   switch (download_step)
        {
        case DOWNLOADING:
            STD_PRINT("\t.continuous receiving...\r\n");
            break;
        case DOWNLOADED:
            STD_PRINT("\t.continuous receive finished.\r\n");
            break;
        case DOWNLOADERR:
            STD_PRINT("\t.unable to check remote version.\r\n");
            break;
        }
    }
}

static void tms_firmware_download_notify(upgrade_callback_info_t *info, UPGRADE_STEP_E upgrade_step, s32 result)
{
    STD_PRINT("result: %d, step: %d.\r\n", result, upgrade_step);
    if (!info)
    {
         STD_PRINT("illeage para.\r\n");
    }
    else
    {
        STD_PRINT("cmd: 0x%x, param: %d, pack_no: %d.\r\n", info->m_backend_cmd, info->m_lparam, info->m_packno);
    }
}

static s32 tms_firmware_url_download_request(u8 *url)
{
    s32 ret;
    upgrade_info_t upgrade_info;
    upgrade_callback_info_t cb_upgrade_info;

    cb_upgrade_info.m_backend_cmd = 01;
    cb_upgrade_info.m_packno      = 99;
    cb_upgrade_info.m_lparam      = 0;
    //
    upgrade_info.m_download_file_type    = FT_BIN_FILE;
    upgrade_info.m_store_filepath        = NULL;
    upgrade_info.self_save_cb            = tms_firmware_download_stream;
    upgrade_info.m_download_url          = url;
    upgrade_info.m_upgrade_callback_info = &cb_upgrade_info;
    upgrade_info.upgrade_result_callback = tms_firmware_download_notify;
    strcpy(upgrade_info.m_default, "smtgmemte");
    //
    ret = trendit_http_schedule_add(&upgrade_info, 0);
    STD_PRINT("\t.ret: %d, url: %s\r\n", ret, url);

    return ret;
}

/********************************************************************************
 *
 *
 *       请求最新版本地址
 *
 */

static s32 tms_firmware_url_response_parse(u8 *data, u32 len)
{
    s32 ret = -1;
    s8 *fw_url, *url_end, *url_raw = NULL;

    if (!data)
    {
        STD_PRINT("illeage para.\r\n");
    }
    else
    {
        fw_url = strstr(data, "\"url\":\"");
        if (!fw_url || !strstr(data, "\"message\":\"found url success\""))
        {
            STD_PRINT("\tillegal para, url field not found.\r\n");
        }
        else
        {
            fw_url += 7;
            url_end = strchr(fw_url, '\"');
            if (!url_end || 512 < url_end - fw_url)
            {
                STD_PRINT("\tillegal para, url end field not found.\r\n");
            }
            else
            {
                url_raw = STD_MALLOC(url_end - fw_url + 1);
                if (!url_raw)
                {
                    STD_PRINT("\tillegal para, alloc memory failed.\r\n");
                }
                else
                {
                    memcpy(url_raw, fw_url, url_end - fw_url);
                    url_raw[url_end - fw_url] = '\0';
                    ret = tms_firmware_url_download_request(url_raw);
                }
            }
        }
    }

    if (url_raw) STD_FREE(url_raw);

    return ret;
}

static s32 tms_request_url_stream(u8 *data, u32 len, DOWNLOAD_STEP_E download_step, upgrade_callback_info_t *info)
{
    s32 ret = -1;

    STD_PRINT("data: %s, len: %d, step: %d.\r\n", data, len, download_step);
    if (!info)
    {
         STD_PRINT("illeage para.\r\n");
    }
    else
    {   switch (download_step)
        {
        case DOWNLOADING:
            STD_PRINT("\t.continuous receiving...\r\n");
            break;
        case DOWNLOADED:
            ret = tms_firmware_url_response_parse(data, len);
            STD_PRINT("\t.continuous receive finished.\r\n");
            break;
        case DOWNLOADERR:
            STD_PRINT("\t.unable to check remote version.\r\n");
            break;
        }
    }
}

static void tms_request_url_notify(upgrade_callback_info_t *info, UPGRADE_STEP_E upgrade_step, s32 result)
{
    STD_PRINT("result: %d, step: %d.\r\n", result, upgrade_step);
    if (!info)
    {
         STD_PRINT("illeage para.\r\n");
    }
    else
    {
        STD_PRINT("cmd: 0x%x, param: %d, pack_no: %d.\r\n", info->m_backend_cmd, info->m_lparam, info->m_packno);
    }
}

static s32 tms_request_new_version_url(s8 *ver_old, s8 *ver_new)
{
    s32 ret = -1;
    s8 *full_url;
    s8 *fw_class = TMS_MACHINE_FIRMWARE_CLASS;
    s8 *url_host = "47.106.157.121";
    s8 *url_page = "api/firmware_common/firmware";
    u16 url_port = TMS_REMOTE_DEBUGING_PORT;
    upgrade_info_t upgrade_info;
    upgrade_callback_info_t cb_upgrade_info;

    full_url = STD_MALLOC(256);
    if (!ver_old || !ver_new || !full_url)
    {
        STD_PRINT("\t.illeage para.\r\n");
    }
    else
    {
        memset(full_url, 0, 256);
        ret = sprintf(full_url, "%s://%s:%d/%s?targetFirmwareCode=%s&firmwareClass=%s&currentFirmwareCode=%s",
                                "http", url_host, url_port, url_page, ver_new, fw_class, ver_old);
        //
        cb_upgrade_info.m_backend_cmd = 01;
        cb_upgrade_info.m_packno      = 99;
        cb_upgrade_info.m_lparam      = 0;
        //
        upgrade_info.m_download_file_type    = FT_TEXT;
        upgrade_info.m_store_filepath        = NULL;
        upgrade_info.self_save_cb            = tms_request_url_stream;
        upgrade_info.m_download_url          = full_url;
        upgrade_info.m_upgrade_callback_info = &cb_upgrade_info;
        upgrade_info.upgrade_result_callback = tms_request_url_notify;
        strcpy(upgrade_info.m_default, "smtgmemte");
        //
        ret = trendit_http_schedule_add(&upgrade_info, 0);

        STD_PRINT("add schedule to download picture with %d.\r\n", ret);

    }

    if (full_url) STD_FREE(full_url);

    return ret;
}

/********************************************************************************
 *
 *
 *       检测版本
 *
 */
static s32 tms_remote_version_parse(u8 *data, u32 len, u8 *new_version)
{
    s32 ret = -1;
    s8 *str_platform, *fw_class, *fw_code, *p;

    str_platform = STD_MALLOC(32);
    fw_class     = STD_MALLOC(32);

    if (!data || !new_version || !str_platform || !fw_class)
    {
        STD_PRINT("\tillegal para, data: 0x%x, version: 0x%x, platform: 0x%x, class: 0x%x\r\n",
                  data, new_version, str_platform, fw_class);
    }
    else
    {
        sprintf(str_platform, "\"platformName\":\"%s\"", TMS_MACHINE_PLATFORM_NAME);
        sprintf(fw_class, "\"firmwareClass\":\"%s\"", TMS_MACHINE_FIRMWARE_CLASS);
        if (!strstr(data, "\"message\":\"found firmwares\"")
            || !strstr(data, str_platform)
            || !strstr(data, fw_class))
        {
            STD_PRINT("\tillegal para, field not found.\r\n");
        }
        else
        {
            fw_code = strstr(data, "\"firmwareCode\":\"");
            if (!fw_code)
            {
                STD_PRINT("\tillegal para, firmwareCode field not found.\r\n");
            }
            else
            {
                fw_code += 16;
                p = strchr(fw_code, '\"');
                if (!p || (16 < p - fw_code))
                {
                    STD_PRINT("\tillegal para, firmwareCode field error\r\n");
                }
                else
                {
                    memcpy(new_version, fw_code, p - fw_code);
                    ret = 0;
                }
            }
        }
    }

    if (str_platform) STD_FREE(str_platform);
    if (fw_class)     STD_FREE(fw_class);


    return ret;
}

static s32 tms_check_version_response_parse(u8 *data, u32 len)
{
    s32 ret = -1;
    s8 *ver_old, *ver_new;

    ver_old  = STD_MALLOC(8);
    ver_new  = STD_MALLOC(8);
    if (!data || !ver_old || !ver_new)
    {
         STD_PRINT("\t.illeage para.\r\n");
    }
    else
    {
        memset(ver_old, 0, 8);
        memset(ver_new, 0, 8);
        ret  = tms_version_code_read(ver_old);
        ret |= tms_remote_version_parse(data, len, ver_new);
        if (0 != ret)
        {
             STD_PRINT("\t.parse remote version failed.\r\n");
        }
        else
        {
            ret = tms_request_new_version_url(ver_old, ver_new);
        }
    }

    if (ver_old)  STD_FREE(ver_old);
    if (ver_new)  STD_FREE(ver_old);

    return ret;
}

static s32 tms_version_check_stream(u8 *data, u32 len, DOWNLOAD_STEP_E download_step, upgrade_callback_info_t *info)
{
    s32 ret = -1;

    STD_PRINT("data: %s, len: %d, step: %d.\r\n", data, len, download_step);
    if (!info)
    {
         STD_PRINT("illeage para.\r\n");
    }
    else
    {
        STD_PRINT("cmd: 0x%x, param: %d, pack_no: %d.\r\n", info->m_backend_cmd, info->m_lparam, info->m_packno);
        switch (download_step)
        {
        case DOWNLOADING:
            STD_PRINT("\t.continuous receiving...\r\n");
            break;
        case DOWNLOADED:
            ret = tms_check_version_response_parse(data, len);
            STD_PRINT("\t.continuous receive finished.\r\n");
            break;
        case DOWNLOADERR:
            STD_PRINT("\t.unable to check remote version.\r\n");
            break;
        }

    }
    return 0;
}

static void tms_version_check_notify(upgrade_callback_info_t *info, UPGRADE_STEP_E upgrade_step, s32 result)
{
    STD_PRINT("result: %d, step: %d.\r\n", result, upgrade_step);
    if (!info)
    {
         STD_PRINT("illeage para.\r\n");
    }
    else
    {
        STD_PRINT("cmd: 0x%x, param: %d, pack_no: %d.\r\n", info->m_backend_cmd, info->m_lparam, info->m_packno);
    }
}

static s32 tms_machine_sn_read(u8 *dev_sn)
{
    s32 ret = 0;
    u8  lens;

    ddi_misc_read_dsn(SNTYPE_POSSN, dev_sn);
    lens = dev_sn[0];
    if (24 <= lens || lens <= 0)
    {
        ret = -1;
    }
    else
    {
        memcpy(&dev_sn[0], &dev_sn[1], lens);
        dev_sn[lens] = '\0';
    }
    STD_PRINT("\t.read sn: %s with %d.\r\n", dev_sn, ret);

    return ret;
}

static s32 tms_version_code_read(u8 *ver_code)
{
    s32 ret = 0;

    strcpy(ver_code, "V0.0.1");

    STD_PRINT("\t.read local version: %s with %d.\r\n", ver_code, ret);

    return ret;
}

// http://47.106.157.121:3021/api/firmware_common/latest_version?deviceSn=00005702160030000305&firmwareClass=TREN_5603&currentFirmwareCode=V0.0.1
static s32 tms_upgrade_version_check(void)
{
    s32 ret;
    s8 *device_sn, *ver_code, *full_url;
    s8 *fw_class = TMS_MACHINE_FIRMWARE_CLASS;
    s8 *url_host = "47.106.157.121";
    s8 *url_page = "api/firmware_common/latest_version";
    u16 url_port = TMS_REMOTE_DEBUGING_PORT;
    upgrade_info_t upgrade_info;
    upgrade_callback_info_t cb_upgrade_info;

    device_sn = STD_MALLOC(32);
    ver_code  = STD_MALLOC(8);
    full_url  = STD_MALLOC(256);
    if (!device_sn || !ver_code || !full_url)
    {
        ret = -1;
    }
    else
    {
        memset(device_sn, 0, 32);
        memset(ver_code, 0, 8);
        memset(full_url, 0, 256);
        ret  = tms_machine_sn_read(device_sn);
        ret |= tms_version_code_read(ver_code);
        if (0 == ret)
        {
            ret = sprintf(full_url, "%s://%s:%d/%s?deviceSn=%s&firmwareClass=%s&currentFirmwareCode=%s",
                                    "http", url_host, url_port, url_page, device_sn, fw_class, ver_code);
            //
            cb_upgrade_info.m_backend_cmd = 01;
            cb_upgrade_info.m_packno      = 99;
            cb_upgrade_info.m_lparam      = 0;
            //
            upgrade_info.m_download_file_type    = FT_TEXT;
            upgrade_info.m_store_filepath        = NULL;
            upgrade_info.self_save_cb            = tms_version_check_stream;
            upgrade_info.m_download_url          = full_url;
            upgrade_info.m_upgrade_callback_info = &cb_upgrade_info;
            upgrade_info.upgrade_result_callback = tms_version_check_notify;
            strcpy(upgrade_info.m_default, "smtgmemte");
            //
            ret = trendit_http_schedule_add(&upgrade_info, 0);

            STD_PRINT("add schedule to check new version with %d.\r\n", ret);
        }
    }

    if (device_sn) STD_FREE(device_sn);
    if (ver_code)  STD_FREE(ver_code);
    if (full_url)  STD_FREE(full_url);

    return ret;
}

void tms_upgrade_process_routine(void)
{
    tms_upgrade_version_check();
}

