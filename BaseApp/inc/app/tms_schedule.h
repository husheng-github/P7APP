#ifndef __TMS_SCHEDULE_H__
#define __TMS_SCHEDULE_H__

#include "ddiglobal.h"
#include "app_global.h"
#include "postmessage.h"


#ifdef __cplusplus
extern "C"{
#endif

#define STD_MALLOC(n)       k_mallocapp(n)
#define STD_FREE(ptr)       k_freeapp(ptr)
#define STD_PRINT           TRACE_DBG


typedef unsigned int PTR;

typedef int (*sch_func)(PTR data);

typedef struct _schedule_info {
    unsigned int  id;
    unsigned char dubug;
} schedule_info_t;

typedef struct _schedule_data {
    PTR data;
    int (*pfn)(PTR data);
} schedule_data_t;

typedef struct _node_data {
    schedule_info_t schedule_info;
    schedule_data_t schedule_data;
} node_data_t;

typedef struct _tms_schedule_node tms_schedule_node_t;
struct _tms_schedule_node {
    tms_schedule_node_t *prev;
    tms_schedule_node_t *next;
    PTR                  data;
};
//
typedef struct _base_module {
    PTR   object;
    s32 (*init)(void);
} base_module_t;

typedef struct _tms_schedule_module {
    //tms_schedule_node_t *object;
    base_module_t base;
    int (*schedule_node_insert)(sch_func pfn, PTR data);
    int (*schedule_node_delete)(int id);
} tms_schedule_module_t;

extern tms_schedule_module_t *tms_schedule_instance(void);
extern void trendit_init_tms_schedule(void);
extern s32 tms_schedule_routine_start(void);
extern s32 tms_schedule_routine_status(void);

#ifdef __cplusplus
}
#endif

#endif /* __TMS_SCHEDULE_H__ */
