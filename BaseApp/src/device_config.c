#include "ddiglobal.h"
#include "app_global.h"

static system_data_instance_t *g_system_data;

/**
 * @brief ��ʼ�����ļ����е�ȫ�ֱ���
 */
void trendit_init_system_data(void)
{
    g_system_data = NULL;
}

/**
 * @brief �����ò������
 * @param[in] pqueue ����ͷ
 * @param[in] cfg_info ������Ϣ
 * @retval  DDI_OK �ɹ�
 * @retval  DDI_EOVERFLOW �ڴ����ʧ��
 */
static s32 cfg_queue_entry(queue_t * pqueue, s8 *cfg_info)
{
    cfg_cmd_key_value_t *cfg_cmd_key_value = k_mallocapp(sizeof(cfg_cmd_key_value_t));
    s32 ret = DDI_ERR;
    s8 *pstr = NULL;
    s8 cmd_string[12] = {0};

    do{
        if(NULL == cfg_cmd_key_value)
        {
            TRACE_ERR("malloc failed");
            ret = DDI_EOVERFLOW;
            break;
        }

        memset(cfg_cmd_key_value, 0, sizeof(cfg_cmd_key_value_t));
        pstr = strstr(cfg_info, ":");
        if(pstr)
        {
            memset(cmd_string, 0, sizeof(cmd_string));
            memcpy(cmd_string, cfg_info, pstr-cfg_info);
            pstr = pstr+1;
            cfg_cmd_key_value->m_cmd_key = trendit_asc_to_u32(cmd_string, strlen(cmd_string), MODULUS_HEX);
        }
        else
        {
            TRACE_ERR("don't contain ':':%s", cfg_info);
            k_freeapp(cfg_cmd_key_value);
            cfg_cmd_key_value = NULL;
            ret = DDI_EINVAL;
            break;
        }
        
        cfg_cmd_key_value->m_value_string = k_mallocapp(strlen(pstr)+3);      //д��ʱ����뻻�з�
        if(NULL == cfg_cmd_key_value->m_value_string)
        {
            TRACE_ERR("malloc failed");
            k_freeapp(cfg_cmd_key_value);
            ret = DDI_EOVERFLOW;
            break;
        }
        
        snprintf(cfg_cmd_key_value->m_value_string, strlen(pstr)+3, "%s", pstr);
        trendit_queue_entry(pqueue, (void *)cfg_cmd_key_value);
        ret = DDI_OK;
    }while(0);

    return ret;
}

/**
 * @brief ���豸��Ĭ�����ü��ص�����
 * @param[in] device �豸�ڵ�
 * @retval  DDI_OK  �ɹ�
 * @retval  DDI_EINVAL  �豸��Ų�����
 */
