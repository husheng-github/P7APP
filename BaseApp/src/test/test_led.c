#include "ddiglobal.h"
#include "app_global.h"
#include "test_global.h"

#define LED_ON 1
#define LED_OFF 0

void set_all_led_status(s32 flg)
{

        ddi_led_sta_set(LED_SIGNAL_Y, flg);

        ddi_led_sta_set(LED_SIGNAL_B, flg);

        ddi_led_sta_set(LED_STATUS_B, flg);

        ddi_led_sta_set(LED_STATUS_R, flg);

}

void set_status_led()
{
    u8 flg=1;
    u8 key;
    u32 timeid;

    while(1)
    {
        if(flg==1)
        {
            flg = 0;
            TRACE_DBG("================状态灯==============");
            TRACE_DBG("1. 亮蓝色");
            TRACE_DBG("2. 亮红色");
            TRACE_DBG("3. 关灯");
        }

        ddi_watchdog_feed();
        if(ddi_com_read(PORT_CDCD_NO, (u8 *)&key, 1) > 0)
        {
            TRACE_DBG("key:%x\r\n",key);
            switch(key)
            {
                case '1':
                    ddi_led_sta_set(LED_STATUS_B, 1);
                    flg = 1;
                    break;

                case '2':
                    ddi_led_sta_set(LED_STATUS_R, 1);
                    flg = 1;
                    break;

                case '3':
                    set_all_led_status(0);
                    flg = 1;
                    break;
                
                case 'e':
                    return ;

            }
        
        }
    }
}

void set_signal_led()
{
    u8 flg=1;
    u8 key;
    u32 timeid;

    while(1)
    {
        if(flg==1)
        {
            flg = 0;
            TRACE_DBG("================状态灯==============");
            TRACE_DBG("1. 亮黄色");
            TRACE_DBG("2. 亮蓝色");
            TRACE_DBG("3. 关灯");
        }

        ddi_watchdog_feed();
        if(ddi_com_read(PORT_CDCD_NO, (u8 *)&key, 1) > 0)
        {
            TRACE_DBG("key:%x\r\n",key);
            switch(key)
            {
                case '1':
                    ddi_led_sta_set(LED_SIGNAL_Y, 1);
                    flg = 1;
                    break;
                    
                case '2':
                    ddi_led_sta_set(LED_SIGNAL_B, 1);
                    flg = 1;
                    break;

                case '3':
                    set_all_led_status(0);
                    flg = 1;
                    break;
                
                case 'e':
                    return ;

            }
        
        }
    }
}

s32 test_led(void)
{
    u8 flg=1;
    u8 key;
    u32 timeid;

    //ddi_misc_get_tick
    ddi_led_ioctl(DDI_LED_CTL_CLOSE_YELLOW, 0, 0);
    while(1)
    {
        if(flg==1)
        {
            flg = 0;
            TRACE_DBG("================LED Test ==============");
            TRACE_DBG("1、状态灯");
            TRACE_DBG("2、信号灯");
        }


        ddi_watchdog_feed();
        if(ddi_com_read(PORT_CDCD_NO, (u8 *)&key, 1) > 0)
        {
            TRACE_DBG("key:%x\r\n",key);
            switch(key)
            {
                case '1':
                    set_status_led();
                    flg = 1;
                    break;
                
                case '2':
                    set_signal_led();
                    flg = 1;
                    break;
                
                case 'e':
                    return 0;

            }
        
        }
    }

}

s32 auto_led_test(void)
{
    u8 key;
    s32 i = 0;
    s32 flag = 0;
    s32 ret = DDI_ERR;
    s32 workmode;
    u8 devicetestmode;


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

    trendit_factory_test_display_msg(AUDIO_TEST_LED_ENTER, AUDIO_PLAY_BLOCK);
    ddi_key_clear();
    TRACE_DBG("enter");
    if(devicetestmode == DEVICE_TEST_MODE_INTEGER)
    {
        trendit_factory_test_display_msg(AUDIO_TEST_KEY_FINISH, AUDIO_PLAY_BLOCK);
        while(1)
        {
            key = trendit_get_key();
            if(key > 0)
            {
                TRACE_DBG("KEY:%04x", key);
                switch(key)
                {
                    case KEY_CFG:
                        return DDI_OK;
                    break;
                    default :
                        return DDI_ERR;
                        break;
                }
            }
        }
        
    }
    else
    {
        trendit_factory_test_display_msg(AUDIO_TEST_KEY_PLUS, AUDIO_PLAY_NOBLOCK);
        while(1)
        {
            key = trendit_get_key();
            if(key > 0)
            {
                TRACE_DBG("KEY:%04x", key);
                switch(key)
                {

                    case KEY_PLUS:
                        if(0 == i)
                        {
                            set_all_led_status(0);
                            ddi_led_sta_set(LED_STATUS_B, 1);
                            trendit_factory_test_display_msg(AUDIO_TEST_LED_STATUS_B, AUDIO_PLAY_BLOCK);
                        }
                        else if(1 == i)
                        {
                            set_all_led_status(0);
                            ddi_led_sta_set(LED_STATUS_R, 1);
                            trendit_factory_test_display_msg(AUDIO_TEST_LED_STATUS_R, AUDIO_PLAY_BLOCK);
                        }
                        else if(2 == i)
                        {
                            set_all_led_status(0);
                            ddi_led_sta_set(LED_SIGNAL_B, 1);
                            trendit_factory_test_display_msg(AUDIO_TEST_LED_SIGNAL_B, AUDIO_PLAY_BLOCK);
                        }
                        else if(3 == i)
                        {
                            set_all_led_status(0);
                            ddi_led_sta_set(LED_SIGNAL_Y, 1);
                            trendit_factory_test_display_msg(AUDIO_TEST_LED_SIGNAL_Y, AUDIO_PLAY_BLOCK);

                            ddi_misc_msleep(100);
                            trendit_factory_test_display_msg(AUDIO_TEST_KEY_FINISH, AUDIO_PLAY_BLOCK);
                            ddi_key_clear();
                        }
                        else
                        {
                            set_all_led_status(FALSE);   //关闭灯
                            if(0 == dev_misc_get_workmod())   //正常模式测试完成后亮蓝灯，避免给人感觉机器关机了
                            {
                                ddi_led_sta_set(LED_STATUS_B, 1);
                            }
                            return DDI_ERR;
                        }
                        i++;
                        break;
                        
                    case KEY_CFG:
                        if(i > 3)
                        {
                            set_all_led_status(FALSE);   //关闭灯
                            if(0 == dev_misc_get_workmod())   //正常模式测试完成后亮蓝灯，避免给人感觉机器关机了
                            {
                                ddi_led_sta_set(LED_STATUS_B, 1);
                            }
                            return DDI_OK;
                        }
                        break;
                }
            }
        }
    }
    

    return 0;
}

