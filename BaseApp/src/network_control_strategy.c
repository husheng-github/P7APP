#include "ddiglobal.h"
#include "app_global.h"

static network_strategy_t *g_network_strategy = NULL;

/**
 * @brief 初始化本文件所有的全局变量

 */
void trendit_init_network_strategy_data(void)
{
    g_network_strategy = NULL;
}

static s32 network_strategy_msg_handle(struct _msg *m_msg)
{
    s32 ret = MSG_KEEP;
    wireless_module_t *wireless_module = wireless_instance();
    wifi_module_t *wifi_module = wifi_instance();
    network_strategy_t *network_strategy = network_strategy_instance();
    tcp_protocol_module_t *tcp_protocol = tcp_protocol_instance();
    u8 sys_rtc[6];

    do{
        if(MSG_TERMINAL == m_msg->m_status)
        {
            ret = MSG_OUT;
            break;
        }
        else if(MSG_INIT == m_msg->m_status)
        {
            network_strategy->m_beg_reg_net_ticks = trendit_get_ticks();
            network_strategy->m_wifi_net_state = DEVICE_NET_ERR;
            network_strategy->m_wireless_net_state = DEVICE_NET_ERR;
            m_msg->m_status = MSG_SERVICE;
            m_msg->m_lParam = REG_NETWORK;
            ret = MSG_KEEP;
            break;
        }
        else
        {
            if(REG_NETWORK == m_msg->m_lParam)
            {
                switch(network_strategy->m_network_strategy_type)
                {
                    case WIFI_ONLY:
                        if(wifi_module->check_network_connect_success())
                        {
                            network_strategy->notify_device_online(COMMU_WIFI);
                            network_strategy->m_wifi_net_state = DEVICE_NET_OK;
                            TRACE_INFO("WIFI connect ap succss");
                            ddi_led_ioctl(DDI_LED_CTL_CLOSE_YELLOW, 0, 0);
                            ddi_led_sta_set(LED_SIGNAL_Y, 1);
                            network_strategy->m_current_type = COMMU_WIFI;
                            m_msg->m_lParam = CHECK_RTC;
                            ret = MSG_KEEP;
                            break;
                        }
                        break;

                    case WIFI_PRIORITY:
                        //wifi模块异常时，策略改为仅2G
                        if(wifi_module->m_msg_status == MSG_TERMINAL)
                        {
                            TRACE_INFO("WIFI module fatal, then change to 2G only");
                            network_strategy->m_network_strategy_type = WIRELESS_2G_ONLY;
                            break;
                        }
                            
                        if(wifi_module->check_network_connect_success())
                        {
                            TRACE_INFO("WIFI connect ap succss");
                            network_strategy->notify_device_online(COMMU_WIFI);
                            network_strategy->m_wifi_net_state = DEVICE_NET_OK;
                            ddi_led_ioctl(DDI_LED_CTL_CLOSE_YELLOW, 0, 0);
                            ddi_led_sta_set(LED_SIGNAL_Y, 1);
                            network_strategy->m_current_type = COMMU_WIFI;
                            network_strategy->m_beg_connect_ticks = trendit_get_ticks();
                            m_msg->m_lParam = CHECK_RTC;
                            ret = MSG_KEEP;
                            break;
                        }

                        if(wireless_module->check_network_active_success())
                        {
                            //wifi在超时时间内仍未连接上
                            //wifi热点名为空
                            if(
                                0 == strlen(wifi_module->m_wifi_ssid) || 
                                trendit_query_timer(network_strategy->m_beg_reg_net_ticks, network_strategy->m_wifi_reg_net_try_overtime))
                            {
                                TRACE_INFO("wifi connect overtime, then try wireless");
                                network_strategy->notify_device_online(COMMU_WIRELESS);
                                network_strategy->m_wireless_net_state = DEVICE_NET_OK;
                                ddi_led_ioctl(DDI_LED_CTL_CLOSE_YELLOW, 0, 0);
                                ddi_led_sta_set(LED_SIGNAL_Y, 1);
                                network_strategy->m_current_type = COMMU_WIRELESS;
                                network_strategy->m_beg_connect_ticks = trendit_get_ticks();
                                m_msg->m_lParam = CHECK_RTC;
                                ret = MSG_KEEP;
                                break;
                            }
                        }
                        break;

                    case WIRELESS_2G_ONLY:
                        if(wireless_module->check_network_active_success())
                        {
                            TRACE_INFO("wireless active network succss");
                            network_strategy->notify_device_online(COMMU_WIRELESS);
                            network_strategy->m_wireless_net_state = DEVICE_NET_OK;
                            ddi_led_ioctl(DDI_LED_CTL_CLOSE_YELLOW, 0, 0);
                            ddi_led_sta_set(LED_SIGNAL_Y, 1);
                            network_strategy->m_current_type = COMMU_WIRELESS;
                            m_msg->m_lParam = CHECK_RTC;
                            ret = MSG_KEEP;
                            break;
                        }
                        break;
                        
                    case WIRELESS_2G_PRIORITY:
                        //2G模块异常时，策略改为仅wifi
                        if(wireless_module->m_msg_status == MSG_TERMINAL)
                        {
                            TRACE_INFO("wireless module fatal, then change to 2G only");
                            network_strategy->m_network_strategy_type = WIFI_ONLY;
                            break;
                        }
                        
                        if(wireless_module->check_network_active_success())
                        {
                            TRACE_INFO("wireless active network succss");
                            network_strategy->notify_device_online(COMMU_WIRELESS);
                            network_strategy->m_wireless_net_state = DEVICE_NET_OK;
                            ddi_led_ioctl(DDI_LED_CTL_CLOSE_YELLOW, 0, 0);
                            ddi_led_sta_set(LED_SIGNAL_Y, 1);
                            network_strategy->m_current_type = COMMU_WIRELESS;
                            network_strategy->m_beg_connect_ticks = trendit_get_ticks();
                            m_msg->m_lParam = CHECK_RTC;
                            ret = MSG_KEEP;
                            break;
                        }

                        if(wifi_module->check_network_connect_success())
                        {
                            //无线在超时时间内仍未连接上
                            if(trendit_query_timer(network_strategy->m_beg_reg_net_ticks, network_strategy->m_wireless_reg_net_try_overtime))
                            {
                                TRACE_INFO("wireless connect overtime, then try wifi");
                                network_strategy->notify_device_online(COMMU_WIFI);
                                network_strategy->m_wifi_net_state = DEVICE_NET_OK;
                                ddi_led_ioctl(DDI_LED_CTL_CLOSE_YELLOW, 0, 0);
                                ddi_led_sta_set(LED_SIGNAL_Y, 1);
                                network_strategy->m_current_type = COMMU_WIFI;
                                network_strategy->m_beg_connect_ticks = trendit_get_ticks();
                                m_msg->m_lParam = CHECK_RTC;
                                ret = MSG_KEEP;
                            }
                            break;
                        }
                        break;
                }
            }
            else if(CHECK_RTC == m_msg->m_lParam)
            {
                if(wireless_instance()->m_have_sync_time || wifi_instance()->m_have_sync_time)
                {
                    TRACE_INFO("rtc sync success");
                    m_msg->m_lParam = TCP_CONNECT;
                    tcp_protocol->start();
                }
                
                ret = MSG_KEEP;
                break;
            }
            else if(TCP_CONNECT == m_msg->m_lParam)
            {
                if(tcp_protocol->check_tcp_connect_success())
                {
                    TRACE_DBG("connect server success, current comm:%d", network_strategy_instance()->m_current_type);
                    ddi_led_sta_set(LED_SIGNAL_Y, 0);
                    ddi_led_sta_set(LED_SIGNAL_B, 1);
                    trendit_get_sys_time(network_strategy->m_last_online_time, sizeof(network_strategy->m_last_online_time), TIME_NO_FORMAT);
                    if(!network_strategy->m_connect_play)
                    {
                        network_strategy->m_connect_play = TRUE;
                        if(COMMU_WIFI == network_strategy->m_current_type)
                        {
                            audio_instance()->audio_play(AUDIO_DEVICE_WIFI_CONECT_SUCCESS, AUDIO_PLAY_BLOCK);
                        }
                        else if(COMMU_WIRELESS == network_strategy->m_current_type)
                        {
                            audio_instance()->audio_play(AUDIO_DEVICE_WIRELESS_CONECT_SUCCESS, AUDIO_PLAY_BLOCK);
                        }
                    }
                    event_instance()->send_event(EVENT_ONLINE, DDI_OK, NULL, 0);
                    network_strategy->notify_online();         //通知其他模块已上线
                    m_msg->m_lParam = CHECK_NETWORK;
                    ret = MSG_KEEP;
                    break;
                }

                if(network_strategy->m_network_strategy_type == WIRELESS_2G_PRIORITY
                    || network_strategy->m_network_strategy_type == WIFI_PRIORITY)
                {
                    if(COMMU_WIFI == network_strategy->m_current_type)
                    {
                        //连接超时
                        if(trendit_query_timer(network_strategy->m_beg_connect_ticks, network_strategy->m_wifi_tcp_con_try_overtime))
                        {
                            //尝试无线
                            if(wireless_module->check_network_active_success())
                            {
                                TRACE_INFO("wifi connect overtime, try wirelss commu:%d", network_strategy->m_wifi_tcp_con_try_overtime);
                                network_strategy->notify_device_dropline(COMMU_WIFI);
                                network_strategy->notify_device_online(COMMU_WIRELESS);
                                network_strategy->m_current_type = COMMU_WIRELESS;
                                network_strategy->m_wifi_net_state = TCP_NET_ERR;
                                tcp_protocol->restart();
                            }
                            
                            network_strategy->m_beg_connect_ticks = trendit_get_ticks();
                            error_model_instance()->notify_error(ERR_CONNECT_BACKEND_FAIL);
                        }
                    }

                    if(COMMU_WIRELESS == network_strategy->m_current_type)
                    {
                        if(trendit_query_timer(network_strategy->m_beg_connect_ticks, network_strategy->m_wireless_tcp_con_try_overtime))
                        {
                            //尝试wifi
                            if(wifi_module->check_network_connect_success())
                            {
                                TRACE_INFO("wireless connect overtime, try wifi commu");
                                network_strategy->notify_device_dropline(COMMU_WIRELESS);
                                network_strategy->notify_device_online(COMMU_WIFI);
                                network_strategy->m_current_type = COMMU_WIFI;
                                network_strategy->m_wireless_net_state = TCP_NET_ERR;
                                tcp_protocol->restart();
                            }

                            network_strategy->m_beg_connect_ticks = trendit_get_ticks();
                            error_model_instance()->notify_error(ERR_CONNECT_BACKEND_FAIL);
                        }
                    }

                    ret = MSG_KEEP;
                }
                break;
            }
            else if(CHECK_NETWORK == m_msg->m_lParam)
            {
                //判断是否有优先通讯方式之前物理网络连不上，现在ok了
                switch(network_strategy->m_network_strategy_type)
                {
                    case WIFI_PRIORITY:
                        if(COMMU_WIRELESS == network_strategy->m_current_type
                            && DEVICE_NET_ERR == network_strategy->m_wifi_net_state
                            && wifi_module->check_network_connect_success())
                        {
                            TRACE_INFO("wifi connect ok, then drop net retry it");
                            tcp_protocol->stop();
                            ddi_led_sta_set(LED_SIGNAL_B, 0);
                            ddi_led_sta_set(LED_SIGNAL_Y, 1);
                            network_strategy->m_online_reason = REASON_NET_CHANGE;
                            trendit_get_sys_time(network_strategy->m_last_drop_time, sizeof(network_strategy->m_last_drop_time), TIME_NO_FORMAT);
                            //audio_instance()->audio_play(AUDIO_NET_DROP, AUDIO_PLAY_BLOCK);
                            network_strategy->notify_device_dropline(COMMU_WIRELESS);
                            network_strategy->notify_network_drop();
                            error_model_instance()->notify_error(ERR_CONNECT_BACKEND_FAIL);
                            m_msg->m_status = MSG_INIT;
                            break;
                        }
                        break;
                        
                    case WIRELESS_2G_PRIORITY:
                        if(COMMU_WIFI == network_strategy->m_current_type
                            && DEVICE_NET_ERR == network_strategy->m_wireless_net_state
                            && wireless_module->check_network_active_success())
                        {
                            TRACE_INFO("wireless connect ok, then drop net retry it");
                            tcp_protocol->stop();
                            ddi_led_sta_set(LED_SIGNAL_B, 0);
                            ddi_led_sta_set(LED_SIGNAL_Y, 1);
                            network_strategy->m_online_reason = REASON_NET_CHANGE;
                            trendit_get_sys_time(network_strategy->m_last_drop_time, sizeof(network_strategy->m_last_drop_time), TIME_NO_FORMAT);
                            //audio_instance()->audio_play(AUDIO_NET_DROP, AUDIO_PLAY_BLOCK);
                            network_strategy->notify_device_dropline(COMMU_WIFI);
                            network_strategy->notify_network_drop();
                            error_model_instance()->notify_error(ERR_CONNECT_BACKEND_FAIL);
                            m_msg->m_status = MSG_INIT;
                            break;
                        }
                        break;
                }

                if(m_msg->m_status == MSG_INIT)
                {
                    ret = MSG_KEEP;
                    break;
                }
                
                //掉网策略
                switch(network_strategy->m_current_type)
                {
                    case COMMU_WIFI:
                        //wifi掉网
                        if(!wifi_module->check_network_connect_success())
                        {
                            tcp_protocol->stop();
                            ddi_led_sta_set(LED_SIGNAL_B, 0);
                            ddi_led_sta_set(LED_SIGNAL_Y, 0);
                            network_strategy->m_online_reason = REASON_NET_DROP;
                            trendit_get_sys_time(network_strategy->m_last_drop_time, sizeof(network_strategy->m_last_drop_time), TIME_NO_FORMAT);
                            //audio_instance()->audio_play(AUDIO_NET_DROP, AUDIO_PLAY_BLOCK);
                            network_strategy->notify_device_dropline(COMMU_WIFI);
                            network_strategy->notify_network_drop();
                            
                            //Add by xiaohonghui 2020.12.25 wifi掉网，如果无线连接，立马使用无线
                            if(wireless_module->check_network_active_success())
                            {
                                TRACE_INFO("detect wifi connect drop, change to wireless for it's online");
                                ddi_led_ioctl(DDI_LED_CTL_CLOSE_YELLOW, 0, 0);
                                ddi_led_sta_set(LED_SIGNAL_Y, 1);
                                network_strategy->m_current_type = COMMU_WIRELESS;
                                network_strategy->m_beg_connect_ticks = trendit_get_ticks();
                                m_msg->m_lParam = CHECK_RTC;

                                network_strategy->m_wifi_net_state = DEVICE_NET_ERR;
                                wifi_module->m_msg->m_status = MSG_INIT;
                                wifi_module->m_msg->m_lParam = WIFI_CHECK_MODEL;
                                wifi_module->m_network_state = FALSE;
                            }
                            else
                            {
                                TRACE_INFO("detect wifi connect drop");
                                m_msg->m_status = MSG_INIT;
                            }
                            error_model_instance()->notify_error(ERR_CONNECT_BACKEND_FAIL);
                            break;
                        }
                        break;

                    case COMMU_WIRELESS:
                        if(!wireless_module->check_network_active_success())
                        {
                            TRACE_INFO("detect wireless network drop");
                            tcp_protocol->stop();
                            ddi_led_sta_set(LED_SIGNAL_B, 0);
                            ddi_led_sta_set(LED_SIGNAL_Y, 0);
                            network_strategy->m_online_reason = REASON_NET_DROP;
                            trendit_get_sys_time(network_strategy->m_last_drop_time, sizeof(network_strategy->m_last_drop_time), TIME_NO_FORMAT);
                            //audio_instance()->audio_play(AUDIO_NET_DROP, AUDIO_PLAY_BLOCK);
                            network_strategy->notify_device_dropline(COMMU_WIRELESS);
                            network_strategy->notify_network_drop();
                            m_msg->m_status = MSG_INIT;
                            error_model_instance()->notify_error(ERR_CONNECT_BACKEND_FAIL);
                            break;
                        }
                        break;
                }

                if(MSG_INIT == m_msg->m_status || CHECK_RTC == m_msg->m_lParam)
                {
                    TRACE_DBG("out");
                    ret = MSG_KEEP;
                    break;
                }
                
                //掉线策略
                if(!tcp_protocol->check_tcp_connect_success())
                {
                    ddi_led_sta_set(LED_SIGNAL_B, 0);
                    ddi_led_sta_set(LED_SIGNAL_Y, 1);
                    network_strategy->m_online_reason = REASON_BACKEND_DENIED;
                    trendit_get_sys_time(network_strategy->m_last_drop_time, sizeof(network_strategy->m_last_drop_time), TIME_NO_FORMAT);
                    //audio_instance()->audio_play(AUDIO_NET_DROP, AUDIO_PLAY_BLOCK);
                    TRACE_INFO("tcp network disconnect");
                    network_strategy->m_beg_connect_ticks = trendit_get_ticks();
                    network_strategy->notify_network_drop();
                    error_model_instance()->notify_error(ERR_CONNECT_BACKEND_FAIL);
                    m_msg->m_lParam = TCP_CONNECT;
                    break;
                }
            }
            ret = MSG_KEEP;
            break;
        }
    }while(0);

    return ret;
}

