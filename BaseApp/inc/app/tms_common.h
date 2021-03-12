#ifndef __TMS_COMMOM__
#define __TMS_COMMOM__

#include "tms_schedule.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef unsigned int PTR;

/********************************************************************************
 *
 *
 *  资源调度
 *
 */
typedef enum _tms_common_status {
    E_TMS_COMMON_INIT = 0,
    E_TMS_COMMON_EXEC,
    E_TMS_COMMON_EXIT,
} tms_common_status_e;

//
typedef struct _schedule_object {
    u8   *url;
    u8    transmit_type;
    PTR   notify_object;
    s32 (*request)(PTR para, PTR reserved, u8 *buff, u32 size, s32 status);
    s32 (*reponse)(PTR para, PTR reserved, u8 *data, u32 len, s32 code, s32 status);
} schedule_object_t;
//
typedef struct _url_object {
    s32 transmit_type;  // 上传或下载
    u8 *dest_host;
    u8 *dest_page;
    u16 dest_port;
    u16 is_https;
} url_object_t;

typedef struct _socket_object {
    url_object_t obj_url;
    s32 initialized;    // 是否初始化内存
    s32 socket_status;
    s32 socket_id;
    s32 network_type;
    u32 create_time;   // 开始创建, 尝试创建超时检测
    u32 query_time;
    //
    u8 *buff_send;
    u8 *buff_recv;
    u32 max_send;
    u32 max_recv;
    u32 received_lens;  // 当前buff接收数据长度
    u32 received_total; // 接收当前响应总长度
} socket_object_t;

typedef struct _socket_instance {
    s32 (*socket_create)(u8 type, u8 mode, u8 *param, u16 port);
    s32 (*socket_send)(s32 socket_id, u8 *wbuf, s32 len);
    s32 (*socket_recv)(s32 socket_id, u8 *rbuff, s32 len);
    s32 (*socket_close)(s32 socket_id);
    s32 (*socket_get_status)(s32 socket_id);
} socket_instance_t;

typedef struct _socket_operate {
    socket_object_t   object;
    socket_instance_t handle;
} socket_operate_t;

/********************************************************************************
 *
 *
 *  资源调度
 *
 */
// stream control
typedef struct _chunck_object {
    u32 first_byte_pos;
    u32 last_byte_pos;
    u32 total_bytes;
    u32 partial_size;
} chunck_object_t;

typedef struct _packet_object {
    u32 total_length;
    u32 header_length;
    u32 content_length;
    u32 received_length;
    u32 notified_length;
    u32 body_length;
    u32 chunck_offset;
    u16 sent_request;
    u16 send_retry;
    u16 recv_retry;
} packet_object_t;

typedef struct _post_object {
    u8 *token;
    u8 *raw_data;
} post_object_t;

typedef struct _stream_object {
    u8              chuncked;
    u16             packet_count;
    s32             status_code;
    post_object_t   post;
    chunck_object_t chunck;
    packet_object_t packet;
} stream_object_t;

typedef struct _stream_instance {
    s32 (*request)(PTR para, PTR reserved, u8 *buff, u32 size, s32 status);
    s32 (*response)(PTR para, PTR reserved, u8 *data, u32 len, s32 code, s32 status);
} stream_instance_t;

typedef struct _stream_operate {
    stream_object_t   object;
    stream_instance_t handle;
} stream_operate_t;

//
typedef struct _common_schedule {
    s32              phase;
    s32              schedule_id;
    PTR              notify_object;
    socket_operate_t socket_operate;
    stream_operate_t stream_operate;
} common_schedule_t;

/********************************************************************************
 *
 *
 *  资源调度
 *
 */

typedef struct _request_object {
    packet_object_t *packet;    // 没有rawdata则文件操作上传
    url_object_t    *url;
    post_object_t   *post;
} request_object_t;

extern s32 tms_common_schedule_routine(schedule_object_t *new_object);
extern s32 tms_common_url_parse(s8 *url, u8 **domain, u8 **page, u16 *port, u16 *is_https);


#ifdef __cplusplus
}
#endif

#endif __TMS_COMMOM__

