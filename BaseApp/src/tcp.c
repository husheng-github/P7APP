#include "ddiglobal.h"
#include "app_global.h"

#define SERVER_IP    "printer.juhesaas.com"
#define SERVER_PORT  8080
static tcp_protocol_module_t *g_tcp_protocol_module;
static s32 g_back_param_modify = FALSE;
    
/**
 * @brief 初始化本文件所有的全局变量
 */
void trendit_init_tcpdata(void)
{
    g_tcp_protocol_module = NULL;
}

static s32 socket_create(u8 type, u8 mode, u8 *param, u16 port)
{
    network_strategy_t *network_strategy = network_strategy_instance();
    
    if(COMMU_WIFI == network_strategy->m_current_type)
    {
        return ddi_wifi_socket_create_start(type, mode, param, port);
    }
    else if(COMMU_WIRELESS == network_strategy->m_current_type)
    {
        return ddi_gprs_socket_create(type, mode, param, port);
    }
}

static s32 socket_get_connect_state(s32 socket_id)
{
    network_strategy_t *network_strategy = network_strategy_instance();
    s32 ret = 0;
    
    if(COMMU_WIFI == network_strategy->m_current_type)
    {
        return ddi_wifi_socket_get_status(socket_id);
    }
    else if(COMMU_WIRELESS == network_strategy->m_current_type)
    {
        ret = ddi_gprs_socket_get_status(socket_id);
        ret = (ret == GPRS_STATUS_NONE) ? GPRS_STATUS_CONNECTING : ret;
        return ret;
    }
}

static s32 socket_recv(s32 socketid, u8 *rbuf, s32 rlen)
{
    network_strategy_t *network_strategy = network_strategy_instance();
    
    if(COMMU_WIFI == network_strategy->m_current_type)
    {
        return ddi_wifi_socket_recv(socketid, rbuf, rlen);
    }
    else if(COMMU_WIRELESS == network_strategy->m_current_type)
    {
        return ddi_gprs_socket_recv(socketid, rbuf, rlen);
    }
}

static s32 socket_send(u8 *wbuf, s32 wlen)
{
    network_strategy_t *network_strategy = network_strategy_instance();
    tcp_protocol_module_t *tcp_protocol_module = tcp_protocol_instance();
    s32 ret = 0;
    
    if(COMMU_WIFI == network_strategy->m_current_type)
    {
        ret = ddi_wifi_socket_send(tcp_protocol_module->m_socketid, wbuf, wlen);
    }
    else if(COMMU_WIRELESS == network_strategy->m_current_type)
    {
        ret = ddi_gprs_socket_send(tcp_protocol_module->m_socketid, wbuf, wlen);
    }

    TRACE_INFO("ret:%d", ret);
}

static s32 socket_close(s32 socket_id)
{
    network_strategy_t *network_strategy = network_strategy_instance();
    s32 ret = 0;
    
    if(COMMU_WIFI == network_strategy->m_current_type)
    {
        return ddi_wifi_socket_close(socket_id);
    }
    else if(COMMU_WIRELESS == network_strategy->m_current_type)
    {
        return ddi_gprs_socket_close(socket_id);
    }
}

static s32 tcp_msg_service_handle(struct _msg *m_msg)
{
    s32 ret = DDI_ERR;
    u8 buff[1024] = {0};
    tcp_protocol_module_t *tcp_protocol_module = tcp_protocol_instance();
    
    switch(tcp_protocol_module->m_state)
    {
        case FALSE:
            if(!trendit_query_timer(tcp_protocol_module->m_thread_sleep_ticks, TCP_CHECK_CONNECT_SLEEP_TIME))
            {
                ret = MSG_KEEP;
                break;
            }
            else
            {
                tcp_protocol_module->m_thread_sleep_ticks = trendit_get_ticks();
            }
            
            ret = socket_get_connect_state(tcp_protocol_module->m_socketid);
            if(WIFI_SOCKET_STATUS_CONNECTING == ret)
            {
                if(trendit_query_timer(tcp_protocol_module->m_beg_con_time, tcp_protocol_module->get_tcp_connect_overtime()))
                {
                    TRACE_INFO("connect tcp overtime, then retry:%d", tcp_protocol_module->m_socketid);
                    socket_close(tcp_protocol_module->m_socketid);
                    ret = MSG_KEEP;
                    m_msg->m_status = MSG_INIT;
                    break;
                }
                else
                {
                    ret = MSG_KEEP;
                    break;
                }
            }
            else if(WIFI_SOCKET_STATUS_CONNECTED == ret)
            {
                TRACE_INFO("connect server success");
                printer_instance()->init_last_print_packno();
                tcp_protocol_module->m_state = TRUE;
                ret = MSG_KEEP;
                break;
            }
            else
            {
                TRACE_INFO("connect server fail:%d, then retry", ret);
                socket_close(tcp_protocol_module->m_socketid);
                ret = MSG_KEEP;
                m_msg->m_status = MSG_INIT;
                break;
            }
            break;

        case TRUE:
            ret = socket_get_connect_state(tcp_protocol_module->m_socketid);
            if(ret != GPRS_STATUS_CONNECTED)
            {
                TRACE_ERR("tcp connect drop");
                tcp_protocol_module->m_state = FALSE;
                socket_close(tcp_protocol_module->m_socketid);
                tcp_protocol_module->m_socketid = 0;
                ret = MSG_KEEP;
                m_msg->m_status = MSG_INIT;
                break;
            }

            memset(buff, 0, sizeof(buff));
            ret = socket_recv(tcp_protocol_module->m_socketid, buff, sizeof(buff));
            if(ret > 0)
            {
                TRACE_INFO("recv data len:%d", ret);
                data_parser_instance()->handle_data(DATA_SOURCE_TCP, buff, ret, tcp_protocol_module->m_socketid);
            }
            ret = MSG_KEEP;
            break;
    }

    return ret;
}

