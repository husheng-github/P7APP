#include "model_tms.h"
#include "model_tms_private.h"

/********************************************************************************
 *
 *
 *        内部函数声明
 *
 */
static s32 tms_utils_msg_handle(msg_t *m_msg);
static s32 tms_check_version_trigger(s8 index);
static s32 download_tms_file_append_head(append_info_t *append_info);
static s32 download_tms_file_receiving(u8 *data, s32 len, s32 status);
static s32 download_tms_file_complete(u8 *data, s32 len, s32 status);
//
static s32 tms_upgrade_result_query(void);
static s32 tms_version_newest_local(s8 *tms_ver);
static s32 tms_tcp_socket_create(tms_utils_t *tms_utils);
//
static s32 tms_download_binary_trigger(s8 *data, s32 len);
//
static void tms_network_connected_routine(void);


/********************************************************************************
 *
 *
 *        内部全局变量声明
 *
 */
static model_device_t *g_model_tms = NULL;      // 模块
static msg_t *g_tms_msg = NULL;                 // 消息
static tms_utils_t *g_tms_utils = NULL;         // 网络下载结构体
static apupdate_header_t g_apupdate_header;     // 组包信息结构体
static version_detail_t *g_binary_info = NULL;
static u8 g_upgrade_check_flag = 1;
static tms_ext_pfn g_cacp_upload_pfn = NULL;
static upgrade_info_t *g_upgrade_info = NULL;

static s32 g_file_downloading = 0;


/********************************************************************************
 *
 *
 *        空中协议部分
 *
 */
static void tms_upgrade_recode_reset_default(void)
{
    s32 ret;
    s8  dump[100] = "upgrade info reset command received, all related info will be zeroed!!!";

    ret = common_vfs_overwrite(TMS_UPGRADE_PROFILE, "upgrade info reset", 0x10, 0);
    ddi_file_insert(TMS_APUPDATE_FILE_NAME, dump, strlen(dump), 0);
    ddi_file_delete(TMS_APUPDATE_FILE_NAME);

    TMS_DEBUG("reset upgrade info with %d\r\n", ret);
}

static s32 tms_command_check_upgrade(s8 *url)
{
    s32 ret = 0;

    if (url && (0 == strcmp(TMS_PARTICULAR_RESET_URL, url)))
    {
        tms_upgrade_recode_reset_default();
        ret = -0xA5;
    }

    return ret;
}

s32 tms_machine_command_parse(u16 cmd, u8 *dst_data, u16 dst_lens, u8 *ret_data, u16 ret_lens)
{
    s32 ret = -1;
    upgrade_info_t upgrade_info;
    upgrade_callback_info_t callback_info;

    memset(&upgrade_info, 0, sizeof(upgrade_info_t));
    memset(&callback_info, 0, sizeof(upgrade_callback_info_t));
    upgrade_info.m_download_url   = TMS_MALLOC(dst_lens + 1);
    upgrade_info.m_store_filepath = TMS_MALLOC(0x20 + 1);
    if (upgrade_info.m_download_url && upgrade_info.m_store_filepath)
    {
        memset(upgrade_info.m_download_url, 0, dst_lens + 1);
        memset(upgrade_info.m_store_filepath, 0, 0x20 + 1);
        memset(upgrade_info.m_upgrade_callback_info, 0, sizeof(upgrade_callback_info_t));
        //
        callback_info.m_packno      = data_parser_instance()->get_current_packno();
        callback_info.m_backend_cmd = data_parser_instance()->get_current_cmd();
        upgrade_info.m_upgrade_callback_info = &callback_info;
        upgrade_info.upgrade_result_callback = upgrade_result_callback;
        //upgrade_info.m_default[32];

        switch (cmd)
        {
        case CMD_COMMAND_MACHINE_UPGRADE_APP:
        case CMD_COMMAND_MACHINE_UPGRADE_CORE:
            upgrade_info.self_save_cb = NULL;
            strcpy(upgrade_info.m_store_filepath, TMS_APUPDATE_FILE_NAME);
            //
            upgrade_info.m_download_file_type = FT_BIN_FILE;
            memcpy(upgrade_info.m_download_url, dst_data, dst_lens);
            upgrade_info.m_download_url[dst_lens] = '\0';
            ret = tms_command_check_upgrade(upgrade_info.m_download_url);
            break;

        case CMD_COMMAND_MACHINE_DOWNLOAD_BITMAP:
            upgrade_info.self_save_cb = 0;
            upgrade_info.m_download_file_type = FT_PHOTO;
            ret = cmd_command_machine_parse_bitmap_url(dst_data, dst_lens, &upgrade_info);
            break;
        }

        if (0 == ret)
        {
            trendit_post_download_msg(&upgrade_info);
        }
    }

    if (upgrade_info.m_download_url)
    {
        TMS_FREE(upgrade_info.m_download_url);
    }
    if (upgrade_info.m_store_filepath)
    {
        TMS_FREE(upgrade_info.m_store_filepath);
    }

    return ret;
}

/********************************************************************************
 *
 *
 *        函数实现
 *
 */
static upgrade_info_t * tms_upgrade_info_store(upgrade_info_t *upgrade_info)
{
    s32 ret = 0;

    if (!upgrade_info || (!upgrade_info->m_store_filepath && !upgrade_info->self_save_cb)
        || (upgrade_info->m_store_filepath && upgrade_info->self_save_cb))
    {
        ret = -1;
        g_upgrade_info = NULL;
        TMS_DEBUG("\t.illegal para...\r\n");
    }
    else
    {
        g_upgrade_info = TMS_MALLOC(sizeof(upgrade_info_t));
        if (g_upgrade_info)
        {
            memset(g_upgrade_info, 0, sizeof(sizeof(upgrade_info_t)));
            g_upgrade_info->m_upgrade_callback_info = TMS_MALLOC(sizeof(upgrade_callback_info_t));
            if (!g_upgrade_info->m_upgrade_callback_info)
            {
                ret = -3;
                TMS_DEBUG("\t.illegal download url with %d.\r\n", ret);
            }
            //
            if (upgrade_info->m_store_filepath)
            {
                g_upgrade_info->m_store_filepath = TMS_MALLOC(strlen(upgrade_info->m_store_filepath) + 1);
                if (!g_upgrade_info->m_store_filepath)
                {
                    ret = -4;
                    TMS_DEBUG("\t.illegal file path with %d.\r\n", ret);
                }
                else
                {
                    strcpy(g_upgrade_info->m_store_filepath, upgrade_info->m_store_filepath);
                    TMS_DEBUG("\t.file path: %s.\r\n", g_upgrade_info->m_store_filepath);
                }
            }
            //
            g_upgrade_info->m_download_url = TMS_MALLOC(strlen(upgrade_info->m_download_url) + 1);
            if (!g_upgrade_info->m_download_url)
            {
                ret = -5;
                TMS_DEBUG("\t.illegal download url with %d.\r\n", ret);
            }
            //
            if (0 == ret)
            {
                strcpy(g_upgrade_info->m_download_url, upgrade_info->m_download_url);
                g_upgrade_info->m_download_file_type    = upgrade_info->m_download_file_type;
                g_upgrade_info->self_save_cb            = upgrade_info->self_save_cb;
                g_upgrade_info->upgrade_result_callback = upgrade_info->upgrade_result_callback;
                g_upgrade_info->m_upgrade_callback_info->m_backend_cmd = upgrade_info->m_upgrade_callback_info->m_backend_cmd;
                g_upgrade_info->m_upgrade_callback_info->m_packno      = upgrade_info->m_upgrade_callback_info->m_packno;
                g_upgrade_info->m_upgrade_callback_info->m_lparam      = upgrade_info->m_upgrade_callback_info->m_lparam;
                //memcpy(&g_upgrade_info->m_default[0], &upgrade_info->m_default[0], 32);
                TMS_DEBUG("\t.upgrade info init with %d.\r\n", ret);
            }
        }

        if (0 != ret && g_upgrade_info)
        {
            if (g_upgrade_info->m_download_url)
            {
                TMS_FREE(g_upgrade_info->m_download_url);
            }
            if (g_upgrade_info->m_store_filepath)
            {
                TMS_FREE(g_upgrade_info->m_store_filepath);
            }
            if (g_upgrade_info->m_upgrade_callback_info)
            {
                TMS_FREE(g_upgrade_info->m_upgrade_callback_info);
            }

            TMS_FREE(g_upgrade_info);
            g_upgrade_info = NULL;
        }
    }
    TMS_DEBUG("\t.upgrade info init finish with 0x%x.\r\n", g_upgrade_info);

    return g_upgrade_info;
}


s32 tms_upgrade_info_callback(s32 status)
{
    if (g_upgrade_info)
    {
        g_upgrade_info->upgrade_result_callback(g_upgrade_info->m_upgrade_callback_info,
                                                status >= 0 ? 1: -1, status);
        if (g_upgrade_info->m_store_filepath) TMS_FREE(g_upgrade_info->m_store_filepath);
        if (g_upgrade_info->m_download_url) TMS_FREE(g_upgrade_info->m_download_url);
        if (g_upgrade_info->m_upgrade_callback_info) TMS_FREE(g_upgrade_info->m_upgrade_callback_info);
        TMS_FREE(g_upgrade_info);
        g_upgrade_info = NULL;
        TMS_DEBUG("\t.tms upgrade callback.\r\n\r\n");
    }
}

static void tms_upgrade_process(u8 *url, u16 len)
{
    s32 ret;

    ret = cacp_command_process_upgrade(url, len);
    if (TMS_OK == ret)
    {
        g_cacp_upload_pfn = tms_upgrade_info_callback;
    }
}

s32 trendit_post_download_msg(upgrade_info_t *upgrade_info)
{
    s32 ret = -1;
    if (tms_upgrade_info_store(upgrade_info))
    {

        switch (g_upgrade_info->m_download_file_type)
        {
            case FT_BIN_FILE:
                if (0 == strcmp(TMS_PARTICULAR_UPGRADE_URL, g_upgrade_info->m_download_url))
                {
                    tms_download_entry();
                    g_cacp_upload_pfn = tms_upgrade_info_callback;
                }
                else
                {
                    tms_upgrade_process(g_upgrade_info->m_download_url, strlen(g_upgrade_info->m_download_url));
                }
                break;
            case FT_PHOTO:
                tms_url_content_download(g_upgrade_info->m_store_filepath,
                                         g_upgrade_info->m_download_url, 1,
                                         tms_upgrade_info_callback);
                break;
            case FT_AUDIO:
            case FT_FONT:
                ret = trendit_http_schedule_add(upgrade_info, 0);
                break;
        }
        ret = 0;
    }

    return ret = 0;
}

void trendit_init_tms_data(void)
{
    u8 i;

    g_tms_msg   = NULL;
    g_model_tms = NULL;
    g_tms_utils = NULL;
    g_binary_info = NULL;
    g_upgrade_check_flag = 1;
    //
    g_cacp_upload_pfn = NULL;
    g_upgrade_info = NULL;

    g_file_downloading = 0;
    //
    memset(&g_apupdate_header, 0, sizeof(apupdate_header_t));
    //
    for (i = 0; i < TMS_FILE_LIST_MAX; i++)
    {
        g_apupdate_header.list[i].crc32= 0xFFFFFFFF;
    }

    tms_set_download_status(0);

    TMS_DEBUG("tms instance init first.\r\n");
}

static s32 tms_download_entry(void)
{
    tms_check_version_trigger(0);
}

static s32 cacp_upload_upgrade_result(s32 status)
{
    s32 ret = -1;

    if (0)
    {
        // 上报升级成功
    }
    else
    {
        // 上报升级失败
    }
    // 确保上报成功，不然新建消息上报结果
    g_cacp_upload_pfn = NULL;

    return ret;
}

static s32 cacp_command_process_upgrade(u8 *url, u16 len)
{
    s32 ret = -1;
    tms_utils_t **tms_utils;

    g_binary_info = TMS_MALLOC(sizeof(version_detail_t));
    if (g_binary_info)
    {
        ret = tms_download_binary_trigger(url, len);
        if (TMS_OK == ret)
        {
            g_cacp_upload_pfn = cacp_upload_upgrade_result;
        }
    }

    return ret;
}

static s32 tms_utils_data_init(tms_utils_t **pptms_utils)
{
    int ret = TMS_ERR_MALLOC_FAILED;

    *pptms_utils = TMS_MALLOC(sizeof(tms_utils_t));
    if (*pptms_utils)
    {
        memset(*pptms_utils, 0, sizeof(tms_utils_t));
        (*pptms_utils)->msg_node = TMS_MALLOC(sizeof(msg_t));
        (*pptms_utils)->socket_info.send_buff = TMS_MALLOC(TMS_NET_SEND_BUFFER_MAX + 1);
        (*pptms_utils)->socket_info.recv_buff = TMS_MALLOC(TMS_NET_RECV_BUFFER_MAX + 1);
        //
        if (!(*pptms_utils)->msg_node
            || !(*pptms_utils)->socket_info.send_buff
            || !(*pptms_utils)->socket_info.recv_buff)
        {
            if ((*pptms_utils)->msg_node)
            {
                TMS_FREE((*pptms_utils)->msg_node);
            }
            if ((*pptms_utils)->socket_info.send_buff)
            {
                TMS_FREE((*pptms_utils)->socket_info.send_buff);
            }
            if ((*pptms_utils)->socket_info.recv_buff)
            {
                TMS_FREE((*pptms_utils)->socket_info.recv_buff);
            }
            if (*pptms_utils)
            {
                TMS_FREE(*pptms_utils);
            }
            g_file_downloading = 0;
        }
        else
        {
            ret = TMS_OK;
            memset((*pptms_utils)->msg_node, 0, sizeof(msg_t));
            memset((*pptms_utils)->socket_info.send_buff, 0, TMS_NET_SEND_BUFFER_MAX + 1);
            memset((*pptms_utils)->socket_info.recv_buff, 0, TMS_NET_RECV_BUFFER_MAX + 1);
        }
    }
    TMS_DEBUG("tms init with %d.\r\n", ret);

    return ret;
}

static tms_utils_t ** tms_utils_handle(void)
{
    return &g_tms_utils;
}

/********************************************************************************
 * Desc: 释放资源
 * Para:
 *
 *
 */
static s32 tms_utils_free(tms_utils_t *tms_utils)
{
    s32 ret = 0;
    tms_utils_t **xtms_utils;

    if (tms_utils)
    {

        if (tms_utils->socket_info.send_buff)
        {
            TMS_FREE(tms_utils->socket_info.send_buff);
            tms_utils->socket_info.send_buff = 0;
        }
        if (tms_utils->socket_info.recv_buff)
        {
            TMS_FREE(tms_utils->socket_info.recv_buff);
            tms_utils->socket_info.recv_buff = 0;
        }
        if (0 < tms_utils->socket_info.socket_id)
        {
            tms_utils->socket_handle.socket_close(tms_utils->socket_info.socket_id);
            tms_utils->socket_info.socket_id = -1;
            tms_utils->msg_node = 0;
        }
        memset(tms_utils, 0, sizeof(tms_utils_t));
        tms_utils = 0;
        xtms_utils = tms_utils_handle();
        if (*xtms_utils)
        {
            TMS_FREE(*xtms_utils);
            *xtms_utils = 0;
        }
        TMS_DEBUG("free g_tms_utils end with 0x%X.\r\n", g_tms_utils);
    }

    return ret;
}


/********************************************************************************
 *
 *
 *  延时消息，释放资源后在创建
 *
 */
typedef s32 (*tms_delay_pfn)(void);
typedef struct tms_delay_ticks
{
    u32 ticks_start;
    u32 delay_count;
    u32 delay_until;
} tms_delay_ticks_t;

