#include "ddiglobal.h"
#include "app_global.h"
#include "test_global.h"

#define MAX_TTS_NUM 12



s32 test_audio(void)
{
    s32 ret = -1;
    u8 flg = 1;
    u8 key;
    u8 vol_level = 0;
    u8  is_playing = 0;
    str_audio_tts audio_tts;

    //u8 test_buf[462] = 0;
    s32 i = 0;
    u8 test_tts_text[MAX_TTS_NUM][100] = 
    {
        "设备已联网成功",
        "你有新订单了",
        "设备升级中,请勿拔掉电源",
        "升级成功",
        "已进入wifi配网模式",
        "已进入wifi网页配置模式",
        "设备配置超时,退出配置模式",
        "音量加",
        "音量最大",
        "音量减",
        "正在恢复出厂设置,请勿拔掉电源",
        "恢复出厂成功",
    };


    while(1)
    {
        if(flg == 1)
        {
            TRACE_DBG("================AUDIO Test===============\n");
            TRACE_DBG("0.AUDIO open\n");
            TRACE_DBG("1.AUDIO play\n");
            TRACE_DBG("2.AUDIO close\n");
            TRACE_DBG("3.AUDIO pause\n");
            TRACE_DBG("4.AUDIO replay\n");
            TRACE_DBG("5.AUDIO KEY +\n");
            TRACE_DBG("6.AUDIO KEY -\n");

            TRACE_DBG("================AUDIO End=================\n");
            flg = 0;

        }
        if(ddi_com_read(PORT_CDCD_NO, (u8 *)&key, 1) > 0)
        {
            switch(key)
            {
                case '0':
                    ret = ddi_audio_open();
                    TRACE_DBG("ddi_audio_open ret:%d\r\n",ret);

                    break;

                case '1':
                    //audio_tts.m_text = "请输入密码撤销成功 好的 ";
                    //audio_tts.m_text = "好的 请输入金额拾圆";
                    audio_tts.m_text = test_tts_text[i];
                    i++;
                    if(i >= MAX_TTS_NUM)
                    {
                        i = 0;
                    }
                    ret = audio_instance()->audio_play(test_tts_text[i], AUDIO_PLAY_BLOCK);

                    TRACE_DBG("dev_dac_play ret:%d\r\n",ret);
                    break;
                case '2':
                    ret = ddi_audio_close();
                    TRACE_DBG("dev_dac_close ret:%d\r\n",ret);
                    break;
                case '3':
                    ret = ddi_audio_pause();
                    TRACE_DBG("dev_dac_pause ret:%d\r\n",ret);
                    break;
                case '4':
                    ret = ddi_audio_resumeplay();
                    TRACE_DBG("dev_dac_replay ret:%d\r\n",ret);
                    break;
                case '5':
                    ddi_audio_ioctl(DDI_AUDIO_GET_VOLUME ,(u32)(&vol_level) ,0);
                    TRACE_DBG("volume_level:%d\r\n",vol_level);
                    ret = ddi_audio_ioctl(DDI_AUDIO_CTL_VOLUME, vol_level + 1, 0);
                    TRACE_DBG("dev_dac_set_volume_level ret:%d\r\n",ret);
                    break;
                case '6':
                    ddi_audio_ioctl(DDI_AUDIO_GET_VOLUME, (u32)(&vol_level), 0);
                    TRACE_DBG("volume_level:%d\r\n",vol_level);
                    ret = ddi_audio_ioctl(DDI_AUDIO_CTL_VOLUME, vol_level - 1, 0);
                    TRACE_DBG("dev_dac_set_volume_level ret:%d\r\n",ret);
                    break;
                case '7':
                    #if 0
                    dev_flash_read(0x00101000+55360, test_buf, sizeof(test_buf));
                    dev_debug_printf("data:addr=%08X, len=%d\r\n",0x00101000+55360, sizeof(test_buf));
                    dev_debug_printformat(NULL, test_buf, sizeof(test_buf));
                    #else
                    ddi_audio_ioctl(DDI_AUDIO_CTL_GET_STATUS, (u32)&is_playing, 0);
                    TRACE_DBG("play status:%d\r\n",is_playing);
                    #endif
                    break;

                case 'e':
                    return 0;

            }
        
        }

        ddi_watchdog_feed();
    }


}

