#include "tms_download.h"

static u32 g_other_downloading = 0;


/******************************************************************************************
 *
 *
 *
 *
 */
static s32 tms_vfs_open(file_object_t *object)
{
    s32 ret = -1;

    if (object && object->full_path)
    {
        if (0 < object->fd && object->opened)
        {
            ret = object->fd;
        }
        else
        {
            object->fd = ddi_vfs_open(object->full_path, "w+");
            if (0 < object->fd)
            {
                object->opened = 0xA5;
            }
        }
    }

    return ret;
}

static s32 tms_vfs_read(file_object_t *object, u8 *buff, u32 len, u32 offset)
{
    s32 ret = -1;

    if (object && 0 < object->fd && object->opened)
    {
        ret = ddi_vfs_seek(object->fd, offset, VFS_SEEK_SET);
        ret = ddi_vfs_read(object->fd, buff, len);
        if (0 < ret)
        {
            object->file_offset += ret;
        }
    }

    return ret;
}

static s32 tms_vfs_write(file_object_t *object, u8 *data, u32 len, u32 offset)
{
    s32 ret = -1;

    if (object && 0 < object->fd && object->opened)
    {
        ret = ddi_vfs_seek(object->fd, offset, VFS_SEEK_SET);
        ret = ddi_vfs_write(object->fd, data, len);
        if (0 < ret)
        {
            object->file_offset += ret;
        }
    }

    return ret;
}

static s32 tms_vfs_close(file_object_t *object)
{
    return ddi_vfs_close(object->fd);
}

/******************************************************************************************
 *
 *
 *
 *
 */

static s32 tms_general_open(file_object_t *object)
{
    s32 ret = -1;

    if (object && object->full_path)
    {
        if (0 < object->fd && object->opened)
        {
            ret = object->fd;
        }
        else
        {
            object->fd = ddi_file_create(object->full_path, 1024, 0);
            if (0 < object->fd)
            {
                object->opened = 0xA5;
            }
        }
    }

    return ret;
}

static s32 tms_general_read(file_object_t *object, u8 *buff, u32 len, u32 offset)
{
    s32 ret = -1;

    if (object && 0 < object->fd && object->opened)
    {
        ret = ddi_file_read(object->full_path, buff, len, offset);
        if (0 < ret)
        {
            object->file_offset = offset + ret;
        }
    }

    return ret;
}

static s32 tms_general_write(file_object_t *object, u8 *data, u32 len, u32 offset)
{
    s32 ret = -1;

    if (object && 0 < object->fd && object->opened)
    {
        ret = ddi_file_insert(object->full_path, data, len, offset);
        if (0 < ret)
        {
            object->file_offset = offset + ret;
        }
    }

    return ret;
}

static s32 tms_general_close(file_object_t *object)
{
    return 0;
}

/******************************************************************************************
 *
 *
 *      升级文件保存检测
 *
 */


/******************************************************************************************
 *
 *
 *
 *
 */

static s32 tms_download_request(PTR para, PTR reserved, u8 *buff, u32 size, s32 status)
{
    s32 ret;
    s32 first_byte_pos, last_byte_pos, remain_byte;
    request_object_t *obj_request;

    obj_request = (request_object_t *)reserved;
    if (!obj_request)
    {
        ret = -2;
        STD_PRINT("illegal request handle...\r\n");
    }
    else
    {
        if (0 >= obj_request->packet->chunck_offset)
        {
            first_byte_pos = 0;
            last_byte_pos  = 1024 * 4 - 1;
        }
        else
        {
            remain_byte    = obj_request->packet->total_length - obj_request->packet->chunck_offset - 1;
            first_byte_pos = obj_request->packet->chunck_offset + 1;
            if (0x2000 < remain_byte)
            {
                last_byte_pos  = first_byte_pos + 0x2000 - 1;
            }
            else
            {
                last_byte_pos  = first_byte_pos + remain_byte - 1;
            }
        }
        //
        ret = sprintf(buff,
                      "GET %s HTTP/1.1\r\n" \
                      "Host: %s:%d\r\n" \
                      "Accept: */*\r\n" \
                      "User-Agent: lfs/7.65.0\r\n" \
                      "Connection: keep-alive\r\n" \
                      "Range: bytes=%d-%d\r\n"\
                      "\r\n",
                      obj_request->url->dest_page,
                      obj_request->url->dest_host,
                      obj_request->url->dest_port,
                      first_byte_pos,
                      last_byte_pos);

        STD_PRINT("request data from %d to %d with %d.\r\n", first_byte_pos, last_byte_pos, ret);
    }

    return ret;
}

