#include "ddiglobal.h"
#include "app_global.h"

static audio_module_t *g_model_audio = NULL;

#ifdef TRENDIT_AUDIO_DOWNLOAD
static audio_file_config g_audio_file_config;
static s32 g_audio_file_fd  = -1;//�ļ� fd
static u8 *g_download_file_data;
#endif
/**
 * @brief ��ʼ�����ļ����е�ȫ�ֱ���
 */
void trendit_init_audio_data(void)
{
    g_model_audio = NULL;
}

static s32 audio_msg_handle(struct _msg *m_msg)
{
    s32 ret = MSG_KEEP;
    audio_module_t *audio_device = audio_instance();
    AUDIO_PLAY_STEP_E audio_step;
    str_audio_tts audio_tts;

    do{
        if(MSG_TERMINAL == m_msg->m_status)
        {
            ret = MSG_OUT;
            break;
        }
        else
        {
            audio_step = audio_device->m_play_info.m_play_step;
            switch(audio_step)
            {
                case AP_STEP_NEW_INSERT:
                case AP_STEP_SLEEP_REPLAY:
                    audio_tts.m_text = audio_device->m_play_info.m_play_string;
                    audio_tts.m_texttype = 1;
                    ddi_audio_playcharacters(audio_tts);
                    audio_device->m_play_info.m_play_step = AP_STEP_CHECK_PLAY_RES;
                    break;

                case AP_STEP_CHECK_PLAY_RES:
                    if(0 == trendit_get_audio_status())
                    {
                        audio_device->m_play_info.m_have_playtimes++;
                        if(audio_device->m_play_info.m_have_playtimes >= audio_device->m_play_info.m_playtimes)
                        {
                            TRACE_DBG("finish play %s", audio_device->m_play_info.m_play_string);
                            audio_device->clear_play_data();
                            audio_device->m_play_info.m_play_step = AP_STEP_IDLE;
                        }
                        else
                        {
                            audio_device->m_play_info.m_cur_ticks = trendit_get_ticks();
                            audio_device->m_play_info.m_play_step = AP_STEP_SLEEP_CHECK;
                        }
                    }
                    break;
                    
                case AP_STEP_SLEEP_CHECK:
                    if(trendit_query_timer(audio_device->m_play_info.m_cur_ticks, audio_device->m_play_info.m_interval_time*1000))
                    {
                        TRACE_DBG("sleep over, then play next");
                        audio_device->m_play_info.m_play_step = AP_STEP_SLEEP_REPLAY;
                    }
                    break;
            }

            ret = MSG_KEEP;
        }
    }while(0);

    return ret;
}

/**
 * @brief ��������
 * @param[in] play_string ��Ҫ���ŵ�����
 * @param[in] audio_play_type ���ŷ�ʽ�����������Ƿ�����
 * @retval  ��
 */
static s32 audio_play(s8 *play_string, AUDIO_PLAY_TYPE audio_play_type)
{
    str_audio_tts audio_tts;
    s32 i = 0;
    s32 ret = DDI_ERR;

    do{
        audio_instance()->clear_play_data();
        audio_tts.m_text = play_string;
        audio_tts.m_texttype = 1;

        ret = ddi_audio_playcharacters(audio_tts);
        if(DDI_OK != ret)
        {
            break;
        }

        if(AUDIO_PLAY_BLOCK == audio_play_type)
        {
            while(1)
            {
                if(0 == trendit_get_audio_status())
                {
                    break;
                }

                ddi_misc_msleep(10);
                ddi_watchdog_feed();
            }
        }
    }while(0);

    return ret;
}

/**
 * @brief ����������С
 * @param[in] value_string
 * @retval  DDI_OK ��
 * @retval  DDI_ERR ��
 */
