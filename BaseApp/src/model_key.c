#include "ddiglobal.h"
#include "app_global.h"

static key_module_t *g_model_key = NULL;


/**
 * @brief 初始化本文件所有的全局变量
 */
void trendit_init_key_data(void)
{
    g_model_key = NULL;
}

static s32 key_msg_handle(struct _msg *m_msg)
{
    s32 ret = MSG_KEEP;
    u8 key;
    s32 key_ret = DDI_ERR;
    printer_module_t *printer_module = printer_instance();
#ifdef PRINT_SPEED
    u32 cur_ticks = 0;
    u32 end_ticks = 0;
    u32 print_len = 0;
    s8 buff[256] = {0};
#endif

    do{
        if(MSG_TERMINAL == m_msg->m_status)
        {
            ret = MSG_OUT;
            break;
        }
        else if(MSG_INIT == m_msg->m_status || MSG_SERVICE == m_msg->m_status)
        {
            key = trendit_get_key();
            if(key > 0)
            {
                TRACE_DBG("KEY:%04x", key);
                switch(key)
                {
                    case KEY_MINUS:
                        if(DDI_OK == trendit_judge_longkey(key, DEFAULT_LONGKEY_DURING_TIME))
                        {
                            TRACE_DBG("KEY -\r\n");
                            if(DDI_OK == audio_instance()->cfg_vol(VOL_CFG_SOURCE_KEY, KEY_VOL_DOWN, 0))
                            {
                                audio_instance()->audio_play(AUDIO_SUB, AUDIO_PLAY_NOBLOCK);
                            }
                        }
                        break;
                        
                    case KEY_PLUS:
                        if(DDI_OK == trendit_judge_longkey(key, DEFAULT_LONGKEY_DURING_TIME))
                        {
                            TRACE_DBG("KEY +\r\n");
                            if(DDI_OK == audio_instance()->cfg_vol(VOL_CFG_SOURCE_KEY, KEY_VOL_UP, 0))
                            {
                                audio_instance()->audio_play(AUDIO_PLUS, AUDIO_PLAY_NOBLOCK);
                            }
                        }
                        break;
                        
                    case KEY_CFG:
#ifdef WIFI_FIXED_FREQ_TEST
                        audio_instance()->audio_play("歪坏复位", AUDIO_PLAY_BLOCK);
                        ddi_wifi_ioctl(WIFI_IOCTL_CMD_REPOWER, 0, 0);
#else
                        TRACE_DBG("KEY CFG\r\n");
                        key_ret = trendit_judge_longkey(key, WIFI_WEB_CONFIG_DURING_TIME);
                        if(DDI_OK == key_ret)
                        {
                            audio_instance()->audio_play(AUDIO_ENTER_WIFI_WEB_CONFIG, AUDIO_PLAY_BLOCK);
                            wifi_instance()->web_config();
                        }
                        else
                        {
                            if(key_ret != KEY_CFG_PAPER && key_ret != KEY_CFG_PLUS)     //<D7><DF>?+<C5><E4><D6><C3>
                            {
                                audio_instance()->audio_play(AUDIO_ENTER_WIFI_CONFIG, AUDIO_PLAY_BLOCK);
                                wifi_instance()->airkiss_config();
                            }
                        }
#endif
                        break;
                        
                    case KEY_PAPER:
                        if(DDI_OK == trendit_judge_longkey(key, DEFAULT_LONGKEY_DURING_TIME))
                        {
                            TRACE_DBG("printer enter");
                            if(printer_module)
                            {
                                printer_module->handle_paper_key_event();
                            }
                        }
                        break;

                    case KEY_PLUS_MINUS:
                        //trendit_auto_test();
                        trendit_factory_test();
                        break;

                    case KEY_CFG_PLUS:
                        wireless_8960_test();
                        break;

                    case KEY_CFG_PAPER:
                        key_ret = trendit_judge_longkey(key, RESTORE_LONGKEY_DURING_TIME);
                        if(DDI_OK == key_ret)
                        {
                            audio_instance()->audio_play(AUDIO_RECOVERY, AUDIO_PLAY_BLOCK);
                            machine_instance()->super.restore_cfg(&(machine_instance()->super));
                            audio_instance()->audio_play(AUDIO_RECOVERY_SUCC, AUDIO_PLAY_BLOCK);
                            tcp_protocol_instance()->restart();
                        }
                        else
                        {
                            #ifdef PRINT_SPEED
                            cur_ticks = trendit_get_ticks();
                            #endif
                            trendit_print_machine_info();
                            #ifdef PRINT_SPEED
                            end_ticks = trendit_get_ticks();
                            ddi_thmprn_ioctl(DDI_THMPRN_GET_PRINT_LEN, (u32)&print_len, 0);
                            TRACE_DBG("cur_ticks:%u, end_ticks:%u, print_len:%u", cur_ticks, end_ticks, print_len);
                            snprintf(buff, sizeof(buff), "打印速度:%d mm/s\r\n\r\n", ((print_len)*1000)/(end_ticks-cur_ticks));
                            printer_module->direct_print(buff, strlen(buff));
                            #endif
                        }
                        break;
                }

                ddi_key_clear();
            }
            ret = MSG_KEEP;
            break;
        }
    }while(0);

    return ret;
}

