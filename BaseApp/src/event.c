#include "ddiglobal.h"
#include "app_global.h"

static event_model_t *g_event_model = NULL;

/**
 * @brief ��ʼ�����ļ����е�ȫ�ֱ���
 */
void trendit_init_event_data(void)
{
    g_event_model = NULL;
}

/**
 * @brief ��list�����cmd�Ƿ���ڣ����ڸ��½�����ݣ� �����ڴ���
 * @param[in] cmd �¼�������
 * @param[in] value_string �¼���Ӧ����������
 * @param[in] value_string_len �¼���Ӧ���������ݶ�Ӧ�ĳ���
 * @retval  DDI_OK  �ɹ�
 * @retval  DDI_ERR ʧ��
 */
static s32 cfg_event_to_list(u16 cmd, u8 *value_string, u16 value_stringlen)
{
    event_node_t *event_node;
    s32 ret = DDI_ERR;
    s8 dst[10][SPLIT_PER_MAX] = {0};
    s32 j = 0;
    u32 value = 0;
    pnode_t pnode = g_event_model->m_event_list->front;
    s32 i = g_event_model->m_event_list->size;

    //����cmd�Ƿ����
    while(i--)
    {
        event_node = (event_node_t *)pnode->data;

        if(cmd == event_node->m_event_code)
        {
            ret = DDI_OK;
            break;
        }
        pnode = pnode->next;
    }

    do{
        if(DDI_OK == ret)
        {
            TRACE_INFO("exist %04x, clear list", cmd);
            trendit_intqueue_clear(event_node->m_list);
        }
        else   //������
        {
            TRACE_INFO("not exist %04x, create list", cmd);
            event_node = (event_node_t *)k_mallocapp(sizeof(event_node_t));
            if(NULL == event_node)
            {
                TRACE_ERR("malloc failed");
                ret = DDI_EOVERFLOW;
                break;
            }

            event_node->m_event_code = cmd;
            event_node->m_list = trendit_intqueue_create();
            if(!event_node->m_list)
            {
                TRACE_ERR("malloc failed");
                k_freeapp(event_node);
                ret = DDI_EOVERFLOW;
                break;
            }

            trendit_queue_entry(g_event_model->m_event_list, (void *)event_node);
            ret = DDI_OK;
         }
    }while(0);

    if(DDI_OK == ret)
    {
        memset(dst, 0, sizeof(dst)/sizeof(dst[0]));
        i = trendit_split(dst, sizeof(dst)/sizeof(dst[0]), value_string, ",", SPLIT_IGNOR_LEN0);
        if(i > 0)
        {
            event_node->m_event_switch = trendit_atoi(dst[0]);
            for(j=1; j<i; j++)
            {
                value = trendit_asc_to_u32(dst[j], strlen(dst[j]), MODULUS_HEX);
                trendit_intqueue_enqueue(event_node->m_list, value);
            }
        }
    }

    return ret;
}

/**
 * @brief ��list�����cmd�Ƿ���ڣ����ڸ��½�����ݣ� �����ڴ���
 * @param[in] cmd �¼�������
 * @param[in] value_string �¼���Ӧ����������
 * @param[in] value_string_len �¼���Ӧ���������ݶ�Ӧ�ĳ���
 * @retval  DDI_OK  �ɹ�
 * @retval  DDI_ERR ʧ��
 */
static s32 cfg_event(u16 cmd, u8 *value_string, u16 value_stringlen)
{
    s32 ret = DDI_ERR;
    
    do{
        if(!g_event_model->m_event_list)
        {
            g_event_model->m_event_list = trendit_queue_create();
        }

        if(!g_event_model->m_event_list)
        {
            TRACE_ERR("malloc failed");
            ret = DDI_EOVERFLOW;
            break;
        }
        
        ret = cfg_event_to_list(cmd, value_string, value_stringlen);
    }while(0);

    return ret;
}

/**
 * @brief ��ȡ�ն����͵İ����
 * @retval  ����u16����ţ��ն˰���ŷ�ΧΪ0000---FF00
 */
static u16 get_device_packno(void)
{
    u16 ret = 0;
    u8 packno = event_instance()->m_device_pack_no++;

    ret = packno << 8;

    return ret;
}

/**
 * @brief ����event�¼�
 * @param[in] event_code �¼��룬��EVENT_HBT(0x1051)
 * @param[in] attach_ret ���������͵�����ִ�еķ���ֵ
 * @param[in] send_string ���������͵�����
 * @param[in] send_len ���������͵����ݳ���
 * @retval  ��
 */