static s32 cmd_comand_audio_play(u8 *value_string, u8 *ret_string, u16 ret_stringlen)
{
    s32 play_times = 0;         //���Ŵ���
    s32 interval_times = 0;     //���ż��ʱ��
    s32 ret = DDI_ERR;
    s8 dst[3][SPLIT_PER_MAX];
    s32 i = 0, no = 0;
    s8 audio_str[][64] = {"�������ˣ����������¶�����", 
                            "�����������¶�����",
                            "�����������¶�����",
                            "�������ˣ������µĶ������뾡�촦��",
                            "�������ˣ���ע�⣬���û�ȡ���������뾡�촦��",
                            "�������ˣ���ע�⣬���û��˵����뾡�촦��",
                            "�������ˣ���ע�⣬���û��ߵ����뾡�촦��",
                            "�������ˣ����������¶������뾡�촦��",
                            "��Ϊ�����򶩵�"};

    do{
        ret = trendit_split(dst, sizeof(dst)/sizeof(dst[0]), value_string, ",", SPLIT_NOIGNOR);
        if(ret < sizeof(dst)/sizeof(dst[0]))
        {
            TRACE_ERR("err msg:%d", ret);
            ret = DDI_EINVAL;
            break;
        }

        play_times = trendit_atoi(dst[0]);
        interval_times = trendit_atoi(dst[1]);
        TRACE_INFO("play_times:%d, interval_times:%d", play_times, interval_times);

        if(play_times > 3 || play_times < 0)
        {
            TRACE_ERR("arg is error:%d", play_times);
            ret = DDI_EINVAL;
            break;
        }
        
        if(interval_times > 10 || interval_times < 0)
        {
            TRACE_ERR("arg is error:%d", interval_times);
            ret = DDI_EINVAL;
            break;
        }

        if(1 == strlen(dst[2]))
        {
            no = trendit_atoi(dst[2]);
            if(no > sizeof(audio_str)/sizeof(audio_str[0]) || no <= 0)
            {
                ret = DDI_EINVAL;
                break;
            }
            else
            {
                audio_instance()->insert_play_data(audio_str[no-1], play_times, interval_times);
            }
        }
        else
        {
            audio_instance()->insert_play_data(dst[2], play_times, interval_times);
        }

        ret = DDI_OK;
    }while(0);

    return ret;
}

static s32 exec_cfg(u16 cmd, u8 *value_string, u16 value_string_len)
{
    s32 ret = DDI_ERR;
    
    TRACE_DBG("cmd:%04x, value_string:%s", cmd, value_string);
    switch(cmd)
    {
        case CMD_CFG_AUDIO_VOLUME:
            ret = cfg_vol(VOL_CFG_SOURCE_WEB, 0, trendit_atoi(value_string));

            if(audio_instance()->m_load_cfg)
            {
                if(ret == DDI_OK)
                {
                    audio_instance()->audio_play(AUDIO_CFG_SUCCESS, AUDIO_PLAY_BLOCK);
                }
                else
                {
                    audio_instance()->audio_play(AUDIO_CFG_FAIL, AUDIO_PLAY_BLOCK);
                }
            }
            break;
    }

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
    switch(cmd)
    {
        case CMD_COMAND_AUDIO_PLAY:
            ret = cmd_comand_audio_play(value_string, ret_string, ret_stringlen);
            break;
    }

    return ret;
}


/**
 * @brief ��������������С
 * @param[in] vol_cfg_source �������û��ߺ�̨����
 * @param[in] key_flag:1-����������0-��С����,��������ʱ��Ч
 * @param[in] vol:������С����̨����ʱ��Ч
 * @retval  DDI_OK �ɹ�
 * @retval  DDI_ERR ʧ��
 * @retval  DDI_EINVAL ��������
 */
static s32 cfg_vol(AUDIO_VOL_CFG_SOURCE_E vol_cfg_source, s32 key_flag, s32 vol)
{
    s32 ret = DDI_OK;
    u8 vol_level = 0;
    model_device_t model_device;
    u8 buff[12] = {0};

    ret = ddi_audio_ioctl(DDI_AUDIO_GET_VOLUME, (u32)(&vol_level), 0);
    if(ret != DDI_OK)
    {
        TRACE_ERR("get vol fail:%d", ret);
        return ret;
    }
    else
    {
        TRACE_DBG("get vol success:%d", vol_level);
    }
    
    switch(vol_cfg_source)
    {
        case VOL_CFG_SOURCE_KEY:
            if(key_flag)
            {
                if(vol_level >= AUDIO_VOL_5)
                {
                    audio_instance()->audio_play(AUDIO_MAX, AUDIO_PLAY_BLOCK);
                    ret = DDI_ERR;
                }
                else
                {
                    vol_level++;
                    ret = DDI_OK;
                }
            }
            else
            {
                if(vol_level <= AUDIO_VOL_0)
                {
                    ret = DDI_ERR;
                }
                else
                {
                    vol_level--;
                    ret = DDI_OK;
                }
            }
            break;

        case VOL_CFG_SOURCE_WEB:
            if(vol < AUDIO_VOL_0 || vol > AUDIO_VOL_5)
            {
                ret = DDI_EINVAL;
                break;
            }

            vol_level = vol;
            break;
    }

    if(DDI_OK == ret)
    {
        ret = ddi_audio_ioctl(DDI_AUDIO_CTL_VOLUME, vol_level, 0);

        if(DDI_OK == ret)
        {
            audio_instance()->m_vol = vol_level;
            //�޸ĵ������ļ�
            if(VOL_CFG_SOURCE_KEY == vol_cfg_source)
            {
                memset(buff, 0, sizeof(buff));
                snprintf(buff, sizeof(buff), "%d", vol_level);
                audio_instance()->super.save_cfg(&(audio_instance()->super), CMD_CFG_AUDIO_VOLUME, buff, strlen(buff));
            }
        }
        else
        {
            TRACE_ERR("cfg vol fail:%d", ret);
        }
    }
    
    return ret;
}

