#include "ddiglobal.h"
#include "app_global.h"

#include "usbcdc_download.h"


#define HARDWAREVER  0x10
#define MACHINECODE  0x51

static DOWNFILEINFO_t *g_downfileinfo;          //��¼���������ļ���Ϣ
static DOWNFILEINFO_t *g_currdownfileinfo;      //��¼��ǰ�����ļ���Ϣ
static u8 g_dl_apupdateflg;     //��¼�����ļ����Ƿ���apupdate�ļ�
static u8 g_dl_recfilenum;      //��¼�����ļ�����
const CANDOWNFILEINFO_t candownfileinfo_tab[] = 
{
    {"apupdate", DOWNLOAD_FILE_TYPE_UPDATEBAK, 0, 0},
    {"gbk2424.bin", DOWNLOAD_FILE_TYPE_RESOURCE, 0, 0},
    {"gbk1616.bin", DOWNLOAD_FILE_TYPE_RESOURCE, 0, 0},
    {"tts.bin", DOWNLOAD_FILE_TYPE_RESOURCE, 0, 0},

    //----------------------------------------------
    {NULL, DOWNLOAD_FILE_TYPE_NULL, -1, 0},
};

void dl_deal_processrecbag(FILEBAGINFO_t *lp_filebaginfo, u8 *wbuf, u32 wlen)
{
	lp_filebaginfo->m_filenum = wbuf[0];
    lp_filebaginfo->m_bagno = wbuf[1];
    lp_filebaginfo->m_bagnosum = wbuf[2];
    lp_filebaginfo->m_machtype = wbuf[3];
    lp_filebaginfo->m_hardver = wbuf[4];
    memset(lp_filebaginfo->m_filename,0,51);
    memcpy(lp_filebaginfo->m_filename,&wbuf[5],51);
    lp_filebaginfo->m_filelen = trendit_hex_to_u32(&wbuf[56], 4);
    lp_filebaginfo->m_crc = trendit_hex_to_u32(&wbuf[60], 4);
    lp_filebaginfo->m_offset = trendit_hex_to_u32(&wbuf[64], 4);
    lp_filebaginfo->m_p = &wbuf[68];
}
s32 dl_deal_getDirAndName(u8 *tempname,u8 *dir,u8 *name)
{
    int i;
    u8 ret = 0;

    if(tempname[0] == 0x2F)
    {
        for(i = (strlen((s8 *)tempname)-1); i >= 0; i--)
        {
            if(tempname[i] == 0x2F)
            {
                memcpy(dir,tempname,i+1);
                strcpy((s8 *)name,(s8 *)&tempname[i+1]);
                dir[i+1] = 0;
                ret = 1;
                break;
            }
        }
    }
    else
    {
        for(i = (strlen((s8 *)tempname)-1); i >= 0; i--)
        {
            if(tempname[i] == 0x2F)
            {
                strncat((s8 *)dir,(s8 *)tempname,i+1);
                strcpy((s8 *)name,(s8 *)&tempname[i+1]);
                dir[i+1] = 0;
                ret = 1;
                break;
            }
        }
        if(i < 0) strcpy((s8 *)name,(s8 *)tempname);
    }

    return ret;
	
}
//��ȡ�����ļ���ַ������
s32 dl_deal_getfileaddr(s8 *filename, u8* tmpname, u8 *filetype)
{
    s32 i;
    u8 dir[100],name[100];
    s32 ret = -1;
    u32 len;

    dir[0] = 0;
    name[0] = 0;
    dl_deal_getDirAndName((u8*)filename, dir, name);
    *filetype = DOWNLOAD_FILE_TYPE_RESOURCE;
//DL_DEBUG("filename=%s, dir=%s, name=%s\r\n", filename, dir, name);
    i = 0;
    while(1)
    {
        if(candownfileinfo_tab[i].m_filetype == -1)
        {
            //û���ҵ�����ļ�
            DL_DEBUG("Not find file(%s)\r\n", name);
            return -1;
        }
//DL_DEBUG("name=[%s],[%s]\r\n", candownfileinfo_tab[i].m_filename, name);        
        if(0 == strcmp(candownfileinfo_tab[i].m_filename, name))
        {           
            *filetype = candownfileinfo_tab[i].m_filetype;
//DL_DEBUG("filetype=%d\r\n", *filetype); 
            ret = 1;
            break;
        }
        i++;
    }
//DL_DEBUG("i=%d\r\n", i); 
    if(tmpname != NULL)
    {
        strcpy((char *)tmpname,"/mtd2/");
        strcat((char *)tmpname,(char *)name);
    }
//DL_DEBUG("tmpname=%s\r\n", tmpname);    
    ret = ddi_static_vfs_getarea(tmpname, 0, &len);  //���ֻ��Ծ�̬�ļ�����
//DL_DEBUG("ret=%d, tmpname=%s,len=%d\r\n", ret, tmpname, len);    
    return ret;
}
//�ж������ļ�
s32 dl_deal_checkfilelist(FILEBAGINFO_t *lp_filebaginfo)
{
    s32 ret;
    s32 i;
    u8 filename[52];
    u8 name[52];
    u8 filetype;
    u32 len;
    s32 addr;
    u8 fileflg= 0;      //0:�����ļ�
                        //1��mtd1�ļ�

    
    strcpy((s8*)name, (s8*)lp_filebaginfo->m_filename);
    trendit_strchr(name, '\\', '/');  //�ַ����
//DL_DEBUG("name=%s\r\n", name);    
    addr = dl_deal_getfileaddr((s8*)name, filename, &filetype);
//DL_DEBUG("addr=%d, fileflg=%d\r\n", addr, fileflg);    
    if((addr < 0) && (0 == fileflg))
    {
        DL_DEBUG("File NOT in list");
        return EXECUTION_DOWNFILENAME;  
    }
    //--����Ƿ���g_downfileinfo�����б���,
    //������ڣ�˵���ǵ�һ�������������
    for(i=0; i<MAXFILE_NUM; i++)
    {
        if(addr == g_downfileinfo[i].m_addr)
        {
            g_currdownfileinfo = &g_downfileinfo[i];
            return 0;
        }
    }
    //�ļ�δ����
    for(i=0; i<MAXFILE_NUM; i++)
    {
        if(g_downfileinfo[i].m_addr == -1)
        {
            break;
        }
    }
    if(i==MAXFILE_NUM)
    {
        DL_DEBUG("Downfilenum err\r\n");
        return EXECUTION_DOWNFILENUM;
    }
    g_currdownfileinfo = &g_downfileinfo[i];
    //��һ�����Ǽ���Ϣ
    g_currdownfileinfo->m_filelen = lp_filebaginfo->m_filelen;
    if((1==fileflg) && (len<g_currdownfileinfo->m_filelen))
    {
        DL_DEBUG("disk not enough(fileflg=%d, len(%d,%d)\r\n", fileflg, len, g_currdownfileinfo->m_filelen);
        return EXECUTION_DISKNOTENOUGH;
    }
    g_currdownfileinfo->m_addr = ddi_static_vfs_getarea(filename, 0, &len);
    if(g_currdownfileinfo->m_addr < 0)
    {
        g_currdownfileinfo->m_addr = -1;
        DL_DEBUG("File Err(addr=%d, filename=%s,len=%d)\r\n", g_currdownfileinfo->m_addr, filename, len);
        return EXECUTION_DOWNFILENAME;
    }
    if(g_currdownfileinfo->m_filelen > len)
    {
        g_currdownfileinfo->m_addr = -1;
        DL_DEBUG("Filelen Err(filename=%s,len=%d,%d)\r\n", filename, g_currdownfileinfo->m_filelen, len);
        return EXECUTION_DISKNOTENOUGH;
    }

    g_currdownfileinfo->m_flg = 1;      //need download
    g_currdownfileinfo->m_crc = lp_filebaginfo->m_crc;
    //memset(g_currdownfileinfo->m_filename, 0, sizeof(g_currdownfileinfo->m_filename));
    strcpy(g_currdownfileinfo->m_filename, (s8*)filename);
    g_currdownfileinfo->m_filetype = filetype;
    return 0;
}

