#include "ddiglobal.h"
#include "app_global.h"

static machine_error_model_t *g_error_model = NULL;
static machine_error_table_t machine_error_table[ERR_MAX];

/**
 * @brief ��ʼ�����ļ����е�ȫ�ֱ���
 */
void trendit_init_errordata()
{
    s32 len = sizeof(machine_error_table[0].m_error_description);
    
    g_error_model = NULL;

    snprintf(machine_error_table[ERR_OUT_OF_PAPAER].m_error_description, len, "%s", AUDIO_DEVICE_OUT_PAPER);
    snprintf(machine_error_table[ERR_NO_SIM_CARD].m_error_description, len, "%s", AUDIO_NO_SIMCARD);
    snprintf(machine_error_table[ERR_SIMCARD_NO_TRAFFIC].m_error_description, len, "%s", AUDIO_SIMCARD_NO_DATA);
    snprintf(machine_error_table[ERR_WIRELESS_REG_NET_DENIED].m_error_description, len, "%s", AUDIO_WIRELESS_REG_NET_DENIED);
    snprintf(machine_error_table[ERR_WIRELESS_REGING].m_error_description, len, "%s", ERROR_WIRELESS_REGING);
    snprintf(machine_error_table[ERR_WIRELESS_ACTIVING_NET].m_error_description, len, "%s", ERROR_WIRELESS_ACTIVING);
    snprintf(machine_error_table[ERR_WIFI_CONNECT_AP_FAIL].m_error_description, len, "%s", ERROR_DES_CONNECT_AP_FAIL);
    snprintf(machine_error_table[ERR_NO_WIRELESS_MODEL].m_error_description, len, "%s", AUDIO_NO_WIRELESS_MODEL);
    snprintf(machine_error_table[ERR_WIFI_AP_DO_NOT_CONFIG].m_error_description, len, "%s", ERROR_DES_WIFI_AP_DO_NOT_CONFIG);
    snprintf(machine_error_table[ERR_CONNECT_BACKEND_FAIL].m_error_description, len, "%s", ERROR_DES_TCP_CONNECT_FAIL);
}

/**
 * @brief �ṩ���ⲿ�׳��쳣�ĺ���
 * @param[in] machine_error_code �쳣��
 * @retval  DDI_OK �ɹ�
 * @retval  DDI_ERR ʧ��
 */
static s32 notify_error(MACHINE_ERROR_CODE_E machine_error_code)
{
    s32 ret = DDI_ERR;
    machine_error_model_t *machine_error_model = error_model_instance();
    PNode pnode;
    s32 i = 0;
    s32 size = 0;
    tcp_protocol_module_t *tcp_protocol_module = tcp_protocol_instance();
    s32 flag = 0;

    switch(machine_error_code)
    {
        case ERR_NO_SIM_CARD:
        case ERR_SIMCARD_NO_TRAFFIC:
        case ERR_WIRELESS_REG_NET_DENIED:
        case ERR_WIRELESS_REGING:
        case ERR_WIRELESS_ACTIVING_NET:
        case ERR_WIFI_CONNECT_AP_FAIL:
        case ERR_WIFI_AP_DO_NOT_CONFIG:
        case ERR_CONNECT_BACKEND_FAIL:
            //���̨��������ʱ�������������쳣����
            if(tcp_protocol_module && tcp_protocol_module->check_tcp_connect_success())
            {
                return DDI_OK;
            }
            flag = 1;
            break;
    }
    
    if(machine_error_model->m_error_queue)
    {        
        pnode = machine_error_model->m_error_queue->front;
        size = machine_error_model->m_error_queue->size;

        for(i=0; i<size; i++)
        {
            //�Ѵ��ڲ��ò���
            if(pnode->data == machine_error_code)
            {
                //TRACE_INFO("error code:%d exist, so ignore it", machine_error_code);
                ret = DDI_OK;
                break;
            }
            
            pnode = pnode->next;
        }

        //������
        if(DDI_OK != ret)
        {
            TRACE_INFO("error code:%d don't exist, then insert queue", machine_error_code);
            trendit_intqueue_enqueue(machine_error_model->m_error_queue, machine_error_code);
            ret = DDI_OK;
        }
    }

    if(DDI_OK == ret)
    {
        if(!flag)  //������쳣������LED
        {
            ddi_led_sta_set(LED_STATUS_B, 0);
            ddi_led_sta_set(LED_STATUS_R, 1);
        }
    }

    return ret;
}

/**
 * @brief �ṩ���ڲ������쳣�ĺ���
 * @param[in] machine_error_code �쳣��
 * @retval  DDI_OK �ɹ�
 * @retval  DDI_ERR ʧ��
 */
static s32 clear_error(MACHINE_ERROR_CODE_E machine_error_code)
{
    s32 ret = DDI_ERR;
    machine_error_model_t *machine_error_model = error_model_instance();

    if(machine_error_model->m_error_queue)
    {
        ret = trendit_intqueue_delnode(machine_error_model->m_error_queue, machine_error_code);
    }

    /*TRACE_DBG("size:%d, %x, %x", machine_error_model->m_error_queue->size,
                                 machine_error_model->m_error_queue->front,
                                 machine_error_model->m_error_queue->rear);*/
    if(trendit_intqueue_is_empty(machine_error_model->m_error_queue))
    {
        ddi_led_sta_set(LED_STATUS_R, 0);
        ddi_led_sta_set(LED_STATUS_B, 1);
    }
    return ret;
}