/**
 * @brief ���ſ������֣�������������ģ���ϣ���������ģ���ok����ִ�д˺���������������С���úͲ���
 * @retval  ��
 */
static void play_poweron_audio(void)
{
    g_model_audio->m_load_cfg = FALSE;
    g_model_audio->super.load_cfg(&(g_model_audio->super), AUDIO_CONFIG_FILE, exec_cfg);
    g_model_audio->m_load_cfg = TRUE;
    ddi_led_ioctl(DDI_LED_CTL_CLOSE_YELLOW, 0, 0);                //�ص��Ƶ���˸
    audio_instance()->audio_play(AUDIO_POWER_ON, AUDIO_PLAY_BLOCK);
}

/**
 * @brief ��̨���͵���������
 * @param[in] play_string ��������
 * @param[in] play_time:���Ŵ���
 * @param[in] interval_times:���ż��ʱ��
 * @retval  ��
 */
static void insert_play_data(s8 *play_string, s32 play_time, s32 interval_times)
{
    memset(&(g_model_audio->m_play_info), 0, sizeof(g_model_audio->m_play_info));
    g_model_audio->m_play_info.m_playtimes = play_time;
    g_model_audio->m_play_info.m_interval_time = interval_times;
    snprintf(g_model_audio->m_play_info.m_play_string, 
                sizeof(g_model_audio->m_play_info.m_play_string), "%s", play_string);
    g_model_audio->m_play_info.m_play_step = AP_STEP_NEW_INSERT;
}

/**
 * @brief �����������
 * @param[in] ��
 * @retval  ��
 */
static void clear_play_data(void)
{
    memset(&(g_model_audio->m_play_info), 0, sizeof(g_model_audio->m_play_info));
}


static s32 post_audio_message()
{
    msg_t *audio_msg = NULL;
    s32 ret = DDI_ERR;

    audio_msg = (msg_t *)k_mallocapp(sizeof(msg_t));
    if(NULL == audio_msg)
    {
        TRACE_ERR("malloc failed");
    }
    else
    {
        TRACE_DBG("audio:%x", audio_msg);
        memset(audio_msg, 0, sizeof(msg_t));
        audio_msg->m_func = audio_msg_handle;
        audio_msg->m_priority = MSG_NORMAL;
        audio_msg->m_lParam = 0;
        audio_msg->m_wparam = 0;
        audio_msg->m_status = MSG_INIT;
        snprintf(audio_msg->m_msgname, sizeof(audio_msg->m_msgname), "%s", "audio");
        trendit_postmsg(audio_msg);
    }

    return ret;
}

/**
 * @brief audio�ĳ�ʼ������
 * @retval  audio���豸�ṹ��
 */
audio_module_t *audio_instance(void)
{
    if(g_model_audio)
    {
        return g_model_audio;
    }
    
    g_model_audio = k_mallocapp(sizeof(audio_module_t));
    if(NULL == g_model_audio)
    {
        TRACE_ERR("malloc fail");
        return NULL;
    }
    
    memset(g_model_audio, 0, sizeof(audio_module_t));
    INIT_SUPER_METHOD(g_model_audio, MODEL_AUDIO, "audio");

    g_model_audio->audio_play = audio_play;
    g_model_audio->cfg_vol = cfg_vol;
    g_model_audio->play_poweron_audio = play_poweron_audio;
    g_model_audio->insert_play_data = insert_play_data;
    g_model_audio->clear_play_data = clear_play_data;

    post_audio_message();
    
    if(DDI_OK != ddi_audio_open())
    {
        TRACE_ERR("don't detect audio device");
    }

    return g_model_audio;
}

