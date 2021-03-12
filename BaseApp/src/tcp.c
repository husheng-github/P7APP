#include "ddiglobal.h"
#include "app_global.h"

#define SERVER_IP    "printer.juhesaas.com"
#define SERVER_PORT  8080
static tcp_protocol_module_t *g_tcp_protocol_module;
static s32 g_back_param_modify = FALSE;
    
/**
 * @brief ��ʼ�����ļ����е�ȫ�ֱ���
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

            //��̨�����޸�ʱ�����´���socket����---����ʱ�����˷�����ip�Ͷ˿����õĽӿڣ�������Ϊ��1
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
            //��̨�����޸�ʱ�����´���socket����
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
 * @brief ����tcpЭ��ģ�飬�����������ߺ����
 * @retval  DDI_OK �ڴ�����ʧ��
 * @retval  DDI_EOVERFLOW �ڴ�����ʧ��
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
 * @brief ����tcpЭ��ģ�飬��̨ip���˿��޸�ʱ����
 * @retval  ��
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
 * @brief ֹͣtcpЭ��ģ�飬�����������ʱʹ��
 * @retval  ��
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
 * @brief ��ȡtcp���������Ƿ�ɹ�
 * @retval  1���ɹ�
 * @retval  0����û���ɹ�
 */
static s32 check_tcp_connect_success(void)
{
    tcp_protocol_module_t *tcp_protocol_module = tcp_protocol_instance();
    
    return TRUE == tcp_protocol_module->m_state;
}

/**
 * @brief ��ȡ���ӷ�������Ϣ
 * @param[in] server_ip   ����̨��ַ��buffer
 * @param[in] server_ip_len   ����̨��ַ��buffer����
 * @param[in] server_port ���Ӷ˿�
 * @param[in] commu_type_string ������䵱ǰͨѶ��ʽ��buffer����������Ϊ16���ֽ�
 * @retval  DDI_OK   �ɹ�
 * @retval  DDI_EINVAL   ����Ϊ��
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
 * @brief ���÷�����ip�Ͷ˿�----usb���̨���޸�
 * @param[in] ipaddr ��̨ip������
 * @param[in] port ��̨�˿�
 * @retval  DDI_OK �ɹ�
 * @retval  DDI_EINVAL �����쳣
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
 * @brief �ڲ�����������ͨѶ��ʽ��ȡtcp�������ֵĳ�ʱʱ��
 * @retval  ���õ�tcp�������ֳ�ʱʱ�䣬��λ����
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
 * @brief �ڲ�����������������ߣ�������Ʋ���õ�callback
 * @retval  ��
 */
static void network_drop_callback(void)
{
    TRACE_INFO("network notify drop event");
    memset(g_tcp_protocol_module->m_local_ip, 0, sizeof(g_tcp_protocol_module->m_local_ip));
}

/**
 * @brief �ڲ������������������ߣ�������Ʋ���õ�callback
 * @retval  ��
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

