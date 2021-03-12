#include "tms_schedule.h"


#define TMS_SCHEDULE_TABLE_MAX      (0x8)
/********************************************************************************
 *
 *
 *
 *
 */
typedef enum {
    E_TMS_SCHEDULE_NONE = 0,
    E_TMS_SCHEDULE_WAIT,
    E_TMS_SCHEDULE_RUN,
} tms_schedule_status_t;

typedef unsigned int PTR;
typedef struct _table_node {
    u8  status;
    PTR data;
    PTR reserved;
} table_node_t;

typedef struct _schedule_table {
    u8           index;
    table_node_t table[8];
} schedule_table_t;

//
typedef struct _common_object {
    u8 initialized;
    u8 reserved;
} common_object_t;
/********************************************************************************
 *
 *
 *
 *
 */
static tms_schedule_module_t *g_tms_schedule_mn = (void *)0;
static schedule_table_t g_tms_schedule_table = {0, 0, {0}};
static common_object_t g_common_info = {0};

static int list_node_specific_query(tms_schedule_status_t status)
{
    int i, index;

    index = g_tms_schedule_table.index;
    for (i = index; i < TMS_SCHEDULE_TABLE_MAX + index; i++)
    {
        if (status == g_tms_schedule_table.table[i % TMS_SCHEDULE_TABLE_MAX].status)
        {
            break;
        }
    }

    if ((TMS_SCHEDULE_TABLE_MAX + index) > i)
    {
        i = i % TMS_SCHEDULE_TABLE_MAX;
    }
    else
    {
        i = -1;
    }

    return i;
}

static table_node_t *list_node_schedule_query(void)
{
    int i;
    table_node_t *find_node = NULL;

    i = list_node_specific_query(E_TMS_SCHEDULE_RUN);
    if (0 <= i)
    {
        find_node = &g_tms_schedule_table.table[i];
    }
    else
    {
        i = list_node_specific_query(E_TMS_SCHEDULE_WAIT);
        if (0 <= i)
        {
            find_node = &g_tms_schedule_table.table[i];
            find_node->status = E_TMS_SCHEDULE_RUN;
        }
    }
    if (0 <= i)
    {
        //STD_PRINT("schedule %d node in status %d\r\n", i, find_node->status);
    }

    return find_node;
}

static int list_node_tail_insert(PTR new_data)
{
    int i;

    i = list_node_specific_query(E_TMS_SCHEDULE_NONE);
    if (0 <= i)
    {
        g_tms_schedule_table.table[i].data   = new_data;
        g_tms_schedule_table.table[i].status = E_TMS_SCHEDULE_WAIT;
    }

    STD_PRINT("insert new node with %d.\r\n", i);

    return i;
}

static int tms_schedule_node_insert(int (*pfn)(PTR), PTR data)
{
    s32 ret = -1;
    node_data_t *node_data;

    STD_PRINT("schedule node insert start\r\n")

    node_data = STD_MALLOC(sizeof(node_data_t));
    if (node_data)
    {
        memset(node_data, 0, sizeof(node_data_t));
        node_data->schedule_data.pfn  = pfn;
        node_data->schedule_data.data = data;
        ret = list_node_tail_insert((PTR)node_data);
        if (0 > ret)
        {
            STD_FREE(node_data);
        }
    }
    else
    {
        STD_PRINT("malloc memory failed, 0x%x, 0x%x\r\n", 0, node_data);
        if (node_data) STD_FREE(node_data);
    }

    return ret;
}

static int tms_schedule_node_delete(int id)
{
    int ret = -1;

    if (TMS_SCHEDULE_TABLE_MAX > id)
    {
        if ((node_data_t *)(g_tms_schedule_table.table[id].data))
        {
            STD_FREE((node_data_t *)(g_tms_schedule_table.table[id].data));
            g_tms_schedule_table.table[id].data   = 0;
            g_tms_schedule_table.table[id].status = E_TMS_SCHEDULE_NONE;
            ret = 0;
        }
        STD_PRINT("schedule node %d deleted with %d\r\n", id, ret);
    }

    return 0;
}

static void tms_schdule_suit_init(void)
{
    s32 i;

    g_tms_schedule_mn   = NULL;
    //
    g_common_info.initialized  = 0;
    //
    g_tms_schedule_table.index = 0;
    for (i = 0; i < TMS_SCHEDULE_TABLE_MAX; i++)
    {
        g_tms_schedule_table.table[i].data     = 0;
        g_tms_schedule_table.table[i].status   = E_TMS_SCHEDULE_NONE;
        g_tms_schedule_table.table[i].reserved = 0;
    }
}