static s32 network_strategy_post_msg(void)
{
    msg_t *network_strategy_msg = NULL;
    s32 ret = DDI_ERR;

    network_strategy_msg = (msg_t *)k_mallocapp(sizeof(msg_t));
    if(NULL == network_strategy_msg)
    {
        TRACE_ERR("malloc failed");
    }
    else
    {
        ret = DDI_OK;
        TRACE_DBG("network_strategy:%x", network_strategy_msg);
        network_strategy_instance()->m_msg = network_strategy_msg;
        memset(network_strategy_msg, 0, sizeof(msg_t));
        network_strategy_msg->m_func = network_strategy_msg_handle;
        network_strategy_msg->m_priority = MSG_NORMAL;
        network_strategy_msg->m_lParam = 0;
        network_strategy_msg->m_wparam = 0;
        network_strategy_msg->m_status = MSG_INIT;
        snprintf(network_strategy_msg->m_msgname, sizeof(network_strategy_msg->m_msgname), "%s", "network_strategy");
        trendit_postmsg(network_strategy_msg);
    }

    return ret;
}


static void network_strategy_start(void)
{
    wireless_module_t *wireless_module = NULL;
    wifi_module_t *wifi_module = NULL;

    wireless_module = wireless_instance();
    wifi_module = wifi_instance();

    switch(g_network_strategy->m_network_strategy_type)
    {
        case WIFI_PRIORITY:
        case WIRELESS_2G_PRIORITY:
            if(wireless_module)
            {
                wireless_module->post_wireless_msg();
            }

            if(wifi_module)
            {
                wifi_module->post_wifi_msg();
            }
            break;

        case WIFI_ONLY:
            if(wifi_module)
            {
                wifi_module->post_wifi_msg();
            }
            break;

        case WIRELESS_2G_ONLY:
            if(wireless_module)
            {
                wireless_module->post_wireless_msg();
            }
            break;
    }

    network_strategy_post_msg();
}