static s32 load_cfg_from_default_cfg(model_device_t *device)
{
    s32 ret = DDI_ERR;
    system_data_instance_t *system_data = system_data_instance();
    s8 tmp_buff[128];
    
    do{
        switch(device->m_no)
        {
            case MODEL_WIRELESS:
                cfg_queue_entry(device->m_cfg_list, "0101:8,14");
                break;

            case MODEL_PRINTER:
                cfg_queue_entry(device->m_cfg_list, "0301:6");
                break;

            case MODEL_AUDIO:
                memset(tmp_buff, 0, sizeof(tmp_buff));
                if(strlen(system_data->m_system_data.m_serverinfor))
                {
                    snprintf(tmp_buff, sizeof(tmp_buff), "0401:%d", system_data->m_system_data.m_vol);
                }
                else
                {
                    snprintf(tmp_buff, sizeof(tmp_buff), "%s", "0401:4");
                }
                cfg_queue_entry(device->m_cfg_list, tmp_buff);
                break;

            case MODEL_MACHINE:
                cfg_queue_entry(device->m_cfg_list, "1001:5,100");
                memset(tmp_buff, 0, sizeof(tmp_buff));
                if(strlen(system_data->m_system_data.m_serverinfor))
                {
                    snprintf(tmp_buff, sizeof(tmp_buff), "1003:%s,%d", system_data->m_system_data.m_serverinfor, system_data->m_system_data.m_serverport);
                }
                else
                {
                    snprintf(tmp_buff, sizeof(tmp_buff), "%s", BACKEND_DEFAULT_CFG_WITH_CMD);
                }
                cfg_queue_entry(device->m_cfg_list, tmp_buff);
                cfg_queue_entry(device->m_cfg_list, "1003:printer.trenditadmin.com,3000");
                cfg_queue_entry(device->m_cfg_list, "1004:60");
                cfg_queue_entry(device->m_cfg_list, "1005:0");
                cfg_queue_entry(device->m_cfg_list, "1006:0");
                cfg_queue_entry(device->m_cfg_list, "1007:1");
                cfg_queue_entry(device->m_cfg_list, "1009:30,35,120,100");
                cfg_queue_entry(device->m_cfg_list, "100A:700");
                cfg_queue_entry(device->m_cfg_list, "100B:cn.ntp.org.cn,ntp.sjtu.edu.cn,us.pool.ntp.org");
                cfg_queue_entry(device->m_cfg_list, "100C:1,235959");
                cfg_queue_entry(device->m_cfg_list, "100D:http://iot.trenditadmin.com/airkiss/airkiss_printer.html");

                //�¼�����
                cfg_queue_entry(device->m_cfg_list, "1051:1,0381,1083");
                cfg_queue_entry(device->m_cfg_list, "1052:1,0381,1083,1084,1086,1088");
                cfg_queue_entry(device->m_cfg_list, "1053:1,1082");
                cfg_queue_entry(device->m_cfg_list, "1054:1,0180,0380,1080,1087,1005");
                cfg_queue_entry(device->m_cfg_list, "1055:1,1081");
                cfg_queue_entry(device->m_cfg_list, "1056:1,0381");
                break;

            default:
                ret = DDI_EINVAL;
                break;
        }

        ret = DDI_OK;
    }while(0);

    return ret;
}

/**
 * @brief �������ļ������豸�����ò���
 * @param[in] device �豸
 * @param[in] cfg_file_name �����ļ���
 * @retval  DDI_OK  �ɹ�
 * @retval  DDI_ERR ʧ��
 * @retval  DDI_EIO ���ļ�ʧ��
 * @retval  DDI_EINVAL �����Ƿ�
 * @retval  DDI_EOVERFLOW �ڴ����
 */
static s32 load_cfg_from_file(model_device_t *device, s8 *cfg_file_name)
{
    s32 fd = 0;
    s32 filesize = 0;
    s8 *buff = NULL;
    s32 ret = DDI_ERR;
    s32 i = 0, len = 0;
    
    do{
        fd = ddi_vfs_open(cfg_file_name, "r");
        if(fd < 0)
        {
            ret = DDI_EIO;
            break;
        }
        
        ddi_vfs_seek(fd, 0, VFS_SEEK_END);
        filesize = ddi_vfs_tell(fd);
        ddi_vfs_seek(fd, 0, VFS_SEEK_SET);

        if(0 == filesize)
        {
            TRACE_ERR("%s file is empty", cfg_file_name);
            ddi_vfs_close(fd);
            ret = DDI_ERR;
            break;
        }
        
        buff = k_mallocapp(filesize);
        if(!buff)
        {
            ddi_vfs_close(fd);
            ret = DDI_EOVERFLOW;
            break;
        }

        filesize = ddi_vfs_read(fd, buff, filesize);
        ddi_vfs_close(fd);
        
        i = 0;
        while((len+i) < filesize)
        {
            if(buff[len+i] == '\r')
            {
                buff[len+i] = 0;
            }

            if(buff[len+i] == '\n')
            {
                buff[len+i] = 0;
                if(0 == i)
                {
                    ret = cfg_queue_entry(device->m_cfg_list, buff+len);
                }
                else
                {
                    ret = cfg_queue_entry(device->m_cfg_list, buff+len+1);
                }

                if(DDI_OK != ret)
                {
                    break;
                }
                len += i;
                i = 0;
            }
            else
            {
                i++;
            }

            ddi_watchdog_feed();
        }

        k_freeapp(buff);
    }while(0);

    return ret;
}

