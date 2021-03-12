#ifndef __TMS_UPLOAD_H__
#define __TMS_UPLOAD_H__

#include "tms_schedule.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef unsigned int PTR;


extern s32 tms_upload_chunked_request(PTR para, PTR reserved, u8 *buff, u32 size, s32 status);
extern s32 tms_upload_chunked_reponse(PTR para, PTR reserved, u8 *data, u32 len, s32 code, s32 status);

extern void tms_upload_file_routine(void);


#ifdef __cplusplus
}
#endif

#endif /* __TMS_UPLOAD_H__ */

