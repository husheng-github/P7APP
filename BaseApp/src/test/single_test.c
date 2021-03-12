#include "ddiglobal.h"
#include "app_global.h"
#include "test_global.h"

static  wifi_apinfo_t wifi_aps[20] = NULL;

s32 testmain(void)
{   
    u8 flg=1;
    u8 key;
    u32 timeid;
    
    while(1)
    {
        if(flg==1)
        {
            flg = 0;
            TRACE_DBG("================Test MainMenu===============");
            TRACE_DBG("1.Wifi");
            TRACE_DBG("2.Gprs");
            TRACE_DBG("3.Audio");
            TRACE_DBG("4.key");
            TRACE_DBG("5.LED");
            TRACE_DBG("6.Printer");
            TRACE_DBG("7.Printer block");
            TRACE_DBG("8.Cash box");
            TRACE_DBG("====================Menu End=================");
        }

        if(ddi_com_read(PORT_CDCD_NO, (u8 *)&key, 1) > 0)
        {
            switch(key)
            {     
                case '1':
                    test_wifi();
                    flg = 1;
                    break;
                
                case '2':
                    test_gprs();
                    flg = 1;
                    break;

                case '3':
                    test_audio();
                    flg = 1;
                    break;
                
                case '4':
                    test_key();
                    flg = 1;
                    break;
                
                case '5':
                    test_led();
                    flg = 1;
                    break;
                    
                case '6':
                    test_printer();
                    flg = 1;
                    break;
                case '7':
                    test_print_block();
                    flg = 1;
                    break;

                case '8':
                    ddi_misc_ioctl(MISC_IOCTL_OPENCASHBOX, 0, 0);
                    flg = 1;
                    break;
                    
                case 'e':
                    return 0;
                    break;
            }

            ddi_watchdog_feed();
            ddi_misc_msleep(1000);
        }
    }

    return 0;
}

static void print_auto_res(auto_test_res_t *auto_test_res, s32 num)
{
    s8 tmp_buff[1024] = {0};
    s32 len = 0;
    s32 i = 0;
    s32 ret = DDI_ERR;
    s8 buff[128] = {0};
    
    memset(tmp_buff, 0, sizeof(tmp_buff));
    memcpy(tmp_buff, "\r\n\r\n\x1B\x61\x01", 7);//居中
    len += 7;

    memcpy(tmp_buff+len, "\x1D\x21\x11", 3); //放大一倍
    len += 3;
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s\r\n", "测试结果");
    len += strlen(tmp_buff);
    
    memcpy(tmp_buff+len, "\x1D\x21\x00", 3); //还原
    len += 3;

    for(i=0; i<num; i++)
    {
        memcpy(tmp_buff+len, "\x1B\x61\x00", 3);//居左
        len += 3;

        memset(buff, 0, sizeof(buff));
        snprintf(buff, sizeof(buff), "%s                     %s\r\n", 
                                                          auto_test_res[i].des, 
                                                          auto_test_res[i].result?"Y":"N");
        snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", buff);
        len += strlen(buff);
    }

    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s", "\r\n\r\n\r\n");
    len +=6;

    ddi_thmprn_esc_init();
    ddi_thmprn_open();
    
    ddi_thmprn_esc_p(tmp_buff, len);
    
    while(1)
    {
        if(ddi_thmprn_esc_loop() != DDI_OK)
        {
            break;
        }

        ddi_watchdog_feed();
    }

    while(1)
    {
        ret = ddi_thmprn_get_status();
        TRACE_DBG("printer status:%d", ret);

        if((ret == DDI_OK) || (ret == DDI_EACCES))
        {   
            if(ret == DDI_EACCES) //缺纸
            {
                TRACE_DBG("printer no paper");
            }
            ddi_thmprn_close();
            break;
        }

        ddi_watchdog_feed();
        ddi_misc_msleep(200);
    }           

}

s32 auto_cashbox_test(void)
{
    u8 key;
    s32 ret = DDI_ERR;
    
    trendit_factory_test_display_msg(AUDIO_TEST_CASHBOX, AUDIO_PLAY_BLOCK);
    ddi_misc_ioctl(MISC_IOCTL_OPENCASHBOX, 0, 0);

    ddi_misc_msleep(500);
    trendit_factory_test_display_msg(AUDIO_TEST_KEY_FINISH, AUDIO_PLAY_NOBLOCK);
    ddi_key_clear();
    while(1)
    {
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

        ddi_watchdog_feed();
    }

_out:
    return ret;
}

void trendit_auto_test(void)
{
    s32 ret = DDI_ERR;
    s32 res_buff[6][128] = {0};
    auto_test_res_t auto_test_res[6] = {
        {0, "按键"},
        {0, "led "},
        {0, "无线"},
        {0, "wifi"},
        {0, "打印"},
        {0, "钱箱"}
    };

    TRACE_DBG("auto test enter");
    ddi_wifi_close();
    ddi_misc_msleep(500);
    ddi_wifi_open();
    ddi_wifi_connectap_start(WIFI_SSID, WIFI_PWD, NULL);
    ddi_led_ioctl(DDI_LED_CTL_CLOSE_YELLOW, 0, 0);
    trendit_factory_test_display_msg(AUDIO_TEST_ENTER, AUDIO_PLAY_BLOCK);

    ret = auto_key_test();
    auto_test_res[0].result = (DDI_OK == ret)?1:0;
    TRACE_DBG("key auto test:%d", ret);

    ret = auto_led_test();
    auto_test_res[1].result = (DDI_OK == ret)?1:0;
    TRACE_DBG("led auto test:%d", ret);

    ret = auto_wireless_test();
    auto_test_res[2].result = (DDI_OK == ret)?1:0;
    TRACE_DBG("wireless auto test:%d", ret);

    ret = auto_wifi_test();
    auto_test_res[3].result = (DDI_OK == ret)?1:0;
    TRACE_DBG("wifi auto test:%d", ret);

    ret = auto_printer_test();
    auto_test_res[4].result = (DDI_OK == ret)?1:0;
    TRACE_DBG("printer auto test:%d", ret);

    ret = auto_cashbox_test();
    auto_test_res[5].result = (DDI_OK == ret)?1:0;
    TRACE_DBG("cashbox auto test:%d", ret);

    print_auto_res(auto_test_res, sizeof(auto_test_res)/sizeof(auto_test_res[0]));
}

static void print_test_result(u8 *ret_result)
{
    s32 ret = DDI_EIO;
    s8 tmp_buff[1024] = {0};
    
    ddi_thmprn_esc_init();
    ddi_thmprn_open();

    //memcpy(tmp_buff, ret_result, s);
    sprintf(tmp_buff, "%s\r\n\r\n\r\n", ret_result);
    ddi_thmprn_esc_p(tmp_buff, strlen(tmp_buff));
    
    while(1)
    {
        if(ddi_thmprn_esc_loop() != DDI_OK)
        {
            break;
        }

        ddi_watchdog_feed();
    }
    
    while(1)
    {
        ret = ddi_thmprn_get_status();
        TRACE_DBG("printer status:%d", ret);
    
        if((ret == DDI_OK) || (ret == DDI_EACCES))
        {   
            if(ret == DDI_EACCES) //缺纸
            {
                TRACE_DBG("printer no paper");
                ret = DDI_OK;  //底层有问题，暂时将缺纸当做正常
            }
            ddi_thmprn_close();
            break;
        }

        ddi_watchdog_feed();
        ddi_misc_msleep(200);
    }

}