/**
 * @brief �����豸�����ò���
 * @param[in] device �豸
 * @param[in] cfg_file_name �����ļ���
 * @param[in] cfg_func �豸�����ò�������ں���
 * @retval  DDI_OK  �ɹ�
 * @retval  DDI_ERR ʧ��
 * @retval  DDI_EINVAL �����Ƿ�
 * @retval  DDI_EOVERFLOW �ڴ����
 */
s32 trendit_load_device_cfg(model_device_t *device, s8 *cfg_file_name, exec_cfg_func cfg_func)
{
    s32 ret = DDI_ERR;
    pnode_t pnode;
    cfg_cmd_key_value_t *cfg_cmd_key_value;
    s32 i = 0;
    
    do{
        if(!cfg_file_name || !cfg_func || !device)
        {
            ret = DDI_EINVAL;
            break;
        }

        snprintf(device->m_cfg_file_name, sizeof(device->m_cfg_file_name), "%s", cfg_file_name);
        
        if(!device->m_cfg_list)
        {
            device->m_cfg_list = trendit_queue_create();
        }

        if(!device->m_cfg_list)
        {
            ret = DDI_EOVERFLOW;
            break;
        }

        if(DDI_OK == trendit_file_access(cfg_file_name))
        {
            TRACE_INFO("%s load cfg from %s", device->m_model_name, cfg_file_name);
            ret = load_cfg_from_file(device, cfg_file_name);
        }

        if(DDI_OK != ret)
        {
            if(DDI_OK != trendit_file_access(cfg_file_name))
            {
                TRACE_INFO("cfg file:%s don't exist, use default cfg", cfg_file_name);
            }
            else
            {
                TRACE_INFO("cfg file:%s load failed:%d, use default cfg", cfg_file_name, ret);
            }
            
            ret = load_cfg_from_default_cfg(device);
            if(DDI_OK == ret)
            {
                device->save_cfg(device, 0xffff, "ffff", strlen("ffff"));  //0xffff������ڣ��������ݻ�ֱ�ӱ����ļ�
            }
        }

        if(DDI_OK == ret)
        {
            pnode = device->m_cfg_list->front;
            i = device->m_cfg_list->size;
            while(i--)
            {
                cfg_cmd_key_value = (cfg_cmd_key_value_t *)pnode->data;
                if(cfg_cmd_key_value)
                {
                    TRACE_INFO("cfg cmd:%04x, value:%s", cfg_cmd_key_value->m_cmd_key, cfg_cmd_key_value->m_value_string);
                    cfg_func(cfg_cmd_key_value->m_cmd_key, cfg_cmd_key_value->m_value_string, strlen(cfg_cmd_key_value->m_value_string));
                }

                ddi_watchdog_feed();
                pnode = pnode->next;
            }
        }
    }while(0);

    return ret;
}

/**
 * @brief �����ò������浽�豸�ļ�
 * @param[in] device �豸
 * @param[in] cmd ����
 * @param[in] valuestring �����Ӧ����������
 * @param[in] valuestring_len �����Ӧ���������ݳ���
 * @retval  DDI_OK  �ɹ�
 * @retval  DDI_ERR ʧ��
 * @retval  DDI_EINVAL �����Ƿ�
 * @retval  DDI_EOVERFLOW �ڴ����
 */