s32 dl_deal_checkifmodulematch(u8 machineid,u8 firmwareversion,u8 flag)
{
  #if 0  //��ʱ��֧�ֻ��ͺź�Ӳ���汾�ж�
    //�жϻ����ͺ�
    if(machineid != MACHINECODE)
    {
        //bootdownload_processerr(FileBagInfo.bagno,EXECUTION_MACHINEMODE);
        DL_DEBUG("Machtype(0x%02X, 0x%02X) err!\r\n", l_filebaginfo.m_machtype, MACHINECODE);
        return -1;
    }
    //�ж�Ӳ���汾��
    if(firmwareversion != HARDWAREVER)
    {
        //bootdownload_processerr(FileBagInfo.bagno,EXECUTION_HARDWAREVERSION);
        DL_DEBUG("hardver(0x%02X, 0x%02X) err!\r\n", l_filebaginfo.m_hardver, HARDWAREVER);
        return -1;
    }
  #endif  
    return 0;
}

void dl_deal_processerr(u8 bagno,u8 errtype)
{
  
    //bootdownload_beep(BOOTDOWNLOAD_BEEP_ERR);
    //bootdownload_prompt(errtype);
    //dev_debug_printf("bo=%d, et=%d\r\n", bagno, errtype);
}

s32 dl_deal_downloadbag(FILEBAGINFO_t *lp_filebaginfo)
{
    s32 curbags, filebags;
    u32 len;
    u32 crc;
    s32 i, count;
    s32 ret;
    u8 disp[32];

    curbags = (lp_filebaginfo->m_offset)>>DOWNLOADBLOCKOFFSET;
    filebags = (lp_filebaginfo->m_filelen)>>DOWNLOADBLOCKOFFSET;
    //if((lp_filebaginfo->m_filelen%PC_DL_BLOCK_SIZE)>0)
    if((lp_filebaginfo->m_filelen&((1<<DOWNLOADBLOCKOFFSET)-1))>0)
    {
        filebags += 1;
    }
//DL_DEBUG("offset=%d, filelen=%d, curbags=%d,filebags=%d\r\n", lp_filebaginfo->m_offset, lp_filebaginfo->m_filelen, curbags, filebags);
  #if 0
    dev_lcd_clear_rowram(DISPAGE2);
    sprintf(disp, "%s(%d/%d)", lp_filebaginfo->filename, curbags+1, filebags);
    dev_lcd_fill_rowram(DISPAGE2, 0, disp, LDISP);
    dev_lcd_brush_screen();
  #endif

    if((curbags+1) != filebags)
    {
        ret = ddi_file_insert(g_currdownfileinfo->m_filename, lp_filebaginfo->m_p, PC_DL_BLOCK_SIZE, (curbags<<DOWNLOADBLOCKOFFSET));//lp_filebaginfo->m_offset);
 //       DL_DEBUG("ret = %d\r\n", ret);
        if(ret != PC_DL_BLOCK_SIZE)
        {
            //д�ļ�ʧ��
            //bootdownload_processerr(lp_filebaginfo->bagno,EXECUTION_DOWNFILEOPS);
            DL_DEBUG("File insert err(ret=%d)!\r\n", ret);
            return 1;
        }
        return 3;       //�����ļ����һ��,��������
    }
    //���һ������
    len = lp_filebaginfo->m_filelen - lp_filebaginfo->m_offset;
    ret = ddi_file_insert(g_currdownfileinfo->m_filename, lp_filebaginfo->m_p, len, (curbags<<DOWNLOADBLOCKOFFSET));
    if(ret != len)
    {
        //bootdownload_processerr(lp_filebaginfo->bagno,EXECUTION_DOWNFILEOPS);
        DL_DEBUG("File insert err(ret=%d)!\r\n", ret);
        return 1;
    }
    //------------------------------------------------------
    //CHECK FILE CRC
    ret = trendit_calc_filecrc32(g_currdownfileinfo->m_filename, 0, lp_filebaginfo->m_filelen, &crc);
    if(ret < 0)
    {
        //bootdownload_processerr(lp_filebaginfo->bagno,EXECUTION_DOWNFILEOPS);
        DL_DEBUG("FILE calc CRC32 err!\r\n");
        return 1;
    }
    DL_DEBUG("CHECK CRC:0x%08X, 0x%08X\r\n", crc, lp_filebaginfo->m_crc);
    if(crc != lp_filebaginfo->m_crc)
    {
        //bootdownload_processerr(lp_filebaginfo->bagno,EXECUTION_CRCERR);
        DL_DEBUG("CRC err(0x%08X, 0x%08X)!\r\n", crc, lp_filebaginfo->m_crc);
        return EXECUTION_CRCERR;//2;
    }
    return 0;
}
//�����ļ�
s32 dl_deal_downloadfilesave(u8* srcfilename, u8* destfilename, u32 filelen)
{
    //Ŀǰ��ֱ�ӱ������ص��ļ���������Ҫת��
    return 0;
}