s32 ddi_misc_set_mcuparam(u16 sn, u8 *param, u16 paramlen, u16 offset)
{
    strMCUparam_ioctl_t l_mcuparam;

    l_mcuparam.m_sn = sn;
    l_mcuparam.m_paramlen = paramlen;
    l_mcuparam.m_param = param;
    l_mcuparam.m_offset = offset;
    return ddi_misc_ioctl(MISC_IOCTL_SETMCUPARAM, (u32)&l_mcuparam, 0);
}

s32 ddi_misc_get_mcuparam(u16 sn, u8 *param, u16 paramlen, u16 offset)
{
    strMCUparam_ioctl_t l_mcuparam;

    l_mcuparam.m_sn = sn;
    l_mcuparam.m_paramlen = paramlen;
    l_mcuparam.m_param = param;
    l_mcuparam.m_offset = offset;
    return ddi_misc_ioctl(MISC_IOCTL_GETMCUPARAM, (u32)&l_mcuparam, 0);
}

s32 dev_misc_get_workmod(void)
{
    s32 ret;
//    u8 tmp[16];

    TRENDITDEV_TEST_INFO devtestinfo;

//return 0;    
    memset(&devtestinfo,0,sizeof(TRENDITDEV_TEST_INFO));
   //ret = ddi_file_read(TRENDITDEVICETESTINFO,(u8 *)&devtestinfo,sizeof(TRENDITDEV_TEST_INFO),0);
   ret = ddi_misc_get_mcuparam(MACHINE_PARAM_DEVTESTINFO,(u8 *)&devtestinfo,sizeof(TRENDITDEV_TEST_INFO),0);
//   memcpy(tmp, devtestinfo.pcbatestflag, 8);
//   tmp[8] = 0;
//Trace("app->", "%s(%d):ret=%d, flg=%s\r\n", __FUNCTION__, __LINE__, ret, tmp);
    if(0 == memcmp("INTEGRAT", devtestinfo.pcbatestflag, 8))
    {
        return 1;  //集成测试模式
    }
    else if(0 == memcmp("WORKMODE", devtestinfo.pcbatestflag, 8))
    {
        return 0;   //工作模式
    }
    else
    {
        return 2;   //板卡测试模式
    }
}
s32 dev_misc_set_workmod(u8 mod,TRENDITDEV_TEST_INFO *devtestresultinfo)
{
    u8 i;
    s32 ret;
    //TRENDITDEV_TEST_INFO devtestinfo;
    //memset(&devtestinfo,0,sizeof(TRENDITDEV_TEST_INFO));

    if(mod>2)
    {
        return -1;  //参数错
    }
    i = dev_misc_get_workmod();

    
    if(i!=mod+1&&(mod != i))  //记录当前测试结果
    {
        return -2;   //只允许逐级修改,板卡模式->集成测试模式->工作模式
    }
    //ret = ddi_file_read(TRENDITDEVICETESTINFO,(u8 *)&devtestinfo,sizeof(TRENDITDEV_TEST_INFO),0);
    //ret = ddi_misc_get_mcuparam(MACHINE_PARAM_DEVTESTINFO,(u8 *)&devtestinfo,sizeof(TRENDITDEV_TEST_INFO),0);
    if(mod == 0)
    {
        memcpy(devtestresultinfo->pcbatestflag,"WORKMODE", 8);
    }
    else if(mod == 1)
    {

        memcpy(devtestresultinfo->pcbatestflag,"INTEGRAT", 8);
    }
    else if(mod == 2)
    {
        memcpy(devtestresultinfo->pcbatestflag,"PCBAMODE", 8);
    }
    //ret = ddi_file_write(TRENDITDEVICETESTINFO,(u8 *)&devtestinfo,sizeof(TRENDITDEV_TEST_INFO));
    ret = ddi_misc_set_mcuparam(MACHINE_PARAM_DEVTESTINFO,(u8 *)devtestresultinfo,sizeof(TRENDITDEV_TEST_INFO),0);
    
//Trace("app->", "%s(%d):ret=%d\r\n", __FUNCTION__, __LINE__, ret);

    return DDI_OK;
}

void print_single_test_result(u8 *title, u8 device_result)
{
    u8 tmpbuf[128] = {0};

    memset(tmpbuf, 0, sizeof(tmpbuf));
    snprintf(tmpbuf, sizeof(tmpbuf),     "%s%s\r\n\r\n\r\n", title, (device_result==1 || device_result==3)?"Y":"N");
    printer_instance()->direct_print(tmpbuf, strlen(tmpbuf));
}

s32 single_test_led(TRENDITDEV_TEST_INFO *devtestinfo, u8 testmode)
{
    s32 ret = DDI_ERR;

    if(testmode == DEVICE_TEST_MODE_SMT)
    {
        //devtestinfo->led = 1;
        ddi_led_sta_set(LED_STATUS_B, 1);
        ddi_led_sta_set(LED_STATUS_R, 1);
        ddi_led_sta_set(LED_SIGNAL_B, 1);
        ddi_led_sta_set(LED_SIGNAL_Y, 1);
        devtestinfo->led = 1;
        return DDI_OK;
    }
    ret = auto_led_test();
    if(ret == DDI_OK)
    {
        devtestinfo->led = 1;
    }
    else
    {
        devtestinfo->led = 2;
    }
    if(testmode == DEVICE_TEST_MODE_SINGLE)
    {
        print_single_test_result("LED指示灯                   - " ,devtestinfo->led);

    }
    return ret;

}

s32 single_test_audio(TRENDITDEV_TEST_INFO *devtestinfo, u8 testmode)
{
    s32 ret = DDI_ERR;
    if(testmode == DEVICE_TEST_MODE_SMT || testmode == DEVICE_TEST_MODE_INTEGER)
    {
        devtestinfo->audio = 3;
        return DDI_OK;
    }

    //发布模式不测试
    if(0 == dev_misc_get_workmod())
    {
        devtestinfo->audio = 3;
        return DDI_OK;
    }
    
    ret = auto_audio_test();
    if(ret == DDI_OK)
    {
        devtestinfo->audio = 1;
    }
    else
    {
        devtestinfo->audio = 2;
    }
    if(testmode == DEVICE_TEST_MODE_SINGLE)
    {
        print_single_test_result("音频测试                     - " ,devtestinfo->audio);

    }
    return ret;

}
s32 single_test_key(TRENDITDEV_TEST_INFO *devtestinfo, u8 testmode)
{
    s32 ret = DDI_ERR;
    
    if(testmode == DEVICE_TEST_MODE_SMT)
    {
        devtestinfo->key = 3;
        return DDI_OK;
    }
    ret = auto_key_test();
    
    if(ret == DDI_OK)
    {
        devtestinfo->key = 1;
        
    }
    else
    {
        devtestinfo->key = 2;
    }
    if(testmode == DEVICE_TEST_MODE_SINGLE)
    {
        print_single_test_result("按键                        - " ,devtestinfo->key);

    }
    return ret;

}
s32 single_test_gprs(TRENDITDEV_TEST_INFO *devtestinfo, u8 testmode)
{
    s32 ret = DDI_ERR;
    

    ret = auto_wireless_test();

    if(testmode == DEVICE_TEST_MODE_SMT)
    {
        if(ret == DDI_OK)
        {
            devtestinfo->gprs = 1;
        }
        else
        {
            devtestinfo->gprs = 2;
        }
        return ret;
    }
    if(ret == DDI_OK)
    {
        devtestinfo->gprs = 1;
    }
    else
    {
        devtestinfo->gprs = 2;
    }
    if(testmode == DEVICE_TEST_MODE_SINGLE)
    {
        print_single_test_result("GPRS无线                    - " ,devtestinfo->gprs);

    }

    return ret;

}