s32 trendit_save_device_cfg(model_device_t *device, u16 cmd, u8 *valuestring, u16 valuestring_len)
{
    s32 ret = DDI_ERR;
    pnode_t pnode;
    cfg_cmd_key_value_t *cfg_cmd_key_value;
    s32 i = 0;
    s32 fd = 0;
    s8 *pstr = NULL;
    s8 tmp_buff[1024] = {0};

    do{
        if(!device || !valuestring || 0 == valuestring_len)
        {
            ret = DDI_EINVAL;
            break;
        }

        ddi_vfs_deletefile(device->m_cfg_file_name);
        
        fd = ddi_vfs_open(device->m_cfg_file_name, "w");
        if(fd < 0)
        {
            TRACE_ERR("open failed:%s", device->m_cfg_file_name);
            ret = DDI_EIO;
            break;
        }
        
        pnode = device->m_cfg_list->front;
        i = device->m_cfg_list->size;
        while(i--)
        {
            cfg_cmd_key_value = (cfg_cmd_key_value_t *)pnode->data;
            if(cfg_cmd_key_value)
            {
                //�����ڴ�ڵ�����
                if(cfg_cmd_key_value->m_cmd_key == cmd)
                {
                    if(valuestring_len > strlen(cfg_cmd_key_value->m_value_string))
                    {
                        pstr = k_mallocapp(valuestring_len+3);
                        if(NULL == pstr)
                        {
                            TRACE_ERR("malloc failed");
                        }
                        else
                        {
                            memset(pstr, 0, valuestring_len+3);
                            snprintf(pstr, valuestring_len+3, "%s", valuestring);

                            k_freeapp(cfg_cmd_key_value->m_value_string);
                            cfg_cmd_key_value->m_value_string = pstr;
                        }
                    }
                    else
                    {
                        memset(cfg_cmd_key_value->m_value_string, 0, strlen(cfg_cmd_key_value->m_value_string));
                        strcpy(cfg_cmd_key_value->m_value_string, valuestring);
                    }
                }

                memset(tmp_buff, 0, sizeof(tmp_buff));
                snprintf(tmp_buff, sizeof(tmp_buff), "%04x:%s\r\n", cfg_cmd_key_value->m_cmd_key, cfg_cmd_key_value->m_value_string);
                ret = ddi_vfs_write(fd, tmp_buff, strlen(tmp_buff));
                if(ret != strlen(tmp_buff))
                {
                    TRACE_ERR("ret:%d", ret);
                }
            }

            ddi_watchdog_feed();
            pnode = pnode->next;
        }

        ddi_vfs_close(fd);
        ret = DDI_OK;
    }while(0);

    return ret;
}

/**
 * @brief �ָ�Ĭ�ϲ���
 * @param[in] device �豸
 * @retval  DDI_OK  �ɹ�
 * @retval  DDI_ERR ʧ��
 * @retval  DDI_EINVAL �����Ƿ�
 * @retval  DDI_EOVERFLOW �ڴ����
 */