static tms_delay_ticks_t g_tms_delay_ticks = {0};


s32 tms_msg_delay_handle(msg_t *msg)
{
    s32 ret = MSG_KEEP;
    tms_delay_pfn delay_pfn;
    tms_delay_ticks_t *msg_delay;

    switch(msg->m_status)
    {
        case MSG_INIT:
            msg_delay = (tms_delay_ticks_t *)msg->m_lParam;
            delay_pfn = (tms_delay_pfn)msg->m_wparam;
            if (0 >= msg_delay->delay_until)
            {
                if (delay_pfn)
                {
                    ret = delay_pfn();
                }
            }
            else
            {
                if (trendit_get_ticks() > msg_delay->ticks_start + msg_delay->delay_count)
                {
                    if (delay_pfn)
                    {
                        delay_pfn();
                    }
                    ret = MSG_OUT;
                    TMS_DEBUG("%s msg callback with %d/%x.\r\n", msg->m_msgname, ret, delay_pfn);
                }
            }
            break;
        case MSG_SERVICE:
            break;
        case MSG_TERMINAL:
            break;
    }

    return ret;
}

static s32 tms_msg_delay_post(tms_delay_pfn delay_pfn, s8 *desc, s32 delay)
{
    s32 ret = -0xFF;
    msg_t *msg_node = TMS_MALLOC(sizeof(msg_t));

    if (msg_node)
    {
        g_tms_delay_ticks.ticks_start = trendit_get_ticks();
        if (0 < delay)
        {
            g_tms_delay_ticks.delay_count = delay;
            g_tms_delay_ticks.delay_until = 1;
        }
        else
        {
            g_tms_delay_ticks.delay_count = 0;
            g_tms_delay_ticks.delay_until = 0;
        }
        //
        memset(msg_node, 0, sizeof(msg_t));
        msg_node->m_func   = tms_msg_delay_handle;
        msg_node->m_status = MSG_INIT;
        msg_node->m_wparam = (u32)delay_pfn;
        msg_node->m_lParam = (u32)&g_tms_delay_ticks;
        strcpy(msg_node->m_msgname, desc);
        ret = trendit_postmsg(msg_node);
    }

    TMS_DEBUG("post msg: %s delay %d with %d.\r\n", desc, delay, ret);
}

/********************************************************************************
 *
 *
 *
 *
 */
static u32 tms_tcp_str_num_get(s8 *raw_data, s32 raw_len, u32 *num)
{
    s8 *k, *p, count = 0;
    u32 i = 0;

    if (NULL != raw_data && NULL != num)
    {
        *num = 0;
        p = raw_data;
        k = raw_data + raw_len;
        do {
            if ('0' <= *p && '9' >= *p)
            {
                *num *= 10;
                *num += (*(p) - '0');
                count++;
                p++;
            }
            else
            {
                break;
            }
        } while (p < k);
        i = k - raw_data;
    }
    if (0 == count) {
        *num = -1;
    }

    //TMS_DEBUG("str2dec: %d, index: %d.\r\n", *num, i);

    return i;
}

/********************************************************************************
 * Desc: 获取http响应 Content-Length 大小
 * Para: raw_data -> http 响应内容起始位置
 *       raw_len  -> http 响应长度
 *
 *
 */
static s32 tms_tcp_content_len_get(u8 *raw_data, s32 raw_len, packet_info_t *packet_info)
{
    s8 *p = NULL, *raw_end;
    s32 bytes = 0;

    p = strstr(raw_data, "Content-Length");
    if (NULL != p) {
        p += 16;
        while (':' == *p || ' ' == *p) p++;
        raw_end = raw_data + raw_len;
        do {
            if ('0' <= *p && '9' >= *p)
            {
                bytes *= 10;
                bytes += (*p - '0');
                p++;
            }
            else
            {
                break;
            }
        } while (p < raw_end);
        if ('\r' != *p)
        {
            bytes = 0;
        }
        else
        {
            packet_info->content_len = bytes;
            TMS_DEBUG("tcp content length: %d.\r\n", bytes);
        }
    }

    return bytes;
}

/********************************************************************************
 * Desc: 获取http分包信息
 * Para: raw_data -> http 响应内容起始位置
 *       raw_len  -> http 响应长度
 *
 *
 */
static u32 tms_tcp_partial_content_info_get(u8 *raw_data, s32 raw_len, flow_control_t *flow_control)
{
    s32 bytes = 0;
    s8 *p, *q, *k;
    s32 block_start = 0, block_end = 0, file_total = 0;
    u32 offset;

    if (NULL != (p = strstr(raw_data, "Partial Content")) &&
        NULL != (q = strstr(p, "Content-Range:")))
    {

        flow_control->partial_content = 1;

        while ('0' > *q || '9' < *q) q++;

        if ((k = strstr(q, "-"))) {
            offset = tms_tcp_str_num_get(q, (k - q), &block_start);
            if ('-' != *(q + offset)) {
                block_start = -2;
            } else {
                q += offset;
                q++;
            }
        }
        //
        if (0 <= block_start && (k = strstr(q, "/")))
        {
            offset = tms_tcp_str_num_get(q, (k - q), &block_end);
            if ('/' != *(q + offset))
			{
                block_end = 0;
            }
			else
			{
                q += offset;
                q++;
            }
        }
        //
        if (0 < block_end && (k = strstr(q, "\n\r")))
		{
            offset = tms_tcp_str_num_get(q, (k - q), &file_total);
            if ('\n' != *(q + offset))
			{
                block_end = 0;
            }
			else
			{
                q += offset;
            }
        }
    }

    if (0 < block_end)
    {
        flow_control->partial.block_start = block_start;
        flow_control->partial.block_end = block_end;
        flow_control->file_total = file_total;
        if (0 <= block_start && block_start <= block_end && block_end < file_total) {
            flow_control->partial.block_size = block_end - block_start + 1;
            bytes = flow_control->partial.block_size;
        }
    }

    TMS_DEBUG("start: %d, end: %d, total: %d.\r\n", block_start, block_end, file_total);

    return bytes;
}

/********************************************************************************
 * Desc: 获取http头解析
 * Para: data        -> http 响应内容起始位置
 *       offset_prev -> 上一次查找 \n\r 时查找的响应内容的起始偏移,防止重复查找
 *       offset_next -> http 响应内容总长度
 *
 *
 */
static s32 tms_tcp_header_parse(u8 *data, s32 offset_prev, s32 offset_next, flow_control_t *flow_control)
{
    u32 i, header_boundary = 0;
    s8 *p;
    s32 len = offset_next - offset_prev;
    s8 *s1, *s2, *s3;

    if (p = strstr((s8 *)data, "\r\n\r\n"))
    {
        flow_control->packet.header_len = (s8 *)p - (s8 *)data + 4;
        header_boundary = 1;
    }

    if (1 == header_boundary)
    {
        tms_tcp_partial_content_info_get(data, offset_next, flow_control);
        tms_tcp_content_len_get(data, offset_next, &flow_control->packet);
    }

    return flow_control->packet.packet_size;
}

/********************************************************************************
 * Desc: 从URL中获取域名、页面和端口等信息
 * Para: append_info -> http 请求头拼接参数
 *       url         -> 待解析的url
 *
 *
 */
s32 tms_http_url_parse(append_info_t *append_info, s8 *url)
{
    s8  *p1, *p2, *p3, *p4;
    s32  lens, ret = 0;

    if (!append_info || !url)
    {
        ret = -1;
    }
    else
    {
        memset(append_info->domain, 0, 63);
        memset(append_info->page, 0, 512);
        if (0 == strncmp(url, "http://", 7))
		{
            p1 = &url[7];
            append_info->https = 0;
        }
		else if (0 == strncmp(url, "https://", 8))
		{
            p1 = &url[8];
            append_info->https = 1;
        }
		else
		{
            p1 = &url[0];
            append_info->https = 0;
        }
        p2 = strstr(p1, "/");
        p3 = strstr(p1, ":");
        p4 = p3;
        if (!p3 || p3 >= p2)
		{
            if (1 == append_info->https)
			{
                append_info->port = 433;
            }
			else
			{
                append_info->port = 80;
            }
        }
		else
		{
            p3++;
            append_info->port = 0;
            while ('0' <= *p3 && '9' >= *p3)
			{
                append_info->port *= 10;
                append_info->port += (*p3 - '0');
                p3++;
            }
            p2 = p3;
            if ('/' != *p3)
			{
                append_info->port = 80;
                ret = -3;
                TRACE_INFO("parse port error.\r\n");
            }
        }
        if (p2 && '/' == p2[0] && '/' != p2[1])
		{
            if (p4 && (p4 < p2))
			{
                memcpy(append_info->domain, p1, p4 - p1);
            }
			else
			{
                memcpy(append_info->domain, p1, p2 - p1);
            }
            strcpy(append_info->page, p2);
        }
        if (0 == append_info->port || 4 > strlen(append_info->domain))
        {
            ret = -2;
        }
        TMS_DEBUG("\r\n%s\r\n%s:%d.", url, append_info->domain, append_info->port);
    }

    TMS_DEBUG("parse url end with %d.\r\n", ret);

    return ret;
}

static s32 request_url_content_append_head(append_info_t *append_info)
{
    s32 ret = 0;
    append_info_t *p = append_info;

    if (p && p->raw_data)
    {
        HTTP_URL_CONTENT_REQUSET(p->raw_data, p->domain, p->page);
        p->raw_size = strlen(p->raw_data);
    }

    return ret;
}
/********************************************************************************
 *
 *  直接下载url内容
 *
 */
s32 tms_url_content_download(u8 *filename, u8 *url, u8 direct, tms_ext_pfn ext_pfn)
{
    s32 fd, ret = -1;
    tms_utils_t ** tms_utils;

    tms_utils = tms_utils_handle();
    ret = tms_utils_data_init(tms_utils);
    if (0 != ret)
    {
        TMS_DEBUG("init tms uitls data failed!\r\n");
    }
    else
    {
        ret = tms_http_url_parse(&(*tms_utils)->append_info, url);
        if (0 != ret)
        {
            tms_utils_free(*tms_utils);
        }
        else
        {
            if (filename)
            {
                if (strstr(filename, "/mtd0/"))
                {
                    (*tms_utils)->flow_control.vfs_save = 1;
                }
                else if (strstr(filename, "/mtd2/"))
                {
                    (*tms_utils)->flow_control.vfs_save = 0;
                }
                else
                {
                    // 没有挂载路径，默认vfs
                    (*tms_utils)->flow_control.vfs_save = 1;
                    strcpy((*tms_utils)->flow_control.file_path, "/mtd0/");
                }
                strcat((*tms_utils)->flow_control.file_path, filename);
            }
            if (1 == (*tms_utils)->flow_control.vfs_save)
            {
                fd = ddi_vfs_open(filename, "r");
                if (0 < fd)
                {
                    ddi_vfs_close(fd);
                    ddi_vfs_deletefile(filename);
                    TMS_DEBUG("vfs delete file %s.\r\n", filename);
                }
            }
            //
            (*tms_utils)->append_info.block_start = 0;
            (*tms_utils)->append_info.block_size  = TMS_PACKAGING_HEADER_SIZE;
            //
            if (0 < direct)
            {
                (*tms_utils)->flow_control.direct_url = 1;
                (*tms_utils)->data_handle.http_append_head = download_tms_file_append_head;
                //(*tms_utils)->data_handle.http_append_head = request_url_content_append_head;
            }
            else
            {
                (*tms_utils)->data_handle.http_append_head = download_tms_file_append_head;
            }
            (*tms_utils)->data_handle.msg_data_update  = download_tms_file_receiving;
            (*tms_utils)->data_handle.msg_data_finish  = download_tms_file_complete;
            (*tms_utils)->data_handle.msg_free         = tms_utils_free;
            (*tms_utils)->data_handle.msg_ext_notify   = ext_pfn;
            //
            (*tms_utils)->msg_node->m_func     = tms_utils_msg_handle;
            (*tms_utils)->msg_node->m_priority = MSG_NORMAL;
            (*tms_utils)->msg_node->m_lParam   = 0;
            (*tms_utils)->msg_node->m_wparam   = 0;
            (*tms_utils)->msg_node->m_status   = MSG_INIT;
            snprintf((*tms_utils)->msg_node->m_msgname, sizeof((*tms_utils)->msg_node->m_msgname), "%s", "tms");
            //
            ret = trendit_postmsg((*tms_utils)->msg_node);
        }
        TMS_DEBUG("post get url msg with %d.\r\n", ret);
    }

    if (0 != ret)
    {
        if (ext_pfn)
        {
            ext_pfn(-1);
        }
    }
    else
    {
        g_file_downloading = 0x5AB6;
    }

    return ret;
}

/********************************************************************************
 *
 *  组包
 *
 */
static s32 common_vfs_read(s8 *file, s8 *buff, s32 size, s32 offset)
{
    s32 lens, ret = -1;
    int fd;
    u32 ticks_start;

    ticks_start = trendit_get_ticks();

    if (file && buff)
    {
        fd = ddi_vfs_open(file, "r");
        if (0 < fd)
        {
            ddi_vfs_seek(fd, 0, VFS_SEEK_END);
            lens = ddi_vfs_tell(fd);
            if (lens >= offset + size)
            {
                ret = ddi_vfs_seek(fd, offset, VFS_SEEK_SET);
                ret = ddi_vfs_read(fd, buff, size);
            }
            else
            {
                ret = -lens;
                TMS_DEBUG("%d bytes file cannot read %d byte data\r\n", lens, size);
            }
            ddi_vfs_close(fd);
        }
    }
    TMS_DEBUG("read %d bytes from %s offset %d with %d, ticks: %d.\r\n",
               size, file, offset, ret, trendit_get_ticks() - ticks_start);

    return ret;
}

static s32 common_vfs_write(s8 *file, s8 *data, u32 len, u32 offset)
{
    s32 fd, ret = -1;
    u32 ticks_start;

    ddi_watchdog_feed();

    ticks_start = trendit_get_ticks();
    if (file && data && len > 0 && offset >= 0)
    {
        fd = ddi_vfs_open(file, "a+");
        if (0 < fd)
        {
            //ret = ddi_vfs_seek(fd, offset, VFS_SEEK_SET);
            //ddi_misc_msleep(3);
            ret = ddi_vfs_write(fd, data, len);
            ddi_vfs_close(fd);
        }
    }

    TMS_DEBUG("write %d bytes to %s offset %d with %d, ticks: %d.\r\n",
               len, file, offset, ret, trendit_get_ticks() - ticks_start);

    return ret;
}

static s32 common_vfs_overwrite(s8 *file, s8 *data, u32 len, u32 offset)
{
    s32 fd, ret = -1;
    u32 ticks_start;

    ticks_start = trendit_get_ticks();
    if (file && data && len > 0 && offset >= 0)
    {
        fd = ddi_vfs_open(file, "w+");
        if (0 < fd)
        {
            //ret = ddi_vfs_seek(fd, offset, VFS_SEEK_SET);
            //ddi_misc_msleep(3);
            ret = ddi_vfs_write(fd, data, len);
            ddi_vfs_close(fd);
        }
    }

    TMS_DEBUG("write %d bytes to %s offset %d with %d, ticks: %d.\r\n",
               len, file, offset, ret, trendit_get_ticks() - ticks_start);

    return ret;
}