s32 single_test_cashbox(TRENDITDEV_TEST_INFO *devtestinfo, u8 testmode)
{
    s32 ret = DDI_ERR;
    if(testmode == DEVICE_TEST_MODE_SMT)
    {
        devtestinfo->cashbox = 3;
        return DDI_OK; 
    }

    ret = auto_cashbox_test();
    if(ret == DDI_OK)
    {
        devtestinfo->cashbox = 1;
    }
    else
    {
        devtestinfo->cashbox = 2;
    }
    if(testmode == DEVICE_TEST_MODE_SINGLE)
    {
        print_single_test_result("钱箱测试                    - " ,devtestinfo->cashbox);

    }
    return ret;
}



s32 single_test_wifi(TRENDITDEV_TEST_INFO *devtestinfo, u8 testmode)
{
    s32 ret = DDI_ERR;
    if(testmode == DEVICE_TEST_MODE_SMT)
    {
        
        ret = ddi_wifi_open();//scanap_wifi_test(wifi_aps, WIFI_SSID, WIFI_PWD);
        if(ret == DDI_OK)
        {
            devtestinfo->wifi = 1;
        }
        else
        {
            devtestinfo->wifi = 2;
        }
        if(wifi_aps != NULL)
        {
            k_freeapp(wifi_aps);
        }
        return ret;
    }
    ret = auto_wifi_test();
    if(ret == DDI_OK)
    {
        devtestinfo->wifi = 1;
    }
    else
    {
        devtestinfo->wifi = 2;
    }
    if(testmode == DEVICE_TEST_MODE_SINGLE)
    {
        print_single_test_result("wifi无线                    - " ,devtestinfo->wifi);

    }

    return ret;

}
s32 single_test_printer(TRENDITDEV_TEST_INFO *devtestinfo, u8 testmode)
{
    s32 ret = DDI_ERR;
    ret = test_printer_twoline(&devtestinfo,testmode);
    if(ret == DDI_OK)
    {
        devtestinfo->printer = 1;
    }
    else
    {
        devtestinfo->printer = 2;
    }
    if(testmode == DEVICE_TEST_MODE_SINGLE)
    {
        print_single_test_result("打印机                      - " ,devtestinfo->printer);

    }
    return ret;

}


static void add2single_test_case_array(single_test_handler *g_single_test_cases, u8 **g_single_test_cases_title, s32 g_single_test_num, test_handler handler,const u8* title)
{

    
    g_single_test_cases[g_single_test_num] = handler;
    if(g_single_test_cases_title[g_single_test_num] == NULL)
    {
        g_single_test_cases_title[g_single_test_num] = (u8*)k_mallocapp(20);
    }
    memset(g_single_test_cases_title[g_single_test_num],0,20);
    strcpy(g_single_test_cases_title[g_single_test_num],title);

}

static s32 processtest_init(TRENDITDEV_TEST_INFO *devtestinfo, single_test_handler *g_single_test_cases, u8 **g_single_test_cases_title)
{

    s32 single_test_num = 0;

    #if 1

    #else
    if(ddi_misc_probe_dev(DEV_LED) == DDI_OK)
    {
       
        add2single_test_case_array(g_single_test_cases, g_single_test_cases_title, single_test_num, test_led,"LED");
    }
    else
    {
        devtestinfo->led = 3;
    }
    #endif
    
    //add2common_test_case_array(test_lcd,"LCD");
    //add2single_test_case_array(test_lcd,"LCD");
    
    if(ddi_misc_probe_dev(DEV_TOUCH_SCREEN) == DDI_OK)
    {
        //add2common_test_case_array(test_tp,"触摸屏");
        //add2single_test_case_array(test_tp,"触摸屏");
    }
    else
    {
        devtestinfo->tp = 3;
    }

    if(ddi_misc_probe_dev(DEV_BEEP) == DDI_OK)
    {
        //add2common_test_case_array(test_beeper, "蜂鸣器");
        //add2single_test_case_array(test_beeper, "蜂鸣器");
    }
    else
    {
        devtestinfo->beeper = 3;
    }

    #if 1
    add2single_test_case_array(g_single_test_cases, g_single_test_cases_title, single_test_num, single_test_audio, "音频测试");
    single_test_num++;
    #else
    if(ddi_misc_probe_dev(DEV_AUDIO) == DDI_OK)
    {
        add2single_test_case_array(g_single_test_cases, g_single_test_cases_title, single_test_num, single_test_audio, "音频测试");
        single_test_num++;

    }
    else
    {
        devtestinfo->audio = 3;
    }
    #endif

    
    if(ddi_misc_probe_dev(DEV_MAG_CARD) == DDI_OK)
    {
        //add2common_test_case_array(test_mag_card,"磁条卡");
        //add2single_test_case_array(test_mag_card,"磁条卡");
    }
    else
    {
        devtestinfo->magcard = 3;
    }
    if(ddi_misc_probe_dev(DEV_IC_CARD) == DDI_OK)
    {
        //add2common_test_case_array(test_ic_card,"接触卡");
        //add2single_test_case_array(test_ic_card,"接触卡");
    }
    else
    {
        devtestinfo->iccard = 3;
    }
    if(ddi_misc_probe_dev(DEV_RF_CARD) == DDI_OK)
    {
        //add2common_test_case_array(test_rf_card,"非接卡");
        //add2single_test_case_array(test_rf_card,"非接卡");
    }
    else
    {
        devtestinfo->rfcard = 3;
    }
    

    if(ddi_misc_probe_dev(DEV_CAMERA) == DDI_OK)
    {
        //add2common_test_case_array(test_scanner,"摄像头");
        //add2single_test_case_array(test_scanner,"摄像头");
    }
    else
    {
        devtestinfo->camera = 3;
    }
    
    //add2common_test_case_array(test_sleep,"休眠");
    //add2single_test_case_array(test_sleep,"休眠");

    //add2common_test_case_array(test_rtc,"RTC");
    //add2single_test_case_array(test_rtc,"RTC");

    //add2single_test_case_array(test_signal,"信号强度");
    #if 1
    add2single_test_case_array(g_single_test_cases, g_single_test_cases_title, single_test_num, single_test_printer,"打印测试");
    single_test_num++;
    #else
    if(ddi_misc_probe_dev(DEV_PRINTER) == DDI_OK)
    {
        add2single_test_case_array(g_single_test_cases, g_single_test_cases_title, single_test_num, test_printer,"打印机");
    }
    else
    {
        devtestinfo->printer = 3;
    }
    #endif
    
    add2single_test_case_array(g_single_test_cases, g_single_test_cases_title, single_test_num, single_test_key,"按键测试");
    single_test_num++;
    
    add2single_test_case_array(g_single_test_cases, g_single_test_cases_title, single_test_num, single_test_led,"led测试");
    single_test_num++;

    add2single_test_case_array(g_single_test_cases, g_single_test_cases_title, single_test_num, single_test_cashbox,"钱箱测试");
    single_test_num++;

    add2single_test_case_array(g_single_test_cases, g_single_test_cases_title, single_test_num, single_test_gprs,"无线测试");
    single_test_num++;

    if(ddi_misc_probe_dev(DEV_WIFI) == DDI_OK)
    {
        add2single_test_case_array(g_single_test_cases, g_single_test_cases_title, single_test_num, single_test_wifi,"wifi测试");
        single_test_num++;
    }
    else
    {
        devtestinfo->wifi = 3;
    }
    //add2common_test_case_array(test_spiflash, "spiflash测试");
    return single_test_num;
}