static s32 post_key_msg()
{
    msg_t *loop_msg = NULL;
    s32 ret = DDI_ERR;

    loop_msg = (msg_t *)k_mallocapp(sizeof(msg_t));
    if(NULL == loop_msg)
    {
        TRACE_ERR("malloc failed");
    }
    else
    {
        TRACE_DBG("loop_msg:%x", loop_msg);
        memset(loop_msg, 0, sizeof(msg_t));
        loop_msg->m_func = key_msg_handle;
        loop_msg->m_priority = MSG_NORMAL;
        loop_msg->m_lParam = 0;
        loop_msg->m_wparam = 0;
        loop_msg->m_status = MSG_INIT;
        snprintf(loop_msg->m_msgname, sizeof(loop_msg->m_msgname), "%s", "key");
        trendit_postmsg(loop_msg);
    }

    return ret;
}


static s32 exec_cfg(u16 cmd, u8 *value_string, u16 value_stringlen)
{
    s32 ret = DDI_ERR;
    
    TRACE_DBG("cmd:%04x, value_string:%s", cmd, value_string);

    return ret;
}

static s32 exec_qry(u16 cmd, u8 *ret_string, u16 ret_stringlen)
{
    s32 ret = DDI_ERR;
    
    TRACE_DBG("cmd:%04x", cmd);
    switch(cmd)
    {
        default:
            ret = DDI_EUNKOWN_CMD;
            break;
    }
    
    return ret;
}

static s32 exec_cmd(u16 cmd, u8 *value_string, u16 value_stringlen, u8 *ret_string, u16 ret_stringlen)
{
    s32 ret = DDI_ERR;
    
    TRACE_DBG("cmd:%04x, value_string:%s", cmd, value_string);

    return ret;
}

static u8 get_key_string(u8 key, u8 *key_des, s32 len)
{
    
    switch(key)
    {
        case KEY_MINUS:
            snprintf(key_des, len, "%s", "-");
            break;
        
        case KEY_PLUS:
            snprintf(key_des, len, "%s", "+");
            break;
        
        case KEY_CFG:
            snprintf(key_des, len, "%s", "CFG");
            break;
        
        case KEY_PAPER:
            snprintf(key_des, len, "%s", "PAPER");
            ddi_thmprn_feed_paper(200);
            break;
        
        case KEY_TBD:
            snprintf(key_des, len, "%s", "TBD");
            break;
        
        case POWER:
            snprintf(key_des, len, "%s", "POWER");
            break;
        
        case KEY_CFG_PLUS:
            snprintf(key_des, len, "%s", "CFG+");
            break;
        
        case KEY_CFG_MINUS:
            snprintf(key_des, len, "%s", "CFG-");
            break;
        
        case KEY_CFG_PAPER:
            snprintf(key_des, len, "%s", "CFG_PAPER");
            break;
        
        case KEY_PAPER_PLUS:
            snprintf(key_des, len, "%s", "PAPER+");
            break;
        
        case KEY_PAPER_MINUS:
            snprintf(key_des, len, "%s", "PAPER-");
            break;
        
        case KEY_PLUS_MINUS:
            snprintf(key_des, len, "%s", "+-");
            break;
    }
}