static s32 apupdate_header_insert(void)
{
    s8 *buff;
    u8 i;
    s32 offset, ret;

    buff = TMS_MALLOC(TMS_APUPDATE_HEADER_SIZE + 1);
    if (!buff)
    {
        ret = -1;
    }
    else
    {
        memset(buff, 0, TMS_APUPDATE_HEADER_SIZE + 1);
        offset = 0;
        offset += sprintf(buff + offset, "%s%s", "[namelist]", "\r\n");
        for (i = 1; i < g_apupdate_header.total_files; i++)
        {
            offset += sprintf(buff + offset, "%s%d=%s%s", "file", (i - 1),
                               g_apupdate_header.list[i - 1].name,"\r\n");
        }
        //
        offset += sprintf(buff + offset, "%s%s", "[addrlist]", "\r\n");
        for (i = 1; i < g_apupdate_header.total_files; i++)
        {
            offset += sprintf(buff + offset, "%s%d=%s%s", "addr", (i - 1),
                               g_apupdate_header.list[i - 1].offset,"\r\n");
        }
        //
        offset += sprintf(buff + offset, "%s%s", "[lenlist]", "\r\n");
        for (i = 1; i < g_apupdate_header.total_files; i++)
        {
            offset += sprintf(buff + offset, "%s%d=%s%s", "len", (i - 1),
                               g_apupdate_header.list[i - 1].size,"\r\n");
        }
        //
        offset += sprintf(buff + offset, "%s%s", "[crclist]", "\r\n");
        for (i = 1; i < g_apupdate_header.total_files; i++)
        {
            offset += sprintf(buff + offset, "%s%d=%X%s", "crc", (i - 1),
                               g_apupdate_header.list[i - 1].crc32,"\r\n");
        }
        ret = common_vfs_write(TMS_APUPDATE_FILE_NAME, buff, offset, 0);
        TMS_DEBUG("apupdate: %s.\r\n", buff);
        TMS_FREE(buff);
    }

    TMS_DEBUG("write apupdate header with %d.", ret);
}

// 必须要先下载core文件否则导致组包失败
static s32 tms_insert_data_to_file(s8 *file, u8 *data, u32 len, s32 offset, u8 fstype, s8 end)
{
    s32 ret;
    u32 mutil_offset = 0, total_offset, crc32calc;
    tms_utils_t **tms_utils;
    s8 buff[0x400];

    if (g_apupdate_header.total_files > 0)
    {
        mutil_offset = g_apupdate_header.list[g_apupdate_header.total_files - 1].offset;
        crc32calc = g_apupdate_header.list[g_apupdate_header.total_files - 1].crc32;
        g_apupdate_header.list[g_apupdate_header.total_files - 1].crc32 = trendit_calc_crc32(crc32calc, data, len);
    }
    else
    {
        mutil_offset = 0;
    }
    total_offset = offset;
    TMS_DEBUG("total_offset: %d, file_end: %d.\r\n.", total_offset, total_offset + len);
    if (0 == fstype)
    {
        ret = ddi_file_insert(TMS_APUPDATE_FILE_NAME, data, len, offset);
    }
    else
    {
        ret = common_vfs_write(file, data, len, offset);
    }

    if (len != ret)
    {
        ret |= -0x201;
        TMS_DEBUG("insert %d bytes failed with %d.", len, ret);

    }
    else
    {
        TMS_DEBUG("insert %d bytes success with %d.", len, ret);
    }

    if (0 < end)
    {
        //apupdate_header_insert();
    }

    return ret;
}


/********************************************************************************
 *
 *  下载升级文件
 *
 */
#define TMS_PACKAGING_TIMESTAMP_MAX  (25)
#define TMS_PACKAGING_POS_MODEL_MAX  (5)
#define TMS_PACKAGING_CUSTOMER_MAX   (30)
#define TMS_PACKAGING_APP_ID_MAX     (10)
#define TMS_PACKAGING_APP_NAME_MAX   (20)
#define TMS_PACKAGING_VERSION_MAX    (20)


typedef struct ms_file_list
{
	u8  compress;	        // 压缩算法  '0'-不压缩  '1'-lz77
	u8  type;		        // '0'-其他文件 '1'-参数文件 '2'-应用文件
	u8  force_upgrade;	    // 是否强制更新'1'-强制更新 '0'-不强制
	u32 start;
	u32 filesize;
	u32 checkvalue;
	u32 orgfilesize;
	u32 orgcheckvalue;
	u8  version[20];		// 版本
	u8  downfile[20];
	u8  downstate;		    // 0-不下载  1-需下载   应用文件,下载完成后不可能出现需下载而flash长度为0情况
	u32 flashstart;		    // 起始位置,只有应用文件才有, 需下载时起始地址不应该为0
	u32 flashfileLen;	    // 文件已下载长度
	u8  tempfile[20];	    // 图片资源，参数等，非app.bin
} tms_file_list_t;

typedef struct packaging_info
{
	u32 headcheckValue;
    u8 total_hex;
    u8 timestamp[TMS_PACKAGING_TIMESTAMP_MAX];
	u8 pos_model[TMS_PACKAGING_POS_MODEL_MAX];
	u8 customer[TMS_PACKAGING_CUSTOMER_MAX];
	u8 appid[TMS_PACKAGING_APP_ID_MAX];
	u8 appname[TMS_PACKAGING_APP_NAME_MAX];
	u8 version[TMS_PACKAGING_VERSION_MAX];
	u8 force_upgrade;			// 当前TMS版本是否强制更新'1'-强制更新 '0'-不强制
	s32 i;
    tms_file_list_t *list;
} packaging_info_t;

#define FLASH_BLOCK                     (4096)
#define FLASH_HEAD_SIZE                 FLASH_BLOCK
#define TMS_PACKAGING_HEADER_MAX        (4096)

static s32 tms_packaging_info_parse(packaging_info_t *packaging_info, u8 *packaging_data, u8 new_data)
{
    s32 ret = 0;
	s32 i = 0, offset = 0;
	u32 flashoffset = FLASH_BLOCK;
	u32 crc32calc = 0xFFFFFFFF;

    crc32calc = trendit_calc_crc32(crc32calc, &packaging_data[4], (TMS_PACKAGING_HEADER_MAX - 4));
    if (0 != memcmp(&crc32calc, &packaging_data[0], 4))
    {
        ret = -1;
        TMS_DEBUG("tms packaging header check error.\r\n");
        goto tms_error;
    }

	offset = 0;
	memcpy(&packaging_info->headcheckValue, &packaging_data[offset], 4);
	offset += 4;
    //
	packaging_info->total_hex = packaging_data[offset++];
	memcpy(packaging_info->timestamp, &packaging_data[offset], TMS_PACKAGING_TIMESTAMP_MAX);
	offset += TMS_PACKAGING_TIMESTAMP_MAX;
	memcpy(packaging_info->pos_model, &packaging_data[offset], TMS_PACKAGING_POS_MODEL_MAX);
	offset += TMS_PACKAGING_POS_MODEL_MAX;
	memcpy(packaging_info->customer, &packaging_data[offset], TMS_PACKAGING_CUSTOMER_MAX);
	offset += TMS_PACKAGING_CUSTOMER_MAX;
	memcpy(packaging_info->appid, &packaging_data[offset], TMS_PACKAGING_APP_ID_MAX);
	offset += TMS_PACKAGING_APP_ID_MAX;
	memcpy(packaging_info->appname, &packaging_data[offset], TMS_PACKAGING_APP_NAME_MAX);
	offset += TMS_PACKAGING_APP_NAME_MAX;
	memcpy(packaging_info->version, &packaging_data[offset], TMS_PACKAGING_VERSION_MAX);
	offset += TMS_PACKAGING_VERSION_MAX;
	packaging_info->force_upgrade = packaging_data[offset++];
    //
    TMS_DEBUG("\r\n-------- TMS Packaging Info --------\r\n");
    TMS_DEBUG("total file: %d.\r\n", packaging_info->total_hex);
    TMS_DEBUG("timestamp : %s.\r\n", packaging_info->timestamp);
    TMS_DEBUG("pos model : %s.\r\n", packaging_info->pos_model);
    TMS_DEBUG("customer  : %s.\r\n", packaging_info->customer);
    TMS_DEBUG("app id    : %s.\r\n", packaging_info->appid);
    TMS_DEBUG("app name  : %s.\r\n", packaging_info->appname);
    TMS_DEBUG("version   : %s.\r\n", packaging_info->version);
    TMS_DEBUG("force exec: %d.\r\n", packaging_info->force_upgrade);
    TMS_DEBUG("-------- TMS Packaging Info --------\r\n");
    return 0;
    //
    if (49 < packaging_info->total_hex)
    {
        ret = -2;
        goto tms_error;
    }

    for (i = 0; i < packaging_info->total_hex; i++)
	{
		offset = i * 96 + 160;

		packaging_info->list[i].compress      = packaging_data[offset++];
		packaging_info->list[i].type          = packaging_data[offset++];
		packaging_info->list[i].force_upgrade = packaging_data[offset++];

		memcpy(&packaging_info->list[i].start, &packaging_data[offset], 4);
		offset += 4;
		memcpy(&packaging_info->list[i].filesize, &packaging_data[offset], 4);
		offset += 4;
		memcpy(&packaging_info->list[i].checkvalue, &packaging_data[offset], 4);
		offset += 4;
		memcpy(&packaging_info->list[i].orgfilesize, &packaging_data[offset], 4);
		offset += 4;
		memcpy(&packaging_info->list[i].orgcheckvalue, &packaging_data[offset], 4);
		offset += 4;

		memcpy(packaging_info->list[i].version, &packaging_data[offset], 20);
		offset += 20;
		memcpy(packaging_info->list[i].downfile, &packaging_data[offset], 20);
		offset += 20;

		TMS_DEBUG("version : %s\r\n", packaging_info->list[i].version);
		TMS_DEBUG("name    : %s\r\n", packaging_info->list[i].downfile);

		if (0 == new_data)
		{
			packaging_info->list[i].downstate = 0;
			if ('2' == packaging_info->list[i].type)
			{
				if (flashoffset & 0xFFF)
				{
					flashoffset = (flashoffset & 0xFFFFF000) + 0x1000;
				}

				packaging_info->list[i].flashstart = flashoffset;
				flashoffset += packaging_info->list[i].orgfilesize;

				packaging_info->list[i].flashfileLen = 0;
				// 压缩的文件,需要下载到虚拟文件系统
				if ('1' == packaging_info->list[i].compress)
				{
					strcpy((s8 *)packaging_info->list[i].tempfile, (s8 *)packaging_info->list[i].downfile);
					strcat((s8 *)packaging_info->list[i].tempfile, ".t");
				}
			}
			else
			{
				//其他文件
				strcpy((s8 *)packaging_info->list[i].tempfile, (s8 *)packaging_info->list[i].downfile);
				strcat((s8 *)packaging_info->list[i].tempfile, ".t");
			}
		}
		else
		{
			packaging_info->list[i].downstate = packaging_data[offset++];
			memcpy(&packaging_info->list[i].flashstart, &packaging_data[offset], 4);
			offset += 4;
			memcpy(&packaging_info->list[i].flashfileLen, &packaging_data[offset], 4);
			offset += 4;
			memcpy(packaging_info->list[i].tempfile, &packaging_data[offset], 20);
			offset += 20;
			//printf(" len = %d \n", len);
		}
    }

tms_error:
tms_free:

    return ret;
}
//
static s32 download_tms_file_append_head(append_info_t *append_info)
{
    s32 ret = 0;
    append_info_t *p = append_info;

    if (p && p->raw_data)
    {
        HTTP_TMS_BLOCK_REQUSET(p->raw_data, p->domain, p->page, p->block_start, p->block_size);
        p->raw_size = strlen(p->raw_data);
    }

    return ret;
}

/********************************************************************************
 * Desc: 检查待转换asiic格式是否正确
 * Para: hex -> 是否是十六进制
 *
 *
 */
static s32 tms_num_format_check(s8 *str, const u32 len, const u8 hex)
{
    s32 i = -1;
    s8  buff[10];

    if (str && 0 < len)
    {
        for (i = 0; i < len; i++)
        {
            if (('0' <= str[i] && '9' >= str[i]) ||
                ((('a' <= str[i] && 'f' >= str[i]) || ('A' <= str[i] && 'F' >= str[i])) && (1 == hex)))
            {
                // Nothing
            }
            else
            {
                break;
            }
        }
    }

    if ((0 != i - len) && (10 > len))
    {
        memset(&buff[0], 0, 10);
        memcpy(&buff[0], str, len);
        TMS_DEBUG("number format check: len(%d), error(%d), %s\r\n", len, i - len, &buff[0]);
    }

    return (i - len);
}

u32 tms_hex_str_to_num(s8 *str, u32 len)
{
    u32 i, byte, val = 0;

    if (str && 8 == len)
    {
        for (i = 0; i < 8; i++)
        {
            //TMS_DEBUG("%d:%c ", i, str[i]);
            if ('0' <= str[i] && '9' >= str[i])
            {
                byte = str[i] - '0';
            }
            else if ('a' <= str[i] && 'f' >= str[i])
            {
                byte = str[i] - 'a' + 10;
            }
            else if ('A' <= str[i] && 'F' >= str[i])
            {
                byte = str[i] - 'A' + 10;
            }
            else
            {
                val = 0;
                break;
            }
            val  = val << 4;
            val |= byte;
        }
    }
    else
    {
        TMS_DEBUG("str2hex: %X:%d\r\n", str, len);
    }

    return val;
}

u32 tms_dec_str_to_num(s8 *str, u32 len)
{
    u32 i, byte, val = 0;

    if (str && 10 >= len)
    {
        for (i = 0; i < len; i++)
        {
            //TMS_DEBUG("%d:%c ", i, str[i]);
            if ('0' <= str[i] && '9' >= str[i])
            {
                val  = val * 10;
                val += (str[i] - '0');
            }
            else
            {
                val = 0;
                break;
            }
        }
    }
    else
    {
        TMS_DEBUG("str2dec: %X:%d\r\n", str, len);
    }

    return val;
}

/********************************************************************************
 * Desc: 从数据中解析apupdate组包数据
 * Para: apupdate_header -> apupdate_header 参数
 *       data            -> 待解析的数据
 *
 *
 */