s32 processtask_device_agingtest(void)
{
#ifndef PRINT_AGING
    unsigned char buf[] = "国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国";
    u32 timeoutvalue = 30000;  //1分钟打印一次 
#else
    unsigned char buf[] = "\x1B\x61\x01\x1B\x21\x00\x1D\x21\x00\x2A\x2A\x2A\x2A\x2A\x2A\x2A\
\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x1D\x21\x11\x23\x31\x1D\x21\x11\x1B\
\x21\x00\x1D\x21\x00\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x2A\
\x2A\x2A\x2A\x0A\x1B\x61\x00\x1B\x21\x00\x1D\x21\x00\x20\x20\x20\
\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x1B\x21\x00\
\x1D\x21\x00\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\
\x20\x20\x20\x0A\x1B\x61\x00\x1B\x21\x00\x1D\x21\x00\x0A\x1B\x61\
\x01\x1B\x21\x00\x1D\x21\x00\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\
\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x1B\x21\x00\x1D\x21\x00\x2D\x2D\x2D\
\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x0A\x1B\x61\
\x00\x1B\x21\x00\x1D\x21\x00\xCF\xC2\xB5\xA5\xCA\xB1\xBC\xE4\xA3\
\xBA\x20\x30\x38\x2D\x32\x36\x20\x31\x37\x3A\x32\x36\x0A\x1B\x61\
\x00\x1D\x21\x11\xB1\xB8\xD7\xA2\xA3\xBA\xB0\xAE\xB3\xD4\xC0\xB1\
\xB6\xE0\xB5\xE3\xC0\xB1\x0A\x1B\x61\x00\x1D\x21\x11\xB7\xA2\xC6\
\xB1\xA3\xBA\xC9\xCF\xBA\xA3\xCA\xD0\xC0\xAD\xC0\xAD\xB6\xD3\xD3\
\xD0\xCF\xDE\xB9\xAB\xCB\xBE\x2D\x6E\x61\x70\x6F\x73\x31\x33\x35\
\x32\x36\x36\x36\x36\x36\x36\x36\x36\x36\x38\x38\x32\x0A\x1B\x61\
\x00\x1B\x21\x00\x1D\x21\x00\x20\x20\x20\x20\x20\x20\x20\x20\x20\
\x20\x20\x20\x20\x20\x20\x20\x1B\x21\x00\x1D\x21\x00\x20\x20\x20\
\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x0A\x1B\x61\
\x01\x1B\x21\x00\x1D\x21\x00\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\
\x2D\x2D\x1B\x21\x00\x1D\x21\x00\x20\x31\xBA\xC5\xC0\xBA\xD7\xD3\
\x20\x1B\x21\x00\x1D\x21\x00\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\
\x2D\x2D\x2D\x0A\x1B\x61\x00\x1D\x21\x01\xCC\xD7\xB2\xCD\x59\x53\
\x59\x32\x32\x46\x4A\x43\x51\x32\x1B\x21\x00\x1D\x21\x00\x20\x20\
\x1B\x21\x00\x1D\x21\x00\x20\x20\x1D\x21\x01\x20\xA1\xBE\xA1\xC1\
\x32\xA1\xBF\x1B\x21\x00\x1D\x21\x00\x20\x1D\x21\x01\x20\x30\x2E\
\x30\x36\x0A\x1B\x61\x00\x1D\x21\x01\x20\x2D\x20\xC4\xC3\xCC\xFA\
\xC2\xED\xC6\xE6\xB6\xE4\xC9\xCC\xC6\xB7\x32\x4E\x46\x4D\x34\x39\
\x4B\x46\x32\x51\x2D\xB9\xE6\xB8\xF1\x30\x5B\xCE\xA2\xCC\xF0\x2B\
\xCA\xF4\xD0\xD4\xCF\xB8\xB7\xD6\x33\x5D\x0A\x1B\x61\x00\x1D\x21\
\x01\x20\x2D\x20\xBF\xA8\xB2\xBC\xC6\xE6\xC5\xB5\xC9\xCC\xC6\xB7\
\x57\x5A\x4F\x58\x51\x43\x49\x4A\x50\x39\x2D\xB9\xE6\xB8\xF1\x31\
\x5B\xCE\xA2\xCC\xF0\x2B\xCA\xF4\xD0\xD4\xCF\xB8\xB7\xD6\x32\x5D\
\x0A\x1B\x61\x01\x1B\x21\x00\x1D\x21\x00\x2D\x2D\x2D\x2D\x2D\x2D\
\x2D\x2D\x2D\x2D\x2D\x1B\x21\x00\x1D\x21\x00\x20\xC6\xE4\xCB\xFB\
\xB7\xD1\xD3\xC3\x20\x1B\x21\x00\x1D\x21\x00\x2D\x2D\x2D\x2D\x2D\
\x2D\x2D\x2D\x2D\x2D\x2D\x0A\x1B\x61\x00\x1B\x21\x00\x1D\x21\x00\
\xC5\xE4\xCB\xCD\xB7\xD1\x1B\x21\x00\x1D\x21\x00\x20\x20\x20\x20\
\x20\x20\x20\x20\x1B\x21\x00\x1D\x21\x00\x20\x20\x20\x20\x20\x20\
\x20\x20\x1B\x21\x00\x1D\x21\x00\x20\x20\x20\x20\x1B\x21\x00\x1D\
\x21\x00\x1B\x21\x00\x1D\x21\x00\x20\x1B\x21\x00\x1D\x21\x00\x20\
\x35\x2E\x30\x30\x0A\x1B\x61\x01\x1B\x21\x00\x1D\x21\x00\x2D\x2D\
\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x1B\x21\
\x00\x1D\x21\x00\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\
\x2D\x2D\x2D\x2D\x0A\x1B\x61\x00\x1D\x21\x11\xD2\xD1\xB8\xB6\x1B\
\x21\x00\x1D\x21\x00\x20\x20\x20\x20\x20\x20\x1B\x21\x00\x1D\x21\
\x00\x20\x20\x20\x20\x20\x20\x1B\x21\x00\x1D\x21\x00\x20\x20\x20\
\x20\x1B\x21\x00\x1D\x21\x00\x1D\x21\x11\x35\x2E\x30\x36\x0A\x1B\
\x61\x01\x1B\x21\x00\x1D\x21\x00\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\
\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x1B\x21\x00\x1D\x21\x00\x2D\x2D\
\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x0A\x1B\
\x61\x00\x1B\x21\x00\x1D\x21\x00\x0A\x1B\x61\x00\x1D\x21\x11\x6E\
\x2A\x2A\x0A\x1B\x61\x00\x1D\x21\x01\x5B\xCA\xD5\xB2\xD8\xD3\xC3\
\xBB\xA7\x5D\x0A\x1B\x61\x00\x1B\x21\x00\x1D\x21\x00\x39\x35\xB9\
\xAB\xD2\xE6\xD6\xDC\x20\xCF\xE0\xD0\xC5\xD0\xA1\xB5\xC4\xCE\xB0\
\xB4\xF3\x0A\x1B\x61\x01\x1B\x21\x00\x1D\x21\x00\x1D\x77\x02\x1D\
\x68\x40\x0A\x1D\x6B\x49\x0C\x7B\x43\x01\x14\x14\x11\x32\x0A\x5E\
\x27\x1E\x56\x31\x32\x30\x32\x30\x31\x37\x35\x30\x31\x30\x39\x34\
\x33\x39\x33\x30\x38\x36\x0A\x1B\x61\x00\x1B\x21\x00\x1D\x21\x00\
\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\
\x1B\x21\x00\x1D\x21\x00\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\
\x20\x20\x20\x20\x20\x20\x0A\x1B\x61\x01\x1B\x21\x00\x1D\x21\x00\
\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\
\x1B\x21\x00\x1D\x21\x00\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\x2D\
\x2D\x2D\x2D\x2D\x2D\x2D\x0A\x1B\x61\x00\x1B\x21\x00\x1D\x21\x00\
\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\
\x1B\x21\x00\x1D\x21\x00\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\
\x20\x20\x20\x20\x20\x20\x0A\x1B\x61\x00\x1B\x21\x00\x1D\x21\x00\
\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\
\x1B\x21\x00\x1D\x21\x00\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\
\x20\x20\x20\x20\x20\x20\x0A\x1B\x61\x01\x1B\x21\x00\x1D\x21\x00\
\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x1D\x21\x11\x20\x23\
\x31\x1B\x21\x00\x1D\x21\x00\xCD\xEA\x20\x1B\x21\x00\x1D\x21\x00\
\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x2A\x0A\x1D\x71\x0C\
\x01\x09\x02\x31\x32\x33\x34\x35\x36\x37\x38\x39\x0D\x0A\x0D\x0A\
\x0D\x0A\x0D\x0A";
    u32 timeoutvalue = 10000;
#endif
    s32 ret;
    u32 timetick;
    u8 key;
    u8 print_buff[2048] = {0};
    u8 time_buff[6];

    trendit_factory_test_display_msg(AUDIO_TEST_AGING, AUDIO_PLAY_BLOCK);
    audio_instance()->cfg_vol(VOL_CFG_SOURCE_WEB, 0, AUDIO_VOL_5);      //最大声音播报
    while(1)
    {
        ddi_watchdog_feed();
        
        ddi_misc_get_time(time_buff);

#ifndef PRINT_AGING
        snprintf(print_buff, sizeof(print_buff), "\x1B\x40%s\r\n\r\n\x1B\x61\x01 %02x/%02x/%02x %02x:%02x:%02x\r\n\r\n\r\n",
                                                    buf, time_buff[0], time_buff[1], time_buff[2], time_buff[3], time_buff[4], time_buff[5]);
        ret = printer_instance()->direct_print(print_buff, strlen(print_buff));
#else
        ret = printer_instance()->direct_print(buf, sizeof(buf));
#endif
        TRACE_DBG("ret:%d", ret);

#ifndef PRINT_AGING
        if(MACHINE_P6_HEX == trendit_get_machine_code())
        {
            ret = audio_instance()->audio_play(AUDIO_RECOVERY, AUDIO_PLAY_BLOCK);
        }
        else
        {
            ret = audio_instance()->audio_play("支付宝收款壹仟五百六拾叁圆八角", AUDIO_PLAY_BLOCK);
        }
#endif

        ddi_key_clear();
        timetick = trendit_get_ticks();
        while(1)
        {
            ddi_watchdog_feed();
            key = trendit_get_key();
            if(key > 0)
            {
                TRACE_DBG("key:%x", key);
                switch(key)
                {
                    case KEY_CFG:
                        return DDI_OK;
                    default:
                        return DDI_ERR;
                }
            }
            
            if(trendit_query_timer(timetick,timeoutvalue))
            {
                TRACE_DBG("timeout");
                break;
            }

            ddi_misc_msleep(500);
        }
    }

    TRACE_DBG("out");
    return ret;
}