s32 dl_deal_task(u8* wbuf, u32 wlen, s32 *errno)
{
    FILEBAGINFO_t l_filebaginfo;
    s32 ret;
    s32 updataflg = 0;
    

    memset((u8*)&l_filebaginfo, 0, sizeof(FILEBAGINFO_t));
    //�����յ�����ת��l_filebaginfo
    dl_deal_processrecbag(&l_filebaginfo, wbuf, wlen);
    //������صĹ̼���ģ���Ƿ�ƥ��
    ret = dl_deal_checkifmodulematch(l_filebaginfo.m_machtype,l_filebaginfo.m_hardver,0);
    if(ret != 0)
    {
        dl_deal_processerr(l_filebaginfo.m_bagno,EXECUTION_HARDWAREVERSION);
        *errno = EXECUTION_HARDWAREVERSION;
        return -1;
    }
    //bootdownload_prompt_downloading(l_filebaginfo.m_filename,gRecFileNum,gFileBagInfo->filenum);
        
//DL_DEBUG("bagno=%d\r\n", l_filebaginfo.m_bagno);  
    //���ڵ�һ����ʾ
    if(l_filebaginfo.m_bagno == 0)
    {
        //bootdownload_prompt_downloading(FileBagInfo.filename);
        DL_DEBUG("downloadfile:%s\r\n", l_filebaginfo.m_filename);
        
    }
    //��Ѱ�ļ��Ƿ��������أ���ȡ�ռ䳤��
    ret = dl_deal_checkfilelist(&l_filebaginfo);
//DL_DEBUG("ret=%d\r\n", ret);  
    if(ret != 0)
    {
        //bootdownload_processerr(FileBagInfo.bagno,ret);
        DL_DEBUG("dealdownloadlist(ret=%d) err!\r\n", ret);
        *errno = ret;
        return -1;
    }
    //--------------------------------------------------
    //�������ذ�
    ret = dl_deal_downloadbag(&l_filebaginfo);
//DL_DEBUG("ret=%d\r\n", ret);  
    if(ret == 1)
    {
        //�ļ���������,�������� 
        //bootdownload_processerr(FileBagInfo.bagno,EXECUTION_DOWNFILEOPS);
        DL_DEBUG("dealdownloadbag(ret=%d) err!\r\n", ret);
        *errno = UPDATE_STATUS_WRITEFAIL;
        return -2;
    }
    else if(ret == EXECUTION_CRCERR)
    {
        //�ļ�У���
        //bootdownload_processerr(FileBagInfo.bagno,EXECUTION_CRCERR);
        DL_DEBUG("dealdownloadbag(ret=%d) err!\r\n", ret);
        *errno = UPDATE_STATUS_CRCERR;
        return -1;
    }
    else if(ret == 3)
    {
        //�ð��������������պ�����
        DL_DEBUG("\r\n");
        *errno = UPDATE_STATUS_SUCCESS;
        return 0;
    }
//DL_DEBUG("filename=%s\r\n", g_currdownfileinfo->m_filename);  
    //���ļ�������ɣ�����ǩ��
    if(0 == strcmp(g_currdownfileinfo->m_filename, "apupdate")
     ||0 == strcmp(g_currdownfileinfo->m_filename, "/mtd2/apupdate"))
    {
        //apupdate����У�飬���ڲ��ļ���ҪУ��
        ret = 0;
    }
    else
    {
        //�ݲ�����
        ret = 0;
    }
    if(ret != 0)
    {
        //�ļ�У���
        g_currdownfileinfo->m_failflg = 2;
        //bootdownload_processerr(gFileBagInfo->bagno,AUTHENTICATEFAIL);
        DL_DEBUG("File authen(ret=%d) err!\r\n", ret);
        *errno = UPDATE_STATUS_AUTHENFAIL;
        return -1;
    }
    else
    {
        //�����ļ�
        //ret = dl_deal_downloadfilesave()
        ret = 0;
        if(ret != 0)
        {
            //bootdownload_processerr(gFileBagInfo->bagno,EXECUTION_SAVEINFOERROR);
            DL_DEBUG("File save(ret=%d) err!\r\n", ret);
            *errno = UPDATE_STATUS_SAVEFAIL;
            return -1;
        }
        else
        {
            DL_DEBUG("save success:%s\r\n", g_currdownfileinfo->m_filename);
            g_currdownfileinfo->m_failflg = 0;      //save success
            if(0 == strcmp(g_currdownfileinfo->m_filename, "apupdate")
             ||0 == strcmp(g_currdownfileinfo->m_filename, "/mtd2/apupdate"))
            {
	            ddi_misc_ioctl(MISC_IOCTL_SET_UPDATEFLG, 0, 0);
                g_dl_apupdateflg = 1;       //��Ҫ��������
            }
        }
    }

    //�������
    g_dl_recfilenum ++;
    if(g_dl_recfilenum >= l_filebaginfo.m_filenum)
    {
        //�������
        DL_DEBUG("Download Success!\r\n");
        *errno = UPDATE_STATUS_SUCCESS;
        return 1;
    }
    *errno = UPDATE_STATUS_SUCCESS;
    return 2;       //������һ���ļ�
}