static s32 tms_download_continue(tms_transmit_t *transmit, u8 *data, u32 len, s32 code, s32 status)
{
    s32 ret;
    file_operate_t *opt_file;

    STD_PRINT("transmit: 0x%x, data: 0x%x, len: %d, status_code: %d, status: .%d\r\n", transmit, data, len, code, status);
    if (!transmit)
    {
        ret = -1;
    }
    else
    {
        if (transmit->notify_operate.handle.stream)
        {
            STD_PRINT("not found path to store, notify streamd...\r\n");
            ret = transmit->notify_operate.handle.stream(transmit->notify_operate.object.para, data, len, code, status);
        }
    }

    return 0;
}

static s32 tms_download_finish(tms_transmit_t *transmit, u8 *data, u32 len, s32 code, s32 status)
{
    // 结束
    if (0 == code)
    {
        // 正常结束
        if (!transmit->file_operate.object.full_path)
        {
            if (transmit->notify_operate.handle.stream)
            {
                STD_PRINT("not found path to store, notify streamd...\r\n");
                transmit->notify_operate.handle.stream(transmit->notify_operate.object.para, data, len, code, status);
            }
        }
        else
        {
            transmit->file_operate.handle.write(&transmit->file_operate.object, data, len,
                                                 transmit->file_operate.object.file_offset);
            transmit->file_operate.handle.close(&transmit->file_operate.object);
        }
    }
    else
    {
        // 异常结束
    }
    //
    if (transmit->notify_operate.handle.notify)
    {
        transmit->notify_operate.handle.notify(transmit->notify_operate.object.para, code, status);
    }

    return 0;
}

static s32 tms_download_reponse(PTR para, PTR reserved, u8 *data, u32 len, s32 code, s32 status)
{
    s32 ret;
    tms_transmit_t *transmit;

    STD_PRINT("download %d phrase received %d to 0x%X with %d.\r\n", status, len, data, code);
    STD_PRINT("para: 0x%X, reserved: 0x%X.\r\n", para, reserved);

    transmit = (tms_transmit_t *)para;
    if (!transmit)
    {
        ret = -1;
        STD_PRINT("illegel transmit object..., exit soon...\r\n");
    }
    else
    {
        switch (status)
        {
        case 0:
            ret = tms_download_continue(transmit, data, len, code, status);
            break;
        case 1:
            ret = tms_download_finish(transmit, data, len, code, status);
            if (transmit->notify_operate.handle.free)
            {
                transmit->notify_operate.handle.free((PTR)transmit);
            }
            break;
        default:
            ret = 0;
            break;
        }
    }

    return ret;
}

static s32 tms_file_instance_install(file_operate_t *opt)
{
    s32 ret = 0;

    if (!opt)
    {
        ret = -0x331;
        STD_PRINT("para error\r\n");
    }
    else
    {
        if (strstr(opt->object.full_path, "/mtd0/"))
        {
            opt->handle.open  = tms_vfs_open;
            opt->handle.read  = tms_vfs_read;
            opt->handle.write = tms_vfs_write;
            opt->handle.close = tms_vfs_close;
        }
        else if (strstr(opt->object.full_path, "/mtd2/"))
        {
            opt->handle.open  = tms_general_open;
            opt->handle.read  = tms_general_read;
            opt->handle.write = tms_general_write;
            opt->handle.close = tms_general_close;
        }
        else
        {
            ret = -0x332;
            STD_PRINT("%s is\r\n", opt->object.full_path);
        }
    }

    return ret;
}

