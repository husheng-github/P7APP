#include "ddiglobal.h"
#include "app_global.h"
#include "test_global.h"

static s32 get_imei()
{
    u8 ret_buff[256] = {0};
    s32 ret = DDI_ERR;

    memset(ret_buff, 0, sizeof(ret_buff));
    ret = ddi_gprs_get_imei(ret_buff, sizeof(ret_buff));

    TRACE_INFO("imei:%s, ret:%d", ret_buff, ret);
    return 0;
}

static s32 get_ccid()
{
    u8 ret_buff[256] = {0};
    s32 ret = DDI_ERR;
    strSimInfo siminfo;

    memset(ret_buff, 0, sizeof(ret_buff));
    ret = ddi_gprs_get_siminfo(&siminfo);
    TRACE_INFO("ccid:%s, imsi:%s ret:%d", siminfo.iccid, siminfo.imsi, ret);
    
    return 0;
}

static s32 get_imsi()
{
    u8 ret_buff[256] = {0};
    s32 ret = DDI_ERR;
    strSimInfo siminfo;

     memset(ret_buff, 0, sizeof(ret_buff));
    ret = ddi_gprs_get_siminfo(&siminfo);
    TRACE_INFO("ccid:%s, imsi:%s ret:%d", siminfo.iccid, siminfo.imsi, ret);
    
    return 0;
}

static s32 get_cur_cell()
{
    strBasestationInfo cellinfo;
    s32 ret = 0;

    memset(&cellinfo, 0, sizeof(cellinfo));
    ret = ddi_gprs_get_basestation_info(&cellinfo);
    TRACE_INFO("ret:%d, mcc:%s, mnc:%s, lac:%s, cid:%s", 
                                        ret, 
                                        cellinfo.asMcc, 
                                        cellinfo.asMnc, 
                                        cellinfo.asLac, 
                                        cellinfo.asCi);
}

static s32 get_csq()
{
    u8 ret_buff[256] = {0};
    s32 ret = DDI_ERR;
    u8 prssi;
    u16 prxfull;

    ret = ddi_gprs_get_signalquality(&prssi, &prxfull);
    TRACE_INFO("csq:%d", prssi, prxfull);
    
    return 0;
}

static s32 wireless_dail_hangup()
{
    s32 ret = DDI_ERR;

    ret = ddi_gprs_telephony_hangup();

    TRACE_DBG("dail hangup ret:%d", ret);

    return 0;
}

static s32 wireless_dail()
{
    s32 ret = DDI_ERR;
    u8 key = 0;
    u8 key_code = 0;
    s32 step = 0;
    s32 try_times = 0;
    s8 buff[128] = {0};
    u32 cur_ticks  = trendit_get_ticks();

    ddi_misc_msleep(2000);
    ddi_gprs_ioctl(0xfe, 0, 0);
    while(1)
    {
        if(trendit_query_timer(cur_ticks, 3000))
        {
            key_code = trendit_get_key();
            if(key_code > 0)
            {
                snprintf(buff, sizeof(buff), "按键退出%04x\r\n\r\n\r\n", key_code);
                printer_instance()->direct_print(buff, strlen(buff));
                ddi_key_clear();
                break;
            }
        }
        else
        {
            ddi_key_clear();
        }
        
        if(ddi_com_read(PORT_CDCD_NO, (u8 *)&key, 1) > 0 && 'e' == key)
        {
            break;
        }
        
        switch(step)
        {
            case 0: //打开飞行模式
                ret = ddi_gprs_ioctl(DDI_GPRS_CTL_OPEN_AIRPLANE, 0, 0);
                if(ret != DDI_OK)
                {
                    continue;
                }

                step = 1;
                break;

            case 1: //关闭飞行模式
                ret = ddi_gprs_ioctl(DDI_GPRS_CTL_CLOSE_AIRPLANE, 0, 0);
                if(ret != DDI_OK)
                {
                    continue;
                }

                step = 2;
                break;

            case 2:  
                if(try_times < 5)     //只尝试5次
                {
                    ret = ddi_gprs_telephony_dial("112");
                    if(ret == DDI_EWIRE_REGING)          //正在注网
                    {
                        TRACE_INFO("network don't reg success");
                        ddi_misc_msleep(1000);
                        continue;
                    }
                    else if(ret == DDI_OK)               //拨号成功
                    {
                        step = 3;
                        TRACE_INFO("dial success");
                    }
                    else if(ret == DDI_ERR || ret == DDI_ETIMEOUT)
                    {
                        step = 4;
                        try_times++;
                        TRACE_INFO("dial fail %d times, ret:%d", try_times, ret);
                        continue;
                    }
                }
                break;

            case 3:   //不做任何事
                break;

            case 4:       //出错后挂断，然后重播
                ddi_gprs_telephony_hangup();
                step = 2;
                break;
        }

        ddi_watchdog_feed();
        ddi_misc_msleep(50);
    }
    
    wireless_dail_hangup();
    ddi_gprs_ioctl(0xff, 0, 0);
    return 0;
}