s32 trendit_restore_device_cfg(model_device_t *device)
{
    s32 ret = DDI_ERR;
    pnode_t pnode;
    cfg_cmd_key_value_t *cfg_cmd_key_value;
    s32 i = 0;
    s32 fd = 0;
    s8 *pstr = NULL;
    s8 tmp_buff[1024] = {0};

    do{
        if(!device || device->m_no != MODEL_MACHINE)   //�ͻ�ֻ��Ҫ�ָ���̨ip�Ͷ˿�
        {
            ret = DDI_EINVAL;
            break;
        }
        
        fd = ddi_vfs_open(device->m_cfg_file_name, "w");
        if(fd < 0)
        {
            TRACE_ERR("open failed:%s", device->m_cfg_file_name);
            ret = DDI_EIO;
            break;
        }
        
        pnode = device->m_cfg_list->front;
        i = device->m_cfg_list->size;
        while(i--)
        {
            cfg_cmd_key_value = (cfg_cmd_key_value_t *)pnode->data;
            if(cfg_cmd_key_value)
            {
                //TRACE_DBG("cfg_cmd_key_value->m_cmd_key:%04x %4x", cfg_cmd_key_value->m_cmd_key, CFG_COMAND_MACHINE_BACKEND_CON);
                //�����ڴ�ڵ�����
                if(cfg_cmd_key_value->m_cmd_key == CFG_COMAND_MACHINE_BACKEND_CON)
                {
                    if(strlen(BACKEND_DEFAULT_CFG) > strlen(cfg_cmd_key_value->m_value_string))
                    {
                        pstr = k_mallocapp(strlen(BACKEND_DEFAULT_CFG)+3);
                        if(NULL == pstr)
                        {
                            TRACE_ERR("malloc failed");
                        }
                        else
                        {
                            memset(pstr, 0, strlen(BACKEND_DEFAULT_CFG)+3);
                            snprintf(pstr, strlen(BACKEND_DEFAULT_CFG)+3, "%s", BACKEND_DEFAULT_CFG);

                            k_freeapp(cfg_cmd_key_value->m_value_string);
                            cfg_cmd_key_value->m_value_string = pstr;
                        }
                    }
                    else
                    {
                        memset(cfg_cmd_key_value->m_value_string, 0, strlen(cfg_cmd_key_value->m_value_string));
                        strcpy(cfg_cmd_key_value->m_value_string, BACKEND_DEFAULT_CFG);
                    }

                    device->exec_cfg(cfg_cmd_key_value->m_cmd_key, cfg_cmd_key_value->m_value_string, strlen(cfg_cmd_key_value->m_value_string));
                }

                memset(tmp_buff, 0, sizeof(tmp_buff));
                snprintf(tmp_buff, sizeof(tmp_buff), "%04x:%s\r\n", cfg_cmd_key_value->m_cmd_key, cfg_cmd_key_value->m_value_string);
                ddi_vfs_write(fd, tmp_buff, strlen(tmp_buff));
            }

            ddi_watchdog_feed();
            pnode = pnode->next;
        }

        ddi_vfs_close(fd);
        ret = DDI_OK;
    }while(0);

    return ret;
}

/**
 * @brief ��ѯ����ִ��ʧ��ʱ���������ļ��ڴ������л�ȡ��������������ɲ�ѯ��
 * @param[in] device �豸
 * @param[in] cmd ����
 * @param[in] ret_string �������ݻ���
 * @param[in] ret_string_len �������ݻ��泤��
 * @retval  DDI_OK  �ɹ�
 * @retval  DDI_ERR ʧ��
 */
s32 trendit_query_from_device_cfg(model_device_t *device           , u16 cmd, u8 *ret_string, u16 ret_stringlen)
{
    pnode_t pnode;
    cfg_cmd_key_value_t *cfg_cmd_key_value;
    s32 i = 0;
    s32 ret = DDI_ERR;

    pnode = device->m_cfg_list->front;
    i = device->m_cfg_list->size;
    while(i--)
    {
        cfg_cmd_key_value = (cfg_cmd_key_value_t *)pnode->data;
        if(cfg_cmd_key_value)
        {
            //�����ڴ�ڵ�����
            if(cfg_cmd_key_value->m_cmd_key == cmd)
            {
                snprintf(ret_string, ret_stringlen, "%s", cfg_cmd_key_value->m_value_string);
                ret = DDI_OK;
                break;
            }
        }
        
        pnode = pnode->next;
    }

    return ret;
}

/**
 * @brief ����ϵͳ����
 * @retval  DDI_OK   �ɹ�
 * @retval  DDI_EIO   �ļ�������
 * @retval  DDI_ERR   CRC����
 */
