#include "tms_common.h"


// upload
#define TMS_UPLOAD_REQUEST_FULL_MAX             (1024 * 0x12)
#define TMS_UPLOAD_REQUEST_BODY_MAX             (1024 * 0x10)
#define TMS_UPLOAD_RESPONSE_FULL_MAX            (1024 * 0x05)
#define TMS_UPLOAD_RESPONSE_BODY_MAX            (1024 * 0x03)
// download
#define TMS_DOWNLOAD_REQUEST_FULL_MAX           (1024 * 0x05)
#define TMS_DOWNLOAD_REQUEST_BODY_MAX           (1024 * 0x03)
#define TMS_DOWNLOAD_RESPONSE_FULL_MAX          (1024 * 0x12)
#define TMS_DOWNLOAD_RESPONSE_BODY_MAX          (1024 * 0x03)
//
#define TMS_CONNECTION_CHECK_INTERVAL           (1024)
//
typedef enum {
    TMS_RESPONSE_NOTIFY_NONE = 0,
    TMS_RESPONSE_NOTIFY_PARTIAL,
    TMS_RESPONSE_NOTIFY_FULL,
    TMS_RESPONSE_NOTIFY_ONE,
    TMS_RESPONSE_NOTIFY_ALL,
    TMS_RESPONSE_NOTIFY_ABORT,
} tms_response_status_e;
//
//
static s32 tms_common_wifi_connected_check(void)
{
    wifi_module_t *wifi_module;
    tcp_protocol_module_t *tcp_protocol;

    wifi_module  = wifi_instance();
    tcp_protocol = tcp_protocol_instance();
    if (wifi_module && tcp_protocol)
    {
	#if 0
        TMS_INFO("network status %d, tcp status %d\r\n", wifi_module->check_network_connect_success(),
                                                         tcp_protocol->check_tcp_connect_success());
	#endif
        return (wifi_module->check_network_connect_success() && tcp_protocol->check_tcp_connect_success());
    }
    else
    {
        return 0;
    }
}

static s32 tms_common_connection_create(socket_operate_t *operate)
{
    operate->object.network_type = network_strategy_instance()->m_current_type; // wifi or gprs
    STD_PRINT("%s socket will being created...\r\n", (COMMU_WIFI == operate->object.network_type) ? "WiFi" : "2G");
    switch (operate->object.network_type)
    {
    case COMMU_WIRELESS:
        operate->object.socket_id = ddi_gprs_socket_create(0, 0, operate->object.obj_url.dest_host,
                                                                 operate->object.obj_url.dest_port);
        operate->handle.socket_send       = ddi_gprs_socket_send;
        operate->handle.socket_recv       = ddi_gprs_socket_recv;
        operate->handle.socket_close      = ddi_gprs_socket_close;
        operate->handle.socket_get_status = ddi_gprs_socket_get_status;
        //
        operate->handle.socket_create = tms_common_connection_create;
        break;
    case COMMU_WIFI:
        operate->object.socket_id = ddi_wifi_socket_create_start(0, 0, operate->object.obj_url.dest_host,
                                                                       operate->object.obj_url.dest_port);
        operate->handle.socket_send       = ddi_wifi_socket_send;
        operate->handle.socket_recv       = ddi_wifi_socket_recv;
        operate->handle.socket_close      = ddi_wifi_socket_close;
        operate->handle.socket_get_status = ddi_wifi_socket_get_status;
        //
        operate->handle.socket_create = tms_common_connection_create;
        break;
    default:
        operate->object.socket_id = -0x5A;
        STD_PRINT("unknow network type %d, please check...", operate->object.network_type);
        break;
    }
    //
    STD_PRINT("%s create connection with %d.\r\n", (COMMU_WIFI == operate->object.network_type) ? "WiFi" : "2G",
                                                    operate->object.socket_id);

    return operate->object.socket_id;
}

static s32 tms_common_create_timeout_check(common_schedule_t *common_data)
{
    if (0 < common_data->socket_operate.object.socket_id)
    {
        common_data->socket_operate.object.query_time    = trendit_get_ticks();
        common_data->socket_operate.object.create_time   = trendit_get_ticks();
        common_data->socket_operate.object.socket_status = SOCKET_CONNECTING;
    }
    else
    {
        common_data->phase = E_TMS_COMMON_EXIT;
        STD_PRINT("\t.create socket failed, current schedule exit!!!\r\n");
    }

    return 0;
}