static s32 test_single(TRENDITDEV_TEST_INFO *devtestinfo, u8 testmode)
{
    s32 ret = DDI_ERR;
    s32 index = 0;
    s32 i = 0;
    s32 currIndex = 0;
    u8 devicetestmode;
    s32 workmode;

    u32 timetick;
    s32 key;
    u8 brushflag = 1;
    str_audio_tts audio_tts;
    s32 single_cnt = 0;
    
    single_test_handler g_single_test_cases[20] = {0};
    u8* g_single_test_cases_title[20] = {0};
    //u32 case_count = sizeof(g_single_test_cases)/sizeof(g_single_test_cases[0]);

    ddi_key_clear();

    ddi_wifi_close();
    ddi_misc_msleep(500);
    ddi_wifi_open();
    ddi_wifi_connectap_start(WIFI_SSID, WIFI_PWD, NULL);
    workmode = dev_misc_get_workmod();
    if(workmode == 2)
    {
        devicetestmode = DEVICE_TEST_MODE_SMT;
    }
    else if(workmode == 1)
    {
        devicetestmode = DEVICE_TEST_MODE_INTEGER;
    }
    else
    {
        devicetestmode = DEVICE_TEST_MODE_SINGLE;
    }

    single_cnt = processtest_init(devtestinfo,g_single_test_cases, g_single_test_cases_title);
    for(i = 0; i < single_cnt; i++)
    {
        TRACE_DBG("single i title:%s",g_single_test_cases_title[i]);
    }
    
    //init_trendit_dev_test_info(&devtestinfo);
    //workmode = dev_misc_get_workmod();
    devicetestmode = DEVICE_TEST_MODE_SINGLE;
    trendit_factory_test_display_msg(AUDIO_TEST_SINGLE_ENTER, AUDIO_PLAY_BLOCK);

    while(1) 
    {
        if(brushflag)
        {
            timetick = trendit_get_ticks();
            brushflag = 0;
        }
        if(trendit_query_timer(timetick,TIMEOUT_TIME) == 1)
        {
            return 0;
        }
        //ddi_watchdog_feed();
        key = trendit_get_key();
        if(key > 0)
        {
            switch (key)
            {
                case KEY_PLUS:
                    trendit_factory_test_display_msg(g_single_test_cases_title[index], AUDIO_PLAY_BLOCK);
                    currIndex = index;
                    if(index >= single_cnt-1)
                    {
                        index = 0;
                    }
                    else
                    {
                        index++;
                    }
                    break;
                case KEY_MINUS:
                    trendit_factory_test_display_msg(g_single_test_cases_title[index], AUDIO_PLAY_BLOCK);
                    currIndex = index;
                    if(index <= 0)
                    {
                        index = single_cnt-1;
                    }
                    else
                    {
                        index--;
                    }
                    break;
                case KEY_CFG:
                    g_single_test_cases[currIndex](devtestinfo, devicetestmode);
                    trendit_factory_test_display_msg(AUDIO_TEST_SINGLE_ENTER, AUDIO_PLAY_BLOCK);
                    break;
                case KEY_PAPER:
                    return 0;
                default:
                    break;
            }
            brushflag = 1;
        }

    }
    /*
    if(index < 0)
    {
        break;
    }
    ret = g_single_test_cases[index](&devtestinfo, devicetestmode);
    if(ret)
    {
        break;
    }
    */
    return ret;
    
}