/**
 * @brief 获取按键，只有获取的event是keydown才认为有按键
 * @retval  键值
 */
u8 trendit_get_key(void)
{
    u32 cur_tick = trendit_get_ticks();
    u32 key;
    u8 key_code = 0;
    u8 key_event = 0;
    s32 ret = DDI_ERR;
    u8 key_des[12] = {0};

    ret = ddi_key_read(&key);
    if(ret > 0)
    {
        key_code = key&0xff;
        key_event = (key&0xff00)>>8;

        memset(key_des, 0, sizeof(key_des));
        get_key_string(key_code, key_des, sizeof(key_des));
        //TRACE_DBG("key:%s(%x) event:%x", key_des, key_code, key_event);
        //单个按键不是按下键，忽略此按键
        if((POWER == key_code) && (key_event == KEY_EVENT_DOWN))
        {
            ddi_key_ioctl(DDI_KEY_CTL_POWERKEY_FLG, POWER_KEY_UP_FLG, 0);
        }
        if(KEY_EVENT_DOWN != key_event && key_code < KEY_CFG_PLUS)
        {
            TRACE_DBG("key:%s(%x) event:%x, is not keydown then ignore", key_des, key_code, key_event);
            key_code = 0;
        }

        if((POWER == key_code) && (ddi_key_ioctl(DDI_KEY_GET_POWERKEY_FLG, 0, 0)))
        {
            if(DDI_OK == trendit_judge_longkey(key_code, machine_instance()->get_power_key_delay_time()))
            {
                system_data_instance()->set_poweroff_reason(RESEASON_KEY_POWER_OFF);
                audio_instance()->audio_play(AUDIO_POWER_OFF, AUDIO_PLAY_BLOCK);
                ddi_misc_poweroff();
                while(1)
                {
                    ddi_misc_msleep(500);
                    ddi_watchdog_feed();
                }
            }
        }
    }

    ddi_watchdog_feed();

    return key_code;
}

/**
 * @brief 判断keycode键是否是长按键，先使用trendit_get_key获取keydown后再调用此接口判断
 * @param[in] keycode
 * @param[in] duration_mstime 长按键需持续的时间
 * @retval  DDI_OK 是
 * @retval  DDI_ERR 是
 */
s32 trendit_judge_longkey(u8 keycode, u32 duration_mstime)
{
    u32 cur_tick = trendit_get_ticks();
    u32 key;
    u8 key_cur_code = 0;
    u8 key_cur_event = 0;
    s32 ret = DDI_ERR;

    cur_tick = trendit_get_ticks();
    while(1)
    {
        ret = ddi_key_read(&key);
        if(ret > 0)
        {
            key_cur_code = key&0xff;
            key_cur_event = (key&0xff00)>>8;

            if(key_cur_code != keycode)
            {
                TRACE_ERR("get another key:%x, keycode:%x", key_cur_code, keycode);
                ret = key_cur_code;
                break;
            }
            
            if(KEY_LONG_PRESS != key_cur_event)
            {
                TRACE_ERR("key_code %x don't hold %d time(%d), key event:%d", 
                                                        key_cur_code, 
                                                        duration_mstime,
                                                        trendit_get_ticks()-cur_tick,
                                                        key_cur_event);
                ret = DDI_ERR;
                break;
            }
        }

        if(trendit_query_timer(cur_tick, duration_mstime))
        {
            TRACE_INFO("get long key_code %x hold %d time(%d)", 
                                            key_cur_code, 
                                            duration_mstime,
                                            trendit_get_ticks()-cur_tick);
            ret = DDI_OK;
            break;
        }

        ddi_watchdog_feed();
    }

    return ret;
}

/**
 * @brief key的初始化函数
 * @retval  key的设备结构体
 */
key_module_t *key_instance(void)
{
    if(g_model_key)
    {
        return g_model_key;
    }

    g_model_key = k_mallocapp(sizeof(key_module_t));
    if(NULL == g_model_key)
    {
        TRACE_ERR("malloc fail");
        return NULL;
    }
    
    memset(g_model_key, 0, sizeof(key_module_t));
    INIT_SUPER_METHOD(g_model_key, MODEL_KEY, "key");

    post_key_msg();

    return g_model_key;
}