static s32 load_system_data(void)
{
    s32 fd = 0;
    s32 ret = DDI_ERR;
    u8 crc[2] = {0};
    system_data_instance_t *system_data = system_data_instance();
    s32 len = 0;

    do{
        fd = ddi_vfs_open(SYSTEM_DATA_FILE, "r");

        if(fd < 0)
        {
            TRACE_ERR("file %s don't exist", SYSTEM_DATA_FILE);
            ret = DDI_EIO;
            break;
        }

        ddi_vfs_read(fd, &(system_data->m_system_data), sizeof(system_data->m_system_data));
        len = sizeof(system_data->m_system_data) - sizeof(system_data->m_system_data.m_reserve) - sizeof(system_data->m_system_data.m_crc);
        trendit_Crc16CCITT(0, (u8 *)&(system_data->m_system_data), len, crc);
        TRACE_INFO("file crc:%x %x", crc[0], crc[1]);
        TRACE_INFO("crc:%x %x", system_data->m_system_data.m_crc[0], system_data->m_system_data.m_crc[1]);
        if(0 == memcmp(crc, system_data->m_system_data.m_crc, sizeof(crc)))
        {
            TRACE_INFO("system_data===");
            TRACE_INFO("power_reason:%d", system_data->m_system_data.m_power_reason);
            TRACE_INFO("print_len:%u", system_data->m_system_data.m_print_len);
            TRACE_INFO("wifi_ssid:%s", system_data->m_system_data.m_wifi_ssid);
            TRACE_INFO("wifi_password:%s", system_data->m_system_data.m_wifi_password);
            ret = DDI_OK;
        }

        ddi_vfs_close(fd);
    }while(0);

    if(ret < 0)
    {
        TRACE_INFO("use default data");
        system_data->m_system_data.m_power_reason = RESEASON_KEY_POWER_OFF;
        system_data->m_system_data.m_print_len = 0;

        snprintf(system_data->m_system_data.m_wifi_ssid, sizeof(system_data->m_system_data.m_wifi_ssid), "");
        snprintf(system_data->m_system_data.m_wifi_password, sizeof(system_data->m_system_data.m_wifi_password), "");
    }

    return ret;
}

/**
 * @brief ����ϵͳ����---wifi��Ϣ
 * @retval  DDI_OK   �ɹ�
 * @retval  DDI_EIO   �ļ�������
 * @retval  DDI_ERR   CRC����
 */
static s32 get_wifi_data(s8 *ssid, s32 ssid_len, s8 *password, s32 password_len)
{
    s32 ret = DDI_ERR;
    system_data_instance_t *system_data = system_data_instance();

    do{
        if(NULL == ssid || NULL == password || 0 == ssid_len || 0 == password_len)
        {
            ret = DDI_EINVAL;
            break;
        }

        snprintf(ssid, ssid_len, "%s", system_data->m_system_data.m_wifi_ssid);
        snprintf(password, password_len, "%s", system_data->m_system_data.m_wifi_password);
        ret = DDI_OK;
    }while(0);
    
    return ret;
}

/**
 * @brief ����ϵͳ����---��ӡ����
 * @retval  ��ӡ����
 */
static u32 get_printlen(void)
{
    system_data_instance_t *system_data = system_data_instance();

    return system_data->m_system_data.m_print_len;
}

/**
 * @brief ����ϵͳ����---���һ�ιػ�ԭ��
 * @retval  ��ӡ����
 */
static s32 get_power_reason(void)
{
    system_data_instance_t *system_data = system_data_instance();
    s32 ret = system_data->m_system_data.m_power_reason;

    set_poweroff_reason(RESEASON_KEY_POWER_OFF);   //��Ϊ���������ػ�
    
    return ret;
}

/**
 * @brief �޸�ϵͳ����---wifi����
 * @retval  DDI_OK  �ɹ�
 * @retval  <0 ʧ��
 */
static s32 set_wifi_data(s8 *ssid, s8 *password)
{
    s32 ret = DDI_ERR;
    system_data_instance_t *system_data = system_data_instance();

    do{
        if(NULL == ssid || NULL == password)
        {
            ret = DDI_EINVAL;
            break;
        }

        snprintf(system_data->m_system_data.m_wifi_ssid, sizeof(system_data->m_system_data.m_wifi_ssid), "%s", ssid);
        snprintf(system_data->m_system_data.m_wifi_password, sizeof(system_data->m_system_data.m_wifi_password), "%s", password);
        ret = system_data->save_data();
    }while(0);
    
    return ret;
}