static s32 wireless_check_networkok()
{
    s32 ret = DDI_ERR;
    s32 ret1 = 0;
    u32 cur_ticks = trendit_get_ticks();

    while(1)
    {
        ddi_watchdog_feed();
        if(trendit_query_timer(cur_ticks, 60*1000))
        {
            ret = DDI_ETIMEOUT;
            break;
        }
        
        ret1 = ddi_gprs_get_pdpstatus(WS_GET_REG_NETWORK_FAIL_REASON);
        switch(ret1)
        {
            case WS_NO_INSERT_SIM:
                TRACE_ERR("don't detect sim card");
                ret = DDI_ESIM;
                goto _out;
                break;

            case WS_REGISTRATION_DENIED:
                TRACE_ERR("reg network denied");
                ret = DDI_EREG_DENIED;
                goto _out;
                break;

            case WS_NO_WIRELESS_MODULE:
                TRACE_ERR("don't detect wireless module");
                ret = DDI_ENODEV;
                goto _out;
                break;

            default:
                break;
        }
        
        if(WS_ACTIVE_PDP_SUCCESS == ddi_gprs_get_pdpstatus(WS_GET_WIRELESS_STATE))
        {
            ret = DDI_OK;
            TRACE_DBG("active network success");
            break;
        }

        ddi_misc_msleep(1000);
    }

_out:
    return ret;
}

static s32 wireless_checkconnect(s32 socket_id)
{
    s32 ret = DDI_ERR;
    u32 cur_ticks = trendit_get_ticks();

    while(1)
    {
        ddi_watchdog_feed();
        if(trendit_query_timer(cur_ticks, 60*1000))
        {
            ret = DDI_ETIMEOUT;
            break;
        }
        
        ret = ddi_gprs_socket_get_status(socket_id);
        if(DDI_EINVAL == ret || GPRS_STATUS_CONNECTED == ret || GPRS_STATUS_DISCONNECTED == ret)
        {
            break;
        }

        ddi_misc_msleep(100);
    }

    return ret;
}

static s32 get_model_ver()
{
    u8 ret_buff[256] = {0};
    s32 ret = DDI_ERR;
    u32 cur_ticks = trendit_get_ticks();

    while(1)
    {
        ddi_watchdog_feed();
        ret = ddi_gprs_ioctl(DDI_GPRS_GET_MODULE_VER, (u32)ret_buff, sizeof(ret_buff));
        if(DDI_OK == ret)
        {
            TRACE_DBG("ret:%d  %s", ret, ret_buff);
            break;
        }

        if(trendit_query_timer(cur_ticks, 3000))
        {
            ret = DDI_ETIMEOUT;
            break;
        }
        ddi_misc_msleep(50);
    }
    
    return ret;
}


static s32 wireless_commu()
{
    s32 ret = DDI_ERR;
    s32 socket_id = 0;
    s8 r_buff[128] = {0};
    s32 r_len = 0;
    s32 i = 0;
    u32 cur_ticks = 0;

    do{
        ddi_gprs_open();

        ret = wireless_check_networkok();
        if(DDI_OK != ret)
        {
            TRACE_ERR("reg network failed:%d", ret);
            break;
        }

        if(dev_misc_get_workmod() == 1)   //组装测试
        {
            ret = DDI_OK;
            break;
        }
                
        socket_id = ddi_gprs_socket_create(SOCKET_TYPE_TCP, HOST_TYPE_IPADDR, "47.106.157.121", 8008);
        if(socket_id < 0)
        {
            ret = DDI_EIO;
            break;
        }

        TRACE_DBG("creat socket ret:%d", socket_id);
        if(GPRS_STATUS_DISCONNECTED == wireless_checkconnect(socket_id))
        {
            ret = DDI_ERR;
            TRACE_ERR("link disconnect");
            break;
        }

        for(i=0; i<sizeof(r_buff); i++)
        {
            r_buff[i] = i;
        }

        ret = ddi_gprs_socket_send(socket_id, r_buff, sizeof(r_buff));
        if(ret < 0)
        {
            TRACE_ERR("send error:%d", ret);
            ret = DDI_ERR;
            break;
        }

        TRACE_DBG("send ret:%d", ret);
        cur_ticks = trendit_get_ticks();
        r_len = 0;
        while(1)
        {
            ddi_watchdog_feed();
            
            ret = ddi_gprs_socket_recv(socket_id, r_buff+r_len, sizeof(r_buff)-r_len);
            if(ret > 0)
            {
                r_len += ret;
                TRACE_INFO("recv ret:%d", ret);
                TRACE_DBG_HEX(r_buff, ret);
            }
            else if(ret < 0)
            {
                TRACE_ERR("recv error:%d", ret);
            }

            if(r_len >= sizeof(r_buff))
            {
                TRACE_DBG("recv len:%d", r_len);
                ret = DDI_OK;
                break;
            }
            
            if(trendit_query_timer(cur_ticks, 30000))
            {
                TRACE_DBG("recv overtime");
                ret = DDI_ETIMEOUT;
                break;
            }

            ddi_misc_msleep(100);
        }
    }while(0);

    if(socket_id)
    {
        ddi_gprs_socket_close(socket_id);
    }

    return ret;
}

