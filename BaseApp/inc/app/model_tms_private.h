#ifndef MODEL_TMS_PRIVATE_H
#define MODEL_TMS_PRIVATE_H

/********************************************************************************
 *
 *
 *
 *
 */
#define TMS_UPGRADE_BREAKPOINT             "/mtd0/tms_bprf"
#define TMS_UPGRADE_CONFIG                 "/mtd0/tms_config"
#define TMS_UPGRADE_PROFILE                "/mtd0/tms_profile"
#define TMS_UPGRADE_FEEDBACK               "/mtd0/tms_feedback"

 //
 #define TMS_PARTICULAR_UPGRADE_URL         "https://dev.trendit.cn/TREN_5603/V9.9.9/V9.9.9.zip"

 #define TMS_PARTICULAR_RESET_URL           "https://dev.trendit.cn/TREN_5603/V0.0.0/V0.0.0.zip"
 //
#define TMS_SERVER_IP                       "47.106.157.121"

#if defined(TMS_DEBUG_MODE)
#define TMS_SERVER_PORT                     3021
#define TMS_DEVICE_SN                       "00005702160030000305"
#else
#define TMS_SERVER_PORT                     80
#endif  /* TMS_DEBUG_MODE */

//#define TMS_DEVICE_CLASS                    "IBOX_0501"
#define TMS_DEVICE_CLASS                    "TREN_5703"
#define TMS_DEFAULT_VERSION_CODE			"V0.0.1"
//
#define TMS_PAGE_LATEST_VERSION			    "/api/firmware_common/latest_version"
#define TMS_PAGE_GET_ADDRESS		        "/api/firmware_common/firmware"
//
#define HTTP_TMS_FIRMWARE_URL(url) \
    do { sprintf(url, "http://%s:%s/%s", TMS_SERVER_IP, TMS_SERVER_PORT, TMS_PAGE_GET_ADDRESS); } while (0)
//
#define HTTP_TMS_LAST_VERSION_URL(url) \
        do { sprintf(url, "http://%s:%s/%s", TMS_SERVER_IP, TMS_SERVER_PORT, TMS_PAGE_LATEST_VERSION); } while (0)

// http://47.106.157.121:3021/api/firmware_common/latest_version?deviceSn=00005702160030000305&firmwareClass=IBOX_0501&currentFirmwareCode=0.0.1
#define HTTP_TMS_LAST_VERSION_HEAD  \
	            "GET %s?deviceSn=%s&firmwareClass=%s&currentFirmwareCode=%s HTTP/1.1\r\n"\
	            "Host: %s:%d\r\n"\
                "Accept: */*\r\n"\
                "User-Agent: GeneralDownloadApplication\r\n"\
                "Connection: close\r\n"\
                "\r\n"
//
#define HTTP_TMS_LAST_VERSION_CHECK(buf, domain, page, port, sn, category, version)\
    { sprintf(buf, HTTP_TMS_LAST_VERSION_HEAD, page, sn, category, version, domain, port); } while (0)
//
#define HTTP_TMS_LAST_VERSION_ADDR_HEAD \
                "GET %s?targetFirmwareCode=%s&firmwareClass=%s&currentFirmwareCode=%s HTTP/1.1\r\n"\
                "Host: %s:%d\r\n"\
                "Accept: */*\r\n" \
                "User-Agent: GeneralDownloadApplication\r\n"\
                "Connection: close\r\n"\
                "\r\n"
//
#define HTTP_TMS_LAST_VERSION_ADDR(buf,  domain, page, port, category, new_verion, old_version)\
    { sprintf(buf, HTTP_TMS_LAST_VERSION_ADDR_HEAD, page, new_verion, category, old_version, domain, port); } while (0)
//
#define HTTP_TMS_BLOCK_REQUSET_HEAD     "GET %s HTTP/1.0\r\n" \
                                        "Host: %s\r\n" \
                                        "Accept: */*\r\n" \
                                        "User-Agent: GeneralDownloadApplication\r\n" \
                                        "Connection: keep-alive\r\n" \
                                        "Range: bytes=%d-%d\r\n"\
                                        "\r\n"
