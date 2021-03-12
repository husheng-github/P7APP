#ifndef __MODEL_KEY_H
#define __MODEL_KEY_H

#define WIFI_WEB_CONFIG_DURING_TIME  5000
#define DEFAULT_LONGKEY_DURING_TIME  20
#define RESTORE_LONGKEY_DURING_TIME  10000
#define POWER_KEY_UP_FLG    1

typedef struct{
    model_device_t super;
}key_module_t;

key_module_t *key_instance(void);
void trendit_init_key_data(void);
s32 trendit_judge_longkey(u8 keycode, u32 duration_mstime);
u8 trendit_get_key(void);

#endif