static s32 tms_file_list_update(apupdate_header_t *apupdate_header, s8 *data, s32 size)
{
    u8  i;
    s8 *ptr_name, *ptr_addr, *ptr_len, *ptr_crc, *ptr_end;
    s8  query[0x20];
    s32 ret;
    u32 val;

    if (strstr(data, "[namelist]") && strstr(data, "[adrlist]") &&
        strstr(data, "[lenlist]") && strstr(data, "[crclist]"))
    {
        for (i = 0; i < TMS_FILE_LIST_MAX; i++)
        {
            memset(&query[0], 0, 0x20);
            ret = sprintf(&query[0], "%s%d%s", "file", i, "=");
            ptr_name = strstr(data, &query[0]);
            if (ptr_name)
            {
                ptr_name += ret;
                ptr_end = strstr(ptr_name, "\r\n");
                if (ptr_end)
                {
                    memcpy(apupdate_header->list[apupdate_header->total_files].name, ptr_name, (ptr_end - ptr_name));
                }
            }
            //
            memset(&query[0], 0, 0x20);
            ret = sprintf(&query[0], "%s%d%s", "adr", i, "=0x");
            ptr_addr = strstr(data, &query[0]);
            if (ptr_addr)
            {
                ptr_addr += ret;
                ptr_end = strstr(ptr_addr, "\r\n");
                if (ptr_end)
                {
                    if (0 != tms_num_format_check(ptr_addr, ptr_end - ptr_addr, 1) ||
                        8 < (ptr_end - ptr_addr))
                    {
                        val = 0;
                    }
                    else
                    {
                        val = trendit_asc_to_u32(ptr_addr, ptr_end - ptr_addr, MODULUS_HEX);
                    }
                    apupdate_header->list[apupdate_header->total_files].offset = val;
                }
            }
            //
            memset(&query[0], 0, 0x20);
            ret = sprintf(&query[0], "%s%d%s", "len", i, "=");
            ptr_len = strstr(data, &query[0]);
            if (ptr_len)
            {
                ptr_len += ret;
                ptr_end = strstr(ptr_len, "\r\n");
                if (ptr_end)
                {
                    if (0 != tms_num_format_check(ptr_len, ptr_end - ptr_len, 0) ||
                        10 < (ptr_end - ptr_len))
                    {
                        val = 0;
                    }
                    else
                    {
                        val = trendit_asc_to_u32(ptr_len, ptr_end - ptr_len, MODULUS_DEC);
                    }
                    apupdate_header->list[apupdate_header->total_files].size = val;
                }
            }
            //
            ret = sprintf(&query[0], "%s%d%s", "crc", i, "=0x");
            ptr_crc = strstr(data, &query[0]);
            if (ptr_crc)
            {
                ptr_crc += ret;
                ptr_end = strstr(ptr_crc, "\r\n");
                if (ptr_end)
                {
                    if (0 != tms_num_format_check(ptr_crc, ptr_end - ptr_crc, 1) ||
                        8 < (ptr_end - ptr_crc))
                    {
                        val = 0;
                    }
                    else
                    {
                        val = trendit_asc_to_u32(ptr_crc, ptr_end - ptr_crc, MODULUS_HEX);
                    }
                    apupdate_header->list[apupdate_header->total_files].crc32 = val;
                }
            }
            //
            if (ptr_name && ptr_addr && ptr_len && ptr_crc)
            {
                TMS_DEBUG("file name: %s, offset: 0x%08X, size: %d, crc32: 0x%08X\r\n",
                                apupdate_header->list[apupdate_header->total_files].name,
                                apupdate_header->list[apupdate_header->total_files].offset,
                                apupdate_header->list[apupdate_header->total_files].size,
                                apupdate_header->list[apupdate_header->total_files].crc32);
                apupdate_header->total_files++;
            }
            else
            {
                break;
            }
        }
    }
    TMS_DEBUG("total %d file in %d\r\n", apupdate_header->total_files, i);

    return apupdate_header->total_files;
}

static void tms_breakpoint_info_backup(tms_utils_t *obj, apupdate_header_t *aph)
{
    u16 i, len, offset = 0;
    u32 block_end, crc32calc = 0xFFFFFFFF;
    s8 *buff;

    if (obj && aph
        && 0 < aph->total_files
        && TMS_FILE_LIST_MAX >= aph->total_files)
    {
        len = (aph->total_files * (TMS_FILE_NAME_MAX + 12) + 19);
        buff = TMS_MALLOC(len + 1);
        if (!buff)
        {
            TMS_DEBUG("\t.alloc memory for tms breakpoint failed.\r\n");
        }
        else
        {
            memset(buff, 0, len + 1);
            block_end = obj->flow_control.partial.block_end;
            offset = 0;
            memcpy(&buff[offset], &offset, 2);
            offset += 2;
            memcpy(&buff[offset], &crc32calc, 4);
            offset += 4;
            memcpy(&buff[offset], &block_end, 4);
            offset += 4;
            memcpy(&buff[offset], &(obj->flow_control.crc32_tms4k), 4);
            offset += 4;
            memcpy(&buff[offset], &(obj->flow_control.crc32_save), 4);
            offset += 4;
            buff[offset] = aph->total_files;
            offset += 1;
            for (i = 0; i < aph->total_files; i++)
            {
                memcpy(&buff[offset], aph->list[i].name, TMS_FILE_NAME_MAX);
                offset += TMS_FILE_NAME_MAX;
                memcpy(&buff[offset], &(aph->list[i].size), 4);
                offset +=  4;
                memcpy(&buff[offset], &(aph->list[i].offset), 4);
                offset +=  4;
                memcpy(&buff[offset], &(aph->list[i].crc32), 4);
                offset +=  4;
            }
            memcpy(&buff[0], &offset, 2);
            crc32calc = trendit_calc_crc32(crc32calc, &buff[0], offset);
            memcpy(&buff[2], &crc32calc, 4);
            common_vfs_overwrite(TMS_UPGRADE_BREAKPOINT, &buff[0], offset, 0);
            //
            //TMS_DEBUG("\t.tms breakpoint backup at 0x%x/0x%x.\r\n", block_end, obj->flow_control.crc32_save);
            //TRACE_DBG_HEX(buff, offset);
            TMS_FREE(buff);
        }
    }
    else
    {
        TMS_DEBUG("\t.tms breakpoint para illegal.\r\n");
    }
}

static s32 tms_breakpoint_info_restore(tms_utils_t *obj, apupdate_header_t *aph)
{
    u16 i, offset;
    s32 ret = -1;
    u8 *buff;
    u32 chunk_size = 0, crc32_apupdate = 0;
    u32 crc32save = 0, crc32tms4k = 0, crc32calc = 0xFFFFFFFF;

    if (!obj || !aph)
    {
        TMS_DEBUG("\t.tms breakpoint para illegel.\r\n");
    }
    else
    {
        ret = common_vfs_read(TMS_UPGRADE_BREAKPOINT, (u8 *)&offset, 2, 0);
        if (2 != ret || 0 >= offset
            || (TMS_FILE_LIST_MAX * (TMS_FILE_NAME_MAX + 12) + 19) < offset)
        {
            TMS_DEBUG("\t.tms breakpoint format illegal, %d, %d.\r\n", ret, offset);
            ret |= -0x201;
        }
        else
        {
            buff = TMS_MALLOC(offset + 1);
            if (!buff)
            {
                ret |= -0x202;
                TMS_DEBUG("\t.tms breakpoint malloc memory failed.\r\n");
            }
            else
            {
                memset(buff, 0, offset + 1);
                ret = common_vfs_read(TMS_UPGRADE_BREAKPOINT, buff, offset, 0);
                if (offset != ret)
                {
                    ret |= -0x203;
                    TMS_DEBUG("\t.read tms breakpoint failed with %d/%d.\r\n", offset, ret);
                }
                else
                {
                    memcpy(&crc32save, &buff[2], 4);
                    memcpy(&buff[2], &crc32calc, 4);
                    crc32calc = trendit_calc_crc32(crc32calc, &buff[0], offset);
                    if (crc32calc != crc32save)
                    {
                        ret |= -0x204;
                        TMS_DEBUG("\t.tms break point record crc32 not match, 0x%x, 0x%x\r\n",
                                  crc32save, crc32calc);
                    }
                    else
                    {
                        memcpy(&crc32tms4k, &buff[10], 4);
                        memcpy(&crc32save, &buff[14], 4);
                        memcpy(&chunk_size, &buff[6], 4);
                        chunk_size = (chunk_size > TMS_PACKAGING_HEADER_SIZE)?(chunk_size - TMS_PACKAGING_HEADER_SIZE + 1):chunk_size;
                        trendit_calc_filecrc32(TMS_APUPDATE_FILE_NAME, 0, chunk_size, &crc32_apupdate);
                        if (crc32save != (~crc32_apupdate) || crc32tms4k != obj->flow_control.crc32_tms4k)
                        {
                            ret |= -0x205;
                            TMS_DEBUG("\t.tms4k->save: 0x%x, calc: 0x%x.\r\n",
                                      crc32tms4k, obj->flow_control.crc32_tms4k);
                            TMS_DEBUG("\t.apudate->size: 0x%x, save: 0x%x, calc: 0x%x, ret: %d.\r\n",
                                      chunk_size, crc32save, crc32_apupdate);
                        }
                        else
                        {
                            offset = 6;
                            memcpy(&(obj->flow_control.partial.block_end), &buff[offset], 4);
                            obj->flow_control.partial.last_offset = obj->flow_control.partial.block_end;
                            // recovery schedule offset
                            //
                            offset += 4;
                            memcpy(&(obj->flow_control.crc32_tms4k), &buff[offset], 4);
                            offset += 4;
                            memcpy(&(obj->flow_control.crc32_save), &buff[offset], 4);
                            offset += 4;
                            aph->total_files = buff[offset];
                            offset += 1;
                            for (i = 0; i < aph->total_files; i++)
                            {
                                memcpy(aph->list[i].name, &buff[offset], TMS_FILE_NAME_MAX);
                                offset += TMS_FILE_NAME_MAX;
                                memcpy(&(aph->list[i].size), &buff[offset], 4);
                                offset += 4;
                                memcpy(&(aph->list[i].offset), &buff[offset], 4);
                                offset += 4;
                                memcpy(&(aph->list[i].crc32), &buff[offset], 4);
                                offset += 4;
                            }
                            ret = 0;
                            TMS_DEBUG("\t.tms recovery to breakpoint 0x%x.\r\n",
                                       obj->flow_control.partial.block_end);
                        }
                    }
                    TMS_FREE(buff);
                }
            }
        }
    }

    return ret;
}

static s32 tms_download_receiving_default(u8 *data, s32 len, s32 status)
{
    s32 ret = -1;
    s32 offset;
    u32 crc32 = 0xFFFFFFFF;
    tms_utils_t **tms_utils;
    packaging_info_t * packaging_info;

    tms_utils = tms_utils_handle();
    TMS_DEBUG("new packet data notify %d:%d:0x%X.\r\n", status, len, data);
    if (916 > len)
    {
        //TMS_DEBUG("status: %d, len: %d, data:\r\n%s\r\n", status, len, data);
        //TRACE_INFO_HEX(data, len);
    }
    else
    {
        TMS_DEBUG("data: 0x%x, len: %d, status: %d.\r\n", data, len, status);
    }

    if (*tms_utils)
    {
        if (1 == (*tms_utils)->flow_control.packet_count)
        {
            (*tms_utils)->flow_control.crc32_tms4k = trendit_calc_crc32(crc32, data, len);
            (*tms_utils)->flow_control.crc32_save  = 0xFFFFFFFF;
        }
        else
        {
            (*tms_utils)->flow_control.crc32_save = trendit_calc_crc32(
                            (*tms_utils)->flow_control.crc32_save, data, len);
        }
        //
        TMS_DEBUG("new packet count %d.\r\n", (*tms_utils)->flow_control.packet_count);
        if (0 != (*tms_utils)->flow_control.direct_url)
        {
            offset = (*tms_utils)->flow_control.partial.last_offset;
            tms_insert_data_to_file((*tms_utils)->flow_control.file_path, data, len, offset,
                                    (*tms_utils)->flow_control.vfs_save, 1);
        }
        else
        {
            switch((*tms_utils)->flow_control.packet_count)
            {
                case 1:
                    packaging_info = TMS_MALLOC(sizeof(packaging_info_t));
                    memset(&g_apupdate_header, 0, sizeof(apupdate_header_t));
                    if (0 != tms_breakpoint_info_restore(*tms_utils, &g_apupdate_header)
                        || 0 != tms_packaging_info_parse(packaging_info, data, 1))
                    {
                        ddi_file_delete(TMS_APUPDATE_FILE_NAME);
                        ddi_file_create(TMS_APUPDATE_FILE_NAME, 0x5000, 0);
                    }
                    TMS_FREE(packaging_info);
                    break;
                    //
                case 2:
                    tms_file_list_update(&g_apupdate_header, data, len);
                default:
                    g_file_downloading |= 0xA56B0000;
                    if ((TMS_PACKAGING_HEADER_SIZE - 1) > (*tms_utils)->flow_control.partial.last_offset)
                    {
                        TMS_DEBUG("file offset incorrently.\r\n");
                    }
                    offset = (*tms_utils)->flow_control.partial.last_offset - (TMS_PACKAGING_HEADER_SIZE - 1);
                    ret = tms_insert_data_to_file((*tms_utils)->flow_control.file_path, data, len, offset,
                                    (*tms_utils)->flow_control.vfs_save, 1);
                    if (ret != len)
                    {
                        g_file_downloading = 0xA56B0000;
                        TMS_DEBUG("\t.length not match, breakpoint not record.\r\n");
                    }
                    else
                    {
                        tms_breakpoint_info_backup(*tms_utils, &g_apupdate_header);
                    }
                    break;
            }
        }
    }

    ret = 0;
    return ret;
}

static s32 download_tms_file_receiving(u8 *data, s32 len, s32 status)
{
    if (g_upgrade_info && g_upgrade_info->self_save_cb)
    {
        g_upgrade_info->self_save_cb(data, len,
                                    (0 == status) ? DOWNLOADING : DOWNLOADERR,
                                     g_upgrade_info->m_upgrade_callback_info);
        if (g_cacp_upload_pfn)
        {
            g_cacp_upload_pfn(status);
        }
    }
    else
    {
        tms_download_receiving_default(data, len, status);
    }
}

static s32 tms_file_integrity_check(s8 *name, u8 fstype)
{
    u8 *buff;
    s32 ret, total_files = -1;
    u32 i, j, remain, crc32calc = 0xFFFFFFFF, offset = 0, file_check = TMS_ERR_INTERGRITY_FAILED;
    u32 file_offset;
    u8 buff_after[0x101];
    u32 crc32file = 0;

    memset(buff_after, 0, 0x100);

    buff = TMS_MALLOC(TMS_NET_BLOCK_SIZE_MAX + 1);
    if (buff)
    {
        memset(buff, 0, TMS_NET_BLOCK_SIZE_MAX + 1);
        if (0 == fstype)
        {
            ret = ddi_file_read(name, buff, 0x400, 0);
        }
        else
        {
            ret = common_vfs_read(name, buff, 0x400, 0);
        }

        if (0x400 == ret)
        {
            memset(&g_apupdate_header, 0, sizeof(apupdate_header_t));
            total_files = tms_file_list_update(&g_apupdate_header, buff, 0x400);
        }
        else
        {
            TMS_DEBUG("\r\n%s\r\n", buff);
        }

        if (0 < total_files)
        {
            for (i = 0; i < total_files; i++)
            {
                offset = 0;
                file_offset = 0x400;
                crc32calc   = 0xFFFFFFFF;
                for (j = 0; j < i; j++)
                {
                    file_offset += g_apupdate_header.list[j].size;
                }
            #if 0
                TMS_DEBUG("file %d offset %d\r\n", i, file_offset);
                //
                trendit_calc_filecrc32(name, file_offset, g_apupdate_header.list[i].size, &crc32file);
                TMS_DEBUG("%d bytes %s offset 0x%X crc32: 0x%X.\r\n",
                          g_apupdate_header.list[i].size, name, file_offset, crc32file);
            #endif
                while (offset < g_apupdate_header.list[i].size)
                {
                    memset(buff, 0, TMS_NET_BLOCK_SIZE_MAX + 1);

                    remain = g_apupdate_header.list[i].size - offset;
                    if (TMS_NET_BLOCK_SIZE_MAX < remain)
                    {
                        remain = TMS_NET_BLOCK_SIZE_MAX;
                    }
                    ret = ddi_file_read(name, buff, remain, file_offset + offset);
                    if (ret == remain)
                    {
                        crc32calc = trendit_calc_crc32(crc32calc, buff, remain);
                        offset += remain;
                        file_check = 0;
                    }
                    else
                    {
                        TMS_DEBUG("tms read file error with %d. \r\n", ret);
                        break;
                    }
                }
                crc32calc ^= 0xFFFFFFFF;
                TMS_DEBUG("%s in %s, save: 0x%X, calc: 0x%X, ~calc: 0x%X.\r\n",
                          g_apupdate_header.list[i].name , name,
                          g_apupdate_header.list[i].crc32, crc32calc, ~crc32calc);
                if (crc32calc != g_apupdate_header.list[i].crc32 || 0 != file_check)
                {
                    file_check = -1;
                    break;
                }
            }
        }
        TMS_FREE(buff);
    }

    TMS_DEBUG("check %s integrity %s.\r\n", name, (0 == file_check) ? "success" : "failed");

    return file_check;
}