s32 tms_transmit_message_post(transmit_node *new_node)
{
    s32 ret = 0;
    schedule_object_t new_object;
    tms_transmit_t   *transmit;

    transmit = STD_MALLOC(sizeof(tms_transmit_t));
    if (!transmit)
    {
        ret = -0x201;
        STD_PRINT("malloc memory failed.\r\n");
    }
    else
    {
        memset(transmit, 0, sizeof(tms_transmit_t));
        transmit->notify_operate.object.type   = new_node->file_type;
        transmit->notify_operate.object.para   = new_node->para1;
        transmit->notify_operate.handle.notify = new_node->notify;
        transmit->notify_operate.handle.free   = new_node->free;
        if (new_node->full_path)
        {
            transmit->file_operate.object.full_path = STD_MALLOC(strlen(new_node->full_path) + 1);
            if (!transmit->file_operate.object.full_path)
            {
                ret = -0x202;
                STD_PRINT("malloc memory failed with %d.\r\n", ret);
            }
            else
            {
                memset(transmit->file_operate.object.full_path, 0, strlen(new_node->full_path) + 1);
                strcpy(transmit->file_operate.object.full_path, new_node->full_path);
                ret = tms_file_instance_install(&transmit->file_operate);
                if (0 != ret)
                {
                    STD_FREE(transmit->file_operate.object.full_path);
                    STD_PRINT("install file operate failed with %d.\r\n", ret);
                }
            }
        }
        else
        {
            transmit->notify_operate.handle.stream = new_node->stream;
            if (!transmit->notify_operate.handle.stream)
            {
                ret = -0x203;
                STD_PRINT("illegal stream handle with %d.\r\n", ret);
            }
        }
        if (0 == ret)
        {
            new_object.transmit_type = new_node->transmit_type;
            new_object.notify_object = (PTR)transmit;
            new_object.url           = new_node->full_url;
            if (0 == new_object.transmit_type)
            {
                new_object.request = tms_download_request;
                new_object.reponse = tms_download_reponse;
            }
            else
            {
                new_object.request = tms_upload_chunked_request;
                new_object.request = tms_upload_chunked_reponse;
            }
            ret = tms_common_schedule_routine(&new_object);

            STD_PRINT("add new schedule with %d.\r\n", ret);
        }
        if (0 != ret)
        {
            if (transmit->file_operate.object.full_path)
            {
                STD_FREE(transmit->file_operate.object.full_path);
            }
            if (transmit)
            {
                STD_FREE(transmit);
            }
        }
    }

    return ret;
}

/********************************************************************************
 *
 *
 *      普通文件下载
 *
 */
static s32 general_stream_data_transmit(PTR para, u8 *buff, u32 size, s32 code, s32 status)
{
    s32 ret = -1;
    upgrade_feedback_t *feedback;
    s32 stream_step;

    STD_PRINT("stream: 0x%x, 0x%x, %d, %d, %d\r\n", para, buff, size, code, status);
    feedback = (upgrade_feedback_t *)para;
    if (feedback && feedback->stream_func)
    {
        tms_set_download_status(0xA56B5AB6);
        stream_step = ((0 != code) ? DOWNLOADERR : ((0 == status) ? DOWNLOADING : DOWNLOADED));
        feedback->stream_func(buff, size, stream_step, &feedback->para);
    }
    else
    {
        tms_set_download_status(0x0);
         STD_PRINT("illegal general stream function!\r\n");
    }

    return ret;
}

static s32 general_stream_finish_notify(PTR para, s32 result, s32 status)
{
    s32 ret = -1;
    upgrade_feedback_t *feedback;

    STD_PRINT("notify: 0x%x, %d, %d\r\n", para, result, status);

    tms_set_download_status(0);

    feedback = (upgrade_feedback_t *)para;
    if (feedback && feedback->notify_func)
    {
        feedback->notify_func(&feedback->para, US_DOWNLOAD_RESULT , (0 == result ? 0 : -1));
    }
    else
    {
        STD_PRINT("illegal general notify function!\r\n");
    }

    if (feedback)
    {

    }

    return ret;
}

static s32 general_stream_memory_free(PTR para)
{
    s32 ret = 0;
    tms_transmit_t *transmit;

    tms_set_download_status(0);

    transmit = (tms_transmit_t *)para;
    if (transmit)
    {
        if (transmit->notify_operate.object.para)
        {
            STD_FREE((void *)(transmit->notify_operate.object.para));
        }
        if (transmit->notify_operate.object.reserved)
        {
            STD_FREE((void *)(transmit->notify_operate.object.reserved));
        }
    }

    return ret;
}