static s32 send_event(u16 event_code, s32 attach_ret, s8 *send_string, s32 send_len)
{
    s32 i = 0;
    s32 ret = DDI_EINVAL;
    pnode_t pnode;    //list
    PNode pint_node;  //u32 list
    s32 size = 0;
    model_device_t *model_device = NULL;
    u8 ret_buff[RETURN_CONTENT_MAXSIZE/4] = {0};          //ָ�������
    u8 ret_content_buff[RETURN_CONTENT_MAXSIZE] = {0};
    u16 ret_content_len = 0, len = 0;
    u8 *p_send_data= NULL;
    event_node_t *event_node;
    s8 sn[64] = {0};
    s32 tmp_len = 0;

    pnode = g_event_model->m_event_list->front;
    i = g_event_model->m_event_list->size;
    while(i--)
    {
        event_node = (event_node_t *)pnode->data;

        if(event_node->m_event_code == event_code)
        {
            ret = DDI_OK;
            break;
        }
        pnode = pnode->next;
    }

    TRACE_DBG("ready to exec %04x eventcode ret:%d", event_code, ret);
    ret_content_len = 0;
    //ret_content_buff[ret_content_len++] = PROTOCOL_VERSION;
    ret_content_buff[ret_content_len++] = trendit_get_machine_code();
    memset(sn, 0, sizeof(sn));
    ddi_misc_read_dsn(SNTYPE_POSSN, sn);
    ret_content_buff[ret_content_len++] = sn[0];
    snprintf(ret_content_buff+ret_content_len, sizeof(ret_content_buff), "%s", sn+1);
    ret_content_len += strlen(sn+1);
    
    trendit_pack_u16(ret_content_buff+ret_content_len, event_code);
    ret_content_len += 2;
    trendit_pack_u16(ret_content_buff+ret_content_len, 0);  //��Ԥ�����������
    ret_content_len += 2;

    tmp_len = ret_content_len;

    #if 0
    //������������������
    if(send_len > 0)
    {
        ret_content_buff[ret_content_len++] = -attach_ret;
        trendit_pack_u16(ret_content_buff+ret_content_len, pnode->data);
        ret_content_len += 2;
        trendit_pack_u16(ret_content_buff+ret_content_len, strlen(ret_buff));  //len
        ret_content_len += 2;
        memcpy(ret_content_buff+ret_content_len, ret_buff, strlen(ret_buff));
        ret_content_len += strlen(ret_buff);
    }
    #endif
    
    if(DDI_OK == ret)
    {
        pint_node = event_node->m_list->front;
        size = event_node->m_list->size;

        for(i=0; i<size; i++)
        {
            memset(ret_buff, 0, sizeof(ret_buff));
            model_device = (model_device_t *)trendit_get_model(pint_node->data);
            if(model_device)
            {
                ret = model_device->exec_qry(pint_node->data, ret_buff, sizeof(ret_buff));
                //�����ظ�������
                ret_content_buff[ret_content_len++] = -ret;
                trendit_pack_u16(ret_content_buff+ret_content_len, pint_node->data);
                ret_content_len += 2;
                trendit_pack_u16(ret_content_buff+ret_content_len, strlen(ret_buff));  //len
                ret_content_len += 2;
                memcpy(ret_content_buff+ret_content_len, ret_buff, strlen(ret_buff));
                ret_content_len += strlen(ret_buff);
            }
            
            pint_node = pint_node->next;
        }
    }

    if(ret_content_len > 0)
    {
        trendit_pack_u16(ret_content_buff+tmp_len-2, ret_content_len-tmp_len);   //���event��ʵ�ʳ���
        
        //TRACE_DBG("PROTOCOL_DATA_LEN_EXCEPT_CONTENT:%d, %d", PROTOCOL_DATA_LEN_EXCEPT_CONTENT, ret_content_len);
        p_send_data = k_mallocapp(PROTOCOL_DATA_LEN_EXCEPT_CONTENT+ret_content_len+1);
        if(NULL == p_send_data)
        {
            TRACE_ERR("malloc failed");
            ret = DDI_EOVERFLOW;
        }
        else
        {
            memset(p_send_data, 0, PROTOCOL_DATA_LEN_EXCEPT_CONTENT+ret_content_len+1);
            len = trendit_pack_answer_data(p_send_data, ret_content_buff, ret_content_len, event_instance()->get_device_packno(), CMD_TYPE_EVENT);
            ret = tcp_protocol_instance()->send(p_send_data, len);
            TRACE_INFO("send ret:%d", ret);
            k_freeapp(p_send_data);
            p_send_data = NULL;
            ret = DDI_OK;
        }
    }

    return ret;
}

/**
 * @brief event�ĳ�ʼ������
 * @retval  event���豸�ṹ��
 */
event_model_t *event_instance(void)
{
    if(g_event_model)
    {
        return g_event_model;
    }
    
    g_event_model = k_mallocapp(sizeof(event_model_t));
    if(NULL == g_event_model)
    {
        TRACE_ERR("malloc fail");
        return NULL;
    }
    
    memset(g_event_model, 0, sizeof(event_model_t));
    g_event_model->cfg_event = cfg_event;
    g_event_model->send_event = send_event;
    g_event_model->get_device_packno = get_device_packno;

    return g_event_model;
}