static void network_strategy_restart(void)
{
    wireless_module_t *wireless_module = NULL;
    wifi_module_t *wifi_module = NULL;

    wireless_module = wireless_instance();
    wifi_module = wifi_instance();

    ddi_led_sta_set(LED_SIGNAL_B, 0);
    ddi_led_sta_set(LED_SIGNAL_Y, 0);
    tcp_protocol_instance()->stop();

    TRACE_DBG("g_network_strategy->m_network_strategy_type:%d", g_network_strategy->m_network_strategy_type);
    switch(g_network_strategy->m_network_strategy_type)
    {
        case WIFI_PRIORITY:
        case WIRELESS_2G_PRIORITY:
            if(wireless_module)
            {
                wireless_module->m_msg->m_status = MSG_INIT;
            }

            if(wifi_module)
            {
                wifi_module->m_msg->m_status = MSG_INIT;
                wifi_module->m_msg->m_lParam = WIFI_CHECK_MODEL;
                wifi_module->m_network_state = FALSE;
            }
            break;

        case WIFI_ONLY:
            if(wifi_module)
            {
                wifi_module->m_msg->m_status = MSG_INIT;
                wifi_module->m_msg->m_lParam = WIFI_CHECK_MODEL;
                wifi_module->m_network_state = FALSE;
            }
            break;

        case WIRELESS_2G_ONLY:
            if(wireless_module)
            {
                wireless_module->m_msg->m_status = MSG_INIT;
            }
            break;
    }

    network_strategy_instance()->m_msg->m_status = MSG_INIT;
}