s32 bootdownload_deal_audiofile(void)
{
    s32 i;
    s32 ret = DDI_OK; 
    
    for(i=0; i<g_dl_recfilenum; i++)
    {
         if(0 == strcmp(g_downfileinfo[i].m_filename, "tts.bin")
            ||0 == strcmp(g_downfileinfo[i].m_filename, "/mtd2/tts.bin"))
         {
            if(g_downfileinfo[i].m_failflg == 0)
            {
                if(MACHINE_P7_HEX == trendit_get_machine_code())
                {
                    //�ȱ�����Ա�ʶ����spi flash������
                    test_savettsfilesyncresult(TTSFILE_SYNC_FAIL); //ͬ��֮ǰ����fail��ʶ
                
                    //ͬ�������Ĺ����У���ledѭ����ʾ
                    ddi_led_sta_set(LED_SIGNAL_Y, 0);
                    ddi_led_sta_set(LED_SIGNAL_B, 
0);
                    ddi_led_sta_set(LED_STATUS_B, 0);
                    ddi_led_sta_set(LED_STATUS_R, 0);
                    ddi_led_ioctl(DDI_LED_CTL_LOOP, 1, 0);//led ѭ����ʾ
                    //�ȱ�����Ա�ʶ����spi flash������
                    //test_savettsfilesyncresult(TTSFILE_SYNC_FAIL); //ͬ��֮ǰ����fail��ʶ
                    audio_instance()->audio_play(AUDIO_SYNCAUDIOWITHMODEM, AUDIO_PLAY_BLOCK);
                    ret = ddi_audio_ioctl(DDI_AUDIO_SYNC_AUDIOFILE, 0, 0);
                    if(ret != DDI_OK)
                    {
                        audio_instance()->audio_play(AUDIO_AUDIOFILE_SYNCERROR, AUDIO_PLAY_BLOCK);
                        
                    }
                    else
                    {
                        test_savettsfilesyncresult(TTSFILE_SYNC_SUCCESS); //ͬ���ɹ�����ͬ���ɹ���ʶ
                        //test_savettsfilesyncresult(TTSFILE_SYNC_SUCCESS); //ͬ���ɹ�����ͬ���ɹ���ʶ
                        audio_instance()->audio_play(AUDIO_AUDIOFILE_SYNCESUCCESS, AUDIO_PLAY_BLOCK);
                    }
                }
            }
         }
    }
    
    return ret;
    
}