static s32 tms_upgrade_version_save(s8 *tms_ver, u8 flag)
{
    u8  ver_log[0x40 * 4 + 4 + 1 + 1];
    s32 len, ret;
    u32 crc32save = 0, crc32calc = 0xFFFFFFFF;

    len = 0x40 * 4 + 4 + 1;
    memset(ver_log, 0, len + 1);
    ret = common_vfs_read(TMS_UPGRADE_PROFILE, ver_log, len, 0);
    if (ret != len)
    {
        memset(ver_log, 0, len + 1);
    }
    else
    {
        memcpy(&crc32save, &ver_log[len - 4], 4);
        crc32calc = trendit_calc_crc32(crc32calc, ver_log, len - 4);
        if (crc32save != crc32calc)
        {
            memset(ver_log, 0, len + 1);
            memcpy(&ver_log[1], "V0.0.1", 4);
            TMS_DEBUG("tms upgrade crc32 not match, reset default V0.0.1.\r\n");
        }
    }
    ver_log[0] = flag;
    memset(&ver_log[0x40 + 1], 0, 0x40 * 3 + 4);
    //
    strcpy(&ver_log[0x20 + 1], tms_ver);
    ddi_misc_get_firmwareversion(FIRMWARETYPE_APBOOT, &ver_log[0x40 + 1]);
    ddi_misc_get_firmwareversion(FIRMWARETYPE_APCORE, &ver_log[0x40 * 2 + 1]);
    strcpy(&ver_log[0x40 * 3 + 1], APP_VER);
    // update crc32
    crc32calc = 0xFFFFFFFF;
    crc32calc = trendit_calc_crc32(crc32calc, ver_log, len - 4);
    memcpy(&ver_log[len - 4], &crc32calc, 4);
    ret = common_vfs_overwrite(TMS_UPGRADE_PROFILE, ver_log, len, 0);
    TMS_DEBUG("%d, %s: %s.\r\n", ret, tms_ver, ver_log);
    TRACE_INFO_HEX(ver_log, len);

    return ret;
}

static void tms_upgrade_file_clean(void)
{
    ddi_file_delete(TMS_APUPDATE_FILE_NAME);
    //播放升级失败
    //trendit_audio_play(AUDIO_UPLOADING);
}

static void upgrade_download_result(s32 result)
{
    if (0 == result)
    {
        system_data_instance()->set_poweroff_reason(RESEASON_REBOOT_UPDATE);
        tms_upgrade_version_save(g_binary_info->new_version, 1);

        ddi_misc_ioctl(MISC_IOCTL_SET_UPDATEFLG, 0, 0);
        ddi_misc_reboot();
        // 升级文件下载并校验成功
    }
    else
    {
        tms_upgrade_file_clean();
        audio_instance()->audio_play(AUDIO_UPLOADED_ERR, AUDIO_PLAY_BLOCK);
        // 升级文件下载或校验失败
    }

    if (g_binary_info)
    {
        TMS_FREE(g_binary_info);
        g_binary_info = 0;
    }
}

static void tms_upgrade_feedback_para_backup(u8 flag)
{
    u32 crc32calc = 0xFFFFFFFF;
    s8 buff[12] = {0};
    upgrade_callback_info_t *info;

    if (0xA5 == flag && g_upgrade_info && g_upgrade_info->m_upgrade_callback_info)
    {
        info = g_upgrade_info->m_upgrade_callback_info;
        buff[0] = flag;
        memcpy(&buff[1], &info->m_backend_cmd, 2);
        memcpy(&buff[3], &info->m_packno, 2);
        crc32calc = trendit_calc_crc32(crc32calc, &buff[0], 5);
        memcpy(&buff[5], &crc32calc, 4);
        common_vfs_overwrite(TMS_UPGRADE_FEEDBACK, &buff[0], 9, 0);
    }
}

static s32 tms_download_finish_default(u8 *data, s32 len, s32 status)
{
    s32 ret = -1, bytes = -1;
    s32 offset;
    tms_utils_t **tms_utils;

    tms_utils = tms_utils_handle();
    TMS_DEBUG("last packet data notify %d:%d:0x%X.\r\n", status, len, data);

    ret = status;
    if (0 == status)
    {
        (*tms_utils)->flow_control.crc32_save = trendit_calc_crc32(
                        (*tms_utils)->flow_control.crc32_save, data, len);

        if (0 != (*tms_utils)->flow_control.direct_url)
        {
            offset = (*tms_utils)->flow_control.partial.last_offset;
            ret = tms_insert_data_to_file((*tms_utils)->flow_control.file_path, data, len, offset,
                                           (*tms_utils)->flow_control.vfs_save, 1);
            if (len == ret)
            {
                ret = TMS_OK;
                status = ret;
            }
            else
            {
                status = ret | (-1);
            }
        }
        else
        {
            if (4096 > (*tms_utils)->flow_control.file_total)
            {
                TMS_DEBUG("parse url failed with %d.\r\n", ret);
            }
            else
            {
                if (0xA5 == (*tms_utils)->flow_control.finish_notify)
                {
                    (*tms_utils)->flow_control.finish_notify = 0;
                    ret = tms_file_integrity_check((*tms_utils)->flow_control.file_path, (*tms_utils)->flow_control.vfs_save);
                    TMS_DEBUG("upgrade file already download, process upgrade...\r\n");
                }
                else
                {
                    offset = (*tms_utils)->flow_control.partial.last_offset - (TMS_PACKAGING_HEADER_SIZE - 1);
                    bytes  = tms_insert_data_to_file((*tms_utils)->flow_control.file_path, data, len, offset,
                                                     (*tms_utils)->flow_control.vfs_save, 1);
                    if (0 < bytes)
                    {
                        ret = tms_file_integrity_check((*tms_utils)->flow_control.file_path, (*tms_utils)->flow_control.vfs_save);
                        if (0 == ret)
                        {
                            status = bytes;
                            tms_breakpoint_info_backup(*tms_utils, &g_apupdate_header);
                        }
                    }
                }
            }
        }
    }

    if (g_cacp_upload_pfn)
    {
        tms_upgrade_feedback_para_backup((*tms_utils)->flow_control.general_file);
        g_cacp_upload_pfn(ret);
    }

    if (0xA5 == (*tms_utils)->flow_control.general_file)
    {
        upgrade_download_result(ret);
    }

    if ((*tms_utils)->data_handle.msg_ext_notify)
    {
        (*tms_utils)->data_handle.msg_ext_notify(status);
    }

    return ret;
}

static s32 download_tms_file_complete(u8 *data, s32 len, s32 status)
{
    s32 ret;

    g_file_downloading = 0;

    if (g_upgrade_info && g_upgrade_info->self_save_cb)
    {
        g_upgrade_info->self_save_cb(data, len,
                                    (0 == status) ? DOWNLOADED : DOWNLOADERR,
                                     g_upgrade_info->m_upgrade_callback_info);
        if (g_cacp_upload_pfn)
        {
            g_cacp_upload_pfn(status);
        }
    }
    else
    {
        tms_download_finish_default(data, len, status);
    }
}

static s32 tms_download_binary_init(void)
{
    s8 *p, *q;
    s32 ret = -1;
    tms_utils_t ** tms_utils;

    if (g_binary_info && g_binary_info->url)
    {
        tms_utils = tms_utils_handle();
        ret = tms_utils_data_init(tms_utils);
        if (-1 < ret)
        {
            ret = tms_http_url_parse(&(*tms_utils)->append_info, g_binary_info->url);
            if (0 != ret)
            {
                tms_utils_free(*tms_utils);
                TMS_DEBUG("parse url failed with %d.\r\n", ret);
            }
            else
            {
                // TMS打包信息保存在前4K
                (*tms_utils)->append_info.block_start = 0;
                (*tms_utils)->append_info.block_size  = 4096;
                (*tms_utils)->flow_control.general_file = 0xA5;
                //
                strcpy((*tms_utils)->flow_control.file_path, TMS_APUPDATE_FILE_NAME);
                //
                (*tms_utils)->data_handle.http_append_head = download_tms_file_append_head;
                (*tms_utils)->data_handle.msg_data_update  = download_tms_file_receiving;
                (*tms_utils)->data_handle.msg_data_finish  = download_tms_file_complete;
                (*tms_utils)->data_handle.msg_free         = tms_utils_free;
                //
                (*tms_utils)->msg_node->m_func     = tms_utils_msg_handle;
                (*tms_utils)->msg_node->m_priority = MSG_NORMAL;
                (*tms_utils)->msg_node->m_lParam   = 0;
                (*tms_utils)->msg_node->m_wparam   = 0;
                (*tms_utils)->msg_node->m_status   = MSG_INIT;
                snprintf((*tms_utils)->msg_node->m_msgname, sizeof((*tms_utils)->msg_node->m_msgname), "%s", "tms");
                    //
                ret = trendit_postmsg((*tms_utils)->msg_node);
                TMS_DEBUG("post get url msg with %d.\r\n", ret);
            }
        }
    }
    else
    {
        TMS_DEBUG("url not pre alloc with %d.\r\n", ret);
    }
}

static s32 tms_download_binary_trigger(s8 *data, s32 len)
{
    s32 ret = -1;

    if (g_binary_info)
    {
        if ((NET_VERSION_URL_MAX - 1) < strlen(data))
        {
            TMS_DEBUG("binary url too long to save, %d/%d.\r\n", NET_VERSION_URL_MAX, strlen(data));
        }
        else
        {
            if (g_binary_info && g_binary_info->url)
            {
                memset(g_binary_info->url, 0, NET_VERSION_URL_MAX);
                strcpy(g_binary_info->url, data);
                ret = tms_msg_delay_post(tms_download_binary_init, "tms_bin_download", 50);
                if (0 == ret)
                {
                    g_file_downloading = 0x5AB6;
                    audio_instance()->audio_play(AUDIO_UPLOADING, AUDIO_PLAY_BLOCK);
                }
            }
            else
            {
                TMS_DEBUG("not alloc memory for this.\r\n");
            }
        }
    }
    else
    {
        TMS_DEBUG("malloc failed for version detail.\r\n");
    }

    return ret;
}


/********************************************************************************
 *
 *  获取最新版本链接
 *
 */
#if 0
static s32 tms_check_version_detail_parse(s8 *data, u32 len, version_detail_t *detail)
{
    s32 ret = -0xFF;
    cJSON *cjson_root = NULL, *cjson_data = NULL, *cjson_item = NULL;
    cJSON *cjson_firmware = NULL;

    if (detail)
    {
        cjson_root = cJSON_Parse(data);
        TMS_DEBUG("csjon parse root.\r\n");
        if (cjson_root)
        {
            TMS_DEBUG("%s.\r\n", cjson_root->string);
            cjson_data = cJSON_GetObjectItem(cjson_root, "data");
            if (cjson_data)
            {
                TMS_DEBUG("csjon parse fir.\r\n");
                cjson_firmware = cJSON_GetObjectItem(cjson_data, "firmwares");
            }
        }
        TMS_DEBUG("csjon parse firmwares.\r\n");
        if (cjson_firmware)
        {
            cjson_item = cJSON_GetObjectItem(cjson_firmware, "firmwareClass");
            if (cjson_item && cJSON_String == cjson_item->type &&
                cjson_item->valuestring && NET_VERSION_CATEGORY_MAX > strlen(cjson_item->valuestring))
            {
                strcpy(detail->category, cjson_item->valuestring);
                ret = 0;
            }
            //
            cjson_item = cJSON_GetObjectItem(cjson_firmware, "firmwareCode");
            if (0 == ret && cjson_item && cJSON_String == cjson_item->type &&
                cjson_item->valuestring && NET_VERSION_NUMBER_MAX > strlen(cjson_item->valuestring))
            {
                strcpy(detail->new_version, cjson_item->valuestring);
                ret = 0;
            }
            else
            {
                ret |= -1;
            }
            //
            cjson_item = cJSON_GetObjectItem(cjson_firmware, "platformName");
            if (0 == ret && cjson_item && cJSON_String == cjson_item->type &&
                cjson_item->valuestring && NET_VERSION_PLATFORM > strlen(cjson_item->valuestring))
            {
                strcpy(detail->platfrom, cjson_item->valuestring);
                ret = 0;
            }
            else
            {
                ret |= -2;
            }
        }

        if (cjson_root)
        {
            cJSON_Delete(cjson_root);
        }
        TMS_DEBUG("\r\ncategory: %s,\r\nnew_version: %s, platfrom: %s.\r\n",
                   detail->category, detail->new_version, detail->platfrom);
    }
    TMS_DEBUG("parse version detail with %d.\r\n", ret);

    return ret;
}
#endif
static s32 get_url_append_head(append_info_t *append_info)
{
    s32 ret = 0;
    append_info_t *p = append_info;

    if (p && p->raw_data)
    {
        HTTP_TMS_LAST_VERSION_ADDR(p->raw_data, p->domain, p->page, p->port, p->catagory, p->new_version, p->old_version);
        p->raw_size = strlen(p->raw_data);
    }

    return ret;
}

static s32 get_url_receiving(s8 *data, s32 len, s32 status)
{
    s32 ret = -1;

    if (916 > len)
    {
        TMS_DEBUG("status: %d, len: %d, data:\r\n%s\r\n", status, len, data);
    }
    else
    {
        TMS_DEBUG("data: 0x%x, len: %d, status: %d.\r\n", data, len, status);
    }

    //ring_buff_write(g_ring_buff_recv, data, len);

    return ret;
}

static s32 get_url_complete(s8 *data, s32 len, s32 status)
{
    s32 ret = -1;
    s8 *ptr_url, *ptr_end = NULL;

    if (916 > len)
    {
        TMS_DEBUG("status: %d, len: %d, data:\r\n%s\r\n", status, len, data);
    }
    else
    {
        TMS_DEBUG("data: 0x%x, len: %d, status: %d.\r\n", data, len, status);
    }

    ptr_url = strstr(data, "url\":\"http");
    if (ptr_url)
    {
        ptr_url += 6;
        ptr_end = strstr(ptr_url, "\",\"");
        if (ptr_end)
        {
            *ptr_end = '\0';
        }
    }
    if (ptr_end)
    {
        ret = tms_download_binary_trigger(ptr_url, strlen(ptr_url));
    }
    else
    {
        TMS_DEBUG("illegal url: %s\r\n,", ptr_url);
    }

    return ret;
}