static void network_strategy_stop(void)
{
    wifi_module_t *wifi_module = wifi_instance();

    tcp_protocol_instance()->stop();
    if(wireless_instance())
    {
        wireless_instance()->terminal_wireless_msg();
    }
	
    if(wifi_module)
    {
        wifi_module->terminal_wifi_msg();
    }
    network_strategy_instance()->m_msg->m_status = MSG_TERMINAL;
}

/**
 * @brief 向网络控制层注册物理网络ok的callback
 * @param[in] 需回调的函数地址
 * @retval  无
 */
static void reg_net_device_online_callback(p_func_device_online_callback p_func)
{
    network_strategy_t *network_strategy = network_strategy_instance();

    if(p_func)
    {
        trendit_intqueue_enqueue(network_strategy->m_device_online_callback_func_list, (u32)p_func);
    }
}

/**
 * @brief 通知物理网络ok的callback
 * @param[in] commu_type 哪个通讯方式可以了
 * @retval  无
 */
static void notify_net_device_online(COMMU_TYPE commu_type)
{
    p_func_device_online_callback p_func;
    PNode pnode;
    s32 size = 0, i = 0;
    network_strategy_t *network_strategy = network_strategy_instance();

    pnode = network_strategy->m_device_online_callback_func_list->front;
    size = network_strategy->m_device_online_callback_func_list->size;

    TRACE_INFO("notify %d func %s online", size, commu_type==COMMU_WIFI?"wifi":"wireless");
    for(i=0; i<size; i++)
    {
        p_func = (p_func_device_online_callback)pnode->data;
        p_func(commu_type);
        
        pnode = pnode->next;
    }
}