#define DLBAG_TIMEROVER         2000                //������֮��ĳ�ʱ
#define DLFILE_TIMEROVER(x)     (((x>>12)*200)+2000)  //�ļ���ʱʱ��
s32 dl_deal_main(void)
{
    u32 dlbagtimeid;
    s32 ret;
    frameinfo_t *lp_frameinfo;
    s32 i;
    u32 timeid;
    s32 errno;
    
//DL_DEBUG("start\r\n");
    //�����¼�����ļ���Ϣ
    g_downfileinfo = (DOWNFILEINFO_t*)k_mallocapp(sizeof(DOWNFILEINFO_t)*MAXFILE_NUM);
    if(NULL == g_downfileinfo)
    {
        DL_DEBUG("malloc err");
        return EXECUTION_MEMORY;//-1;
    }
//DL_DEBUG("\r\n");
    memset((u8*)g_downfileinfo, 0, sizeof(DOWNFILEINFO_t)*MAXFILE_NUM);
    for(i=0; i<MAXFILE_NUM; i++)
    {
        g_downfileinfo[i].m_addr = -1;
    }
    g_dl_apupdateflg = 0;
    g_dl_recfilenum = 0;

    //��������ģʽ
    audio_instance()->audio_play(AUDIO_UPLOADING, AUDIO_PLAY_NOBLOCK);//AUDIO_PLAY_BLOCK);
    //ddi_misc_get_tick(&dlbagtimeid);
    dlbagtimeid = DLBAG_TIMEROVER;
    ddi_user_timer_open(&dlbagtimeid);
    lp_frameinfo = usb_frame_parse_getinfo();
    timeid = trendit_get_ticks();
//DL_DEBUG("\r\n");    
    while(1)
    {
        ddi_watchdog_feed();
        ret = usb_frame_parse();
//DL_DEBUG("ret=%d, protocoltype=%d\r\n", ret, lp_frameinfo->m_protocoltype); 
        if(ret)
        {
            if(lp_frameinfo->m_protocoltype == PROTOCOL_TYPE_55AA)
            {
                ddi_user_timer_close(&dlbagtimeid);
                //������������
                errno = 0;
                ret = dl_deal_task(lp_frameinfo->m_rxbuf, lp_frameinfo->m_rxlen, &errno);
//DL_DEBUG("ret=%d\r\n", ret); 
                bootdownload_sendresponse(errno);
                dlbagtimeid = DLBAG_TIMEROVER;
                if(ret < 0)
                {
                    //
                }
                else if(ret == 1)
                {
                    //���tts�ļ��Ƿ��и���,��������ļ�
                    ret = bootdownload_deal_audiofile();
                    break;
                    
                }
                else if(ret == 2)
                {
                    //һ���ļ��������
                    dlbagtimeid = DLFILE_TIMEROVER(g_currdownfileinfo->m_filelen);
                }
                ddi_user_timer_open(&dlbagtimeid);
            }
            else
            {
                //�������������
            }
            usb_frameinfo_free();
        }
        if(0 == dlbagtimeid)
        {
            //ʱ�䵽
            ret = -1;
            DL_DEBUG("timerover!\r\n");
            errno = UPDATE_STATUS_OTHERERR;    //��ʱ����
            bootdownload_sendresponse(errno);
            break;
        }
      #if 0
        if(trendit_query_timer(timeid, 1000))
        {
            timeid = trendit_get_ticks();
            DL_DEBUG("timeid=%d, dlt=%d\r\n", timeid, dlbagtimeid);
        }
      #endif
        
        
    }
    k_freeapp(g_downfileinfo);
    g_downfileinfo = NULL;
    
    //ret    <0,�����쳣��=0���سɹ�
    if(ret == 0)
    {
//        bootdownload_sendresponse(ret);
        trenit_remove_all_device_cfg_file();
        audio_instance()->audio_play(AUDIO_UPLOADED, AUDIO_PLAY_BLOCK);
        if(g_dl_apupdateflg==1)
        {
            //����
            //����������ɣ�����
            system_data_instance()->set_poweroff_reason(RESEASON_REBOOT_UPDATE);
            bootdownload_sendresponse(ret);
            audio_instance()->audio_play(AUDIO_REBOOT, AUDIO_PLAY_BLOCK);
            ddi_misc_reboot();
        }
    }
    else
    {
//        bootdownload_sendresponse(ret);
        audio_instance()->audio_play(AUDIO_UPLOADED_ERR, AUDIO_PLAY_BLOCK);
    }
    return ret;
}