// 获取版本信息
static s32 tms_get_url_init(void)
{
    s32 ret = -0xFF;
    u8  sn_no[0x20] = {0};
    tms_utils_t ** tms_utils;

    if (g_binary_info)
    {
        tms_utils = tms_utils_handle();
        ret = tms_utils_data_init(tms_utils);
        if (-1 < ret)
        {
            //
            (*tms_utils)->append_info.port = TMS_SERVER_PORT;
            strcpy((*tms_utils)->append_info.domain, TMS_SERVER_IP);
            strcpy((*tms_utils)->append_info.page, TMS_PAGE_GET_ADDRESS);
            //strcpy((*tms_utils)->append_info.sn, TMS_DEVICE_SN);
            ddi_misc_read_dsn(SNTYPE_POSSN, sn_no);
            if (24 <= sn_no[0])
            {
                memset((*tms_utils)->append_info.sn, '0', 0x14);
            }
            else
            {
                memcpy((*tms_utils)->append_info.sn, &sn_no[1], sn_no[0]);
            }
            strcpy((*tms_utils)->append_info.catagory, g_binary_info->category);
            strcpy((*tms_utils)->append_info.old_version, "V0.0.1");
            strcpy((*tms_utils)->append_info.new_version, g_binary_info->new_version);
            //
            (*tms_utils)->data_handle.http_append_head = get_url_append_head;
            (*tms_utils)->data_handle.msg_data_update  = get_url_receiving;
            (*tms_utils)->data_handle.msg_data_finish  = get_url_complete;
            (*tms_utils)->data_handle.msg_free         = tms_utils_free;
            //
            (*tms_utils)->msg_node->m_func     = tms_utils_msg_handle;
            (*tms_utils)->msg_node->m_priority = MSG_NORMAL;
            (*tms_utils)->msg_node->m_lParam   = 0;
            (*tms_utils)->msg_node->m_wparam   = 0;
            (*tms_utils)->msg_node->m_status   = MSG_INIT;
            snprintf((*tms_utils)->msg_node->m_msgname, sizeof((*tms_utils)->msg_node->m_msgname), "%s", "tms");
                //
            ret = trendit_postmsg((*tms_utils)->msg_node);
            TMS_DEBUG("post get url msg with %d.\r\n", ret);
        }
        else
        {
            TMS_FREE(g_binary_info);
            g_binary_info = NULL;
        }
    }

    return ret;
}

static s32 tms_get_url_trigger(s8 *data, s32 len)
{
    s32 ret = 0;
    version_detail_t *version_detail = NULL;

    if (g_binary_info)
    {
        TMS_DEBUG("parse version detail start.\r\n");
        strcpy(g_binary_info->category, TMS_DEVICE_CLASS);
        ret = 0;
    }
    else
    {
        TMS_DEBUG("malloc failed for version detail.\r\n");
    }

    tms_msg_delay_post(tms_get_url_init, "tms_get_url", 50);

    return ret;
}

/********************************************************************************
 *
 *
 *      Check Version
 *
 */
static s32 check_version_append_head(append_info_t *append_info)
{
    s32 ret = 0;
    append_info_t *p = append_info;

    if (p && p->raw_data)
		{
	        HTTP_TMS_LAST_VERSION_CHECK(p->raw_data, p->domain, p->page, p->port, p->sn, p->catagory, p->old_version);
            p->raw_size = strlen(p->raw_data);
    //HTTP_TMS_LAST_VERSION_CHECK(p->raw_data, TMS_SERVER_IP, TMS_PAGE_LATEST_VERSION, TMS_SERVER_PORT, TMS_DEVICE_SN, TMS_DEVICE_CLASS, "0.0.1");
		}
    return ret;
}

static s32 check_version_receiving(s8 *data, s32 len, s32 status)
{
    s32 ret = -1;

    if (916 > len)
    {
        TMS_DEBUG("status: %d, len: %d, data:\r\n%s\r\n", status, len, data);
    }
    else
    {
        TMS_DEBUG("data: 0x%x, len: %d, status: %d.\r\n", data, len, status);
    }

    //ring_buff_write(g_ring_buff_recv, data, len);

    return ret;
}

static s32 local_version_is_newest(s8 *data, s32 len)
{
    s32 ret = 1;
    s8 *q, *p, *ptr_class, *ptr_code, *ptr_platform, *ptr_msg, *ptr_end;

    ptr_msg   = strstr(data, "message\":\"");
    ptr_class = strstr(data, "firmwareClass\":\"");
    ptr_code  = strstr(data, "firmwareCode\":\"");
    ptr_platform = strstr(data, "platformName\":\"");
    if (ptr_class && ptr_code && ptr_platform)
    {
        ptr_class += 0x10;
        ptr_code  += 0xF;
        ptr_platform += 0xF;
        ptr_end = strstr(ptr_class, "\",\"");
        if (ptr_end)
        {
            *ptr_end = '\0';
        }
        ptr_end = strstr(ptr_code, "\",\"");
        if (ptr_end)
        {
            *ptr_end = '\0';
        }
        ptr_end = strstr(ptr_platform, "\",\"");
        if (ptr_end)
        {
            *ptr_end = '\0';
        }
        ret = tms_version_newest_local(ptr_code);
        if (0 == ret)
        {
            g_binary_info = TMS_MALLOC(sizeof(version_detail_t));
            if (g_binary_info)
            {
                memset(g_binary_info, 0, sizeof(version_detail_t));
                strcpy(g_binary_info->new_version, ptr_code);
                strcpy(g_binary_info->category, ptr_class);
                strcpy(g_binary_info->platfrom, ptr_platform);
            }
        }
        TMS_DEBUG("%s, %s, %s\r\n", ptr_class, ptr_code, ptr_platform);
    }

    //ret = tms_check_version_detail_parse(data, len, g_binary_info);

    return ret;
}

static void tms_update_daliy_profile(u8 flag)
{
    u32 crc32calc  = 0xFFFFFFFF;
    u8  system_time[12] = {0};

    system_time[0] = flag;
    ddi_misc_get_time(&system_time[1]);
    crc32calc = trendit_calc_crc32(crc32calc, &system_time[1], 6);
    memcpy(&system_time[7], &crc32calc, 4);
    common_vfs_overwrite(TMS_UPGRADE_CONFIG, system_time, 11, 0);
}

static s32 check_version_complete(s8 *data, s32 len, s32 status)
{
    s32 ret = 0;

    if (916 > len)
    {
        TMS_DEBUG("status: %d, len: %d, data:\r\n%s\r\n", status, len, data);
    }
    else
    {
        TMS_DEBUG("data: 0x%x, len: %d, status: %d.\r\n", data, len, status);
    }

    if (0 == local_version_is_newest(data, len))
    {
        tms_update_daliy_profile(1);
        tms_get_url_trigger(data, len);
    }
    else
    {
        tms_update_daliy_profile(0);
        TMS_DEBUG("tms local version is update to date.\r\n");
    }

    return ret;
}

static void tms_local_version_query(s8 *buff)
{
    s8 *p;
    u8  ver_log[0x40 * 4 + 4 + 1 + 1];
    s32 len, ret = -1;
    u32 crc32set = 0xFFFFFFFF, crc32save = 0, crc32calc;

    if (!buff)
    {
        TMS_DEBUG("the buff is null.\r\n");
    }
    else
    {
        len = 0x40 * 4 + 4 + 1;
        memset(ver_log, 0, 0x40 * 4 + 4 + 1 + 1);
        if (common_vfs_read(TMS_UPGRADE_PROFILE, ver_log, len, 0) == len)
        {
            memcpy(&crc32save, &ver_log[len - 4], 4);
            crc32calc = trendit_calc_crc32(crc32set, ver_log, len - 4);
            if (crc32calc != crc32save)
            {
                TMS_DEBUG("tms profile crc32 not match.\r\n");
            }
            else
            {   // V0.0.1
                p = &ver_log[1 + 0x20];
                if ('V' == p[0] && '.' == p[2] && '.' == p[4])
                {
                    // 版本号格式检查
                }
                ret = 0;
            }
        }

        if (0 == ret)
        {
            strcpy(buff, &ver_log[1 + 0x20]);
        }
        else
        {
            strcpy(buff, TMS_DEFAULT_VERSION_CODE);
        }
        TMS_DEBUG("query local tms version: %s.\r\n", buff);
    }
}

static s32 tms_check_version_trigger(s8 index)
{
    s32 ret = MSG_KEEP;
    tms_utils_t ** tms_utils;
    u8  sn_no[0x20];

    tms_utils = tms_utils_handle();
    ret = tms_utils_data_init(tms_utils);
    if (TMS_OK <= ret)
    {
        //
        memset(sn_no, 0, 0x20);
        ddi_misc_read_dsn(SNTYPE_POSSN, sn_no);
        (*tms_utils)->append_info.port = TMS_SERVER_PORT;
        strcpy((*tms_utils)->append_info.domain, TMS_SERVER_IP);
        strcpy((*tms_utils)->append_info.page, TMS_PAGE_LATEST_VERSION);
        if (24 <= sn_no[0])
        {
            memset((*tms_utils)->append_info.sn, '0', 0x14);
            TMS_DEBUG("error format sn: %s\r\n", sn_no);
        }
        else
        {
            memcpy((*tms_utils)->append_info.sn, &sn_no[1], sn_no[0]);
        }
        strcpy((*tms_utils)->append_info.catagory, TMS_DEVICE_CLASS);
        tms_local_version_query((*tms_utils)->append_info.old_version);
        TMS_DEBUG("Device Info: %s, %s, %s.\r\n", (*tms_utils)->append_info.sn,
                  (*tms_utils)->append_info.catagory, (*tms_utils)->append_info.old_version);
        //
        (*tms_utils)->data_handle.http_append_head = check_version_append_head;
        (*tms_utils)->data_handle.msg_data_update  = check_version_receiving;
        (*tms_utils)->data_handle.msg_data_finish  = check_version_complete;
        (*tms_utils)->data_handle.msg_free         = tms_utils_free;
        //
        (*tms_utils)->msg_node->m_func     = tms_utils_msg_handle;
        (*tms_utils)->msg_node->m_priority = MSG_NORMAL;
        (*tms_utils)->msg_node->m_lParam   = 0;
        (*tms_utils)->msg_node->m_wparam   = 0;
        (*tms_utils)->msg_node->m_status   = MSG_INIT;
        snprintf((*tms_utils)->msg_node->m_msgname, sizeof((*tms_utils)->msg_node->m_msgname), "%s", "tms");
            //
        ret = trendit_postmsg((*tms_utils)->msg_node);
    }

    return ret;
}

/********************************************************************************
 *
 *
 *
 *
 */

static s32 tms_connection_type_get(void)
{
    s32 connection_type = 0;
    network_strategy_t *network_strategy;

    network_strategy = network_strategy_instance();
    if (network_strategy)
    {
        connection_type = network_strategy->m_current_type;
    }

    return connection_type;
}

static s32 tms_network_connection_check(void)
{
    return tcp_protocol_instance()->check_tcp_connect_success();
}

static s32 tms_socket_connecting_timeout(tms_utils_t *tms_utils)
{
    s32 ret = 1;
    u32 conn_try_overtime = TCP_CONNECT_OVERTIME;
    network_strategy_t *network_strategy;

    network_strategy = network_strategy_instance();
    if (tms_utils)
    {
        if (network_strategy)
        {
            if (COMMU_WIRELESS == tms_utils->socket_info.prev_net)
            {
                conn_try_overtime = network_strategy->m_wireless_tcp_con_try_overtime;
            }
            else if (COMMU_WIFI == tms_utils->socket_info.prev_net)
            {
                conn_try_overtime = network_strategy->m_wifi_tcp_con_try_overtime;
            }
            else
            {
                // default
            }
        }
        ret = trendit_query_timer(tms_utils->socket_info.create_time, conn_try_overtime);
    }

    return ret;
}

static s32 tms_tcp_connection_init(tms_utils_t *tms_utils)
{
    tms_utils->socket_info.net_change = 0;
    tms_utils->socket_info.prev_net = tms_connection_type_get(); // wifi or gprs
    //
    TRACE_INFO("socket: %d, %s:%d.", tms_utils->socket_info.socket_id, tms_utils->append_info.domain,
                                     tms_utils->append_info.port);

    switch (tms_utils->socket_info.prev_net)
    {
        case COMMU_WIRELESS:
            tms_utils->socket_info.socket_id = ddi_gprs_socket_create(0, 0, tms_utils->append_info.domain,
                                                                            tms_utils->append_info.port);
            tms_utils->socket_handle.socket_send = ddi_gprs_socket_send;
            tms_utils->socket_handle.socket_recv = ddi_gprs_socket_recv;
            tms_utils->socket_handle.socket_close = ddi_gprs_socket_close;
            tms_utils->socket_handle.socket_get_status = ddi_gprs_socket_get_status;
            //
            tms_utils->socket_handle.socket_create = tms_tcp_socket_create;
            break;

        case COMMU_WIFI:
            tms_utils->socket_info.socket_id = ddi_wifi_socket_create_start(0, 0, tms_utils->append_info.domain,
                                                                                  tms_utils->append_info.port);
            tms_utils->socket_handle.socket_send = ddi_wifi_socket_send;
            tms_utils->socket_handle.socket_recv = ddi_wifi_socket_recv;
            tms_utils->socket_handle.socket_close = ddi_wifi_socket_close;
            tms_utils->socket_handle.socket_get_status = ddi_wifi_socket_get_status;
            //
            tms_utils->socket_handle.socket_create = tms_tcp_socket_create;
            break;
        default:
            tms_utils->socket_info.socket_id = -0xFF;
            break;
    }
    TRACE_INFO("socket: %s, %d, %s:%d.", (COMMU_WIFI == tms_utils->socket_info.prev_net) ? "WiFi" : "2G",
        tms_utils->socket_info.socket_id, tms_utils->append_info.domain, tms_utils->append_info.port);
    return tms_utils->socket_info.socket_id;
}

static s32 tms_tcp_socket_create(tms_utils_t *tms_utils)
{
    s32 socketid = -1;
    s32 ret = MSG_KEEP;

    socketid = tms_tcp_connection_init(tms_utils);
    if (0 < socketid)
    {
        tms_utils->socket_info.socket_id     = socketid;
        tms_utils->socket_info.create_time   = trendit_get_ticks();
        tms_utils->socket_info.query_time    = trendit_get_ticks();
        tms_utils->socket_info.socket_status = SOCKET_CONNECTING;
        ret = MSG_KEEP;
    }
    else
    {
        tms_utils->socket_info.status_code = TMS_ERR_CREATE_SOCKET_FAILED;
        tms_upgrade_info_callback(tms_utils->socket_info.status_code);
        if (tms_utils->data_handle.msg_ext_notify)
        {
            tms_utils->data_handle.msg_ext_notify(TMS_ERR_CREATE_SOCKET_FAILED);
        }
        //tms_utils->data_handle.msg_free(tms_utils);
        ret = MSG_OUT;
    }

    return ret;
}

static s32 tms_tcp_socket_connecting(msg_t *m_msg, tms_utils_t *tms_utils)
{
    s32 ret = MSG_KEEP;

    if (1 == trendit_query_timer(tms_utils->socket_info.query_time, TCP_CONNECTION_CHECK_INTERVAL))
    {
        tms_utils->socket_info.socket_status = tms_utils->socket_handle.socket_get_status(tms_utils->socket_info.socket_id);
        tms_utils->socket_info.query_time = trendit_get_ticks();
        TMS_DEBUG("socket %d query end with %d.", tms_utils->socket_info.socket_id, tms_utils->socket_info.socket_status);
    }

    switch (tms_utils->socket_info.socket_status)
    {
        TMS_DEBUG("socket status: %d end\r\n", tms_utils->socket_info.socket_status);
        case 1:
            if (1 == tms_socket_connecting_timeout(tms_utils))
            {
                tms_utils->socket_info.status_code = TMS_ERR_CONNECTING_TIMEOUT;
                tms_upgrade_info_callback(tms_utils->socket_info.status_code);
                m_msg->m_status = MSG_INIT;
                ret = MSG_OUT;
                TMS_DEBUG("socket %d connect timeout, msg out\r\n", tms_utils->socket_info.socket_id);
                TMS_DEBUG("create: %d, query: %d, %d\r\n", tms_utils->socket_info.create_time,
                            tms_utils->socket_info.query_time, trendit_get_ticks());
            }
            else
            {
                ret = MSG_KEEP;
                //TMS_DEBUG("socket status check continue.\r\n");
            }
            break;
        case 2:
            // 已经连接
            tms_utils->socket_info.socket_status = SOCKET_CONNECTED;
            m_msg->m_status = MSG_SERVICE;
            ret = MSG_KEEP;
            TMS_DEBUG("socket %d connected status %d/%d.",
                       tms_utils->socket_info.socket_id, tms_utils->socket_info.socket_status, SOCKET_CONNECTED);
            break;
        default:
            ret = MSG_OUT;
            TMS_DEBUG("msg status error.\r\n");
            break;
    }


    return ret;
}