s32 test_gprs_transparent(void)
{
#define GPRS_PORT   PORT_UART1_NO
    u8 dispflg=1;
    s32 ret;
    u8 key;
    u8 buff[512];
    strComAttr lpstrAttr;

    ddi_gprs_ioctl(DDI_GPRS_CMD_TRANSPARENT, TRUE, 0);
    TRACE_DBG("ret:%d\r\n", ret);

    lpstrAttr.m_baud = 115200;
    lpstrAttr.m_databits = 8;
    lpstrAttr.m_parity = 'n';
    lpstrAttr.m_stopbits = 0;
    ret = ddi_com_open(GPRS_PORT, &lpstrAttr);
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
            
            ddi_com_write(GPRS_PORT, buff, strlen(buff));
        }

        memset(buff, 0, sizeof(buff));
        ret = ddi_com_read(GPRS_PORT, buff, sizeof(buff));
        if(ret > 0)
        {
            TRACE_DBG("<< %s", buff);
        }
        ddi_misc_msleep(100);
    }

    ddi_gprs_ioctl(DDI_GPRS_CMD_TRANSPARENT, FALSE, 0);

    return 0;
}

s32 test_gprs(void)
{   
    u8 flg=1;
    u8 key;
    u32 timeid;
    strComAttr comattr;

    while(1)
    {
        if(flg==1)
        {
            flg = 0;
            TRACE_DBG("================Test MainMenu===============");
            TRACE_DBG("1.获取imei");
            TRACE_DBG("2.获取ccid");
            TRACE_DBG("3.获取imsi");
            TRACE_DBG("4.获取基站");
            TRACE_DBG("5.信号强度");
            TRACE_DBG("6.拨号");
            TRACE_DBG("7.网络通讯");
            TRACE_DBG("8.固件版本");
            TRACE_DBG("9.命令透明传输")
            TRACE_DBG("====================Menu End=================");
        }

        if(ddi_com_read(PORT_CDCD_NO, (u8 *)&key, 1) > 0)
        {
            switch(key)
            {     
                case '1':
                    get_imei();
                    flg = 1;
                    break;
                
                case '2':
                    get_ccid();
                    flg = 1;
                    break;

                case '3':
                    get_imsi();
                    flg = 1;
                    break;
                
                case '4':
                    get_cur_cell();
                    flg = 1;
                    break;

                case '5':
                    get_csq();
                    flg = 1;
                    break;
                
                case '6':
                    wireless_dail();
                    flg = 1;
                    break;
                    
                case '7':
                    wireless_commu();
                    flg = 1;
                    break;

                case '8':
                    get_model_ver();
                    flg = 1;
                    break;

                case '9':
                    test_gprs_transparent();
                    flg = 1;
                    break;
                    
                case 'e':
                    return 0;
                    break;
            }

            ddi_watchdog_feed();
            ddi_misc_msleep(500);
        }
    }

    return 0;
}

s32 auto_wireless_test(void)
{
    s32 ret = DDI_ERR;
    s8 print_buff[] = "无线正在驻网，请稍后\r\n\r\n\r\n";

    do{
        if(DDI_OK != get_model_ver())
        {
            TRACE_ERR("get wire module failed");
            break;
        }

        while(1)
        {
            ddi_watchdog_feed();
            
            ret = ddi_gprs_ioctl(DDI_GPRS_CTL_CHECKSIM, 0, 0);
            if(DDI_ERR == ret || DDI_OK == ret)
            {
                TRACE_ERR("simcard don't detect");
                break;
            }

            ddi_misc_msleep(100);
        }

        if(DDI_ERR == ret)
        {
            TRACE_DBG("don't detect simcard");
            break;
        }

        TRACE_DBG("workmod:%d", dev_misc_get_workmod());
        //一次测试、smt以外的测试，加入数据通信
        if(dev_misc_get_workmod() == 0 || dev_misc_get_workmod() == 1)
        {
            if(MACHINE_P7_HEX == trendit_get_machine_code())
            {
                audio_instance()->audio_play(AUDIO_ATTACHING_WIRELESS, AUDIO_PLAY_BLOCK);
            }
            else
            {
                printer_instance()->direct_print(print_buff, strlen(print_buff));
            }
            if(DDI_OK != wireless_commu())
            {
                ret = DDI_ERR;
                break;
            }
        }

        ret = DDI_OK;
    }while(0);

    return ret;
}

s32 wireless_8960_test(void)
{
    trendit_factory_test_display_msg(AUDIO_TEST_8960, AUDIO_PLAY_BLOCK);

    wireless_dail();

    return 0;
}



