#include "ddiglobal.h"
#include "app_global.h"
#include "test_global.h"

//TRACE_DBG("%s\r\n", str);
s32 test_wifi_signle(u8* target_ap, u8* target_pwd)
{
    u8 flg;
    s32 err;
    s32 ap_num;
    u32 index;
    wifi_apinfo_t aps[20];
    s32 select_ap_index;
    wifi_apinfo_t *select_ap = NULL;
//  TrenditInputText* pwd_input = NULL;
    u8 pwd[20] = {0};
    u8 ap_name[64] = {0};
    u32 ap_found = 0;
    u8 ap_name_with_quote[64] = {0};
    u8 error_info[128] = {0};
//  target_ap = "Trendit-Guest";
//  target_pwd = "trenditguest001";
    u32 key;
    u32 timeid;
    
    u32 get_signal_start_tick = 0;
    u32 cur_sys_tick = 0;

    do
    {
        #if 0
        lcd_interface_clear_ram();
        lcd_interface_fill_rowram(LINE1, 0, "WIFI测试", CDISP);   
        lcd_interface_fill_rowram(LINE3, 0, "正在打开...", CDISP);   
        lcd_interface_brush_screen();
        #else
        TRACE_DBG("WIFI测试\r\n");
        TRACE_DBG("正在打开...\r\n");
        #endif
        err = ddi_wifi_open();//dev_wifi_open();
        if(err!=0)
        {
            TRACE_DBG("正在打开...\r\n");//lcd_interface_fill_rowram(LINE3, 0, "正在打开...", CDISP);   
            //lcd_interface_brush_screen();
            //dev_user_delay_ms(1000);
            break;
        }
        ap_num = sizeof(aps)/sizeof(aps[0]);
        TRACE_DBG("正在搜网...\r\n");
        //lcd_interface_fill_rowram(LINE3, 0, "正在搜网...", CDISP);   
        //lcd_interface_brush_screen();
        err = ddi_wifi_scanap_start(aps, ap_num);//dev_wifi_scanap_start(aps, ap_num);
        if(err == 0)
        {
            while(1)
            {
                ddi_watchdog_feed();
                err = ddi_wifi_scanap_status(&ap_num);//dev_wifi_scanap_status(&ap_num);
                if(err == 0)
                {
                    //正在搜索
                }
                else if(err == 1)
                {
                    //搜索成功
                    err = 0;
                    break;
                }
                else
                {
                    break;
                }
            }
        }
        if(err != 0)
        {
            TRACE_DBG("搜网失败:%d", err);
            //lcd_interface_clear_rowram(LINE3);
            //lcd_interface_fill_rowram(LINE3, 0, "搜网失败", CDISP);   
            break;
        }
        //if((target_ap != NULL) && (target_pwd != NULL))
        {
            for(index=0; index<ap_num; index++)
            {
                sprintf(ap_name_with_quote, "\"%s\"", target_ap);
                if(strcmp(ap_name_with_quote, aps[index].m_ssid) == 0)
                {
                        strcpy(ap_name, target_ap);
                        strcpy(pwd, target_pwd);
                        ap_found = 1;
                }
                //Trace("debug", "wifi: %s \r\n", aps[index].m_ssid);
//             dev_debug_printf("%s(%d):aps[%d]=%s\r\n", __FUNCTION__, __LINE__, index, aps[index].m_ssid);
            }
            if(!ap_found)
            {
                sprintf(error_info, "没有找到目标热点:%s", target_ap);
                TRACE_DBG("%s\r\n",error_info);
                //lcd_interface_clear_rowram(LINE3);
                //lcd_interface_fill_rowram(LINE3, 0, error_info, CDISP);
                err = -1;
                break;
            }
        }

        TRACE_DBG("正在连接AP...\r\n");
        //lcd_interface_clear_rowram(LINE3);
        //lcd_interface_fill_rowram(LINE3, 0, "正在连接AP...", CDISP);
        //lcd_interface_brush_screen();
        err = ddi_wifi_connectap_start(ap_name, pwd, NULL);//dev_wifi_connetap(ap_name, pwd, NULL); //ddi_wifi_connectap_start
        if(err == 0)
        {
            while(1)
            {
                ddi_watchdog_feed();
                err = ddi_wifi_connectap_status();//dev_wifi_get_connectap_status(); //ddi_wifi_connectap_status
                if(err == 0)
                {
                    
                }
                else if(err = 2)
                {
                    //连接成功
                    err = 0;
                    break;
                }
                else
                {
                    break;
                }
            }
        }
        if(err != 0)
        {
            TRACE_DBG("连接AP失败..\r\n");
            //lcd_interface_clear_rowram(LINE3);
            //lcd_interface_fill_rowram(LINE3, 0, "连接AP失败..", CDISP);
            break;
        }
        timeid = 0;
        flg = 1;

        ddi_misc_get_tick(&get_signal_start_tick);
        while(1)
        {
            #if 1
            ddi_misc_get_tick(&cur_sys_tick);
            if(cur_sys_tick - get_signal_start_tick > 2000)
            {
                flg = 1;
            }
            #else
            if(dev_user_querrytimer(timeid, 1000))
            {
                timeid = dev_user_gettimeID();
                flg = 1;
            }
            #endif
            if(flg==1)
            {
                err = ddi_wifi_get_signal(&ap_num);
                if(err < 0)
                {
                    ap_num = 0;
                }
                sprintf(error_info, "信号强度:%d", ap_num);
                err = ddi_wifi_connectap_status(); //ddi_wifi_connectap_status();
                #if 1
                //TRACE_DBG("连接成功！\r\n");
                #else
                lcd_interface_clear_rowram(LINE3);
                lcd_interface_clear_rowram(LINE4);
                lcd_interface_fill_rowram(LINE4, 0, "连接成功！", CDISP);
                #endif
                if(err == 2)
                {
                    TRACE_DBG("连接成功！\r\n");
                    //lcd_interface_fill_rowram(LINE3, 0, "连接成功！", CDISP);
                }
                else
                {
                    TRACE_DBG("连接失败！\r\n");
                    //lcd_interface_fill_rowram(LINE3, 0, "连接失败！", CDISP);
                }
                TRACE_DBG("%s\r\n",error_info);
                //lcd_interface_fill_rowram(LINE4, 0, error_info, CDISP);
                //lcd_interface_brush_screen();
                flg = 0;
            }

            ddi_watchdog_feed();
            if(ddi_com_read(PORT_CDCD_NO, (u8 *)&key, 1) > 0)//if(keypad_interface_read_beep(&key)>0)
            {           
                switch(key)
                {
                case DIGITAL1:
                    break;
                case DIGITAL2:
                    break;
                case DIGITAL3:
                    break;
                case DIGITAL4:
                    break;
                case 'e':
                    return -1;
                case POWER:
                    //flg = 1;
                    break;
                }
            }

            ddi_misc_msleep(1000);
        }
        
        
    }while(0);

        
    
    ddi_misc_msleep(1000);
    return err;
}