static s32 tcp_msg_handle(struct _msg *m_msg)
{
    s32 ret = MSG_KEEP;
    s32 socketid = 0;
    tcp_protocol_module_t *tcp_protocol_module = tcp_protocol_instance();

    do{
        if(MSG_TERMINAL == m_msg->m_status)
        {
            if(tcp_protocol_module->m_socketid > 0)
            {
                socket_close(tcp_protocol_module->m_socketid);
                tcp_protocol_module->m_socketid = -1;
            }

            TRACE_DBG("terminal tcp msg");
            ret = MSG_OUT;
            break;
        }
        else if(MSG_INIT == m_msg->m_status)
        {
            if(!trendit_query_timer(tcp_protocol_module->m_thread_sleep_ticks, TCP_CREATE_SOCKET_SLEEP_TIME))
            {
                ret = MSG_KEEP;
                break;
            }
            else
            {
                tcp_protocol_module->m_thread_sleep_ticks = trendit_get_ticks();
            }

            //后台参数修改时，重新创建socket连接---启动时调用了服务器ip和端口配置的接口，变量置为了1
            if(g_back_param_modify)
            {
                g_back_param_modify = FALSE;
            }
            
            socketid = socket_create(SOCKET_TYPE_TCP, HOST_TYPE_IPADDR, tcp_protocol_module->m_serverinfor, tcp_protocol_module->m_serverport);
            if(socketid < 0)
            {
                tcp_protocol_module->m_thread_sleep_ticks = trendit_get_ticks();
                TRACE_ERR("creat socket failed, then retry:%d, %s", socketid, network_strategy_instance()->m_current_type==COMMU_WIFI?"wifi":"wireless");
                break;
            }
            
            m_msg->m_status = MSG_SERVICE;
            tcp_protocol_module->m_state = FALSE;
            tcp_protocol_module->m_socketid = socketid;
            tcp_protocol_module->m_beg_con_time = trendit_get_ticks();
            tcp_protocol_module->m_thread_sleep_ticks = trendit_get_ticks();
            ret = MSG_KEEP;
            TRACE_INFO("begin to connect server:%s,%d, socketid:%x, cur_commu_type:%s",
                                            tcp_protocol_module->m_serverinfor, 
                                            tcp_protocol_module->m_serverport, 
                                            socketid,
                                            network_strategy_instance()->m_current_type==COMMU_WIFI?"wifi":"wireless");
            break;
        }
        else
        {
            //后台参数修改时，重新创建socket连接
            if(g_back_param_modify)
            {
                g_back_param_modify = FALSE;
                socket_close(tcp_protocol_module->m_socketid);
                tcp_protocol_module->m_socketid = -1;

                m_msg->m_status = MSG_INIT;
                ret = MSG_KEEP;
                break;
            }
            ret = tcp_msg_service_handle(m_msg);
        }
    }while(0);

    return ret;
}


/**
 * @brief 开启tcp协议模块，物理网络上线后调用
 * @retval  DDI_OK 内存申请失败
 * @retval  DDI_EOVERFLOW 内存申请失败
 */