/**
 * @brief 向网络控制层注册物理网络掉线的callback
 * @param[in] 需回调的函数地址
 * @retval  无
 */
static void reg_net_device_dropline_callback(p_func_device_dropline_callback p_func)
{
    network_strategy_t *network_strategy = network_strategy_instance();

    if(p_func)
    {
        trendit_intqueue_enqueue(network_strategy->m_device_dropline_callback_func_list, (u32)p_func);
    }
}

/**
 * @brief 通知物理掉线给注册的callback
 * @param[in] commu_type 哪个通讯方式掉线了
 * @retval  无
 */
static void notify_net_device_dropline(COMMU_TYPE commu_type)
{
    p_func_device_dropline_callback p_func;
    PNode pnode;
    s32 size = 0, i = 0;
    network_strategy_t *network_strategy = network_strategy_instance();

    pnode = network_strategy->m_device_dropline_callback_func_list->front;
    size = network_strategy->m_device_dropline_callback_func_list->size;

    TRACE_INFO("notify %d func %s dropline", size, commu_type==COMMU_WIFI?"wifi":"wireless");
    for(i=0; i<size; i++)
    {
        p_func = (p_func_device_dropline_callback)pnode->data;
        p_func(commu_type);
        
        pnode = pnode->next;
    }
}