//
#define HTTP_TMS_BLOCK_REQUSET_FORM     HTTP_TMS_BLOCK_REQUSET_HEAD"\r\n%s\r\n"
//
#define HTTP_TMS_BLOCK_REQUSET(get, domain, page, start, len)  \
    do { sprintf(get, HTTP_TMS_BLOCK_REQUSET_HEAD, page, domain, start, (start + len - 1)); } while (0)
//
#define HTTP_URL_CONTENT_397083         "http://oss.xlyprint.cn/30092abf-3f37-4f73-a9b5-acbff5efa8ce.jpg"
#define HTTP_URL_CONTENT_1662           "http://downloadtest.yinlijun.com/image_unicom.bmp"
//
#define HTTP_CONTENT_REQUEST_HEADER     "GET %s HTTP/1.1\r\n" \
                                        "Host: %s\r\n" \
                                        "User-Agent: lfs/7.65.0\r\n" \
                                        "Accept: */*\r\n"\
                                        "\r\n"
//
#define HTTP_URL_CONTENT_REQUSET(get, domain, page)     \
    do { sprintf(get, HTTP_CONTENT_REQUEST_HEADER, page, domain); } while (0)
//
#define TMS_APUPDATE_FILE_NAME          "/mtd2/apupdate"
#define TMS_FILE_LIST_MAX               (8)
#define TMS_FILE_NAME_MAX               (32)
#define TMS_APUPDATE_HEADER_SIZE        (0x400)
#define TMS_PACKAGING_HEADER_SIZE       (0x400 * 4)
//
#define TMS_NET_RECV_HEADER_MAX         (0x800)          // 请求报文头允许最大值
#define TMS_NET_BLOCK_SIZE_MAX          (0x400 * 8)      // 断电续传请求文件块最大字节数
#define TMS_NET_RECV_BUFFER_MAX         (TMS_NET_RECV_HEADER_MAX + TMS_NET_BLOCK_SIZE_MAX)
//
#define TMS_NET_SEND_HEADER_MAX         (1024)
#define TMS_NET_SEND_BODY_MAX           (0)
#define TMS_NET_SEND_BUFFER_MAX         (TMS_NET_SEND_HEADER_MAX + TMS_NET_SEND_BODY_MAX)
//
#define TCP_UART_BAUD_RATE_MAX          (115200)
#define TCP_RECV_BYTES_PER_SECOND       (TCP_UART_BAUD_RATE_MAX / 12)
#define TCP_NET_RECV_NONE_COUNT_MAX     (8)
#define TCP_CONNECT_OVERTIME            (10000)         // 连接最大超时时间
#define TCP_CONNECTION_CHECK_INTERVAL   (96)            // 网络连接状态检查间隔
#define TCP_RECEIVING_TIMEOUT_MAX       (TMS_NET_RECV_BUFFER_MAX * 4)   // 单次接收数据最大超时
#define TCP_RECEIVING_INTERVAL_MAX      (1)             // 两次连续接收socket间隔
#define TCP_RECEIVING_WAITTING_MAX      (8)             //
#define TCP_RECEIVING_RETRY_MAX         (8)
//
#define NET_VERSION_MESSAGE_MAX         (32)
#define NET_VERSION_CATEGORY_MAX        (16)
#define NET_VERSION_NUMBER_MAX          (12)
#define NET_VERSION_PLATFORM            (12)
#define NET_VERSION_DESCRIPTION         (64)
#define NET_VERSION_HASH_MAX            (32)
#define NET_VERSION_URL_MAX             (512)

/********************************************************************************/
typedef struct version_detail
{
    s32 code;
    s8  message[NET_VERSION_MESSAGE_MAX];
    s8  category[NET_VERSION_CATEGORY_MAX];
    s8  new_version[NET_VERSION_NUMBER_MAX];
    s8  platfrom[NET_VERSION_PLATFORM];
    s8  description[NET_VERSION_DESCRIPTION];
    s8  hash[NET_VERSION_HASH_MAX];
    S8  url[NET_VERSION_URL_MAX];
    s8  status;
} version_detail_t;