s32 auto_audio_test(void)
{
    u8 flg = 0;
    u8 key;
    u8 key_tested[3] = {0};
    s32 have_tested_num = 0;
    s32 ret = DDI_ERR;

    ddi_audio_ioctl(DDI_AUDIO_CTL_VOLUME, AUDIO_VOL_5, 0);
    audio_instance()->audio_play(AUDIO_TEST, AUDIO_PLAY_BLOCK);
    
    ddi_audio_ioctl(DDI_AUDIO_CTL_VOLUME, AUDIO_VOL_4, 0);
    audio_instance()->audio_play(AUDIO_TEST, AUDIO_PLAY_BLOCK);
    ddi_audio_ioctl(DDI_AUDIO_CTL_VOLUME, AUDIO_VOL_3, 0);
    audio_instance()->audio_play(AUDIO_TEST, AUDIO_PLAY_BLOCK);
    ddi_audio_ioctl(DDI_AUDIO_CTL_VOLUME, AUDIO_VOL_2, 0);
    audio_instance()->audio_play(AUDIO_TEST, AUDIO_PLAY_BLOCK);
    ddi_audio_ioctl(DDI_AUDIO_CTL_VOLUME, AUDIO_VOL_1, 0);
    audio_instance()->audio_play(AUDIO_TEST, AUDIO_PLAY_BLOCK);
    memset(key_tested, 0, sizeof(key_tested));
    ddi_key_clear();
    while(1)
    {
        if(!flg)
        {
            flg = 1;
            audio_instance()->audio_play(AUDIO_TEST_KEY_FINISH, AUDIO_PLAY_BLOCK);
            ddi_key_clear();
        }

        key = trendit_get_key();
        if(key > 0)
        {
            switch(key)
            {
                case KEY_CFG:

                    ret = DDI_OK;
                    goto _out;
                    break;

                default:

                    ret = DDI_ERR;
                    goto _out;
                    break;
            }
        }
    }

_out:

    return ret;
}