/**
 * @brief ��������ok��Ļص�����������ص��쳣
 * @param[in] ��
 * @retval  ��
 */
static void net_device_online_callback_to_errsystem(COMMU_TYPE commu_type)
{
    switch(commu_type)
    {
        case COMMU_WIRELESS:
            clear_error(ERR_NO_SIM_CARD);
            clear_error(ERR_SIMCARD_NO_TRAFFIC);
            clear_error(ERR_WIRELESS_REG_NET_DENIED);
            clear_error(ERR_WIRELESS_REGING);
            clear_error(ERR_WIRELESS_ACTIVING_NET);
            break;

        case COMMU_WIFI:
            clear_error(ERR_WIFI_CONNECT_AP_FAIL);
            clear_error(ERR_WIFI_AP_DO_NOT_CONFIG);
            break;    
    }
}

/**
 * @brief ���̨���ӳɹ����֪ͨ��������������ص��쳣
 * @param[in] ��
 * @retval  ��
 */
static void online_callback_to_errsystem(void)
{
    clear_error(ERR_NO_SIM_CARD);
    clear_error(ERR_SIMCARD_NO_TRAFFIC);
    clear_error(ERR_WIRELESS_REG_NET_DENIED);
    clear_error(ERR_WIRELESS_REGING);
    clear_error(ERR_WIRELESS_ACTIVING_NET);
    clear_error(ERR_NO_WIRELESS_MODEL);
    clear_error(ERR_WIFI_CONNECT_AP_FAIL);
    clear_error(ERR_WIFI_AP_DO_NOT_CONFIG);
    clear_error(ERR_CONNECT_BACKEND_FAIL);
}

/**
 * @brief �ṩ���ⲿ��ӡ��ǰ�쳣�Ľӿ�
 * @retval  DDI_OK �����쳣��Ϣ����ӡ�ɹ�
 * @retval  DDI_ERR �������쳣��Ϣ
 */
static s32 print_error_msg(void)
{
    machine_error_model_t *machine_error_model = error_model_instance();
    tcp_protocol_module_t *tcp_protocol_module = tcp_protocol_instance();
    s32 ret = DDI_ERR;
    s8 tmp_buff[2048] = {0};
    s8 buff[1024] = {0};
    s32 len = 0;
    s32 i = 0;
    PNode pnode;
    s32 size = 0;
    
    //��Ҫ���쳣---����
    if(tcp_protocol_module->check_tcp_connect_success())
    {
        online_callback_to_errsystem();
    }

    do{
        //��Ҫ���쳣---ȱֽ
        if(!printer_instance()->check_paper())
        {
            clear_error(ERR_OUT_OF_PAPAER);
        }
        else    //����ȱֽ
        {
            audio_instance()->audio_play(AUDIO_DEVICE_OUT_PAPER, AUDIO_PLAY_BLOCK);
            ret = DDI_OK;
            break;
        }

        if(machine_error_model->m_error_queue)
        {
            if(trendit_intqueue_is_empty(machine_error_model->m_error_queue))
            {
                ret = DDI_ERR;
            }
            else
            {
                memset(tmp_buff, 0, sizeof(tmp_buff));
                memcpy(tmp_buff, "\x1B\x61\x01", 3);//����
                len += 3;

                memcpy(tmp_buff+len, "\x1D\x21\x11", 3); //�Ŵ�һ��
                len += 3;
                snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "״̬��Ϣ\r\n");
                len += strlen("״̬��Ϣ\r\n");
                
                memcpy(tmp_buff+len, "\x1D\x21\x00", 3); //��ԭ
                len += 3;

                memcpy(tmp_buff+len, "\x1B\x61\x00", 3);//����
                len += 3;

                pnode = machine_error_model->m_error_queue->front;
                size = machine_error_model->m_error_queue->size;
                for(i=0; i<size; i++)
                {
                    if(ERR_OUT_OF_PAPAER != pnode->data)
                    {
                        snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s\r\n", machine_error_table[pnode->data].m_error_description);
                        len += strlen(machine_error_table[pnode->data].m_error_description)+2;
                        pnode = pnode->next;
                    }
                }

                snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", "\r\n\r\n\r\n");
                len +=6;

                printer_instance()->direct_print(tmp_buff, len);
                ret = DDI_OK;
            }
        }
    }while(0);

    return ret;
}

machine_error_model_t *error_model_instance(void)
{
    if(g_error_model)
    {
        return g_error_model;
    }

    g_error_model = k_mallocapp(sizeof(machine_error_model_t));
    if(NULL == g_error_model)
    {
        TRACE_ERR("malloc fail");
        return NULL;
    }
    
    memset(g_error_model, 0, sizeof(machine_error_model_t));
    g_error_model->m_error_queue = trendit_intqueue_create();
    g_error_model->notify_error = notify_error;
    g_error_model->print_error_msg = print_error_msg;
    g_error_model->clear_error = clear_error;

    network_strategy_instance()->reg_device_online_callback(net_device_online_callback_to_errsystem);
    network_strategy_instance()->reg_online_callback(online_callback_to_errsystem);

    return g_error_model;
}