typedef struct _packet_info
{
    u32 packet_size;    // 当前packcet总共
    u32 header_len;     // 当前packcet 中报文头长度：头长度 + \n\r\r\r
    u32 content_len;    // 当前packcet 中body长度
    u32 received_len;   // 当前packcet 已接收长度
    u16 send_retry;
    u16 recv_retry;
    u16 recv_error;
    s32 packet_err;
} packet_info_t;

typedef struct _partial_info
{
    u32 block_start;
    u32 block_end;
    u32 block_size;
    u32 last_offset;
} partial_info_t;

typedef struct flow_control
{
    packet_info_t  packet;              // 数据包详细
    partial_info_t partial;             // 分包粗略
    u32 packet_count;                   // 包计数
    u32 file_total;                     // 总文件大小
    u32 content_offset;
    u8  finish_notify;                  // 结束通知
    u8  vfs_save;                       // 是否使用vfs
    u8  file_path[TMS_FILE_NAME_MAX];   // 文件保存路径
    u8  direct_url;
    u8  general_file;                   // 普通文件下载
    u8  partial_content;                // 分包下载标志
    u8  packet_resume;                  // 发送标志
    u8  msg_post;
    u8  capacity_enough;                // 检查内存
    u32 crc32_tms4k;
    u32 crc32_save;
} flow_control_t;

typedef struct append_info
{
    s8  https;
    s8  domain[63];
    u16 port;
    s8  page[512];
    s8  sn[24];
    s8  catagory[16];
    s8  new_version[12];
    s8  old_version[12];
    s32 block_start;
    s32 block_size;
    s8 *body_data;
    s32 body_size;
    s8 *raw_data;
    s32 raw_size;
} append_info_t;

typedef struct socket_info
{
    s32 socket_id;
    s32 socket_status;
    s32 status_code;
    u32 create_time;
    u32 reconnect;
    u32 query_time;
    u32 recv_time;
    s32 prev_net;
    s32 net_change;
    u8 *send_buff;
    s32 send_size;
    u8 *recv_buff;
    u32 recv_size;
    u32 block_start;
    u32 block_size;
} socket_info_t;

typedef struct tms_utils tms_utils_t;

typedef struct socket_handle
{
    s32 (*socket_create)(tms_utils_t *tms_utils);
    s32 (*socket_send)(s32 socket_id, u8 *wbuf, s32 len);
    s32 (*socket_recv)(s32 socket_id, u8 *rbuff, s32 len);
    s32 (*socket_close)(s32 socket_id);
    s32 (*socket_get_status)(s32 socket_id);
} socket_handle_t;

typedef struct data_handle
{
    s32 (*http_append_head)(append_info_t *append_detail);
    s32 (*msg_data_update)(s8 *data, s32 len, s32 status);
    s32 (*msg_data_finish)(s8 *data, s32 len, s32 status);
    s32 (*msg_ext_notify)(s32 status);
    s32 (*msg_free)(tms_utils_t *tms_utils);
} data_handle_t;

struct tms_utils {
    msg_t          *msg_node;
    append_info_t   append_info;
    data_handle_t   data_handle;
    flow_control_t  flow_control;
    socket_info_t   socket_info;
    socket_handle_t socket_handle;
};
/********************************************************************************
 *
 *
 *        组包结构体
 *
 */
typedef struct apupdate_file {
    u8  name[TMS_FILE_NAME_MAX];
    u32 size;
    u32 offset;
    u32 crc32;
} apupdate_file_t;

typedef struct apupdate_header {
    u8 total_files;
    apupdate_file_t list[TMS_FILE_LIST_MAX];
} apupdate_header_t;
//
/********************************************************************************
 *
 *
 *
 *
 */
// TMS 传输错误
enum {
    TMS_TCP_TRANSMIT_ERROR_BASE = -0x200,
    TMS_TCP_TRANSMIT_SPACE_NOT_ENOUGH,
    TMS_TCP_TRANSMIT_RECONNECT_OVER_STEP,
    TMS_TCP_TRANSMIT_RETRY_TIMEOUT,
};

//
enum {
    CPAP_TMS_DOWNLOAD_RESULT = 0,
    CPAP_TMS_UPGRADE_RESULT,
};

#endif  /* MODEL_TMS_PRIVATE_H */