static s32 test_aging(TRENDITDEV_TEST_INFO *devtestinfo, u8 testmode)
{
    s32 ret = DDI_ERR;
    processtask_device_agingtest();
    return ret;

}

static s32 test_auto(TRENDITDEV_TEST_INFO *devtestinfo, u8 testmode)
{
    s32 ret = DDI_ERR;
    trendit_auto_test();
    return ret;

}

static s32 test_8909(TRENDITDEV_TEST_INFO *devtestinfo, u8 testmode)
{
    s32 ret = DDI_ERR;

    ret = wireless_8960_test();
    return ret;

}

void trendit_factory_test(void)
{


    test_handler g_test_handler[TEST_HANDLER_MAX] = 
    {
        test_single,
        processtask_device_agingtest,
        trendit_auto_test,
        wireless_8960_test,
    };
    u8 g_test_handler_title[TEST_HANDLER_MAX][128] = 
    {
        AUDIO_TEST_SINGLE,
        AUDIO_TEST_AGING,
        AUDIO_TEST_AUTO,
        AUDIO_TEST_8960,
    };


    str_audio_tts audio_tts;
    s32 index = 0;
    s32 currIndex = 0;
    s32 ret;
    s32 workmode;
    u8 devicetestmode;

    TRENDITDEV_TEST_INFO devtestinfo;

    u32 timetick;
    s32 key;
    u8 brushflag = 1;


    
    ddi_key_clear();
    //播放音频，"已进入生产测试,按音量加减键"
    trendit_factory_test_display_msg(AUDIO_TEST_FACTORY, AUDIO_PLAY_BLOCK);

    ddi_wifi_open();
    ddi_wifi_connectap_start(WIFI_SSID, WIFI_PWD, NULL);

    while(1)
    {
        if(brushflag)
        {
            timetick = trendit_get_ticks();

            brushflag = 0;
        }
        if(trendit_query_timer(timetick,TIMEOUT_TIME) == 1)
        {
            return;
        }
        key = trendit_get_key();
        if(key > 0)
        {
            switch (key)
            {
                case KEY_PLUS:
                    trendit_factory_test_display_msg(g_test_handler_title[index], AUDIO_PLAY_BLOCK);
                    currIndex = index;
                    if(index >= TEST_HANDLER_MAX-1)
                    {
                        index = 0;
                    }
                    else
                    {
                        index++;
                    }
                    break;
                case KEY_MINUS:
                    trendit_factory_test_display_msg(g_test_handler_title[index], AUDIO_PLAY_BLOCK);
                    currIndex = index;
                    if(index <= 0)
                    {
                        index = TEST_HANDLER_MAX-1;
                    }
                    else
                    {
                        index--;
                    }
                    break;
                case KEY_CFG:
                    TRACE_DBG("currIndex:%d",currIndex);
                    g_test_handler[currIndex](&devtestinfo, devicetestmode);
                    trendit_factory_test_display_msg(AUDIO_TEST_FACTORY, AUDIO_PLAY_BLOCK);
                    break;
                case KEY_PAPER:
                    return ;
                default:
                    break;
            }
            brushflag = 1;
        }

    }    
    

}

static void init_trendit_dev_test_info(TRENDITDEV_TEST_INFO *devtestinfo)
{
    memset(devtestinfo,0,sizeof(TRENDITDEV_TEST_INFO));
#if 0
    ddi_file_read(TRENDITDEVICETESTINFO,(u8 *)&devtestinfo,sizeof(TRENDITDEV_TEST_INFO),0);
#else
    ddi_misc_get_mcuparam(MACHINE_PARAM_DEVTESTINFO, (u8 *)devtestinfo,sizeof(TRENDITDEV_TEST_INFO),0);
#endif
    if(devtestinfo->flag[0] != 0x55||devtestinfo->flag[1] != 0xAA)
    {
        devtestinfo->flag[0] = 0x55;
        devtestinfo->flag[1] = 0xAA;
    }
}
void save_device_test_info(s32 workmode, TRENDITDEV_TEST_INFO *devtestinfo, u8 show_result) 
{
    s32 ret;

    //ret = ddi_file_write(TRENDITDEVICETESTINFO,(u8 *)devtestinfo,sizeof(TRENDITDEV_TEST_INFO));
    ret = ddi_misc_set_mcuparam(MACHINE_PARAM_DEVTESTINFO, (u8 *)devtestinfo,sizeof(TRENDITDEV_TEST_INFO), 0);
    #if 1
    if(precesstask_check_deviceinfo_result(devtestinfo) == DDI_OK)
    #else
    if((devtestinfo->led==1||devtestinfo->led==3)
        &&    (devtestinfo->lcd==1||devtestinfo->lcd==3)
        && (devtestinfo->beeper==1||devtestinfo->beeper==3)
        && (devtestinfo->key==1)
        && (devtestinfo->magcard==1||devtestinfo->magcard==3)
        && (devtestinfo->iccard==1||devtestinfo->iccard==3)
        && (devtestinfo->rfcard==1 || devtestinfo->rfcard == 3)
        && (devtestinfo->gprs == 1)
        && (devtestinfo->rtc == 1)
        && (devtestinfo->camera == 1 || devtestinfo->camera == 3)
        && (devtestinfo->tp == 1||devtestinfo->tp==3)
        && (devtestinfo->sleep == 1)
        && (devtestinfo->wifi == 1 || devtestinfo->wifi == 3)
        && (devtestinfo->audio == 1 || devtestinfo->audio == 3)
        && (devtestinfo->spiflash == 1 || devtestinfo->spiflash ==3)
        && (devtestinfo->printer == 1 || devtestinfo->printer == 3))
    #endif
    {
        
        if(workmode == 2)
        {
            dev_misc_set_workmod(1, devtestinfo);
        }
        else if(workmode == 1)
        {
            dev_misc_set_workmod(0, devtestinfo);
        }
        else if(workmode == 0)
        {
            devtestinfo->sencond_test = 1;
            dev_misc_set_workmod(0, devtestinfo);
            //del_wire_optimization_switch_flag();   //二次测完删除无线优化标志文件，默认开启无线优化策略
        }
    }
    else
    {

        if(workmode == 2)
        {
            dev_misc_set_workmod(2, devtestinfo);
        }
        else if(workmode == 1)
        {
            dev_misc_set_workmod(1, devtestinfo);
        }
        #if 0
        if(show_result)
        {
            //processtask_devicetest_dispresult(devtestinfo);
        }
        #endif
    }
    if(workmode != 2)
    {
        //sysmanager_pciconfig_open(DRYICE_TYPE_TEST);
    }
    
}