static s32 tms_common_connection_waitting(socket_operate_t *operate)
{
    if (1 == trendit_query_timer(operate->object.query_time, TMS_CONNECTION_CHECK_INTERVAL))
    {
        operate->object.socket_status = operate->handle.socket_get_status(operate->object.socket_id);
        operate->object.query_time    = trendit_get_ticks();
        STD_PRINT("socket %d query end with %d.", operate->object.socket_id,
                                                  operate->object.socket_status);
    }

    return operate->object.socket_status;
}

static s32 tms_connecting_timeout_predefine(socket_object_t *object)
{
    s32 ret = 0;
    u32 conn_try_overtime = 10000;
    network_strategy_t *network_strategy;

    network_strategy = network_strategy_instance();
    if (network_strategy)
    {
        if (COMMU_WIRELESS == object->network_type)
        {
            conn_try_overtime = network_strategy->m_wireless_tcp_con_try_overtime;
        }
        else if (COMMU_WIFI == object->network_type)
        {
            conn_try_overtime = network_strategy->m_wifi_tcp_con_try_overtime;
        }
        else
        {
            // default
        }
    }
    ret = trendit_query_timer(object->create_time, conn_try_overtime);

    return ret;
}

static s32 tms_common_resource_init(common_schedule_t *common_data)
{
    s32 ret = 0;

    if (0xA5 == common_data->socket_operate.object.initialized)
    {
        STD_PRINT("tms memory pool already initialized, continue...\r\n");
    }
    else
    {
        if (1 != common_data->socket_operate.object.obj_url.transmit_type)
        {
            common_data->socket_operate.object.max_send  = TMS_DOWNLOAD_REQUEST_FULL_MAX;
            common_data->socket_operate.object.max_recv  = TMS_DOWNLOAD_RESPONSE_FULL_MAX;
        }
        else
        {
            common_data->socket_operate.object.max_send  = TMS_UPLOAD_REQUEST_FULL_MAX;
            common_data->socket_operate.object.max_recv  = TMS_UPLOAD_RESPONSE_FULL_MAX;
        }
        common_data->socket_operate.object.buff_send = STD_MALLOC(common_data->socket_operate.object.max_send + 1);
        common_data->socket_operate.object.buff_recv = STD_MALLOC(common_data->socket_operate.object.max_recv + 1);

        if (common_data->socket_operate.object.buff_send && common_data->socket_operate.object.buff_recv)
        {
            common_data->socket_operate.object.initialized = 0xA5;
            memset(common_data->socket_operate.object.buff_send, 0, common_data->socket_operate.object.max_send + 1);
            memset(common_data->socket_operate.object.buff_recv, 0, common_data->socket_operate.object.max_recv + 1);
        }
        else
        {
            if (common_data->socket_operate.object.buff_send)
            {
                STD_FREE(common_data->socket_operate.object.buff_send);
            }
            if (common_data->socket_operate.object.buff_recv)
            {
                STD_FREE(common_data->socket_operate.object.buff_recv);
            }

            STD_PRINT("init tms memory pool failed, please check...\r\n");

            ret = -1;
        }
    }

    return ret;
}

static s32 tms_common_connect_timeout_check(common_schedule_t *common_data)
{
		s32 ret;
    if (SOCKET_CONNECTED == common_data->socket_operate.object.socket_status)
    {
        ret = tms_common_resource_init(common_data);
        if (0 != ret)
        {
            common_data->phase = E_TMS_COMMON_EXIT;
            STD_PRINT("\t.init schedule resource failed, exit...\r\n");
        }
        else
        {
            common_data->phase = E_TMS_COMMON_EXEC;
        }
    }
    else
    {
        if (1 == tms_connecting_timeout_predefine(&common_data->socket_operate.object))
        {
            common_data->phase = E_TMS_COMMON_EXIT;
            STD_PRINT("\t.connect socket timeout, current schedule exit!!!\r\n");
        }
    }

    return 0;
}

/********************************************************************************
 *
 *
 *  资源调度
 *
 */
