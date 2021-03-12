#ifndef __MODEL_TMS_H
#define __MODEL_TMS_H

#include "ddiglobal.h"
#include "app_global.h"

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************************
 *
 *
 *
 *
 */
#define TMS_MALLOC(n)       k_mallocapp(n)
#define TMS_FREE(ptr)       k_freeapp(ptr)
#define TMS_DEBUG           TRACE_DBG
#define TMS_ERR             TRACE_ERR
#define TMS_INFO            TRACE_INFO

#define DEV_HTTP_DEBUG      TRACE_DBG

#define TMS_INFO_HEX        TRACE_INFO_HEX

//
/********************************************************************************
 *
 *
 *
 *
 */
typedef s32 (*tms_ext_pfn)(s32 status);
/********************************************************************************
 *
 *
 *
 *
 */
typedef enum tms_err_code {
    TMS_OK = 0,
    TMS_ERR_DEFAULT = -0x7F,
    TMS_ERR_CREATE_SOCKET_FAILED = TMS_ERR_DEFAULT + 3,
    TMS_ERR_RECEIVING_ABORT = TMS_ERR_DEFAULT + 4,
    TMS_ERR_MALLOC_FAILED = TMS_ERR_DEFAULT + 5,
    TMS_ERR_CONNECTING_TIMEOUT = TMS_ERR_DEFAULT + 6,
    TMS_ERR_RECEIVING_TIMEOUT = TMS_ERR_DEFAULT + 7,
    TMS_ERR_BUFFER_OVERFLOW = TMS_ERR_DEFAULT + 8,
    TMS_ERR_INTERGRITY_FAILED = TMS_ERR_DEFAULT + 9,
} tms_err_code_t;

/********************************************************************************
 *
 *
 *
 *
 */
extern model_device_t* tms_instance(void);
extern void trendit_init_tms_data(void);
extern s32  tms_download_entry(void);
extern s32  tms_url_content_download(u8 *filename, u8 *url, u8 direct, tms_ext_pfn ext_pfn);
//
extern s32 cmd_command_mechine_upgrade_app(u8 *value_string, u16 value_stringlen, u8 *ret_string, u16 ret_stringlen);
extern s32 cmd_command_mechine_upgrade_core(u8 *value_string, u16 value_stringlen, u8 *ret_string, u16 ret_stringlen);
extern s32 tms_machine_command_parse(u16 cmd, u8 *dst_data, u16 dst_lens, u8 *ret_data, u16 ret_lens);
extern s32 http_download_routine_status(void);

/*******************************************************************
 *
 * filename: 文件路径全称 /mtd0/xxx.bmp
 * url     : 文件网址
 * ext_pfn : 回调参数返回写入的字节数，<= 0 为错误返回
 */

#ifdef __cplusplus
}
#endif

#endif /* __MODEL_TMS_H */