/********************************************************************************
 *
 *
 *
 *
 */
static s32 upgrade_tcp_socket_connecting(msg_t *m_msg, tms_utils_t *tms_utils)
{
		s32 ret = MSG_KEEP;

    tms_utils->socket_info.socket_status = tms_utils->socket_handle.socket_get_status(tms_utils->socket_info.socket_id);
    switch (tms_utils->socket_info.socket_status)
    {
        case SOCKET_CONNECTING:
            break;

        case SOCKET_CONNECTED:
            tms_utils->flow_control.packet_resume = 1;
            m_msg->m_status = MSG_SERVICE;
            break;

        case SOCKET_CLOSE:
            break;
    }

    return ret;
}

static s32 tms_tcp_request_check(tms_utils_t *tms_utils)
{
    s32 ret = -1;
    if (1 == tms_utils->flow_control.packet_resume &&
        tms_utils->flow_control.content_offset >= tms_utils->flow_control.packet.content_len
        + tms_utils->flow_control.packet.header_len) {
        ret = 0;
    }
    TMS_DEBUG("http request will %s send.\r\n", (0 == ret) ? "" : "not");
    return ret;
}

static s32 tms_tcp_request_send(tms_utils_t *tms_utils)
{
    s32 ret = -1;

    if (tms_utils->data_handle.http_append_head) {
        tms_utils->append_info.raw_data = tms_utils->socket_info.send_buff;
        tms_utils->data_handle.http_append_head(&tms_utils->append_info);
        tms_utils->socket_info.send_size = tms_utils->append_info.raw_size;
        ret = tms_utils->socket_handle.socket_send(tms_utils->socket_info.socket_id, tms_utils->socket_info.send_buff,
                                             tms_utils->socket_info.send_size);
    }
#if 1
    TMS_DEBUG("send_ret: %d, send_len: %d, send_raw:\r\n%s.\r\n", ret,
               tms_utils->socket_info.send_size, tms_utils->socket_info.send_buff);
#endif
    return ret;
}

static s32 tms_tcp_response_recv(tms_utils_t *tms_utils)
{
    s32 ret;
    u32 ticks_start;
    s32 last_offset = 0, recv_none = 0;

    ticks_start = trendit_get_ticks();
    TMS_DEBUG("tcp start receive with ticks %d\r\n", ticks_start);
    do {
        // wait for server response.
        ret = tms_utils->socket_handle.socket_recv(tms_utils->socket_info.socket_id,
              tms_utils->socket_info.recv_buff + tms_utils->flow_control.packet.received_len,
              TMS_NET_RECV_BUFFER_MAX - tms_utils->socket_info.recv_size);
        if (0 < ret)
        {
            recv_none = 0;
            last_offset = tms_utils->flow_control.packet.received_len;
            //
            tms_utils->flow_control.content_offset += ret;
            //
            tms_utils->flow_control.packet.received_len += ret;
            tms_utils->socket_info.recv_size = tms_utils->flow_control.packet.received_len;
            TMS_DEBUG("recv: %d, total: %d, head: %d, body: %d.\r\n", ret, tms_utils->flow_control.packet.received_len,
                      tms_utils->flow_control.packet.header_len, tms_utils->flow_control.packet.content_len);
            //
        }
        // 解析
        if (0 >= tms_utils->flow_control.packet.content_len || 0 >= tms_utils->flow_control.packet.header_len)
        {
            tms_tcp_header_parse(&tms_utils->socket_info.recv_buff[0], last_offset,
                                  tms_utils->flow_control.packet.received_len, &tms_utils->flow_control);
            last_offset = tms_utils->flow_control.packet.received_len;
        }

        if ((0 < tms_utils->flow_control.packet.content_len && 0 < tms_utils->flow_control.packet.header_len) &&
            ((tms_utils->flow_control.packet.content_len + tms_utils->flow_control.packet.header_len) <= tms_utils->flow_control.content_offset))
        {
            TMS_DEBUG("one packet received finish, goto next step.\r\n");
            break;
        }
        //
        if (TMS_NET_RECV_BUFFER_MAX <= tms_utils->flow_control.packet.received_len)
        {
            tms_utils->socket_info.status_code = TMS_ERR_BUFFER_OVERFLOW;
            TMS_DEBUG("recv buff full, next poll will continue.\r\n");
            break;
        }
        if (1 == trendit_query_timer(ticks_start, TCP_RECEIVING_TIMEOUT_MAX))
        {
            tms_utils->socket_info.status_code = TMS_ERR_RECEIVING_TIMEOUT;
            TMS_DEBUG("recv data timeout.\r\n");
            break;
        }
    } while (0 <= ret);
    TMS_DEBUG("ticks for packet receive: %d, %d.\r\n", ticks_start, trendit_get_ticks());
#if 0
    TMS_DEBUG("received-len: %d, header-len: %d, content-len: %d, ret: %d.", tms_utils->flow_control.packet.received_len,
              tms_utils->flow_control.packet.header_len, tms_utils->flow_control.packet.content_len, ret);
    TMS_DEBUG("received: \r\n%s\r\n", tms_utils->socket_info.recv_buff);

#endif

    return ret;
}

static s32 tms_tcp_one_packet_finished(tms_utils_t *tms_utils)
{
    s32 ret = -1;


    if (0 < tms_utils->flow_control.content_offset && 0 < tms_utils->flow_control.packet.content_len)
    {
        if (tms_utils->flow_control.content_offset >= tms_utils->flow_control.packet.content_len +
                                                           tms_utils->flow_control.packet.header_len)
        {
            ret = 0;
        }
    }

    TMS_DEBUG("current packet receive %s.\r\n", (0 == ret) ? "finish" : "continue");

    return ret;
}

static s32 tms_tcp_last_packet_finished(tms_utils_t *tms_utils)
{
    s32 ret = -1;

    if (0 == tms_utils->flow_control.partial_content)
    {
        ret = 0;
    }
    else
    {
        if ((tms_utils->flow_control.file_total - 1) <= tms_utils->flow_control.partial.last_offset +
                                                        tms_utils->flow_control.partial.block_size)
        {
            ret = 0;
        }
    }
    TMS_DEBUG("current %d packet %s the last one.\r\n", tms_utils->flow_control.packet_count, (0 == ret) ? "is" : "not");

    return ret;
}

static void tms_tcp_header_discard(s8 *raw_data, u32 *raw_len)
{
    s8 *p;
    s32 offset = -1, lens = 0;

    if (raw_data && raw_len)
    {
        p = strstr(raw_data, "\r\n\r\n");
        if (p)
        {
            p += 4;
            offset = p - raw_data;
            lens   = *raw_len - offset;
            if (0 < offset)
            {
                memcpy(raw_data, p, lens);
                memset(&raw_data[lens], 0, (*raw_len - lens));
                raw_data[lens] = '\0';
                *raw_len = lens;
            }
        }
    }
    TMS_DEBUG("packet header discard %s, offset: %d, len: %d.\r\n",
               (-1 == offset) ? "failed" : "success", offset, lens);
}

static s32 tms_tcp_response_error_parse(msg_t *m_msg, tms_utils_t *tms_utils)
{
    s32 ret = MSG_KEEP, status;
    // request again
    status = tms_utils->data_handle.msg_data_finish(tms_utils->socket_info.recv_buff,
                                                    tms_utils->socket_info.recv_size,
                                                    tms_utils->socket_info.status_code);
    TMS_DEBUG("parse error response with %d\r\n.", ret);
    return ret;
}

static s32 tms_file_capacity_check(s8 *file, u32 lens)
{
    u32 remain_bytes = 0;
    s32 ret = -1;

    return 0;
    if (file)
    {
        ddi_vfs_deletefile(file);
    }

    ddi_vfs_free_space(&remain_bytes);
    if (lens < remain_bytes)
    {
        ret = 0;
    }

    return ret;
}

static s32 tms_tcp_response_parse(msg_t *m_msg, tms_utils_t *tms_utils)
{
    s32 ret = MSG_KEEP, status = 0;
    s32 remain_bytes;

    if (0 != tms_tcp_one_packet_finished(tms_utils))
    {
        // handle receive buff full
        tms_tcp_header_discard(tms_utils->socket_info.recv_buff, &tms_utils->socket_info.recv_size);
        if (0 < tms_utils->socket_info.recv_size && 0 == tms_utils->socket_info.status_code)
        {
            tms_utils->flow_control.packet.recv_retry = 0;
            // update content offset
            tms_utils->flow_control.partial.last_offset += tms_utils->socket_info.recv_size;
            //
            status = tms_utils->data_handle.msg_data_update(tms_utils->socket_info.recv_buff,
                          tms_utils->socket_info.recv_size, tms_utils->socket_info.status_code);
        }
        else
        {
            tms_utils->flow_control.packet.recv_retry++;
            if (TCP_RECEIVING_RETRY_MAX < tms_utils->flow_control.packet.recv_retry)
            {

            }
        }

        tms_utils->socket_info.status_code |= TMS_TCP_TRANSMIT_RETRY_TIMEOUT;
        // 最后一包数据
        status = tms_utils->data_handle.msg_data_finish(tms_utils->socket_info.recv_buff,
                      tms_utils->socket_info.recv_size, tms_utils->socket_info.status_code);
        //  释放所有资源
        //tms_utils->data_handle.msg_free(tms_utils);
        //
        m_msg->m_status = MSG_INIT;
        ret = MSG_OUT;

        // clear packet
        memset(tms_utils->socket_info.recv_buff, 0, TMS_NET_RECV_BUFFER_MAX + 1);
        tms_utils->socket_info.recv_size = 0;
        tms_utils->flow_control.packet.received_len  = 0;
        TMS_DEBUG("content-total: %d, last-offset: %d.\r\n",
                  tms_utils->flow_control.packet.content_len,
                  tms_utils->flow_control.partial.last_offset);
    }
    else
    {
        tms_utils->flow_control.packet_count++;
        if (2 == tms_utils->flow_control.packet_count
            && (TMS_NET_BLOCK_SIZE_MAX + TMS_PACKAGING_HEADER_SIZE) <= tms_utils->flow_control.partial.block_end+1)
        {
            tms_utils->flow_control.packet_count =
                ((TMS_NET_BLOCK_SIZE_MAX + tms_utils->flow_control.partial.block_end - TMS_PACKAGING_HEADER_SIZE) / TMS_NET_BLOCK_SIZE_MAX) + 1;
        }
        tms_tcp_header_discard(tms_utils->socket_info.recv_buff, &tms_utils->socket_info.recv_size);
        //
        if (0 == tms_utils->flow_control.capacity_enough)
        {
            if (0 == tms_file_capacity_check(TMS_APUPDATE_FILE_NAME, tms_utils->flow_control.file_total))
            {
                tms_utils->flow_control.capacity_enough = 1;
            }
            else
            {
                tms_utils->socket_info.status_code |= TMS_TCP_TRANSMIT_SPACE_NOT_ENOUGH;
            }
        }

        if (0 != tms_tcp_last_packet_finished(tms_utils) && 0 == tms_utils->socket_info.status_code)
        {
            status = tms_utils->data_handle.msg_data_update(tms_utils->socket_info.recv_buff,
                          tms_utils->socket_info.recv_size, tms_utils->socket_info.status_code);
            if (1 == tms_utils->flow_control.packet_count
                && (0 == tms_tcp_last_packet_finished(tms_utils)))
            {
                tms_utils->flow_control.finish_notify = 0xA5;
                // 数据已经下载，等待更新
                status = tms_utils->data_handle.msg_data_finish(tms_utils->socket_info.recv_buff,
                         0, tms_utils->socket_info.status_code);
                //  释放所有资源
                //tms_utils->data_handle.msg_free(tms_utils);
                //
                m_msg->m_status = MSG_INIT;
                ret = MSG_OUT;
            }
            else
            {
                remain_bytes = tms_utils->flow_control.file_total - tms_utils->flow_control.partial.block_end - 1;
                tms_utils->append_info.block_start = tms_utils->flow_control.partial.block_end + 1;
                if (TMS_NET_BLOCK_SIZE_MAX > remain_bytes)
                {
                    tms_utils->append_info.block_size = remain_bytes;
                }
                else
                {
                    tms_utils->append_info.block_size = TMS_NET_BLOCK_SIZE_MAX;
                }
                // clear prev block info
                memset(tms_utils->socket_info.recv_buff, 0, TMS_NET_RECV_BUFFER_MAX + 1);
                tms_utils->socket_info.recv_size = 0;
                //
                memset(&tms_utils->flow_control.packet, 0, sizeof(packet_info_t));
                //
                tms_utils->flow_control.content_offset = 0;
                tms_utils->flow_control.partial.last_offset = tms_utils->flow_control.partial.block_end;
                tms_utils->flow_control.partial.block_start = 0;
                tms_utils->flow_control.partial.block_end   = 0;
                tms_utils->flow_control.partial.block_size  = 0;
                //
                tms_utils->flow_control.packet_resume = 1;
                TMS_DEBUG("<next block> start: %d, size: %d, last: %d.\r\n", tms_utils->append_info.block_start,
                           tms_utils->append_info.block_size, tms_utils->flow_control.partial.last_offset);
            }
        }
        else
        {
            // 最后一包数据
            status = tms_utils->data_handle.msg_data_finish(tms_utils->socket_info.recv_buff,
                     tms_utils->socket_info.recv_size, tms_utils->socket_info.status_code);
            //  释放所有资源
            //tms_utils->data_handle.msg_free(tms_utils);
            //
            m_msg->m_status = MSG_INIT;
            ret = MSG_OUT;
        }
    }

    return ret;
}

static void tms_tcp_socket_alive_check(tms_utils_t *tms_utils)
{
    tms_utils->socket_info.socket_status = tms_utils->socket_handle.socket_get_status(tms_utils->socket_info.socket_id);
    switch (tms_utils->socket_info.socket_status)
    {
        case SOCKET_CONNECTED:
            TMS_DEBUG("tms socket connected.\r\n");
            break;
        case SOCKET_CONNECTING:
            TMS_DEBUG("tms socket connecting...\r\n");
            break;
        case SOCKET_CLOSE:
        default:
            if (5 < tms_utils->socket_info.reconnect)
            {
                tms_utils->socket_info.status_code = TMS_TCP_TRANSMIT_RECONNECT_OVER_STEP;
            }
            else
            {
                tms_utils->socket_handle.socket_close(tms_utils->socket_info.socket_id);
                tms_utils->socket_info.socket_id = -1;
                tms_utils->socket_handle.socket_create(tms_utils);
                tms_utils->socket_info.reconnect++;
            }
            TMS_DEBUG("tms reconnect %s times.\r\n", tms_utils->socket_info.reconnect);
            break;
    }
}

