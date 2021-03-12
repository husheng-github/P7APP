#ifndef __TMS_DOWNLOAD_H__
#define __TMS_DOWNLOAD_H__

#include "tms_common.h"
#include "tms_schedule.h"

#define TMS_DOWNLOAD_ALIPAY_42280   "http://downloadtest.yinlijun.com/alipay_previewer_007.jpg"
#define TMS_DOWNLOAD_MEDIA_WAV      "http://downloadtest.yinlijun.com/inputAmount.wav"


#ifdef __cplusplus
extern "C"{
#endif

typedef unsigned int PTR;

typedef struct _transmit_node {
    u8    transmit_type;
    u8    file_type;
    u8   *full_url;
    u8   *full_path;
    u8   *being_post;
    u8   *token;
    PTR   para1;
    s32 (*stream)(PTR para, u8 *buff, u32 size, s32 code, s32 status);
    s32 (*notify)(PTR para, s32 code, s32 status);
    s32 (*free)(PTR para);
} transmit_node;

//
typedef struct _splice_base {
    u8  name[32];
    u32 size;
    u32 offset;
    u32 crc32;
} splice_base_t;

typedef struct _bin_splice {
    u8 total_files;
    splice_base_t list[8];
} bin_splice_t;

typedef struct _bin_pack {
		u8 status;
} bin_pack_t;

typedef struct _verify_object {
    PTR splice;
    PTR pack;
} verify_object_t;

typedef struct _verify_instance {
    s32 (*sign_get)(verify_object_t *obj);
    s32 (*sign_check)(verify_object_t *obj);
} verify_instance_t;

typedef struct _verify_operate {
    verify_instance_t handle;
} verify_operate_t;

typedef struct _file_object {
    u8 *full_path;
    u8  opened;
    s32 fd;
    u32 total_size;
    u32 file_offset;
} file_object_t;

typedef struct _file_instance {
    s32 (*open)(file_object_t *object);
    s32 (*write)(file_object_t *object, u8 *data, u32 len, u32 offset);
    s32 (*read)(file_object_t *object, u8 *buff, u32 len, u32 offset);
    s32 (*close)(file_object_t *object);
} file_instance_t;

typedef struct _file_operate {
    file_object_t    object;
    file_instance_t  handle;
} file_operate_t;
//
typedef struct _notify_object {
    u8  type;
    PTR para;
    PTR reserved;
} notify_object_t;

typedef struct _notify_instance {
    s32 (*stream)(PTR para, u8 *data, u32 len, s32 code, s32 status);
    s32 (*notify)(PTR para, s32 code, s32 status);
    s32 (*free)(PTR para);
} notify_instance_t;

typedef struct _notify_operate {
    notify_object_t   object;
    notify_instance_t handle;
} notify_operate_t;

typedef struct _tms_transmit {
    verify_operate_t verify_operate;
    file_operate_t   file_operate;
    notify_operate_t notify_operate;
} tms_transmit_t;

//
typedef struct _upgrade_feedback {
    upgrade_callback_info_t  para;
    s32                    (*stream_func)(u8 *data, u32 data_len, DOWNLOAD_STEP_E download_step, upgrade_callback_info_t *upgrade_callback_info);
    void                   (*notify_func)(upgrade_callback_info_t *para, UPGRADE_STEP_E upgrade_step, s32 result);
} upgrade_feedback_t;

extern void tms_download_picture_routine(void);
extern s32  tms_transmit_message_post(transmit_node *new_node);
extern s32  trendit_http_schedule_add(upgrade_info_t *upgrade_info, PTR reserved_para);
extern s32  tms_get_download_status(void);
extern s32  tms_set_download_status(u32 status);


#ifdef __cplusplus
}
#endif

#endif /* __TMS_DOWNLOAD_H__ */