static s32 tms_common_schedule_init(common_schedule_t *common_data)
{
    s32 ret;

    switch (common_data->socket_operate.object.socket_status)
    {
    case SOCKET_CLOSE:
        ret = tms_common_connection_create(&common_data->socket_operate);
        tms_common_create_timeout_check(common_data);
        break;

    case SOCKET_CONNECTING:
        ret = tms_common_connection_waitting(&common_data->socket_operate);
        tms_common_connect_timeout_check(common_data);
        break;

    case SOCKET_CONNECTED:
        common_data->phase = E_TMS_COMMON_EXEC;
        STD_PRINT("%d already connected, continue...", common_data->socket_operate.object.socket_id);
        break;
    default:
        break;
    }
    return 0;
}

 /********************************************************************************
 *
 *  范围: 发送请求，接收响应，执行回调
 *  功能: 接收
 *
 */
static s32 tms_common_request_sent_check(stream_object_t *obj)
{
    s32 ret;

    if (0xA5 != obj->packet.sent_request)
    {
        ret = -1;
        STD_PRINT("request haven't been sent, sending ...\r\n");
    }
    else
    {
        ret = 0;
        STD_PRINT("request already sent, continue receiving...\r\n");
    }

    return ret;
}

static s32 tms_common_request_send_timeout(stream_object_t *obj)
{
    s32 ret = 0;

    if (0x8 < obj->packet.send_retry)
    {
        ret = -1;
    }

    return ret;
}

static s32 tms_common_request_send_exec(common_schedule_t *common_data)
{
    s32 ret = -1, request_result;
    socket_operate_t *opt_socket;
    stream_operate_t *opt_stream;
    request_object_t  obj_request;

    opt_socket = &common_data->socket_operate;
    opt_stream = &common_data->stream_operate;
    if (!opt_stream->handle.request)
    {
        STD_PRINT("illegal response handle be found.\r\n");
    }
    else
    {
        obj_request.packet = &common_data->stream_operate.object.packet;
        obj_request.url    = &common_data->socket_operate.object.obj_url;
        obj_request.post   = &common_data->stream_operate.object.post;
        memset(opt_socket->object.buff_send, 0, opt_socket->object.max_send + 1);
        request_result = opt_stream->handle.request(common_data->notify_object,
                                                    (PTR)&obj_request,
                                                    opt_socket->object.buff_send,
                                                    opt_socket->object.max_send,
                                                    0);
        if (0 < request_result)
        {
            ret = opt_socket->handle.socket_send(opt_socket->object.socket_id, opt_socket->object.buff_send, request_result);
            STD_PRINT("socket send %d byte end with %d.\r\n", request_result, ret);
            if (ret > 0)
            {
                ret = 0;
            }
        }
        else
        {
            STD_PRINT("reqeust raw data failed with %d.\r\n", request_result);
        }
    }

    return ret;
}

/********************************************************************************
 *
 *  范围: 处理数据接收
 *  功能: 接收
 *
 */
static u32 tms_common_str2num(s8 *raw_data, s32 raw_len, u32 *num)
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

static s32 tms_common_response_chunck_parse(u8 *raw_data, s32 raw_len, stream_object_t *obj)
{
    s32 bytes = 0;
    s8 *p, *q, *k;
    s32 block_start = 0, block_end = 0, file_total = 0;
    u32 offset;

    ;
    if (NULL != (p = strstr(raw_data, "Partial Content"))
        && NULL != (q = strstr(p, "Content-Range:")))
    {

        obj->chuncked = 1;

        while ('0' > *q || '9' < *q) q++;

        if ((k = strstr(q, "-")))
        {
            offset = tms_common_str2num(q, (k - q), &block_start);
            if ('-' != *(q + offset))
            {
                block_start = -2;
            }
            else
            {
                q += offset;
                q++;
            }
        }
        //
        if (0 <= block_start && (k = strstr(q, "/")))
        {
            offset = tms_common_str2num(q, (k - q), &block_end);
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
            offset = tms_common_str2num(q, (k - q), &file_total);
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
        obj->chunck.first_byte_pos = block_start;
        obj->chunck.last_byte_pos  = block_end;
        obj->chunck.total_bytes    = file_total;
        if (0 <= block_start && block_start <= block_end && block_end < file_total)
        {
            obj->chunck.partial_size = block_end - block_start + 1;
            bytes = obj->chunck.partial_size;
        }
    }

    STD_PRINT("chunck_first: %d, chunck_last: %d, chunck_total: %d, partial_size: %d.\r\n",
               obj->chunck.first_byte_pos, obj->chunck.last_byte_pos, obj->chunck.total_bytes, obj->chunck.partial_size);

    return bytes;
}

static s32 tms_common_response_general_parse(u8 *raw_data, s32 raw_len, packet_object_t *obj)
{
    s8 *p = NULL, *raw_end;
    s32 bytes = 0;

    p = strstr(raw_data, "Content-Length");
    if (NULL != p)
    {
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
            obj->content_length = bytes;
            STD_PRINT("header length: %d, content length: %d.\r\n", obj->header_length, bytes);
        }
    }

    return bytes;
}