void wifi_airkiss_test(void)
{
    s32 ret = 0, old_ret = 0;
    u8 ssid[32];
    u8 password[32];
    u32 get_airkiss_start_tick = 0;
    u32 cur_sys_tick = 0;
    
    ddi_misc_get_tick(&get_airkiss_start_tick);
    
    ret = ddi_wifi_start_airkiss();
    if(DDI_OK == ret)
    {
        while(1)
        {
            ddi_watchdog_feed();
            ddi_misc_get_tick(&cur_sys_tick);
            if((cur_sys_tick - get_airkiss_start_tick) > 90 *1000)
            {
                ddi_wifi_stop_airkiss();
                break;
            }
            
            ret = ddi_wifi_get_airkiss_status();//dev_wifi_get_connectap_status(); //ddi_wifi_connectap_status
 //           TRACE_DBG("ret:%d ", ret);
            if(ret == WIFI_AIRKISS_SEARCHING)
            {
                if(ret != old_ret)
                {
                    TRACE_DBG("微信配网ing...");
                    old_ret = ret;
                }
            }
            else if(WIFI_AIRKISS_GETCONFIG == ret)
            {
                if(ret != old_ret)
                {
                    if(DDI_OK == ddi_wifi_get_airkiss_config(ssid, password))
                    {
                        TRACE_DBG("ssid:%s, password:%s", ssid, password);
                    }
                    old_ret = ret;
                }
            }
            else if(WIFI_AIRKISS_CONNECT_OK == ret)
            {
                //连接成功
                TRACE_DBG("微信配网成功");
                break;
            }
            else if(WIFI_AIRKISS_CONNECT_ERR == ret)
            {
                ddi_wifi_stop_airkiss();
                break;
            }
            ddi_misc_msleep(100);
        }
    }
}