#ifdef TRENDIT_AUDIO_DOWNLOAD


/**
 *@brief ���ļ��ļ�
 *@param[in]
 *@retval:�ļ�fd
 */
s32 trendit_audio_vfs_open()
{

    g_audio_file_fd = ddi_vfs_open(AUDIO_SOURCE_PATH, "r+");

    return g_audio_file_fd;
}

/**
 *@brief /mtd0/audio�ļ�
 *@param[in] ��
 *@retval:�ļ��ر�״̬
 */
s32 trendit_audio_vfs_close()
{
    s32 ret = -1;
    if(g_audio_file_fd > 0)
    {
        ret = ddi_vfs_close(g_audio_file_fd);
        g_audio_file_fd = -1;
    }
    return ret;
}

u8 *audiofile_instance(void)
{
    #if 0
    if(g_download_file_data)
    {
        return g_download_file_data;
    }
    #endif
    g_download_file_data = (u8*)k_mallocapp(AUDIO_MAX_TEXT_FILE_LEN);
    if(NULL == g_download_file_data)
    {
        TRACE_ERR("malloc fail");
        return NULL;
    }
    return g_download_file_data;
}


s32 audiofile_destroy(void)
{
    if(g_download_file_data != NULL)
    {
        k_freeapp(g_download_file_data);
    }
}


s32 trendit_save_file_data(u8 *data, u32 data_len)
{
    if(g_download_file_data == NULL)
    {
        return DDI_ERR;
    }

    memcpy(g_download_file_data+g_audio_file_config.m_file_offset, data, data_len);
    g_audio_file_config.m_file_offset = g_audio_file_config.m_file_offset+data_len;
    TRACE_DBG("******************total len:%d\r\n",g_audio_file_config.m_file_offset);
}

/**
 *@brief ��/mtd0/audio�ļ���ȡ����
 *@param[in]fd �ļ�fd
            data ����
            len  ����
            offset λ��
 *@retval:ʧ��DDI_ERR���ɹ�DDI_OK
 */

s32 trendit_audio_vfs_read(s8 *buff, s32 size, s32 offset)
{
    s32 lens, ret = -1;
    s32 fd = -1;

    fd = ddi_vfs_open(AUDIO_SOURCE_PATH, "r");
    if (fd < 0)
    {
        TRACE_DBG("ddi_vfs_open fail\r\n");
        return DDI_ERR;
    }

    ret = ddi_vfs_seek(fd, offset, 0);

    ret = ddi_vfs_read(fd, buff, size);
    ddi_vfs_close(fd);

    return ret;
}

/**
 *@brief д��/mtd0/audio�ļ���
 *@param[in]fd �ļ�fd
            data ����
            len  ����
 *@retval:ʧ��DDI_ERR���ɹ�DDI_OK
 */
s32 trendit_audio_vfs_write(s8 *data, u32 len, s32 offset)
{
    s32 ret = -1;
    u32 ticks_start;
    s32 fd = -1;

    ticks_start = trendit_get_ticks();

    fd = ddi_vfs_open(AUDIO_SOURCE_PATH, "r+");
    if (fd < 0)
    {
        TRACE_DBG("ddi_vfs_open fail\r\n");
        return DDI_ERR;
    }
    ret = ddi_vfs_seek(fd, offset, 0);

    ret = ddi_vfs_write(fd, data, len);
    ddi_vfs_close(fd);

    TRACE_DBG("write time:%d\r\n",trendit_get_ticks() - ticks_start);


    return ret;
}


/**
 *@brief ��/mtd0/audio�����ļ�����λ��,�����ļ���
 *@param[in]filename �ļ���
 *@retval:DDI_OK��DDI_ERR
 */
s32 trendit_search_download_status (u8 *filename)
{
    s32 i = 0;
    s32 ret = 0;
    u8 down_flg = 0;
    u8 tmpfilename[128];

    if((NULL == filename) || (strlen(filename) > AUDIO_MAX_TEXT_NAME_LEN) || (strlen(filename) <= 0))
    {
        TRACE_DBG("param error\r\n");
        return DDI_EINVAL;
    }
    
    for(i = 0; i < AUDIO_MAX_NUM; i++)
    {
        down_flg = 0;
        ret = trendit_audio_vfs_read(&down_flg,1,audio_filedownflg_addr(i));
        //TRACE_DBG("ref down_flg:%x, file index:%d\r\n",down_flg,i);
        if(down_flg != 1)
        {
            //to do

            strcpy(g_audio_file_config.m_file_name, filename);
            g_audio_file_config.m_file_index = i;
            g_audio_file_config.m_file_offset = 0;
            g_audio_file_config.m_file_flg = 0;
            //TRACE_DBG("set file name1:%s, file index:%d\r\n",g_audio_file_config.m_file_name,g_audio_file_config.m_file_index);
            return DDI_OK;

        }

    }
    return DDI_ERR;
}