static s32 tms_common_response_header_parse(u8 *data, s32 offset_prev, s32 offset_next, stream_object_t *obj)
{
    u32 i, header_boundary = 0;
    s8 *p;
    s32 len = offset_next - offset_prev;

    if (p = strstr((s8 *)data, "\r\n\r\n"))
    {
        obj->packet.header_length = (s8 *)p - (s8 *)data + 4;
        header_boundary = 1;
    }

    if (1 == header_boundary)
    {
        STD_PRINT("header boundary found, header parse next...\r\n");
        tms_common_response_chunck_parse(data, offset_next, obj);
        tms_common_response_general_parse(data, offset_next, &obj->packet);
        if (1 == obj->chuncked)
        {
            obj->packet.total_length = obj->chunck.total_bytes;
        }
        else
        {
            obj->packet.total_length = obj->packet.content_length;
        }
    }

    return obj->packet.content_length;
}

/********************************************************************************
 *
 *  范围: 处理数据接收
 *  功能: 接收
 *
 */
static s32 tms_common_response_simple_packet_finish(stream_object_t *obj)
{
    s32 ret = -1;

    if (0 < obj->packet.content_length && 0 < obj->packet.header_length
        && (obj->packet.content_length + obj->packet.header_length <= obj->packet.received_length))
    {
        ret = 0;
    }

    return ret;
}

static s32 tms_common_response_last_packet_finish(stream_object_t *obj)
{
    s32 ret = -1;

    if (0 == obj->chuncked)
    {
        ret = 0;
    }
    else
    {
        if (obj->chunck.total_bytes <= obj->chunck.last_byte_pos + 1)
        {
            ret = 0;
        }
    }
    STD_PRINT("current %d packet %s the last one.\r\n", obj->packet_count, (0 == ret) ? "is" : "not");

    return ret;
}

static void tms_common_response_header_discard(s8 *raw_data, u32 *raw_len, u32 head_len)
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
            lens   = *raw_len;
            if (0 < offset)
            {
                memcpy(raw_data, p, lens);
                memset(&raw_data[lens], 0, offset);
                if (head_len != offset)
                {
                     STD_PRINT("\t.header length not match, head: %d, body: %d, offset: %d.\r\n",
                               head_len, lens, offset);
                }
            }
        }
    }
    STD_PRINT("%sfound packet header, offset: %d, after: %d, before: %d.\r\n",
               (-1 == offset) ? "not " : "", offset, lens, (raw_len)?(*raw_len):0);
}