s32 trendit_http_schedule_add(upgrade_info_t *upgrade_info, PTR reserved_para)
{
    s32 ret = 0;
    transmit_node new_node;
    upgrade_feedback_t *feedback;

    tms_set_download_status(0);

    feedback = STD_MALLOC(sizeof(upgrade_feedback_t));
    if (!upgrade_info || !upgrade_info->m_download_url || !feedback
        || (!upgrade_info->self_save_cb && !upgrade_info->m_store_filepath)
        || (upgrade_info->self_save_cb && upgrade_info->m_store_filepath))
    {
        ret = -1;
        STD_PRINT("\t.illegal para, \r\n");
    }
    else
    {
        STD_PRINT("\t.http schedule start ... \r\n");
        memset(feedback, 0, sizeof(upgrade_feedback_t));
        switch (upgrade_info->m_download_file_type)
        {
        case FT_BIN_FILE:
        case FT_PHOTO:
        case FT_AUDIO:
        case FT_TEXT:
            feedback->para.m_backend_cmd = upgrade_info->m_upgrade_callback_info->m_backend_cmd;
            feedback->para.m_packno = upgrade_info->m_upgrade_callback_info->m_packno;
            feedback->para.m_lparam = upgrade_info->m_upgrade_callback_info->m_lparam;
            feedback->stream_func   = upgrade_info->self_save_cb;
            feedback->notify_func   = upgrade_info->upgrade_result_callback;
            //
            memset(&new_node, 0, sizeof(transmit_node));
            new_node.transmit_type = 0;
            new_node.file_type     = upgrade_info->m_download_file_type;
            new_node.full_url      = upgrade_info->m_download_url;
            new_node.full_path     = upgrade_info->m_store_filepath;
            if (reserved_para)
            {
                //new_node.being_post = obj_post->raw_data;
                //new_node.token      = obj_post->token;
            }
            new_node.para1         = (PTR)feedback;
            new_node.stream        = general_stream_data_transmit;
            new_node.notify        = general_stream_finish_notify;
            new_node.free          = general_stream_memory_free;
            ret = tms_transmit_message_post(&new_node);
            break;
        default:
            STD_PRINT("\t.illegal para, unknow requet type.\r\n");
            break;
        }
    }

    if (0 != ret)
    {
        if (feedback)
        {
            STD_FREE(feedback);
        }
    }

    return ret;
}

/****************************************************************************************
 *
 *
 *       模拟其他模块下载
 *
 */
static s32 pciture_stream_received(PTR para, u8 *buff, u32 size, s32 code, s32 status)
{
    STD_PRINT("stream: 0x%x, 0x%x, %d, %d, %d\r\n", para, buff, size, code, status);
    return 0;
}

static s32 picture_stream_finish_notify(PTR para, s32 code, s32 status)
{
    STD_PRINT("notify: 0x%x, %d, %d\r\n", para, code, status);
    return 0;
}

static s32 url_stream_transmit_func(u8 *data, u32 data_len, DOWNLOAD_STEP_E download_step, upgrade_callback_info_t *info)
{
    STD_PRINT("data: 0x%x, len: %d, step: %d.\r\n", data, data_len, download_step);
    if (!info)
    {
         STD_PRINT("illeage para.\r\n");
    }
    else
    {
        STD_PRINT("cmd: 0x%x, param: %d, pack_no: %d.\r\n", info->m_backend_cmd, info->m_lparam, info->m_packno);
    }
    return 0;
}

static void url_stream_finished_notify(upgrade_callback_info_t *info, UPGRADE_STEP_E upgrade_step, s32 result)
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

void tms_download_picture_routine(void)
{
    s32 ret;
    upgrade_info_t upgrade_info;
    upgrade_callback_info_t cb_upgrade_info;

    cb_upgrade_info.m_backend_cmd = 88;
    cb_upgrade_info.m_packno      = 99;
    cb_upgrade_info.m_lparam      = 0;
    //
    upgrade_info.m_download_file_type    = FT_AUDIO;
    upgrade_info.m_store_filepath        = NULL;
    upgrade_info.self_save_cb            = url_stream_transmit_func;
    upgrade_info.m_download_url          = TMS_DOWNLOAD_MEDIA_WAV;
    upgrade_info.m_upgrade_callback_info = &cb_upgrade_info;
    upgrade_info.upgrade_result_callback = url_stream_finished_notify;
    strcpy(upgrade_info.m_default, "smtgmemte");
    //
    ret = trendit_http_schedule_add(&upgrade_info, 0);

    STD_PRINT("add schedule to download picture with %d.\r\n", ret);
}

s32 tms_get_download_status(void)
{
    return (0xA56B5AB6 == g_other_downloading);
}

s32 tms_set_download_status(u32 status)
{
    g_other_downloading = status;
}


