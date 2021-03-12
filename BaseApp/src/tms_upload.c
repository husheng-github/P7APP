#include "tms_upload.h"

#define TMS_UPLOAD_LOG_SIMPLE       "/mtd2/upload_simple"
#define TMS_LOG_DOMAIN              "device-info-log.s3-cn-south-1.qiniucs.com "
//
typedef enum {
    E_PHASE_CHUNKED_UPLOAD_TOKEN = 0,
    E_PHASE_CHUNKED_UPLOAD_MKBLK,
    E_PHASE_CHUNKED_UPLOAD_PUTBLK,
    E_PHASE_CHUNKED_UPLOAD_MKFILE,
} CHUNKED_UPLOAD_PHASE_E;

// Single Linked List
typedef struct _slist slist_t;
struct _slist {
    slist_t *next;
    PTR      data;
};
// Doubly Linked List
typedef struct _dlist dlist_t;
struct _dlist {
    dlist_t *prev;
    dlist_t *next;
    PTR      data;
};

static s32 chunked_upload_request(PTR para, PTR reserved, u8 *buff, u32 size, s32 status)
{
    s32 ret = -1;
    u32 filesize, ctx_list_size;
    s8 *encode_key, *encoded_fname, *encoded_mime_type, *encoded_user_vars;
    s8 *uphost;
    switch (reserved)
    {
#if 0
    case E_PHASE_CHUNKED_UPLOAD_MKBLK:
        break;
    case E_PHASE_CHUNKED_UPLOAD_PUTBLK:
        ret = sprintf(buff,
                      "POST /bput/%s/%d HTTP/1.1\r\n"\
                      "Host: %s\r\n"\
                      "Content-Type:   application/octet-stream\r\n"\
                      "Content-Length: %d\r\n"\
                      "Authorization:  UpToken %s\r\n"\
                      "%s",
                      chunk_ctk, chunk_offset, upload_host, chunk_size, upload_token, chunk_data);
        break;
    case E_PHASE_CHUNKED_UPLOAD_MKFILE:
        ret = sprintf(buff,
                     "POST %s HTTP/1.1\r\n" \
                     "Host: %s\r\n" \
                     "Content-Type: text/plain\r\n"\
                     "Content-Length: %d\r\n" \
                     "Authorization: %d\r\n"\
                     "%s",
                     page, host, ctx_list_size, upload_token, ctx_list_data);
        break

#endif
    }

    return ret;
}



typedef struct {
    slist_t *ctxs;

};

static int tms_upload_response_received(PTR data)
{
    if (data)
    {

    }
    STD_PRINT("upload response received...");
}
/********************************************************************************
 *
 *
 *
 *
 */
/****************************************************************************************
 *
 *
 *       创建上传块
 *
 */
static s32 upload_mkfile_stream_received(PTR para, u8 *buff, u32 size, s32 code, s32 status)
{
    STD_PRINT("stream: 0x%x, 0x%x, %d, %d, %d\r\n", para, buff, size, code, status);
    return 0;
}

static s32 upload_mkfile_stream_finish_notify(PTR para, s32 code, s32 status)
{
    STD_PRINT("notify: 0x%x, %d, %d\r\n", para, code, status);
    return 0;
}

void tms_upload_mkfile_routine(void)
{


}

/********************************************************************************
 *
 *
 *      获取请求token
 *
 */



/********************************************************************************
 *
 *
 *
 *
 */

static void tms_upload_file_create(void)
{
    int  i, fd, ret = 0;
    int  offset = 0;
    char buff[0x100];

    fd = ddi_vfs_open(TMS_UPLOAD_LOG_SIMPLE, "w+");
    if (0 < fd)
    {
        for (i = 0; i < 16; i++)
        {
            ret = ddi_vfs_seek(fd, offset, VFS_SEEK_SET);
            STD_PRINT("offset %d bytes in file with %d.\r\n", offset, ret);
            ret = ddi_vfs_write(fd, buff, 0x100);
            STD_PRINT("write %d bytes to file with %d.\r\n", 0x100, ret);
            offset += 0x100;
        }
        ddi_vfs_close(fd);
    }

}

/********************************************************************************
 *
 *
 *      上传接口
 *
 */
s32 tms_upload_chunked_request(PTR para, PTR reserved, u8 *buff, u32 size, s32 status)
{
    s32 ret;
    s32 first_byte_pos, last_byte_pos, remain_byte;
    request_object_t *obj_request;
    s8 *upload_token;

    if (!obj_request)
    {
        ret = -2;
        STD_PRINT("illegal request handle...\r\n");
    }
    else
    {
        if (obj_request->post->raw_data)
        {
            ret = sprintf(buff,
                         "POST %s HTTP/1.1\r\n" \
                         "Host: %s\r\n" \
                         "Content-Type: text/plain\r\n"\
                         "Content-Length: %d\r\n" \
                         "Authorization: %d\r\n"\
                         "%s",
                         obj_request->url->dest_page,
                         obj_request->url->dest_host,
                         strlen(obj_request->post->raw_data),
                         upload_token,
                         obj_request->post->raw_data);
        }
        else
        {

        }

        if (0 >= obj_request->packet->chunck_offset)
        {
            first_byte_pos = 0;
            last_byte_pos  = 1024 * 4 - 1;
        }
        else
        {

        }
    }

    return ret;
}

s32 tms_upload_chunked_reponse(PTR para, PTR reserved, u8 *data, u32 len, s32 code, s32 status)
{
    s32 ret;
    tms_transmit_t *transmit;

    STD_PRINT("download %d phrase received %d to 0x%X with %d.\r\n", status, len, data, code);
    STD_PRINT("para: 0x%X, reserved: 0x%X.\r\n", para, reserved);

    transmit = (tms_transmit_t *)para;
}


//
/********************************************************************************
 *
 *
 *      上传接口
 *
 */
static s32 tms_upload_message_post(u8 *filename)
{
    s32 ret = -1;

    return ret;
}

static s32 tms_upload_request(PTR para, PTR reserved, u8 *buff, u32 size, s32 status)
{

}

static s32 tms_upload_reponse(PTR para, PTR reserved, u8 *data, u32 len, s32 code, s32 status)
{


}


void tms_upload_file_routine(void)
{
    //tms_upload_file_create();
    //tms_upload_message_post();
    //tms_schedule_instance()->schedule_node_insert(tms_upload_response_received, 0);
    tms_download_picture_routine();
    //tms_upgrade_process_routine();
}