static s32 tms_common_response_parse(common_schedule_t *common_data)
{
    s32 ret = 0;
    s32 response_status, need_notify = 1;
    stream_object_t *obj_stream;

    obj_stream = &common_data->stream_operate.object;
    obj_stream->packet.body_length = obj_stream->packet.received_length\
                                   - obj_stream->packet.notified_length\
                                   - obj_stream->packet.header_length;
    //
    tms_common_response_header_discard(common_data->socket_operate.object.buff_recv,
                                       &obj_stream->packet.body_length, obj_stream->packet.header_length);
    if (0 < obj_stream->packet.body_length)
    {
        need_notify = 1;
        obj_stream->packet.notified_length += obj_stream->packet.body_length;
        //
        if (0 != tms_common_response_simple_packet_finish(obj_stream))
        {
            response_status = (0 == obj_stream->status_code) ? TMS_RESPONSE_NOTIFY_PARTIAL
                                                             : TMS_RESPONSE_NOTIFY_ABORT;
        }
        else
        {
            if (0 != tms_common_response_last_packet_finish(obj_stream)
                && 0 == common_data->stream_operate.object.status_code)
            {
                response_status = TMS_RESPONSE_NOTIFY_ONE;
            }
            else
            {
                response_status = (0 == obj_stream->status_code) ? TMS_RESPONSE_NOTIFY_ALL
                                                                 : TMS_RESPONSE_NOTIFY_ABORT;
            }
        }
    }
    else
    {
        response_status = (0 == obj_stream->status_code) ? TMS_RESPONSE_NOTIFY_NONE
                                                         : TMS_RESPONSE_NOTIFY_ABORT;
        if (0x80 < obj_stream->packet.recv_retry++)
        {
            obj_stream->status_code |= -0x202;
        }
    }
    //
    if (0 != obj_stream->status_code)
    {
        need_notify     = 2;
        response_status = TMS_RESPONSE_NOTIFY_ABORT;
    }

    //
    if (0 < need_notify)
    {
        ret = common_data->stream_operate.handle.response(common_data->notify_object,
                                                          0,
                                                          common_data->socket_operate.object.buff_recv,
                                                          obj_stream->packet.body_length,
                                                          obj_stream->status_code,
                                                          (TMS_RESPONSE_NOTIFY_ONE < response_status ? 1 : 0));
    }
    STD_PRINT("response_status: %d, chunck_last: %d.\r\n", response_status, obj_stream->chunck.last_byte_pos);
    //
    switch (response_status)
    {
    case TMS_RESPONSE_NOTIFY_NONE:
        STD_PRINT("notify none, packed_recv: %d, buffer_recv: %d\r\n",
                   obj_stream->packet.received_length, common_data->socket_operate.object.received_lens);
        break;
    case TMS_RESPONSE_NOTIFY_PARTIAL:
        common_data->socket_operate.object.received_lens = 0;
        memset(common_data->socket_operate.object.buff_recv, 0, common_data->socket_operate.object.max_recv + 1);
        break;
    case TMS_RESPONSE_NOTIFY_ONE:
        //
        obj_stream->packet_count++;
        obj_stream->packet.chunck_offset   = obj_stream->chunck.last_byte_pos;
        //
        obj_stream->packet.header_length   = 0;
        obj_stream->packet.content_length  = 0;
        //
        obj_stream->packet.sent_request    = 0;
        //
        obj_stream->packet.received_length = 0;
        obj_stream->packet.notified_length = 0;
        //
        obj_stream->chunck.first_byte_pos  = 0;
        obj_stream->chunck.last_byte_pos   = 0;
        obj_stream->chunck.partial_size    = 0;
        //
        obj_stream->packet.send_retry      = 0;
        obj_stream->packet.recv_retry      = 0;
        //
        common_data->socket_operate.object.received_lens = 0;
        //
        memset(common_data->socket_operate.object.buff_recv, 0, common_data->socket_operate.object.max_recv + 1);
        break;
    case TMS_RESPONSE_NOTIFY_ALL:
        common_data->phase = E_TMS_COMMON_EXIT;
        break;
    case TMS_RESPONSE_NOTIFY_ABORT:
        common_data->phase = E_TMS_COMMON_EXIT;
        break;
    default:
        STD_PRINT("unknow step be found...\r\n");
        break;
    }

    return ret;
}