static s32 tcp_protocol_start(void)
{
    msg_t *tcp_connect_msg = NULL;
    s32 ret = DDI_ERR;
    tcp_protocol_module_t *tcp_protocol_module = tcp_protocol_instance();

    tcp_connect_msg = (msg_t *)k_mallocapp(sizeof(msg_t));
    if(NULL == tcp_connect_msg)
    {
        TRACE_ERR("malloc failed");
        ret = DDI_EOVERFLOW;
    }
    else
    {
        TRACE_DBG("tcp_protocol_module:%x %d", tcp_protocol_module, network_strategy_instance()->m_current_type);
        snprintf(tcp_connect_msg->m_msgname, sizeof(tcp_connect_msg->m_msgname), "%s", "tcp");
        tcp_protocol_module->m_msg = tcp_connect_msg;
        tcp_connect_msg->m_func = tcp_msg_handle;
        tcp_connect_msg->m_priority = MSG_NORMAL;
        tcp_connect_msg->m_lParam = 0;
        tcp_connect_msg->m_wparam = 0;
        tcp_connect_msg->m_status = MSG_INIT;
        trendit_postmsg(tcp_connect_msg);
        ret = DDI_OK;
    }

    return ret;
}

/**
 * @brief 重启tcp协议模块，后台ip、端口修改时调用
 * @retval  无
 */
static void tcp_protocol_restart(void)
{
    tcp_protocol_module_t *tcp_protocol_module = tcp_protocol_instance();

    tcp_protocol_module->m_state = FALSE;
    if(tcp_protocol_module->m_socketid > 0)
    {
        socket_close(tcp_protocol_module->m_socketid);
        tcp_protocol_module->m_socketid = -1;
    }

    if(tcp_protocol_instance()->m_msg)
    {
        tcp_protocol_instance()->m_msg->m_status = MSG_INIT;
    }
    
    TRACE_INFO("restart tcp protocol, current commu type:%s", network_strategy_instance()->m_current_type==COMMU_WIFI?"wifi":"wireless");
}

/**
 * @brief 停止tcp协议模块，物理网络掉线时使用
 * @retval  无
 */
static void tcp_protocol_stop(void)
{
    tcp_protocol_module_t *tcp_protocol_module = tcp_protocol_instance();

    tcp_protocol_module->m_state = FALSE;
    if(tcp_protocol_module->m_msg)
    {
        socket_close(tcp_protocol_module->m_socketid);
        tcp_protocol_module->m_msg->m_status = MSG_TERMINAL;
    }
}

/**
 * @brief 获取tcp三次握手是否成功
 * @retval  1：成功
 * @retval  0：还没连成功
 */
static s32 check_tcp_connect_success(void)
{
    tcp_protocol_module_t *tcp_protocol_module = tcp_protocol_instance();
    
    return TRUE == tcp_protocol_module->m_state;
}

/**
 * @brief 获取连接服务器信息
 * @param[in] server_ip   填充后台地址的buffer
 * @param[in] server_ip_len   填充后台地址的buffer长度
 * @param[in] server_port 连接端口
 * @param[in] commu_type_string 用来填充当前通讯方式的buffer，长度至少为16个字节
 * @retval  DDI_OK   成功
 * @retval  DDI_EINVAL   参数为空
 */
s32 get_server_info(cur_server_info_t *cur_server_info)
{
    s32 ret = DDI_ERR;
    tcp_protocol_module_t *tcp_protocol_module = tcp_protocol_instance();
    wireless_net_info_t wireless_net_info;
    wifi_net_info_t wifi_net_info;

    do{
        if(NULL == cur_server_info)
        {
            ret = DDI_EINVAL;
            break;
        }

        if(COMMU_WIRELESS == network_strategy_instance()->m_current_type)
        {
            if(DDI_OK == ddi_gprs_ioctl(DDI_GPRS_GET_NET_INFO, (u32)&wireless_net_info, 0))
            {
                snprintf(cur_server_info->m_local_ip, sizeof(cur_server_info->m_local_ip), "%s", wireless_net_info.m_localip);
            }
        }
        else if(COMMU_WIFI == network_strategy_instance()->m_current_type)
        {
            if(DDI_OK == ddi_wifi_ioctl(WIFI_IOCTL_GET_NET_INFO, (u32)&wifi_net_info, 0))
            {
                snprintf(cur_server_info->m_local_ip, sizeof(cur_server_info->m_local_ip), "%s", wifi_net_info.m_localip);
            }
        }
        
        snprintf(cur_server_info->m_serverinfo, sizeof(tcp_protocol_module->m_serverinfor), "%s", tcp_protocol_module->m_serverinfor);
        cur_server_info->m_server_port = tcp_protocol_module->m_serverport;
        snprintf(cur_server_info->m_cur_commu_type, sizeof(cur_server_info->m_cur_commu_type), "%s", 
                                                    network_strategy_instance()->m_current_type==COMMU_WIFI?"wifi":"wireless");

        ret = DDI_OK;
    }while(0);

    return ret;
}

/**
 * @brief 配置服务器ip和端口----usb或后台可修改
 * @param[in] ipaddr 后台ip或域名
 * @param[in] port 后台端口
 * @retval  DDI_OK 成功
 * @retval  DDI_EINVAL 参数异常
 */