s32 precesstask_check_deviceinfo_result(TRENDITDEV_TEST_INFO *devtestinfo)
{
        if((devtestinfo->led==1)
        && (devtestinfo->key==1 || devtestinfo->key == 3)
        && (devtestinfo->gprs == 1)
        && (devtestinfo->wifi == 1)
        && (devtestinfo->audio == 1 || devtestinfo->audio == 3)
        && (devtestinfo->cashbox == 1 || devtestinfo->cashbox == 3)
        && (devtestinfo->printer == 1))
        
        {
            return DDI_OK;
        }
        TRACE_DBG("led:%d,key:%d,gprs:%d,wifi:%d,audio:%d,printer:%d,cashbox:%d\r\n",devtestinfo->led, devtestinfo->key, devtestinfo->gprs, devtestinfo->wifi, devtestinfo->audio, devtestinfo->printer, devtestinfo->cashbox);
        return DDI_ERR;
}

void processtask_print_all_test_result(TRENDITDEV_TEST_INFO *devtestinfo, u8 devicetestmode)
{
    s32 i = 0;
    s32 len = 0;
    u8 printinfo[1024] = {0};
    u8 tmpbuf[128] = {0};



    memset(printinfo, 0, sizeof(printinfo));
    memcpy(printinfo, "\r\n\r\n\x1B\x61\x01", 7);//居中
    len += 7;

    memcpy(printinfo+len, "\x1D\x21\x11", 3); //放大一倍
    len += 3;
    memset(tmpbuf, 0, sizeof(tmpbuf));
    if(devicetestmode == DEVICE_TEST_MODE_SMT)
    {
        snprintf(tmpbuf, sizeof(tmpbuf), "%s\r\n", "smt测试结果");
    }
    else
    {
        snprintf(tmpbuf, sizeof(tmpbuf), "%s\r\n", "测试结果");
    }
    
    
    snprintf(printinfo+len, sizeof(printinfo)-len, "%s\r\n", tmpbuf);
    len += strlen(tmpbuf);
    //TRACE_DBG("start print result0:%d,%s\r\n", len, printinfo);

    #if 1
    memcpy(printinfo+len, "\x1D\x21\x00", 3); //还原
    len += 3;

    memcpy(printinfo+len, "\x1B\x61\x00", 3);//居左
    len += 3;
    #endif
    //snprintf(printinfo+len, sizeof(printinfo)-len, "%s\r\n", "@@@@111");
    //TRACE_DBG("start print result000:%d,%s\r\n", len, printinfo);
    printer_instance()->direct_print(printinfo, len);



    len = 0;
    memset(printinfo, 0, sizeof(printinfo));
    
    memset(tmpbuf, 0, sizeof(tmpbuf));
    snprintf(tmpbuf, sizeof(tmpbuf),     "GPRS无线                    - %s\r\n", (devtestinfo->gprs==1 || devtestinfo->gprs==3)?"Y":"N");
    snprintf(printinfo+len, sizeof(printinfo)-len, "%s\r\n", tmpbuf);
    len += strlen(tmpbuf);
    TRACE_DBG("%s\r\n", tmpbuf);
    

    if(devicetestmode == DEVICE_TEST_MODE_INTEGER)
    {
        memset(tmpbuf, 0, sizeof(tmpbuf));
        snprintf(tmpbuf, sizeof(tmpbuf), "按键                        - %s\r\n", (devtestinfo->key==1 || devtestinfo->key==3)?"Y":"N");
        snprintf(printinfo+len, sizeof(printinfo)-len, "%s\r\n", tmpbuf);
        len += strlen(tmpbuf);
        TRACE_DBG("%s\r\n", tmpbuf);
    }

    memset(tmpbuf, 0, sizeof(tmpbuf));
    snprintf(tmpbuf, sizeof(tmpbuf), "LED指示灯                   - %s\r\n", (devtestinfo->led==1 || devtestinfo->led==3)?"Y":"N");
    snprintf(printinfo+len, sizeof(printinfo)-len, "%s\r\n", tmpbuf);
    len += strlen(tmpbuf);
    TRACE_DBG("%s\r\n", tmpbuf);


    if(devicetestmode == DEVICE_TEST_MODE_INTEGER)
    {
        memset(tmpbuf, 0, sizeof(tmpbuf));
        snprintf(tmpbuf, sizeof(tmpbuf),  "钱箱测试                    - %s\r\n", (devtestinfo->cashbox==1 || devtestinfo->cashbox==3)?"Y":"N");
        snprintf(printinfo+len, sizeof(printinfo)-len, "%s\r\n", tmpbuf);
        len += strlen(tmpbuf);
        TRACE_DBG("%s\r\n", tmpbuf);
    }
    
    memset(tmpbuf, 0, sizeof(tmpbuf));
    snprintf(tmpbuf, sizeof(tmpbuf), "打印机                      - %s\r\n", (devtestinfo->printer==1 || devtestinfo->printer==3)?"Y":"N");
    snprintf(printinfo+len, sizeof(printinfo)-len, "%s\r\n", tmpbuf);
    len += strlen(tmpbuf);
    TRACE_DBG("%s\r\n", tmpbuf);
    
    memset(tmpbuf, 0, sizeof(tmpbuf));
    snprintf(tmpbuf, sizeof(tmpbuf), "wifi无线                    - %s\r\n", (devtestinfo->wifi==1 || devtestinfo->wifi==3)?"Y":"N");
    snprintf(printinfo+len, sizeof(printinfo)-len, "%s\r\n", tmpbuf);
    len += strlen(tmpbuf);
    TRACE_DBG("%s\r\n", tmpbuf);

    memcpy(printinfo+len, "\r\n\r\n\r\n\r\n", 8);
    len += 8;
    //TRACE_DBG("start print result222:%d,%s\r\n", len, printinfo);
    printer_instance()->direct_print(printinfo, len);
    //print_test_result(printinfo);

}