static s32 tms_common_response_recv_exec(common_schedule_t *common_data)
{
    s32 ret;
    u32 ticks_start;
    s32 last_offset = 0, recv_none = 0;
    socket_operate_t *opt_socket;
    stream_object_t  *obj_stream;


    STD_PRINT("\r\n******************************** receiving start ********************************\r\n");

    opt_socket  = &common_data->socket_operate;
    obj_stream  = &common_data->stream_operate.object;
    ticks_start = trendit_get_ticks();
    while (1)
    {
        ret = opt_socket->handle.socket_recv(opt_socket->object.socket_id,
                                            &opt_socket->object.buff_recv[opt_socket->object.received_lens],
                                             opt_socket->object.max_recv - opt_socket->object.received_lens);
        if (0 < ret)
        {
            last_offset = opt_socket->object.received_lens;
            opt_socket->object.received_lens   += ret;
            obj_stream->packet.received_length += ret;
        #if 1
            STD_PRINT("ret: %d, offset: %d, received: %d, head: %d, body: %d.\r\n",
                       ret, last_offset, opt_socket->object.received_lens,
                       obj_stream->packet.header_length, obj_stream->packet.content_length);
        #endif
        }

        if (0 >= obj_stream->packet.content_length || 0 >= obj_stream->packet.header_length)
        {
            tms_common_response_header_parse(&opt_socket->object.buff_recv[0], last_offset,
                                             opt_socket->object.received_lens, obj_stream);
        }

        if ((0 < obj_stream->packet.content_length && 0 < obj_stream->packet.header_length)
            && (obj_stream->packet.content_length + obj_stream->packet.header_length <=
            obj_stream->packet.received_length))
        {
            STD_PRINT("header-length: %d, content-length: %d, received-length: %d, received finished, goto next step.\r\n",
            obj_stream->packet.header_length, obj_stream->packet.content_length, opt_socket->object.received_lens);
            //STD_PRINT("\r\n%s\r\n", &opt_socket->object.buff_recv[0]);
            break;
        }
        //
        if (opt_socket->object.max_recv <= opt_socket->object.received_lens)
        {
            STD_PRINT("recv buff is full, next poll will continue.\r\n");
            break;
        }
        if (0 != trendit_query_timer(ticks_start, opt_socket->object.max_recv * 12))
        {
            STD_PRINT("recv response timeout, exit.\r\n");
            break;
        }
        if (0 > ret)
        {
            STD_PRINT("recv data from socket with %d.\r\n", ret);
            break;
        }
    }
    STD_PRINT("\r\n******************************** receiving end ********************************\r\n");

    return ret;
}

static s32 tms_common_schedule_exec(common_schedule_t *common_data)
{
    s32 ret = 0;

    if (0 != tms_common_request_sent_check(&common_data->stream_operate.object))
    {
        ret = tms_common_request_send_exec(common_data);
        if (0 != ret)
        {
            common_data->stream_operate.object.packet.send_retry++;
            if (0 != tms_common_request_send_timeout(&common_data->stream_operate.object))
            {
                common_data->stream_operate.object.status_code = -0x302;
            }
        }
        else
        {
            common_data->stream_operate.object.packet.sent_request = 0xA5;
            common_data->stream_operate.object.packet.send_retry   = 0;
        }
    }

    if (0 == ret)
    {
        ret = tms_common_response_recv_exec(common_data);
    }

    ret = tms_common_response_parse(common_data);

    return ret;
}


/********************************************************************************
 *
 *
 *      进入退出阶段，释放资源
 *
 */
static s32 tms_common_schedule_exit(common_schedule_t *common_data)
{
    if (common_data)
    {
        tms_schedule_instance()->schedule_node_delete(common_data->schedule_id);
        // 释放资源
        if (0 < common_data->socket_operate.object.socket_id)
        {
            common_data->socket_operate.handle.socket_close(common_data->socket_operate.object.socket_id);
        }
        if (common_data->socket_operate.object.obj_url.dest_host)
        {
            STD_FREE(common_data->socket_operate.object.obj_url.dest_host);
        }
        if (common_data->socket_operate.object.obj_url.dest_page)
        {
            STD_FREE(common_data->socket_operate.object.obj_url.dest_page);
        }
        if (common_data->socket_operate.object.buff_send)
        {
            STD_FREE(common_data->socket_operate.object.buff_send);
        }
        if (common_data->socket_operate.object.buff_recv)
        {
            STD_FREE(common_data->socket_operate.object.buff_recv);
        }
        STD_FREE(common_data);
        STD_PRINT("current schedule node exit, free resource...\r\n");
    }

    return 0;
}

/********************************************************************************
 *
 *
 *  资源调度
 *
 */
static s32 tms_common_schedule_handle(PTR data)
{
    common_schedule_t *common_data;

    if (TRUE == tcp_protocol_instance()->check_tcp_connect_success())
    {
        common_data = (common_schedule_t *)data;
        if (common_data)
        {
            switch (common_data->phase)
            {
            case E_TMS_COMMON_INIT:
                tms_common_schedule_init(common_data);
                break;
            case E_TMS_COMMON_EXEC:
                tms_common_schedule_exec(common_data);
                break;
            case E_TMS_COMMON_EXIT:
                tms_common_schedule_exit(common_data);
                break;
            default:
                STD_PRINT("unknow common schedule phase %d.\r\n", common_data->phase);
                break;
            }
        }
    }
}

/********************************************************************************
 *
 *
 *  网址拆分
 *
 */