static s32 cfg_server_info(s8 *ipaddr, s32 port)
{
    s32 ret = DDI_ERR;

    do{
        if(NULL == ipaddr || 0 == port)
        {
            ret = DDI_EINVAL;
            break;
        }

        snprintf(g_tcp_protocol_module->m_serverinfor, sizeof(g_tcp_protocol_module->m_serverinfor), "%s", ipaddr);
        g_tcp_protocol_module->m_serverport = port;

        TRACE_DBG("%s, %d", g_tcp_protocol_module->m_serverinfor, g_tcp_protocol_module->m_serverport);

        g_back_param_modify = 1;
        ret = DDI_OK;
    }while(0);

    return ret;
}

/**
 * @brief 内部函数，依据通讯方式获取tcp三次握手的超时时间
 * @retval  配置的tcp三次握手超时时间，单位毫秒
 */
static u32 get_tcp_connect_overtime(void)
{
    network_strategy_t *network_strategy = network_strategy_instance();
    s32 ret = 0;
    
    if(COMMU_WIFI == network_strategy->m_current_type)
    {
        return tcp_protocol_instance()->m_wifi_tcp_connect_overtime;
    }
    else if(COMMU_WIRELESS == network_strategy->m_current_type)
    {
        return tcp_protocol_instance()->m_wireless_tcp_connect_overtime;
    }
}

/**
 * @brief 内部函数，物理网络掉线，网络控制层调用的callback
 * @retval  无
 */
static void network_drop_callback(void)
{
    TRACE_INFO("network notify drop event");
    memset(g_tcp_protocol_module->m_local_ip, 0, sizeof(g_tcp_protocol_module->m_local_ip));
}

/**
 * @brief 内部函数，物理网络上线，网络控制层调用的callback
 * @retval  无
 */
static void network_online_callback(void)
{
    network_strategy_t *network_strategy = network_strategy_instance();
    wireless_net_info_t wireless_net_info;
    wifi_net_info_t wifi_net_info;
    s32 ret = DDI_ERR;

    if(COMMU_WIFI == network_strategy->m_current_type)
    {
        memset(&wifi_net_info, 0, sizeof(wifi_net_info));
        ret = ddi_wifi_ioctl(WIFI_IOCTL_GET_NET_INFO, (u32)&wifi_net_info, 0);
        if(DDI_OK == ret)
        {
            TRACE_INFO("network notify online event, wifi local ip:%s", wifi_net_info.m_localip);
            snprintf(tcp_protocol_instance()->m_local_ip, sizeof(tcp_protocol_instance()->m_local_ip), 
                                    "%s", wifi_net_info.m_localip);
            
        }
    }
    else if(COMMU_WIRELESS == network_strategy->m_current_type)
    {
        memset(&wireless_net_info, 0, sizeof(wireless_net_info));
        ret = ddi_gprs_ioctl(DDI_GPRS_GET_NET_INFO, (u32)&wireless_net_info, 0);
        if(DDI_OK == ret)
        {
            TRACE_INFO("network notify online event, wireless local ip:%s", wireless_net_info.m_localip);
            snprintf(tcp_protocol_instance()->m_local_ip, sizeof(tcp_protocol_instance()->m_local_ip), 
                                    "%s", wireless_net_info.m_localip);
        }
    }
}


tcp_protocol_module_t *tcp_protocol_instance(void)
{
    if(g_tcp_protocol_module)
    {
        return g_tcp_protocol_module;
    }

    g_tcp_protocol_module = k_mallocapp(sizeof(tcp_protocol_module_t));
    if(NULL == g_tcp_protocol_module)
    {
        TRACE_ERR("malloc fail");
        return NULL;
    }
    
    memset(g_tcp_protocol_module, 0, sizeof(tcp_protocol_module_t));
    g_tcp_protocol_module->start = tcp_protocol_start;
    g_tcp_protocol_module->stop = tcp_protocol_stop;
    g_tcp_protocol_module->restart = tcp_protocol_restart;
    g_tcp_protocol_module->send = socket_send;
    g_tcp_protocol_module->check_tcp_connect_success = check_tcp_connect_success;
    g_tcp_protocol_module->cfg_server_info = cfg_server_info;
    g_tcp_protocol_module->get_server_info = get_server_info;
    g_tcp_protocol_module->get_tcp_connect_overtime = get_tcp_connect_overtime;
    network_strategy_instance()->reg_network_drop_callback(network_drop_callback);

    //todo for test
    snprintf(g_tcp_protocol_module->m_serverinfor, sizeof(g_tcp_protocol_module->m_serverinfor), "%s", SERVER_IP);
    g_tcp_protocol_module->m_serverport = SERVER_PORT;
    g_tcp_protocol_module->m_wifi_tcp_connect_overtime = 10*1000;
    g_tcp_protocol_module->m_wireless_tcp_connect_overtime = 25*1000;

    return g_tcp_protocol_module;
}