static s32 tms_tcp_socket_transmit(msg_t *m_msg, tms_utils_t *tms_utils)
{
    s32 send_result = 1, ret = MSG_KEEP;

    if (0 == tms_tcp_request_check(tms_utils)) {
        send_result = tms_tcp_request_send(tms_utils);
        if (0 >= send_result)
        {
            tms_tcp_socket_alive_check(tms_utils);
            tms_utils->flow_control.packet.send_retry++;
            TMS_DEBUG("send retry %d with %d.\r\n",
                       tms_utils->flow_control.packet.send_retry, send_result);
        }
        else
        {
            // clear send flag
            tms_utils->flow_control.packet_resume = 0;
            tms_utils->flow_control.packet.send_retry = 0;
        }
    }

    if (MSG_KEEP == ret)
    {
        if (1/*1 == trendit_query_timer(tms_utils->socket_info.recv_time, TCP_RECEIVING_INTERVAL_MAX)*/)
        {
            //tms_utils->socket_info.recv_time = trendit_get_ticks();
            //
            do
            {
                tms_tcp_response_recv(tms_utils);
                if (1 == tms_utils->flow_control.packet.recv_error)
                {
                    ret = tms_tcp_response_error_parse(m_msg, tms_utils);
                }
                else
                {
                    ret = tms_tcp_response_parse(m_msg, tms_utils);
                }
            } while (0);
        }
    }

    return ret;
}

/********************************************************************************
 *
 *
 *
 *
 */
static s32 tms_msg_srv_init(msg_t *m_msg, tms_utils_t *tms_utils)
{
    s32 ret = MSG_KEEP;

    switch (tms_utils->socket_info.socket_status)
    {
        case SOCKET_CLOSE:
            ret = tms_tcp_socket_create(tms_utils);
            TRACE_INFO("socket create status: %d, msg_status: %d/%d.\r\n",
                        tms_utils->socket_info.socket_status, m_msg->m_status, MSG_TERMINAL);
            break;

        case SOCKET_CONNECTING:
            ret = tms_tcp_socket_connecting(m_msg, tms_utils);
            if (SOCKET_CONNECTED == tms_utils->socket_info.socket_status)
            {
                tms_utils->flow_control.packet_resume = 1;
            }
        #if 0
            TRACE_INFO("socket connecting status: %d, msg_status: %d/%d.\r\n",
                        tms_utils->socket_info.socket_status, m_msg->m_status, MSG_INIT);
        #endif
            break;
        case SOCKET_CONNECTED:
            // 不可能到这
            m_msg->m_status = MSG_SERVICE;
            TRACE_INFO("socket connected status: %d, msg_status: %d/%d.\r\n",
                        tms_utils->socket_info.socket_status, m_msg->m_status, MSG_SERVICE);
            ret = MSG_KEEP;
            break;
        default:
            // 释放资源
            TRACE_INFO("socket error %d.\r\n", m_msg->m_status);
            ret = MSG_OUT;
            break;
    }

    return ret;
}

static s32 tms_msg_srv_exec(msg_t *m_msg, tms_utils_t *tms_utils)
{
    s32 ret = MSG_KEEP;

    switch(tms_utils->socket_info.socket_status)
    {
        case SOCKET_CLOSE:
            //ret = tms_tcp_socket_closed(m_msg, net_utils);
            break;
        case SOCKET_CONNECTING:
            ret = tms_tcp_socket_connecting(m_msg, tms_utils);
            break;
        case SOCKET_CONNECTED:
            ret = tms_tcp_socket_transmit(m_msg, tms_utils);
            break;
    }

    return ret;
}

static s32 tms_utils_msg_handle(msg_t *m_msg)
{
    s32 ret = MSG_KEEP;

    if (TRUE == tms_network_connection_check())
    {
        switch (m_msg->m_status)
        {
            case MSG_INIT:
                //TRACE_INFO("message init at %d.\r\n", m_msg->m_status);
                ret = tms_msg_srv_init(m_msg, g_tms_utils);
                break;
            case MSG_SERVICE:
                TRACE_INFO("message service at %d.\r\n", m_msg->m_status);
                ret = tms_msg_srv_exec(m_msg, g_tms_utils);
                break;
            case MSG_TERMINAL:
                TRACE_INFO("message terminal at %d.\r\n", m_msg->m_status);
                ret = MSG_OUT;
                break;
            default:
                TRACE_INFO("unknow message status %d.\r\n", m_msg->m_status);
                break;
        }
    }

    if (MSG_KEEP != ret)
    {
        g_file_downloading = 0;
        if (g_tms_utils)
        {
            g_tms_utils->data_handle.msg_free(g_tms_utils);
        }
        TRACE_INFO("message destory at %d/%x.\r\n", m_msg->m_status, g_tms_utils);
    }

    return ret;
}


static int tms_exec_cmd(u16 cmd, u8 *value_string, u8 *ret_string, u16 ret_stringlen)
{

    return 1;
}

static int tms_exec_qry(u16 cmd, u8 *ret_string, u16 ret_stringlen)
{

    return 1;
}

static int tms_exec_cfg(u16 cmd, u8 *value_string)
{


    return 1;
}

static s32 tms_connection_status_check(void)
{
    s32 ret = MSG_KEEP;

    if (TRUE == tms_network_connection_check())
    {
        tms_network_connected_routine();
        ret = MSG_OUT;
    }

    return ret;
}

static s32 tms_msg_common_handle(msg_t *m_msg)
{
    s32 ret = MSG_KEEP;

    switch (m_msg->m_status)
    {
        case MSG_INIT:
            // process once
            tms_upgrade_result_query();
            m_msg->m_status = MSG_SERVICE;
            break;
        case MSG_SERVICE:
            break;
        case MSG_TERMINAL:
            break;
        default:
            break;
    }
    return MSG_KEEP;
}

model_device_t* tms_instance(void)
{
    s32 ret = -1;
    msg_t *msg_node;

    if (NULL == g_model_tms)
    {
        g_model_tms = TMS_MALLOC(sizeof(model_device_t));
        if (NULL == g_model_tms)
        {
            TRACE_ERR("alloc memory for tms instance failed.\r\n");
        }
        else
        {
            memset(g_model_tms, 0, sizeof(model_device_t));
            g_model_tms->exec_cfg = tms_exec_cmd;
            g_model_tms->exec_qry = tms_exec_qry;
            g_model_tms->exec_cmd = tms_exec_cmd;
        }
        //
        msg_node = TMS_MALLOC(sizeof(msg_t));
        if (msg_node)
        {
            memset(msg_node, 0, sizeof(msg_t));
            msg_node->m_func   = tms_msg_common_handle;
            msg_node->m_status = MSG_INIT;
            msg_node->m_wparam = 0;
            msg_node->m_lParam = 0;
            strcpy(msg_node->m_msgname, "tms_common");
            ret = trendit_postmsg(msg_node);
        }
        TRACE_ERR("tms instance enter with %d.\r\n", ret);

        tms_msg_delay_post(tms_connection_status_check, "tms connection check>", 0);
    }

    return g_model_tms;
}

static void tms_upgrade_feedback_upload(s32 result)
{
    s32 ret;
    u32 crc32calc = 0xFFFFFFFF;
    u32 crc32save = 0;
    s8 buff[12] = {0};
    upgrade_callback_info_t info;

    memset(&info, 0, sizeof(upgrade_callback_info_t));

    ret = common_vfs_read(TMS_UPGRADE_FEEDBACK, &buff[0], 9, 0);
    crc32calc = trendit_calc_crc32(crc32calc, &buff[0], 5);
    memcpy(&crc32save, &buff[5], 4);
    if (9 == ret && 0xA5 == buff[0] && crc32calc == crc32save)
    {
        memcpy(&info.m_backend_cmd, &buff[1], 2);
        memcpy(&info.m_packno, &buff[3], 2);
        upgrade_result_callback(&info, US_UPGRADE_RESULT, result);
        memset(&buff[0], 0, 9);
        common_vfs_overwrite(TMS_UPGRADE_FEEDBACK, &buff[0], 9, 0);
    }
}

// TMS下载标志，TMS当前版本, 三个版本信息:boot+core+app各0x40
static void tms_upgrade_result_check(void)
{
    u8 ver_all[0x40 * 3 + 1];
    u8 ver_log[0x40 * 4 + 4 + 1 + 1];
    s32 len, ret, upgrade_try = -1;
    u32 crc32set = 0xFFFFFFFF, crc32save = 0, crc32calc;
    u8  system_time[12] = {0};

    TMS_DEBUG("tms upgrade result query start.\r\n");

    len = 0x40 * 4 + 4 + 1;
    memset(ver_all, 0, 0x40 * 3 + 1);
    memset(ver_log, 0, 0x40 * 4 + 4 + 1 + 1);
    ret = common_vfs_read(TMS_UPGRADE_PROFILE, ver_log, len, 0);
    if (ret == len)
    {
        memcpy(&crc32save, &ver_log[len - 4], 4);
        crc32calc = trendit_calc_crc32(crc32set, ver_log, len - 4);
        if (crc32calc != crc32save)
        {
            TMS_DEBUG("tms profile crc32 not match.\r\n");
        }
        if (1 != ver_log[0])
        {
            TMS_DEBUG("tms upgrade flag not found.\r\n");
        }
        else
        {
            ddi_misc_get_firmwareversion(FIRMWARETYPE_APBOOT, &ver_all[0]);
            ddi_misc_get_firmwareversion(FIRMWARETYPE_APCORE, &ver_all[0x40]);
            strcpy(&ver_all[0x80], APP_VER);
            //

            if (0 == memcmp(&ver_all[0], &ver_log[1 + 0x40], 0x40 * 3))
            {
                // 升级失败
                upgrade_try = 0;
                TMS_INFO_HEX(ver_all, 0x40 * 3);
                TMS_DEBUG("----------------- version boundary -----------------\r\n");
                TMS_INFO_HEX(ver_log, len);
            }
            else
            {
                TMS_DEBUG("tms already upgrade to the newest version: %s\r\n", &ver_log[1 + 0x20]);
                // 升级成功
                upgrade_try = 1;
                memset(&ver_log[1 + 0x40], 0, 0x40 * 3);
                memcpy(&ver_log[1], &ver_log[1 + 0x20], 0x20);
                //
                ddi_misc_ioctl(MISC_IOCTL_CLR_UPDATEFLG, 0, 0);
                // 日更新
                ddi_misc_get_time(&system_time[1]);
                crc32calc = trendit_calc_crc32(crc32calc, &system_time[1], 6);
                memcpy(&system_time[7], &crc32calc, 4);
                common_vfs_overwrite(TMS_UPGRADE_CONFIG, system_time, 11, 0);
            }
            ver_log[0] = 0;
            // update crc32
            crc32set  = 0xFFFFFFFF;
            crc32calc = trendit_calc_crc32(crc32set, ver_log, len - 4);
            memcpy(&ver_log[len - 4], &crc32calc, 4);
            ret = common_vfs_overwrite(TMS_UPGRADE_PROFILE, ver_log, len, 0);
        }
    }

    if (0 == upgrade_try)
    {
        // 上送报文，播放升级失败
        tms_upgrade_feedback_upload(-1);
        audio_instance()->audio_play(AUDIO_UPLOADED_ERR, AUDIO_PLAY_BLOCK);
        TMS_DEBUG("tms upgrade failed.\r\n");
    }
    else if (1 == upgrade_try)
    {
        // 上送报文，播放升级成功
        tms_upgrade_feedback_upload(0);
        memset(ver_all, 0, 0x40 * 3 + 1);
        common_vfs_overwrite(TMS_UPGRADE_BREAKPOINT, ver_all, 0x40 * 3, 0);
        ddi_file_write(TMS_APUPDATE_FILE_NAME, ver_all, 0x40 * 3);
        ddi_file_delete(TMS_APUPDATE_FILE_NAME);
        trenit_remove_all_device_cfg_file();
        audio_instance()->audio_play(AUDIO_UPLOADED, AUDIO_PLAY_BLOCK);
        TMS_DEBUG("tms upgrade success.\r\n");
    }
    else
    {
        // 读取文件失败，或没有升级
        TMS_DEBUG("no upgrade process found.\r\n");
    }
}

static s32 tms_upgrade_result_query(void)
{
    if (1 == g_upgrade_check_flag)
    {
        g_upgrade_check_flag = 0;
        tms_upgrade_result_check();
    }

    return 0;
}

static s32 tms_version_newest_local(s8 *tms_ver)
{
    s32 ret = -1;
    u8  local[0x10];

    if (tms_ver)
    {
        memset(local, 0, 0x10);
        tms_local_version_query(local);
        if (0 < memcmp(tms_ver, local, 6))
        {
            ret = 0;
        }
    }
    TMS_DEBUG("ret: %d, local: %s, tms: %s.\r\n", ret, local, tms_ver);

    return ret;
}

static u8 tms_hex2dec(u8 data)
{
    return (data / 16) * 10 + (data % 16);
}

static s32 tms_upgrade_daily_check(void)
{
    s32 ret, need_check = -1;
    u8 system_time[7] = {0};
    u8 convent_time[4];
    u8 local_time[12] = {0};
    u32 crc32calc = 0xFFFFFFFF, crc32save = 0;
    u32 stamp_sys, stamp_local;

    ddi_misc_get_time(system_time);
    convent_time[0] = tms_hex2dec(system_time[0]);
    convent_time[1] = tms_hex2dec(system_time[1]);
    convent_time[2] = tms_hex2dec(system_time[2]);
    convent_time[3] = tms_hex2dec(system_time[3]);
    stamp_sys = (convent_time[0] * 365 + convent_time[1] * 30
                + convent_time[2]) * 24 + convent_time[1] * 12 + convent_time[3];
    //
    ret = common_vfs_read(TMS_UPGRADE_CONFIG, local_time, 11, 0);
    if (11 == ret)
    {
        memcpy(&crc32save, &system_time[6], 4);
        crc32calc = trendit_calc_crc32(crc32calc, system_time, 6);
        if (crc32save == crc32calc)
        {
            if (0 != local_time[0])
            {
                convent_time[0] = tms_hex2dec(local_time[1]);
                convent_time[1] = tms_hex2dec(local_time[2]);
                convent_time[2] = tms_hex2dec(local_time[3]);
                convent_time[3] = tms_hex2dec(local_time[4]);
                // 大约24小时
                stamp_local = (convent_time[0] * 365 + convent_time[1] * 30
                              + convent_time[2]) * 24 + convent_time[1] * 12 + convent_time[3];
                if (stamp_sys < (stamp_local + 24))
                {
                    tms_update_daliy_profile(0);
                    need_check = 0;
                    ret = -0x56;
                }
            }
        }
    }

    TMS_DEBUG("tms upgrade daily check finish with %d/%d.\r\n", need_check, ret);

    return need_check;
}

static void tms_network_connected_routine(void)
{
    TMS_DEBUG("network connected, exec routine...\r\n");

    if (machine_instance()->m_tms_auto_switch
        && (0 != tms_upgrade_daily_check()))
	{
		tms_machine_command_parse(CMD_COMMAND_MACHINE_UPGRADE_APP,
	                              TMS_PARTICULAR_UPGRADE_URL,
                                  strlen(TMS_PARTICULAR_UPGRADE_URL),
                                  NULL, 0);
	}
}

/********************************************************************************
 * Function Name: tms_upgrade_routine_status
 * Function Disc: 获取tms是否处于下载更新
 * Function Para: None
 * Function Retn: 0-idle, 1-running
 */
s32 tms_upgrade_routine_status(void)
{
    s32 ret;
    tms_utils_t **pputils;

    pputils = tms_utils_handle();

    return ((pputils&&(*pputils) && 0xA56B5AB6 == g_file_downloading)?1:0);
}

/********************************************************************************
 * Function Name: http_download_routine_status
 * Function Disc: 获取tms是否处于下载状态
 * Function Para: None
 * Function Retn: 0-idle, 1-running
 */
s32 http_download_routine_status(void)
{
    s32 ret;

    if (tms_upgrade_routine_status()
        || tms_schedule_routine_status())
    {
        ret = 1;
        //TMS_DEBUG("%x, tms is%s downing...\r\n", g_file_downloading, (!ret?"n't":""));
    }
    else
    {
        ret = 0;
    }

    return ret;
}