/**
 *@brief ���µ�ǰ/mtd0/audio�ļ�head ��Ϣ
 *@param[in]filehead head ��Ϣ
            head_len head����
 *@retval:DDI_EINVALʧ�ܣ�DDI_OK�ɹ�
 */
s32 trendit_set_file_headinfo(u8 *filehead, s32 head_len,s32 offset)
{
    s32 i = 0;


    if((head_len <= 0) || head_len >(AUDIO_MAX_TEXT_NAME_LEN+AUDIO_MAX_TEXT_DOWNLOAD_FLG))
    {
        return DDI_EINVAL;
    }

    //�����ļ�head��Ϣ(file_name+file_status)

    return trendit_audio_vfs_write(filehead, head_len, offset);


}

/**
 *@brief ��/mtd0/audio�����ļ��Ƿ����
 *@param[in]filename �ļ���
 *@retval:С��0ʧ�ܣ��ɹ�0-49
 */
s32 trendit_search_file_status (u8 *filename)
{
    s32 i = 0;
    s32 ret = 0;
    u8 down_flg = 0;
    u8 tmpfilename[128];

    if((NULL == filename) || (strlen(filename) > AUDIO_MAX_TEXT_NAME_LEN) || (strlen(filename) <= 0))
    {
        TRACE_DBG("param error\r\n");
        return DDI_EINVAL;
    }

    memset(&g_audio_file_config, 0x00, sizeof(audio_file_config));

    for(i = 0; i < AUDIO_MAX_NUM; i++)
    {
        memset(tmpfilename, 0, sizeof(tmpfilename));
        down_flg = 0;
        trendit_audio_vfs_read(tmpfilename,sizeof(tmpfilename),audio_filename_addr(i));
        trendit_audio_vfs_read(&down_flg,1,audio_filedownflg_addr(i));
        //TRACE_DBG("read filename:%s\r\n",tmpfilename);
        if((strcmp(filename, tmpfilename) == 0) && (down_flg == 1))
        {
            //TRACE_DBG("audio exist\r\n");
            TRACE_DBG("read filename:%s,index:%d\r\n",filename,i);
            strcpy(g_audio_file_config.m_file_name, filename);
            g_audio_file_config.m_file_index = i;
            g_audio_file_config.m_file_offset = 0;
            g_audio_file_config.m_file_flg = 1;//����ѡ�����
            return DDI_OK;//����Դ�Ѵ���,��ɾ��
        }
    }

    return DDI_ERR;

}


s32 trendit_download_audio_file(u8 *filename, u8 *audio_file_data, u32 audio_file_len)
{
    u8 audio_data[1024];
    
    u8 filehead[AUDIO_MAX_TEXT_NAME_LEN+AUDIO_MAX_TEXT_DOWNLOAD_FLG];

    s32 ret = -1;
    s32 i = 0;

    
    if((audio_file_data == NULL) || (audio_file_len <=0) || (audio_file_len>AUDIO_MAX_TEXT_FILE_LEN))
    {
        return DDI_EINVAL;
    }
    #if 0
    audio_file_data = (u8*)k_malloc(AUDIO_MAX_TEXT_FILE_LEN);
    if(audio_file_data == NULL)
    {
        TRACE_DBG("k_mallocapp fail\r\n");
    }
    #endif

    if(trendit_search_file_status(filename) == DDI_OK)
    {
        return DDI_OK;
    }

    if(trendit_search_download_status(filename) == DDI_OK)
    {

        //������Դ��Ϣ
        //ret = ddi_vfs_seek(g_audio_file_fd, audio_filedata_addr(g_audio_file_config.m_file_index), 0);
        ret = trendit_audio_vfs_write(audio_file_data, AUDIO_MAX_TEXT_FILE_LEN,audio_filedata_addr(g_audio_file_config.m_file_index));
        if(ret > 0)
        {
            ret = DDI_OK;
        }

        //�����ļ�headinfo��Ϣ
        memset(filehead, 0,sizeof(filehead));
        memcpy(filehead, filename, strlen(filename));
        filehead[AUDIO_MAX_TEXT_NAME_LEN+AUDIO_MAX_TEXT_DOWNLOAD_FLG -1] = 1;
        //ret = ddi_vfs_seek(g_audio_file_fd, audio_filename_addr(g_audio_file_config.m_file_index), 0);
        ret = trendit_set_file_headinfo(filehead, sizeof(filehead),audio_filename_addr(g_audio_file_config.m_file_index));
        if(ret > 0)
        {
            ret = DDI_OK;
        }

        
    }
    return ret;

}