void processtask_show_led(u8 devicetestmode)
{
    //u32 loop_time = 1000;
    u8 key;
    TRACE_DBG("processtask_show_led\r\n");
    ddi_led_ioctl(DDI_LED_CTL_CLOSE_YELLOW, 0, 0);
    ddi_key_clear();

    ddi_led_ioctl(DDI_LED_CTL_LOOP, 1, 0);//led 循环显示
    while(1)
    {

    
        //processtask_set_led_status(LED_STATUS_B, 1);
        //ddi_misc_msleep(loop_time);
        //processtask_set_led_status(LED_STATUS_R, 1);
        //ddi_misc_msleep(loop_time);
        //processtask_set_led_status(LED_SIGNAL_B, 1);
        //ddi_misc_msleep(loop_time);
        //processtask_set_led_status(LED_SIGNAL_Y, 1);
        //ddi_misc_msleep(loop_time);
        key = trendit_get_key();
        if(devicetestmode == DEVICE_TEST_MODE_INTEGER)
        {
            if(key > 0)
            {
                ddi_key_clear();
                return;
            }
        }
    }
}

void processtask_device_test(void)
{
    TRENDITDEV_TEST_INFO devtestinfo;

    u32 ap_num;

    s32 workmode;
    u8 devicetestmode;
    s32 single_cnt = 0;
    s32 i = 0;
    s32 ret = DDI_EIO;
    single_test_handler g_single_test_cases[20] = {0};
    u8* g_single_test_cases_title[20] = {0};
    u32 cur_ticks = 0;

    trendit_cdc_output_switch(TRUE);
    ddi_key_clear();
    workmode = dev_misc_get_workmod();
    if(workmode == 2)
    {
        devicetestmode = DEVICE_TEST_MODE_SMT;
    }
    else if(workmode == 1)
    {
        devicetestmode = DEVICE_TEST_MODE_INTEGER;
    }
    else
    {
        trendit_cdc_output_switch(FALSE);
        return;
    }
    

    init_trendit_dev_test_info(&devtestinfo);
    #if 0
    if((precesstask_check_deviceinfo_result(&devtestinfo) == DDI_OK) && (devicetestmode == DEVICE_TEST_MODE_SMT))
    {
        processtask_show_led();
    }
    #endif
    //processtest_init(&devtestinfo);
    single_cnt = processtest_init(&devtestinfo,g_single_test_cases, g_single_test_cases_title);

    cur_ticks = trendit_get_ticks();
    while(1)
    {
        ddi_watchdog_feed();
        
        if(ddi_gprs_get_pdpstatus(WS_GET_WIRELESS_STATE) >= WS_SEARCHING_NETWORK)
        {
            TRACE_DBG("detect wireless module now");
            break;
        }

        if(trendit_query_timer(cur_ticks, 10*1000))
        {
            TRACE_DBG("timeout to detect wireless module");
            break;
        }

        ddi_misc_msleep(100);
    }
    
    if(devicetestmode == DEVICE_TEST_MODE_SMT)//smt 测试
    {
        //播放测试音频
        audio_instance()->audio_play(AUDIO_SMT_TEST_ENTER, AUDIO_PLAY_BLOCK);

        TRACE_INFO("smt test000\r\n");
        ddi_led_ioctl(DDI_LED_CTL_CLOSE_YELLOW, 0, 0);//关闭led 循环显示

        ddi_wifi_open();
        ap_num = sizeof(wifi_aps)/sizeof(wifi_aps[0]);
        ddi_wifi_scanap_start(wifi_aps, ap_num);
        TRACE_INFO("smt test111\r\n");
        for(i = 0; i < single_cnt; i++)
        {
            ret = g_single_test_cases[i](&devtestinfo, devicetestmode);
            TRACE_DBG("e1 smt test:%d,%d",i,ret);
            #if 0
            if(ret != DDI_OK)
            {
                return;
            }
            #endif
        }
        //TRACE_DBG("save device result\r\n");
        //TRACE_DBG("save led:%d,key:%d,gprs:%d,wifi:%d,audio:%d,printer:%d\r\n",devtestinfo.led, devtestinfo.key, devtestinfo.gprs, devtestinfo.wifi, devtestinfo.audio, devtestinfo.printer);
        save_device_test_info(workmode, &devtestinfo, 0);
        //TRACE_DBG("start print result\r\n");
        processtask_print_all_test_result(&devtestinfo, devicetestmode);
        if(precesstask_check_deviceinfo_result(&devtestinfo) == DDI_OK)
        {
            processtask_show_led(devicetestmode);
        }
        
    }
    else if(devicetestmode == DEVICE_TEST_MODE_INTEGER)//一次测试
    {
        audio_instance()->audio_play(AUDIO_ASSEMBLY_TEST_ENTER, AUDIO_PLAY_NOBLOCK);

        ddi_wifi_open();

        ret = ddi_wifi_ioctl(WIFI_IOCTL_CLEAR_RF_PARAM, 0, 0);
        if(DDI_ERR == ret)
        {
            printer_instance()->direct_print("wifi射频参数初始化失败，请确认固件是否正确\r\n\r\n\r\n\r\n", strlen("wifi射频参数初始化失败，请确认固件是否正确\r\n\r\n\r\n\r\n"));
        }
        else if(DDI_ETIMEOUT == ret)
        {
            printer_instance()->direct_print("wifi射频参数初始化超时\r\n\r\n\r\n\r\n", strlen("wifi射频参数初始化超时\r\n\r\n\r\n\r\n"));
        }
        ddi_wifi_connectap_start(WIFI_SSID, WIFI_PWD, NULL);
        
        processtask_show_led(devicetestmode);
                
        TRACE_DBG("start once test\r\n");
        for(i = 0; i < single_cnt; i++)
        {
            ret = g_single_test_cases[i](&devtestinfo, devicetestmode);
            TRACE_DBG("e1 once test:%d,%d",i,ret);

        }
        TRACE_DBG("e1 once test save:%d",workmode);
        save_device_test_info(workmode, &devtestinfo, 0);
        processtask_print_all_test_result(&devtestinfo, devicetestmode);
        ddi_led_ioctl(DDI_LED_CTL_LOOP, 0, 0);//关闭 led 循环显示
    }
    trendit_cdc_output_switch(FALSE);
}

void test_savettsfilesyncresult(TTSFILE_SYNC_RESULT ttsfilesyncresult)
{
    
    TRENDITDEV_TEST_INFO devtestinfo;
    
    memset(&devtestinfo,0,sizeof(TRENDITDEV_TEST_INFO));

    sdk_misc_get_mcuparam(MACHINE_PARAM_DEVTESTINFO, (u8 *)&devtestinfo,sizeof(TRENDITDEV_TEST_INFO),0);

    devtestinfo.audiofilesyncresult = ttsfilesyncresult;
    sdk_misc_set_mcuparam(MACHINE_PARAM_DEVTESTINFO, (u8 *)&devtestinfo,sizeof(TRENDITDEV_TEST_INFO),0);
    
}


u8 test_readttsfilesyncresult(void)
{
    
    TRENDITDEV_TEST_INFO devtestinfo;
    
    memset(&devtestinfo,0,sizeof(TRENDITDEV_TEST_INFO));

    sdk_misc_get_mcuparam(MACHINE_PARAM_DEVTESTINFO, (u8 *)&devtestinfo,sizeof(TRENDITDEV_TEST_INFO),0);

    return devtestinfo.audiofilesyncresult;
    
}