s32 tms_common_url_parse(s8 *url, u8 **domain, u8 **page, u16 *port, u16 *is_https)
{
    s8  *p1, *p2, *p3, *p4;
    s32  lens, ret = 0;

    if (!url)
    {
        ret = -1;
        STD_PRINT("invalid url text, please check...\r\n");
    }
    else
    {
        if (0 == strncmp(url, "http://", 7))
        {
            p1 = &url[7];
            *is_https = 0;
        }
        else if (0 == strncmp(url, "https://", 8))
        {
            p1 = &url[8];
            *is_https = 1;
        }
        else
        {
            p1 = &url[0];
            *is_https = 0;
        }
        p2 = strstr(p1, "/");
        p3 = strstr(p1, ":");
        p4 = p3;
        if (!p3 || p3 >= p2)
        {
            if (1 == *is_https)
            {
                *port = 433;
            }
            else
            {
                *port = 80;
            }
        }
        else
        {
            p3++;
            *port = 0;
            while ('0' <= *p3 && '9' >= *p3)
            {
                *port = (*port) * 10;
                *port = (*port) + (*p3 - '0');
                p3++;
            }
            p2 = p3;
            if ('/' != *p3)
            {
                *port = 80;
                ret   = -3;
                STD_PRINT("parse port error.\r\n");
            }
        }
        //
        if (p2 && '/' == p2[0] && '/' != p2[1])
        {
            if (p4 && (p4 < p2))
            {
                lens = p4 - p1;
            }
            else
            {
                lens = p2 - p1;
            }
            *domain = STD_MALLOC(lens + 1);
            *page   = STD_MALLOC(strlen(p2) + 1);
            if (*domain && *page)
            {
                memset(*domain, 0, lens + 1);
                memcpy(*domain, p1, lens);
                strcpy(*page, p2);
            }
            else
            {
                if (*domain)
                {
                    STD_FREE(*domain);
                    *domain = NULL;
                }
                if (*page)
                {
                    STD_FREE(*page);
                    *page = NULL;
                }
                STD_PRINT("malloc memory for url failed, please check...\r\n");
            }
        }
        //
        if (0 == *port || !(*domain) || !(*port) || 4 > strlen(*domain))
        {
            ret = -2;
        }
        STD_PRINT("\r\n%s\r\n%d, %s:%d.", url, ret, *domain, *port);
    }

    return ret;
}

/********************************************************************************
 *
 *
 *  添加新调度节点
 *
 */
s32 tms_common_schedule_routine(schedule_object_t *new_object)
{
    s32 ret = -1;
    common_schedule_t *new_schedule;

    if (!new_object || !new_object->url
        || !new_object->request || !new_object->reponse)
    {
        ret = -1;
        STD_PRINT("illegal parameter, please check...\r\n");
    }
    else
    {
        new_schedule = STD_MALLOC(sizeof(common_schedule_t));
        if (!new_schedule)
        {
            ret = -2;
            STD_PRINT("malloc memory for new schedule failed, please check...\r\n");
        }
        else
        {
            memset(new_schedule, 0, sizeof(common_schedule_t));
            ret = tms_common_url_parse(new_object->url,
                                       &new_schedule->socket_operate.object.obj_url.dest_host,
                                       &new_schedule->socket_operate.object.obj_url.dest_page,
                                       &new_schedule->socket_operate.object.obj_url.dest_port,
                                       &new_schedule->socket_operate.object.obj_url.is_https);
            if (0 != ret)
            {
                STD_FREE(new_schedule);
                STD_PRINT("parse url error, insert schedule abort!\r\n");
            }
            else
            {
                new_schedule->notify_object = new_object->notify_object;
                new_schedule->stream_operate.handle.request  = new_object->request;
                new_schedule->stream_operate.handle.response = new_object->reponse;
                new_schedule->socket_operate.object.obj_url.transmit_type = new_object->transmit_type;
                ret = tms_schedule_instance()->schedule_node_insert(tms_common_schedule_handle, (PTR)new_schedule);
                if (0 > ret)
                {
                    STD_FREE(new_schedule->socket_operate.object.obj_url.dest_host);
                    STD_FREE(new_schedule->socket_operate.object.obj_url.dest_page);
                    STD_FREE(new_schedule);
                }
                else
                {
                    new_schedule->schedule_id = ret;
                    ret = 0;
                }
                STD_PRINT("insert new schedule node with %d.\r\n", ret);
            }
        }
    }

    return ret;
}