void audio_download_test(void)
{
    u32 key;
    u8 flg = 1;
    s32 ret = -1;
    s32 i = 0;
    s32 j = 0;
    u8 tmpfilename[128];
    u8 audio_data[1024];
    s32 audio_index = 0;
    s32 fileoffset = 0;
    s32 g_test_file_fd = -1;
    u8 *audio_file_data;
    str_audio_tts audio_tts;

    u8 test_tts_text[MAX_TTS_NUM][100] = 
    {
        "你有新订单了",
        "请输入金额，升级成功",
        "退出",
        "退出配置模式",
        "已进入wifi配网模式",
        "已进入wifi网页配置模式",
        "设备配置超时,退出配置模式",
        "音量加",
        "音量最大",
        "音量减",
        "正在恢复出厂设置,请勿拔掉电源",
        "恢复出厂成功",
    };

    while(1)
    {
        if(flg == 1)
        {
            TRACE_DBG("================audio download Test===============\n");
            TRACE_DBG("1.audio download from tts\n");
            TRACE_DBG("2.audio updata from tts\n");
            TRACE_DBG("3.audio delete\n");
            TRACE_DBG("4.audio play\n");
            TRACE_DBG("5.audio read filename\n");
            TRACE_DBG("6.audio read file data\n");
            TRACE_DBG("7.audio write file name\n");
            TRACE_DBG("8.clear write file name\n");

            TRACE_DBG("================audio download End=================\n");
            flg = 0;

        }
        if(ddi_com_read(PORT_CDCD_NO, (u8 *)&key, 1) > 0)
        {
            TRACE_DBG("key:%x\r\n",key);
            switch(key)
            {
                case '1':

                    #if 1

                    audio_file_data = (u8*)k_mallocapp(AUDIO_MAX_TEXT_FILE_LEN);
                    if(audio_file_data == NULL)
                    {
                        TRACE_DBG("k_mallocapp fail\r\n");
                        break;
                    }
                    memset(audio_file_data, 1, AUDIO_MAX_TEXT_FILE_LEN);
                    #endif
                    trendit_download_audio_file("请输入金额", audio_file_data, AUDIO_MAX_TEXT_FILE_LEN);
                    k_freeapp(audio_file_data);
                    break;
                case '2':
                    #if 1
                    audio_file_data = (u8*)k_mallocapp(AUDIO_MAX_TEXT_FILE_LEN);
                    if(audio_file_data == NULL)
                    {
                        TRACE_DBG("k_mallocapp fail\r\n");
                        break;
                    }
                    memset(audio_file_data, 2, AUDIO_MAX_TEXT_FILE_LEN);
                    #endif
                    trendit_update_audio_file("请输入金额", audio_file_data, AUDIO_MAX_TEXT_FILE_LEN);
                    k_freeapp(audio_file_data);

                    break;
                case '3':

                    trendit_delete_audio_file("请输入金额");

                    break;
                case '4':
                    //audio_instance()->audio_play("升级成功,请输入金额", AUDIO_PLAY_NOBLOCK);
                    //ddi_audio_ioctl(DDI_AUDIO_CTL_VOICE,0,(u32)"升级成功,");
                    audio_tts.m_text = test_tts_text[i];
                    i++;
                    if(i >= MAX_TTS_NUM)
                    {
                        i = 0;
                    }
                    ret = audio_instance()->audio_play(audio_tts.m_text, AUDIO_PLAY_BLOCK);
                    //ret = ddi_audio_ioctl(DDI_AUDIO_CTL_VOICE,0,(u32)audio_tts.m_text);
                    break;
                case '5':



                    #if 0
                    memset(tmpfilename, 0, sizeof(tmpfilename));
                    dev_audio_vfs_read(AUDIO_SOURCE_PATH, tmpfilename, sizeof(tmpfilename), 0);

                    #else
                    for(i = 0; i < AUDIO_MAX_NUM; i++)
                    {
                        memset(tmpfilename, 0, sizeof(tmpfilename));
                        trendit_audio_vfs_read( tmpfilename, sizeof(tmpfilename), audio_filename_addr(i));
                        if(tmpfilename != NULL)
                        {
                            TRACE_DBG("file name:%s \r\n",tmpfilename);
                        }
                    }
                    #endif

                    break;
                case '6':


                    fileoffset = 0;

                    #if 1
                    for(i = 0; i < 10; i++)
                    {
                        memset(audio_data, 0, sizeof(audio_data));
                        trendit_audio_vfs_read(audio_data, sizeof(audio_data), audio_filedata_addr(0)+fileoffset);
                        fileoffset = fileoffset + 1024;

                        for(j = 0; j < 1024; j++)
                        {
                            TRACE_DBG("%x,",audio_data[j]);
                        }

                    }
                    #else
                    for(i = 0; i < AUDIO_MAX_TEXT_FILE_CNT; i++)
                    {
                        memset(audio_data, 0, sizeof(audio_data));
                        fs_read_file(AUDIO_SOURCE_PATH, audio_data, sizeof(audio_data), audio_filedata_addr(audio_index)+fileoffset);
                        fileoffset = fileoffset + 1024;
                        if(i == 0)
                        {
                            for(j = 0; j < 1024; j++)
                            {
                                TRACE_DBG("%x,",audio_data[i]);
                            }
                        }
                        
                    }
                    audio_index++;
                    if(audio_index >= AUDIO_MAX_NUM)
                    {
                        audio_index = 0;
                    }
                    #endif

                    break;

                case '7':

                    memset(tmpfilename, 0x00, sizeof(tmpfilename));
                    strcpy(tmpfilename,"请输入金额");
                    TRACE_DBG("set file name start\r\n");
                    trendit_audio_vfs_write(tmpfilename, strlen(tmpfilename),audio_filename_addr(0));
                    TRACE_DBG("set file name end\r\n");

                    break;
                case '8':


                    memset(tmpfilename, 0x0, sizeof(tmpfilename));
                    strcpy(tmpfilename,"测试");
                    TRACE_DBG("clear file name start\r\n");

                    trendit_audio_vfs_write(tmpfilename, strlen(tmpfilename),audio_filename_addr(0));
                    //fs_write_file(AUDIO_SOURCE_PATH, tmpfilename, sizeof(tmpfilename));
                    TRACE_DBG("clear file name end\r\n");

                    break;
                case '9':


                    memset(tmpfilename, 0x0, sizeof(tmpfilename));
                    strcpy(tmpfilename,"请输入金额");
                    TRACE_DBG("9999set file name start\r\n");

                    trendit_audio_vfs_write(g_test_file_fd, tmpfilename, sizeof(tmpfilename),audio_filename_addr(0));
                    //fs_write_file("/mtd0/test", tmpfilename, strlen(tmpfilename));
                    TRACE_DBG("999set file name end\r\n");

                    break;
                case 'e':
                    return ;

            }
            flg  = 1;
        
        }

        ddi_watchdog_feed();
    }

}