/**
 * @brief 向网络控制层注册与后台联网成功的callback
 * @param[in] 需回调的函数地址
 * @retval  无
 */
static void reg_online_callback(p_func_online_callback p_func)
{
    network_strategy_t *network_strategy = network_strategy_instance();

    if(p_func)
    {
        trendit_intqueue_enqueue(network_strategy->m_online_callback_func_list, (u32)p_func);
    }
}

/**
 * @brief 通知与后台连接成功给注册的callback
 * @param[in] 无
 * @retval  无
 */
static void notify_online(void)
{
    p_func_online_callback p_func;
    PNode pnode;
    s32 size = 0, i = 0;
    network_strategy_t *network_strategy = network_strategy_instance();

    pnode = network_strategy->m_online_callback_func_list->front;
    size = network_strategy->m_online_callback_func_list->size;

    TRACE_INFO("notify %d func network witch backend online", size);
    for(i=0; i<size; i++)
    {
        p_func = (p_func_online_callback)pnode->data;
        p_func();
        
        pnode = pnode->next;
    }
}

/**
 * @brief 向网络控制层注册后台链路掉线的callback
 * @param[in] 需回调的函数地址
 * @retval  无
 */
static void reg_network_drop_callback(p_func_network_drop_callback p_func)
{
    network_strategy_t *network_strategy = network_strategy_instance();

    if(p_func)
    {
        trendit_intqueue_enqueue(network_strategy->m_drop_callback_func_list, (u32)p_func);
    }
}