static s32 tms_schedule_module_init(void)
{
    return 0;
}

static void tms_schedule_suit_install(tms_schedule_module_t *handle)
{
    if (!handle)
    {
        STD_PRINT("illegal input para...\r\n");
    }
    else
    {
        handle->base.init            = tms_schedule_module_init;
        handle->schedule_node_delete = tms_schedule_node_delete;
        handle->schedule_node_insert = tms_schedule_node_insert;
    }
}

static table_node_t * tms_schedule_processing(void)
{
    table_node_t *tail_node;
    node_data_t  *node_data;

    tail_node = list_node_schedule_query();
    if (tail_node)
    {
        node_data = (node_data_t *)(tail_node->data);
        if (node_data)
        {
            if (0xA5 != node_data->schedule_info.dubug)
            {
                node_data->schedule_info.dubug = 0xA5;
                STD_PRINT("\r\n\r\ntips: tms schedule start from 0x%x, processing...\r\n\r\n", node_data);
            }
            if (node_data->schedule_data.pfn)
            {
                node_data->schedule_data.pfn(node_data->schedule_data.data);
            }
            else
            {
                STD_PRINT("schedule node func illegal.\r\n");
            }
        }
        else
        {
            STD_PRINT("schedule node data illegal.\r\n");
        }
    }
    //STD_PRINT("no schedule node be found.\r\n");
    return tail_node;
}

s32 tms_schedule_routine_start(void)
{
    s32 ret;

    do {
        ret = 0;
    } while (NULL != tms_schedule_processing());

    return ret;
}

static s32 tms_schedule_msg_handle(msg_t *m_msg)
{
    switch (m_msg->m_status)
    {
        case MSG_INIT:
            m_msg->m_status = MSG_SERVICE;
            break;
        case MSG_SERVICE:
            if (tcp_protocol_instance()->check_tcp_connect_success())
            {
                tms_schedule_processing();
            }
            break;
        case MSG_TERMINAL:
            break;
        default:
            break;
    }
    return MSG_KEEP;
}

static void tms_schedule_message_init(void)
{
    s32 ret = -0xA5;
    msg_t *msg_node;

    msg_node = STD_MALLOC(sizeof(msg_t));
    if (msg_node)
    {
        memset(msg_node, 0, sizeof(msg_t));
        msg_node->m_func   = tms_schedule_msg_handle;
        msg_node->m_status = MSG_INIT;
        msg_node->m_wparam = 0;
        msg_node->m_lParam = 0;
        strcpy(msg_node->m_msgname, "tms_schedule");
        ret = trendit_postmsg(msg_node);
    }
    TRACE_ERR("tms instance enter with %d.\r\n", ret);
}

tms_schedule_module_t *tms_schedule_instance(void)
{
    if (!g_tms_schedule_mn)
    {
        g_tms_schedule_mn = STD_MALLOC(sizeof(tms_schedule_module_t));
        if (g_tms_schedule_mn)
        {
            memset(g_tms_schedule_mn, 0, sizeof(tms_schedule_module_t));
            if (!(common_object_t *)(g_tms_schedule_mn->base.object))
            {
                g_tms_schedule_mn->base.object = (PTR)&g_common_info;
            }
            if (0xA5 != g_common_info.initialized)
            {
                g_common_info.initialized = 0xA5;
                tms_schedule_suit_install(g_tms_schedule_mn);
                tms_schedule_message_init();
            }
        }
        else
        {
            STD_PRINT("malloc memory failed...\r\n");
        }
    }

    return g_tms_schedule_mn;
}

/*****************************************************************/
void trendit_init_tms_schedule(void)
{
    tms_schdule_suit_init();
}

/********************************************************************************
 * Function Name: tms_schedule_routine_status
 * Function Disc: 查询是否有任务调度
 * Function Para: None
 * Function Retn: 0-idle, 1-running
 */
s32 tms_schedule_routine_status(void)
{
    s32 ret = 0;

    if (NULL != list_node_schedule_query()
        && 0 < tms_get_download_status())
    {
        ret = 1;
    }

    //STD_PRINT("%x, %d\r\n", list_node_schedule_query(), tms_get_download_status());

    return ret;
}