s32 test_wifi_transparent(void)
{
#define WIFI_PORT   PORT_UART3_NO
    u8 dispflg=1;
    s32 ret;
    u8 key;
    u8 buff[512];
    strComAttr lpstrAttr;

    ddi_wifi_ioctl(WIFI_IOCTL_CMD_TRANSPARENT, TRUE, 0);
    TRACE_DBG("ret:%d\r\n", ret);

    lpstrAttr.m_baud = 115200;
    lpstrAttr.m_databits = 8;
    lpstrAttr.m_parity = 'n';
    lpstrAttr.m_stopbits = 0;
    ret = ddi_com_open(WIFI_PORT, &lpstrAttr);
    TRACE_DBG("ret:%d", ret);
    while(1)
    {
        ddi_watchdog_feed();
        memset(buff, 0, sizeof(buff));
        ret = ddi_com_read(PORT_CDCD_NO, buff, sizeof(buff));
        if(ret > 0)
        {
            TRACE_DBG(">> %s", buff);

            if(strstr(buff, "exittransparent"))
            {
                break;
            }
            
            ddi_com_write(WIFI_PORT, buff, strlen(buff));
        }

        memset(buff, 0, sizeof(buff));
        ret = ddi_com_read(WIFI_PORT, buff, sizeof(buff));
        if(ret > 0)
        {
            TRACE_DBG("<< %s", buff);
        }
        ddi_misc_msleep(100);
    }

    ddi_wifi_ioctl(WIFI_IOCTL_CMD_TRANSPARENT, FALSE, 0);

    return 0;
}

s32 test_wifi(void)
{
//    TRENDITDEV_TEST_INFO devtestinfo;
        u8 dispflg=1;
        s32 ret;
        u8 key;
        u8 tmp[32];
        
        while(1)
        {
            if(dispflg==1)
            {
                dispflg = 0;
                TRACE_DBG("================WIFI Test ==============");
                TRACE_DBG("1.Open");
                TRACE_DBG("2.Close");
                TRACE_DBG("3.测试信号");
                //TRACE_DBG("4.Wifi DownLoad");
                TRACE_DBG("5.微信配网");
                TRACE_DBG("6.命令透明传输");

            }
        
            ddi_watchdog_feed();
            if(ddi_com_read(PORT_CDCD_NO, (u8 *)&key, 1) > 0)
            {
                TRACE_DBG("key:%c\r\n",key);
                switch(key)
                {
                    case '1':
                        ret = ddi_wifi_open();
                        TRACE_DBG("ret:%d\r\n", ret);
                        dispflg = 1;
                        break;
                    case '2':
                        ret = ddi_wifi_close();
                        TRACE_DBG("ret:%d\r\n", ret);
                        dispflg = 1;
                        break;
                    case '3':
                        //test_wifi(&devtestinfo, DEVICE_TEST_MODE_SINGLE); 
                        test_wifi_signle("Trendit", "trendit123");
                        //dev_debug_printf("devtestinfo->wifi=%d\r\n", devtestinfo.wifi);
                        dispflg = 1;
                        break;
                    case '4':
                        ddi_wifi_ioctl(2, 0, (u32)tmp);
                        TRACE_DBG("tmp:%s\r\n", tmp);
                        //dev_test_wifi_download();
                        //dispflg = 1;
                        break;
                        //dev_test_wifi_download();
                        //dispflg = 1;
                        //break;
                    case '5':
                        TRACE_DBG(" -- tmp:%s\r\n", tmp);
                        wifi_airkiss_test();
                        break;

                    case '6':
                        TRACE_DBG(" -- tmp:%s\r\n", tmp);
                        test_wifi_transparent();
                        break;
                        
                    case 'e':
                        return 0;
                    case POWER:
                        break;

        
                }
            
            }
        }

}