/**
 * @brief 通知后台掉线给注册的callback
 * @param[in] 无
 * @retval  无
 */
static void notify_network_drop(void)
{
    p_func_network_drop_callback p_func;
    PNode pnode;
    s32 size = 0, i = 0;
    network_strategy_t *network_strategy = network_strategy_instance();

    pnode = network_strategy->m_drop_callback_func_list->front;
    size = network_strategy->m_drop_callback_func_list->size;

    TRACE_INFO("notify %d func network witch backend dropline", size);
    for(i=0; i<size; i++)
    {
        p_func = (p_func_network_drop_callback)pnode->data;
        p_func();
        
        pnode = pnode->next;
    }
}

network_strategy_t *network_strategy_instance(void)
{
    if(g_network_strategy)
    {
        return g_network_strategy;
    }

    g_network_strategy = k_mallocapp(sizeof(network_strategy_t));
    if(NULL == g_network_strategy)
    {
        TRACE_ERR("malloc fail");
        return NULL;
    }
    
    memset(g_network_strategy, 0, sizeof(network_strategy_t));
    g_network_strategy->start = network_strategy_start;
    g_network_strategy->stop = network_strategy_stop;
    g_network_strategy->restart = network_strategy_restart;
    g_network_strategy->m_device_online_callback_func_list = trendit_intqueue_create();
    g_network_strategy->m_device_dropline_callback_func_list = trendit_intqueue_create();
    g_network_strategy->m_online_callback_func_list = trendit_intqueue_create();
    g_network_strategy->m_drop_callback_func_list = trendit_intqueue_create();

    //物理网络ok或掉线后注册函数及通知函数
    g_network_strategy->reg_device_online_callback = reg_net_device_online_callback;
    g_network_strategy->notify_device_online = notify_net_device_online;
    g_network_strategy->reg_device_dropline_callback = reg_net_device_dropline_callback;
    g_network_strategy->notify_device_dropline = notify_net_device_dropline;

    //与后台连接ok或掉线后注册函数及通知函数
    g_network_strategy->reg_online_callback = reg_online_callback;
    g_network_strategy->notify_online = notify_online;
    g_network_strategy->reg_network_drop_callback = reg_network_drop_callback;
    g_network_strategy->notify_network_drop = notify_network_drop;

    
    return g_network_strategy;
}