/**
 * @brief �޸�ϵͳ����---��ӡ����
 * @retval  DDI_OK  �ɹ�
 * @retval  <0 ʧ��
 */
static s32 set_printlen(u32 printlen)
{
    system_data_instance_t *system_data = system_data_instance();

    system_data->m_system_data.m_print_len = printlen;
    
    return system_data->save_data();
}

/**
 * @brief �޸�ϵͳ����---���һ�ιػ�ԭ��
 * @retval  DDI_OK  �ɹ�
 * @retval  <0 ʧ��
 */
static s32 set_poweroff_reason(LAST_POWEROFF_RESEASON_E poweroff_reason)
{
    system_data_instance_t *system_data = system_data_instance();

    system_data->m_system_data.m_power_reason = poweroff_reason;

    return system_data->save_data();
}

/**
 * @brief ����ϵͳ�������ļ�
 * @retval  DDI_OK   �ɹ�
 * @retval  DDI_EIO   �ļ�������
 * @retval  DDI_ERR   CRC����
 */
static s32 save_system_data(void)
{
    s32 fd = 0;
    s32 ret = DDI_ERR;
    u8 crc[2] = {0};
    system_data_instance_t *system_data = system_data_instance();
    s32 len = 0;

    do{
        fd = ddi_vfs_open(SYSTEM_DATA_FILE, "w");

        if(fd < 0)
        {
            TRACE_ERR("file %s don't exist", SYSTEM_DATA_FILE);
            ret = DDI_EIO;
            break;
        }

        len = sizeof(system_data->m_system_data) - sizeof(system_data->m_system_data.m_reserve) - sizeof(system_data->m_system_data.m_crc);
        trendit_Crc16CCITT(0, (u8 *)&(system_data->m_system_data), len, crc);
        system_data->m_system_data.m_crc[0] = crc[0];
        system_data->m_system_data.m_crc[1] = crc[1];
        
        ddi_vfs_write(fd, &(system_data->m_system_data), sizeof(system_data->m_system_data));
        ddi_vfs_close(fd);
        ret = DDI_OK;
    }while(0);

    return ret;
}

/**
 * @brief ϵͳ����
 * @retval  ϵͳ�����Ľṹ��
 */
system_data_instance_t *system_data_instance(void)
{    
    s32 ret = 0;
    
    if(g_system_data)
    {
        return g_system_data;
    }

    g_system_data = k_mallocapp(sizeof(system_data_instance_t));
    if(NULL == g_system_data)
    {
        TRACE_ERR("malloc fail");
        return NULL;
    }
    
    memset(g_system_data, 0, sizeof(system_data_instance_t));
    g_system_data->load_data = load_system_data;
    g_system_data->save_data = save_system_data;
    g_system_data->get_power_reason = get_power_reason;
    g_system_data->set_poweroff_reason = set_poweroff_reason;
    g_system_data->get_printlen = get_printlen;
    g_system_data->set_printlen = set_printlen;
    g_system_data->get_wifi_data = get_wifi_data;
    g_system_data->set_wifi_data = set_wifi_data;
    g_system_data->load_data();

    ret = ddi_misc_ioctl(MISC_IOCTL_GET_RESET_FLAG, 0, 0);
    if(2 == ret)   //���Ź���λ
    {
        TRACE_INFO("detect watchdog reset");
        machine_instance()->m_power_on_reason = RESEASON_WATCHDOG;
        ddi_misc_ioctl(MISC_IOCTL_CLEAR_RESET_FLAG, 0, 0);
    }
    else if(3 == ret) //hardfault��λ
    {
        TRACE_INFO("detect hardfault reset");
        machine_instance()->m_power_on_reason = RESEASON_HARD_FAULT;
        ddi_misc_ioctl(MISC_IOCTL_CLEAR_RESET_FLAG, 0, 0);
    }
    
    return g_system_data;
}