s32 scanap_wifi_test(wifi_apinfo_t *aps, u8* target_ap, u8* target_pwd)
{
    s32 err;
    u32 ap_num;
    //wifi_apinfo_t aps[20];
    u32 index;
    u8 ap_name_with_quote[64] = {0};
    u32 ap_found = 0;
    u8 error_info[128] = {0};
    
    u32 cur_ticks = trendit_get_ticks();

    if(aps == NULL || target_ap == NULL)
    {
        return DDI_ERR;
    }
    if(0 != ddi_wifi_open())
    {
        return DDI_ERR;
    }
    ap_num = sizeof(aps)/sizeof(aps[0]);
    //ddi_wifi_scanap_start(aps, ap_num);
    while(1)
    {
        ddi_watchdog_feed();
        if(trendit_get_ticks() - cur_ticks >= WIFI_CONNECT_OVERTIME/2)
        {
            return DDI_ERR;
        }
        err = ddi_wifi_scanap_status(&ap_num);//dev_wifi_scanap_status(&ap_num);
        if(err == 0)
        {
            //正在搜索
        }
        else if(err == 1)
        {
            //搜索成功
            err = 0;
            TRACE_DBG("scanap time:%d\r\n",trendit_get_ticks() - cur_ticks);
            break;
        }
        else
        {
            break;
        }
    }

        if(err != 0)
        {
            TRACE_DBG("搜网失败\r\n");
        }
        for(index=0; index<ap_num; index++)
        {
            sprintf(ap_name_with_quote, "\"%s\"", target_ap);
            if(strcmp(ap_name_with_quote, aps[index].m_ssid) == 0)
            {
                    //strcpy(ap_name, target_ap);
                    //strcpy(pwd, target_pwd);
                    ap_found = 1;
            }
            TRACE_DBG("wifi: %s \r\n", aps[index].m_ssid);
            //Trace("debug", "wifi: %s \r\n", aps[index].m_ssid);
    //             dev_debug_printf("%s(%d):aps[%d]=%s\r\n", __FUNCTION__, __LINE__, index, aps[index].m_ssid);
        }
        if(!ap_found)
        {
            sprintf(error_info, "没有找到目标热点:%s", target_ap);
            TRACE_DBG("%s\r\n",error_info);
            err = -1;
        }

    return err;
}

s32 auto_wifi_test(void)
{
    s32 ret = DDI_ERR;
    u32 cur_ticks = trendit_get_ticks();

    if(0 != ddi_wifi_open())
    {
        return DDI_ERR;
    }
    
    while(1)
    {
        ddi_watchdog_feed();
        ret = ddi_wifi_connectap_status();
        if(0 == ret)
        {
            if(trendit_get_ticks() - cur_ticks >= WIFI_CONNECT_OVERTIME/2)
            {
                TRACE_INFO("connect wifi overtime");
                ret = DDI_ERR;
                break;
            }
            else
            {
                ddi_misc_msleep(100);
            }
        }
        else if(1 == ret)
        {
            TRACE_INFO("connect wifi fail");
            ret = DDI_ERR;
            break;
        }
        else if(2 == ret)
        {
            TRACE_INFO("connect wifi success, then creat tcp");
            ret = DDI_OK;
            break;
        }
    }

    return ret;
}