s32 trendit_download_curr_audio_file(void)
{
    u8 audio_data[1024];
    
    u8 filehead[AUDIO_MAX_TEXT_NAME_LEN+AUDIO_MAX_TEXT_DOWNLOAD_FLG];

    s32 ret = -1;
    s32 i = 0;

    

    //������Դ��Ϣ
    //ret = ddi_vfs_seek(g_audio_file_fd, audio_filedata_addr(g_audio_file_config.m_file_index), 0);
    ret = trendit_audio_vfs_write(g_download_file_data, g_audio_file_config.m_file_offset,audio_filedata_addr(g_audio_file_config.m_file_index));
    if(ret > 0)
    {
        ret = DDI_OK;
    }

    //����flg�ж��Ǹ��»�������
    if(g_audio_file_config.m_file_flg == 0)
    {
        //�����ļ�headinfo��Ϣ
        memset(filehead, 0,sizeof(filehead));
        memcpy(filehead, g_audio_file_config.m_file_name, strlen(g_audio_file_config.m_file_name));
        filehead[AUDIO_MAX_TEXT_NAME_LEN+AUDIO_MAX_TEXT_DOWNLOAD_FLG -1] = 1;
        //ret = ddi_vfs_seek(g_audio_file_fd, audio_filename_addr(g_audio_file_config.m_file_index), 0);
        ret = trendit_set_file_headinfo(filehead, sizeof(filehead),audio_filename_addr(g_audio_file_config.m_file_index));
        if(ret > 0)
        {
            ret = DDI_OK;
        }
    }
    audiofile_destroy();
    return ret;
    
}



s32 trendit_update_audio_file(u8 *filename, u8 *audio_file_data, u32 audio_file_len)
{
    s32 ret = -1;
    s32 i = 0;

    if((audio_file_data == NULL) || (audio_file_len <=0) || (audio_file_len>AUDIO_MAX_TEXT_FILE_LEN))
    {
        return DDI_EINVAL;
    }
    #if 0
    audio_file_data = (u8*)k_malloc(AUDIO_MAX_TEXT_FILE_LEN);
    if(audio_file_data == NULL)
    {
        TRACE_DBG("k_mallocapp fail\r\n");
    }
    memset(audio_file_data, 1, AUDIO_MAX_TEXT_FILE_LEN);
    #endif

    if(trendit_search_file_status(filename) == DDI_OK)
    {
        TRACE_DBG("start update  curr file\r\n");

        //ret = ddi_vfs_seek(g_audio_file_fd, audio_filedata_addr(g_audio_file_config.m_file_index), 0);

        ret = trendit_audio_vfs_write(audio_file_data, AUDIO_MAX_TEXT_FILE_LEN, audio_filedata_addr(g_audio_file_config.m_file_index));
        if(ret > 0)
        {
            ret = DDI_OK;
        }
        TRACE_DBG("end update curr file\r\n");

    }
    return ret;

}

s32 trendit_delete_audio_file(u8 *filename)
{
    s32 ret = -1;
    u8 filehead[AUDIO_MAX_TEXT_NAME_LEN+AUDIO_MAX_TEXT_DOWNLOAD_FLG];

    memset(filehead, 0,sizeof(filehead));
    if(trendit_search_file_status(filename) == DDI_OK)
    {
        TRACE_DBG("start del curr file:%d\r\n",g_audio_file_config.m_file_index);
        //ret = ddi_vfs_seek(g_audio_file_fd, audio_filename_addr(g_audio_file_config.m_file_index), 0);
        ret = trendit_set_file_headinfo(filehead, sizeof(filehead),audio_filename_addr(g_audio_file_config.m_file_index));
        if(ret > 0)
        {
            ret = DDI_OK;
        }
        TRACE_DBG("end del curr file\r\n");

    }
    return ret;
}



#endif


