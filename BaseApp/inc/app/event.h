#ifndef __EVENT_H
#define __EVENT_H

#define EVENT_HBT                              0x1051
#define EVENT_ONLINE                           0x1052
#define EVENT_CHECK_DEVICE                     0x1053
#define EVENT_POWERON                          0x1054
#define EVENT_VER                              0x1055
#define EVENT_PAPER_CHANGE                     0x1056


typedef struct{
    s32 m_event_code;
    s32 m_event_switch;
    Queue *m_list;
}event_node_t;

typedef struct{
    queue_t *m_event_list;
    s32 m_event_list_num;
    u8 m_device_pack_no;

    u16 (*get_device_packno)(void);
    s32 (*cfg_event)(u16 cmd, u8 *value_string, u16 value_stringlen);
    s32 (*send_event)(u16 event_code, s32 ret, s8 *send_string, s32 send_len);
}event_model_t;

event_model_t *event_instance(void);

#endif
