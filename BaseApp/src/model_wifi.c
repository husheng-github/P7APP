#include "ddiglobal.h"
#include "app_global.h"

static wifi_module_t *g_model_wifi = NULL;

/**
 * @brief 初始化本文件所有的全局变量
 */
void trendit_init_wifi_data(void)
{
    g_model_wifi = NULL;
}

static s32 get_index_page(s8 *fill_buff, s32 fill_buff_len)
{
    //s8 buff[] = "<html><head><meta http-equiv='Content-Language' content='zh-cn'><meta http-equiv='Content-Type' content='text/html; charset=gb2312'><meta name='viewport' content='width=device-width, initial-scale=1' /><title>云打印机--wifi网页配置</title><style type='text/css'>.s1{box-sizing:border-box;border:1px solid #999;padding:5px;height:auto;border-radius:3px;font-family:'宋体,Microsoft Yahei,Arial';}.signOut{width: 84%; background: #69a1ed;border: none;border-top:1px solid #e5e5e5;font-size: 16px;padding: 5px 0;border-radius: 5px;}.text_r{text-align: right;}.text_c{text-align: center;}.td_c{font-family: '宋体,Microsoft Yahei,Arial'; font-size: 10.5pt;padding: 3px;}</style></head><body bgcolor='#F0F0F0'><form method='GET' action='wifi_cfg.cgi'> <div align='center'><br><br><table border='0' cellpadding='0' style='width:auto; border-collapse: collapse'><tr><td class='td_c text_c font14' colspan='2'><b>wifi网页配置</b></td></tr><tr><td class='td_c text_r'>热点名：</td><td class='td_c'><input type='' id='wifi_ssid' name='wifi_ssid' value='' class='s1' size='30'></td></tr><tr><td class='td_c text_r'>热点密码：</td><td class='td_c'><input type='' id='wifi_password' name='wifi_password' value='' class='s1' size='30'></td></tr><tr><td class='td_c text_c' colspan='2'><input type='submit' id='sumbit_b' value='设置' name='sumbit_b' class='signOut'></td></tr></table></div></form></body></html>";
    s8 buff[] = "<!doctype html><html lang='Zh'><head><meta charset='GBK'><meta name='viewport' content='width=device-width, initial-scale=1, shrink-to-fit=no'><title>WIFI配置</title><style type='text/css'>*,*::before,*::after {box-sizing: border-box;}html {line-height: 1.15;-webkit-text-size-adjust: 100%;}body {margin: 0;font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI',Arial;line-height: 1.5;background-color: #f5f5f5;padding-bottom: 200px;align-items: center;-ms-flex-align: center;display: flex;}p {margin-top: 0;margin-bottom: 1rem;}button{cursor: pointer;}.h3 {margin-top: 0;margin-bottom: 0.5rem;font-weight: 500;line-height: 1.2;font-size: 1.75rem;}.form-control {width: 100%;height: calc(1.5em + 0.75rem + 2px);padding: 0.375rem 0.75rem;font-size: 1rem;font-weight: 400;line-height: 1.5;color: #495057;background-color: #fff;background-clip: padding-box;border: 1px solid #ced4da;border-radius: 0.25rem;transition: border-color 0.15s ease-in-out, box-shadow 0.15s ease-in-out;}.form-control::-ms-expand {background-color: transparent;border: 0;}.form-control:focus {color: #495057;background-color: #fff;border-color: #80bdff;outline: 0;box-shadow: 0 0 0 0.2rem rgba(0, 123, 255, 0.25);}.form-control::-webkit-input-placeholder {color: #6c757d;opacity: 1;}.form-control::-moz-placeholder {color: #6c757d;opacity: 1;}.form-control:-ms-input-placeholder {color: #6c757d;opacity: 1;}.form-control::-ms-input-placeholder {color: #6c757d;opacity: 1;}.form-control:disabled {background-color: #e9ecef;opacity: 1;}.btn {color: #fff;background-color: #007bff;border-color: #007bff;padding: 0.5rem 1rem;font-size: 1.25rem;line-height: 1.5;border-radius: 0.3rem;display: block;width: 100%;}.mb-3 {margin-bottom: 1rem !important;}.mb-4 {margin-bottom: 1.5rem !important;}.text-center {text-align: center !important;}.font-weight-normal {font-weight: 400 !important;}html,body {height: 100%;}.form-signin {width: 100%;max-width: 420px;padding: 15px;margin: auto;}.form-label-group {position: relative;margin-bottom: 1rem;}.form-label-group > input,.form-label-group > label {height: 3.125rem;padding: .75rem;}.form-label-group > label {position: absolute;top: 0;left: 0;display: block;width: 100%;margin-bottom: 0;line-height: 1.5;color: #495057;pointer-events: none;cursor: text;border: 1px solid transparent;border-radius: .25rem;transition: all .1s ease-in-out;}.form-label-group input::-webkit-input-placeholder {color: transparent;}.form-label-group input:-ms-input-placeholder {color: transparent;}.form-label-group input::-ms-input-placeholder {color: transparent;}.form-label-group input::-moz-placeholder {color: transparent;}.form-label-group input::placeholder {color: transparent;}.form-label-group input:not(:placeholder-shown) {padding-top: 1.25rem;padding-bottom: .25rem;}.form-label-group input:not(:placeholder-shown) ~ label {padding-top: .25rem;padding-bottom: .25rem;font-size: 12px;color: #777;}@supports (-ms-ime-align: auto) {.form-label-group > label {display: none;}.form-label-group input::-ms-input-placeholder {color: #777;}}@media all and (-ms-high-contrast: none), (-ms-high-contrast: active) {.form-label-group > label {display: none;}.form-label-group input:-ms-input-placeholder {color: #777;}}</style></head><body><form method='GET' action='wifi_cfg.cgi' class='form-signin'><div class='text-center mb-4'><h1 class='h3 mb-3 font-weight-normal'>WIFI配置</h1></div><p>1.wifi名字不能为中文.<br> 2.wifi密码为空可以不填.</p><div class='form-label-group'><input type='text' name='wifi_ssid' id='wifi_ssid' class='form-control' placeholder='wifi名称' required autofocus><label for='inputEmail'>wifi名称</label></div><div class='form-label-group'><input type='text' id='wifi_password' name='wifi_password' class='form-control' placeholder='wifi密码'><label for='inputPassword'>wifi密码</label></div><button class='btn' type='submit'>确定</button></form></body></html>";
    s8 head_buff[] = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n";
 
    snprintf(fill_buff, fill_buff_len, "%sContent-Length:%d\r\n\r\n%s", head_buff, strlen(buff), buff);

    return strlen(fill_buff);
}

static s32 get_response_page(s8 *fill_buff, s32 fill_buff_len, s8 *content)
{
    s8 buff[] = "<html><head><meta http-equiv='Content-Language' content='zh-cn'><meta http-equiv='Content-Type' content='text/html; charset=gb2312'><meta name='viewport' content='width=device-width, initial-scale=1' /><title>云打印机--wifi网页配置</title></head><body bgcolor='#F0F0F0'><p align='center' style=\"font-family: '宋体,Microsoft Yahei,Arial'; font-size: 10.5pt;\">";
    s8 head_buff[] = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n";
    s8 end_buff[] = "</p></body></html>";
    
    snprintf(fill_buff, fill_buff_len, "%sContent-Length:%d\r\n\r\n%s%s%s", head_buff, strlen(content)+strlen(buff)+strlen(end_buff), buff, content, end_buff);
    
    return strlen(fill_buff);
}

static void print_wifi_web_config_info(void)
{
    s8 tmp_buff[2048] = {0};
    s32 len = 0;
    s32 ret = DDI_ERR;
    s8 *pstr = NULL;
    u8 tmp_len = 0;
    
    memset(tmp_buff, 0, sizeof(tmp_buff));
    memcpy(tmp_buff, "\x1B\x21\x00", 3);//字体设置为24X24
    len += 3;
    
    memcpy(tmp_buff+len, "\x1B\x61\x01", 3);//居中
    len += 3;

    memcpy(tmp_buff+len, "\x1D\x21\x11", 3); //放大一倍
    len += 3;
    pstr = "网页配网步骤\r\n";
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, pstr);
    len += strlen(pstr);
    
    memcpy(tmp_buff+len, "\x1D\x21\x00", 3); //还原
    len += 3;

    memcpy(tmp_buff+len, "\x1B\x61\x00", 3);//居左
    len += 3;

    pstr = "\r\n1. 用手机连上打印机发出的wifi热点:Cloud-Printer;\r\n";
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, pstr);
    len += strlen(pstr);

    pstr = "\r\n2. 打开手机浏览器，输入地址:192.168.4.1 或者扫描下面二维码:\r\n";
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, pstr);
    len += strlen(pstr);

    tmp_len= strlen("http://192.168.4.1");
    tmp_buff[len++] = 0x1D;
    tmp_buff[len++] = 0x71;
    tmp_buff[len++] = tmp_len+3;
    tmp_buff[len++] = 1;   //容错0
    tmp_buff[len++] = 0x9; //放大9倍
    tmp_buff[len++] = 0x2; //居中
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s\r\n", "http://192.168.4.1");
    len += 2+tmp_len;

    pstr = "\r\n3. 在WIFI配置页面中，输入wifi热点名和密码，点击“确定”按钮，等待打印机语音提示即可。\r\n";
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, pstr);
    len += strlen(pstr);

    pstr = "\r\n4. 注意：wifi热点不能是5G热点名;\r\n\r\n\r\n\r\n";
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, pstr);
    len += strlen(pstr);

    printer_instance()->direct_print(tmp_buff, len);
}

static void print_wifi_airkiss_config_info(void)
{
    s8 tmp_buff[2048] = {0};
    s32 len = 0;
    s32 ret = DDI_ERR;
    s8 *pstr = NULL;
    u8 tmp_len = 0;
    machine_module_t *machine_module = machine_instance();
    s8 airkiss_url[64] = {0};
    
    memset(tmp_buff, 0, sizeof(tmp_buff));
    memcpy(tmp_buff, "\x1B\x21\x00", 3);//字体设置为24X24
    len += 3;
    
    memcpy(tmp_buff+len, "\x1B\x61\x01", 3);//居中
    len += 3;

    memcpy(tmp_buff+len, "\x1D\x21\x11", 3); //放大一倍
    len += 3;

    pstr = "微信配网步骤\r\n";
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, pstr);
    len += strlen(pstr);
    
    memcpy(tmp_buff+len, "\x1D\x21\x00", 3); //还原
    len += 3;

    memcpy(tmp_buff+len, "\x1B\x61\x00", 3);//居左
    len += 3;

    pstr = "\r\n1. 用手机连接要设置的wifi热点;\r\n";
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, pstr);
    len += strlen(pstr);
    
    pstr = "\r\n2. 用微信扫一扫，扫描下方二维码，进入配置页:\r\n";
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, pstr);
    len += strlen(pstr);

    tmp_len = strlen(machine_module->m_wifi_airkiss_url);
    memset(airkiss_url, 0, sizeof(airkiss_url));
    if(tmp_len)
    {
        snprintf(airkiss_url, sizeof(airkiss_url), "%s", machine_module->m_wifi_airkiss_url);
    }
    else
    {
        if(MACHINE_P6_HEX == trendit_get_machine_code())
        {
            snprintf(airkiss_url, sizeof(airkiss_url), "%s", "http://wificfg.trenditadmin.com/wxconfig/index");
            tmp_len = strlen("http://wificfg.trenditadmin.com/wxconfig/index");
        }
        else
        {
            snprintf(airkiss_url, sizeof(airkiss_url), "%s", "http://iot.trenditadmin.com/airkiss/airkiss_printer.html");
            tmp_len = strlen("http://iot.trenditadmin.com/airkiss/airkiss_printer.html");
        }
    }
    tmp_buff[len++] = 0x1D;
    tmp_buff[len++] = 0x71;
    tmp_buff[len++] = tmp_len+3;
    tmp_buff[len++] = 1;   //容错0
    tmp_buff[len++] = 0x7; //放大9倍
    tmp_buff[len++] = 0x2; //居中
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, "%s\r\n", airkiss_url);
    len += 2+tmp_len;

    pstr = "\r\n3. 在配置页中，输入wifi密码后点击“连接”按钮，等待提示即可;\r\n";
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, pstr);
    len += strlen(pstr);

    pstr = "\r\n4. 如果尝试多次配置超时或不成功，请参考说明书，选用网页配网方式设置wifi;\r\n";
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, pstr);
    len += strlen(pstr);

    pstr = "\r\n5. 注意：wifi热点不能是5G热点名;\r\n\r\n\r\n\r\n";
    snprintf(tmp_buff+len, sizeof(tmp_buff)-len, pstr);
    len += strlen(pstr);

    printer_instance()->direct_print(tmp_buff, len);
}

static s32 send_wifi_data(s32 client_socketid, s8 *buff, s32 len)
{
    s32 send_len = 0;
    s32 tmp_len = 0;
    s32 ret = DDI_ERR;

    while(1)
    {
        if(len-send_len > 1792)
        {
            tmp_len = 1792;
        }
        else
        {
            tmp_len = len-send_len;
        }
        
        ret = ddi_wifi_socket_send(client_socketid, buff+send_len, tmp_len);
        if(ret <= 0)
        {
            TRACE_ERR("socket send error:%d", ret);
            break;
        }
        else
        {
            send_len += ret;
        }

        if(send_len >= len)
        {
            ret = DDI_OK;
            break;
        }

        ddi_watchdog_feed();
    }

    return ret;
}

static void fill_socket_id(u32 *client_socketid_tab, u32 socket_id)
{
    s32 i = 0;

    for(i=0; i<3; i++)
    {
        if(client_socketid_tab[i] == socket_id)
        {
            return ;
        }
    }
    
    for(i=0; i<3; i++)
    {
        if(client_socketid_tab[i] == 0)
        {
            TRACE_DBG("i:%d", i);
            client_socketid_tab[i] = socket_id;
            break;
        }
    }
}

static void wifi_web_config(void)
{
    s32 ret = 0;
    s32 step = 0;
    s32 client_socketid_tab[3] = {0};
    s8 *recv_buffer = NULL;
    s32 recv_buffer_len = 4096+512;
    s8 *pstr = NULL;
    s8 *pstr_tmp = NULL;
    s8 dst[2][SPLIT_PER_MAX];
    s8 dst_son[2][SPLIT_PER_MAX];
    s8 wifi_ssid[32];
    s8 wifi_password[32];
    u32 cur_ticks = trendit_get_ticks();
    wifi_module_t *wifi_module = wifi_instance();
    s8 tmp_buff[256] = {0};
    s32 i = 0;
    s32 led_flg = 0;
    s32 tmp_socket_id = 0;
    s32 len = 0;

    print_wifi_web_config_info();
    ddi_key_clear();
    ddi_led_sta_set(LED_STATUS_R, 0);
    do{
        if(!wifi_module)
        {
            TRACE_ERR("wifi_module is null");
            ret = DDI_EIO;
            break;
        }
        
        recv_buffer = (s8 *)k_mallocapp(recv_buffer_len);
        if(!recv_buffer)
        {
            TRACE_ERR("malloc failed");
            ret = DDI_EOVERFLOW;
            break;
        }
        
        ret = ddi_wifi_ioctl(WIFI_IOCTL_CREAT_TCP_SERVER, (u32)"Cloud-Printer", 80);
        if(ret != DDI_OK)
        {
            audio_instance()->audio_play(AUDIO_WIFI_CONFIG_ERROR, AUDIO_PLAY_BLOCK);
            TRACE_ERR("creat tcp server fail:%d", ret);
            break;
        }

        memset(client_socketid_tab, 0, sizeof(client_socketid_tab));
        while(1)
        {
            if(0 == led_flg) //配网闪蓝灯
            {
                ddi_led_sta_set(LED_STATUS_B, 1);
                led_flg = 1;
            }
            else
            {
                ddi_led_sta_set(LED_STATUS_B, 0);
                led_flg = 0;
            }
            
            if(trendit_get_key())
            {
                TRACE_DBG("web config out for key enter");
                audio_instance()->audio_play(AUDIO_WIFI_CONFIG_EXIT, AUDIO_PLAY_BLOCK);
                ret = DDI_ERR;
                break;
            }
            
            if(trendit_query_timer(cur_ticks, CFG_WIFI_OVERTIME))
            {
                TRACE_INFO("web config timeout");
                audio_instance()->audio_play(AUDIO_CONFIG_OVERTIME, AUDIO_PLAY_BLOCK);
                ret = DDI_ETIMEOUT;
                break;
            }
            
            switch(step)
            {
                case LISTEN_CLIENT:
                    tmp_socket_id = ddi_wifi_ioctl(WIFI_IOCTL_TCP_SERVER_ACCEPT, 0, 0);
                    if(tmp_socket_id >= 0)
                    {
                        TRACE_DBG("have new client %x enter", tmp_socket_id);
                        fill_socket_id(client_socketid_tab, tmp_socket_id);
                    }

                    for(i=0; i<sizeof(client_socketid_tab)/sizeof(client_socketid_tab[0]); i++)
                    {
                        ddi_watchdog_feed();
                        if(client_socketid_tab[i] == 0)
                        {
                            continue;
                        }

                        if(WIFI_SOCKET_STATUS_DISCONNECTED == ddi_wifi_socket_get_status(client_socketid_tab[i]))
                        {
                            TRACE_DBG("client %x close", client_socketid_tab[i]);
                            client_socketid_tab[i] = 0;
                            continue;
                        }
                        
                        memset(recv_buffer, 0, recv_buffer_len);
                        ret = ddi_wifi_socket_recv(client_socketid_tab[i], recv_buffer, recv_buffer_len);
                        if(ret > 0)
                        {
                            TRACE_DBG("recv:%s", recv_buffer);
                            if(strstr(recv_buffer, "GET / HTTP"))   //第一次请求
                            {
                                TRACE_INFO("post index page");
                                memset(recv_buffer, 0, recv_buffer_len);
                                len = get_index_page(recv_buffer, recv_buffer_len);
                                send_wifi_data(client_socketid_tab[i], recv_buffer, len);
                            }
                            else if(strstr(recv_buffer, "GET /favicon.ico HTTP"))   //第二次请求 log
                            {
                                memset(recv_buffer, 0, recv_buffer_len);
                                len = get_index_page(recv_buffer, recv_buffer_len);
                                send_wifi_data(client_socketid_tab[i], recv_buffer, len);
                            }
                            else if(strstr(recv_buffer, "GET /wifi_cfg.cgi?"))   //应答
                            {
                                TRACE_INFO("response result");
                                pstr = strstr(recv_buffer, "GET /wifi_cfg.cgi?");
                                pstr += strlen("GET /wifi_cfg.cgi?");
                                pstr_tmp = pstr;

                                //wifi_ssid=123&wifi_password=456
                                while(1)
                                {
                                    if(*pstr == ' ')
                                    {
                                        *pstr = 0;
                                        break;
                                    }

                                    pstr++;
                                }

                                memset(dst, 0, sizeof(dst));
                                ret = trendit_split(dst, sizeof(dst)/sizeof(dst[0]), pstr_tmp, "&", SPLIT_NOIGNOR);
                                if(sizeof(dst)/sizeof(dst[0]) == ret)
                                {
                                    memset(dst_son, 0, sizeof(dst_son));
                                    trendit_split(dst_son, sizeof(dst_son)/sizeof(dst_son[0]), dst[0], "=", SPLIT_NOIGNOR);
                                    trendit_trim(dst_son[1]);
                                    snprintf(wifi_ssid, sizeof(wifi_ssid), "%s", dst_son[1]);
                                    

                                    memset(dst_son, 0, sizeof(dst_son));
                                    trendit_split(dst_son, sizeof(dst_son)/sizeof(dst_son[0]), dst[1], "=", SPLIT_NOIGNOR);
                                    trendit_trim(dst_son[1]);
                                    snprintf(wifi_password, sizeof(wifi_password), "%s", dst_son[1]);

                                    TRACE_INFO("wifi_ssid:%s, password:%s", wifi_ssid, wifi_password);
                                    memset(tmp_buff, 0, sizeof(tmp_buff));
                                    snprintf(tmp_buff, sizeof(tmp_buff), WIFI_WEB_CONFIG_ANSWER_INFO, wifi_ssid, wifi_password);
                                    get_response_page(recv_buffer, recv_buffer_len, tmp_buff);
                                    send_wifi_data(client_socketid_tab[i], recv_buffer, strlen(recv_buffer));
                                    ddi_wifi_ioctl(WIFI_IOCTL_TCP_SERVER_CLOSE, 0, 0);    //恢复为模式1连接
                                    ddi_wifi_connectap_start(wifi_ssid, wifi_password, NULL);
                                    step = CHECK_CONNECT_AP;
                                    goto _next;
                                }
                            }
                            else
                            {
                                TRACE_INFO("post index page");
                                memset(recv_buffer, 0, recv_buffer_len);
                                len = get_index_page(recv_buffer, recv_buffer_len);
                                send_wifi_data(client_socketid_tab[i], recv_buffer, len);
                            }
                        }
                    }
_next:
                    ret = DDI_ERR;
                    break;

               case CHECK_CONNECT_AP:
                    ret = ddi_wifi_connectap_status();
                    if(WIFI_STATUS_CONNECT_AP_CONNECTED == ret)
                    {
                        TRACE_INFO("connect ap success");
                        audio_instance()->audio_play(AUDIO_ENTER_WIFI_CONFIG_OK, AUDIO_PLAY_BLOCK);
                        ret = DDI_OK;
                        goto _out;
                    }
                    else if(WIFI_STATUS_CONNECT_AP_DISCONNECTED == ret)
                    {
                        TRACE_INFO("connect ap failed");
                        audio_instance()->audio_play(AUDIO_WIFI_CONFIG_ERROR, AUDIO_PLAY_BLOCK);
                        ret = DDI_ERR;
                        goto _out;
                    }
                    break;
            }

            ddi_watchdog_feed();
            ddi_misc_msleep(500);
        }
    }while(0);

_out:
    if(recv_buffer)
    {
        k_freeapp(recv_buffer);
        recv_buffer = NULL;
    }
    
    if(error_model_instance()->m_error_queue->size > 0)
    {
        ddi_led_sta_set(LED_STATUS_B, 0);
        ddi_led_sta_set(LED_STATUS_R, 1);
    }
    else
    {
        ddi_led_sta_set(LED_STATUS_B, 1);
        ddi_led_sta_set(LED_STATUS_R, 0);
    }
    
    if(DDI_OK != ret && CHECK_CONNECT_AP == step)
    {
        ddi_wifi_disconnectap();
    }
    
    TRACE_DBG("out");
    if(CHECK_CONNECT_AP != step)
    {
        ddi_wifi_ioctl(WIFI_IOCTL_TCP_SERVER_CLOSE, 0, 0);
    }
    
    if(DDI_OK == ret)
    {
        wifi_module->cfg_wifi_ssid(wifi_ssid, wifi_password);
        network_strategy_instance()->restart();
    }
}

static void wifi_airkiss_config(void)
{
    s32 ret = 0, old_ret = 0;
    u8 ssid[32];
    u8 password[32];
    u32 get_airkiss_start_tick = 0;
    u32 led_flg = 0;

    print_wifi_airkiss_config_info();
    get_airkiss_start_tick = trendit_get_ticks();
    ret = ddi_wifi_start_airkiss();
    ddi_key_clear();
    ddi_led_sta_set(LED_STATUS_R, 0);
    if(DDI_OK == ret)
    {
        while(1)
        {
            if(trendit_query_timer(get_airkiss_start_tick, 1*1000))
            {
                if(trendit_get_key() > 0)
                {
                    audio_instance()->audio_play(AUDIO_WIFI_CONFIG_EXIT, AUDIO_PLAY_BLOCK);
                    TRACE_DBG("press key exit wifi config");
                    ddi_wifi_stop_airkiss();
                    ret = DDI_ERR;
                    break;
                }
            }
            
            if(0 == led_flg) //配网闪蓝灯
            {
                ddi_led_sta_set(LED_STATUS_B, 1);
                led_flg = 1;
            }
            else
            {
                ddi_led_sta_set(LED_STATUS_B, 0);
                led_flg = 0;
            }
        
            if(trendit_query_timer(get_airkiss_start_tick, CFG_WIFI_OVERTIME))
            {
                ddi_wifi_stop_airkiss();
                audio_instance()->audio_play(AUDIO_CONFIG_OVERTIME, AUDIO_PLAY_BLOCK);
                ret = DDI_ERR;
                break;
            }
            
            ret = ddi_wifi_get_airkiss_status();//dev_wifi_get_connectap_status(); //ddi_wifi_connectap_status
            //TRACE_DBG("ret:%d ", ret);
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
                audio_instance()->audio_play(AUDIO_ENTER_WIFI_CONFIG_OK, AUDIO_PLAY_BLOCK);
                trendit_trim_enter(ssid);
                trendit_trim_enter(password);
                TRACE_DBG("微信配网成功:%s,%s", ssid, password);
                ddi_wifi_stop_airkiss();
                ret = DDI_OK;
                break;
            }
            else if(WIFI_AIRKISS_CONNECT_ERR == ret)
            {
                audio_instance()->audio_play(AUDIO_WIFI_CONFIG_ERROR, AUDIO_PLAY_BLOCK);
                ddi_wifi_stop_airkiss();
                ret = DDI_ERR;
                break;
            }
            
            ddi_watchdog_feed();
            ddi_misc_msleep(500);
        }
    }

    if(error_model_instance()->m_error_queue->size > 0)
    {
        ddi_led_sta_set(LED_STATUS_B, 0);
        ddi_led_sta_set(LED_STATUS_R, 1);
    }
    else
    {
        ddi_led_sta_set(LED_STATUS_B, 1);
        ddi_led_sta_set(LED_STATUS_R, 0);
    }

    TRACE_DBG("ret:%d", ret);
    if(DDI_OK == ret)
    {
        wifi_instance()->cfg_wifi_ssid(ssid, password);
        network_strategy_instance()->restart();
    }
}

static s32 check_network_connect_success(void)
{
    wifi_module_t *wifi_module = wifi_instance();

    return (wifi_module->m_network_state == TRUE && WIFI_STATUS_CONNECT_AP_CONNECTED == ddi_wifi_connectap_status());
}

static u8 month_to_val(s8 *month_string)
{
    s8 month_info[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    u8 month_hex_value[12] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0x10, 0x11, 0x12};
    s32 i = 0;
    s32 ret = DDI_ERR;

    for(i=0; i<12; i++)
    {
        if(0 == memcmp(month_string, month_info[i], strlen(month_info[i])))
        {
            ret = month_hex_value[i];
            break;
        }
    }

    return ret;
}

static s32 format_time(s8 *time_info, u8 *out_time_buff, s32 out_time_buff_len)
{
    //Thu Feb 20 16:27:06 2020
    s32 ret = DDI_ERR;
    s8 dst[5][SPLIT_PER_MAX];
    s8 tmp_buff[64] = {0};

    do{
        if(NULL == time_info || NULL == out_time_buff || out_time_buff_len < 6)
        {
            ret = DDI_EINVAL;
            break;
        }
        
        memset(dst, 0, sizeof(dst));
        ret = trendit_split(dst, sizeof(dst)/sizeof(dst[0]), time_info, " ", SPLIT_NOIGNOR);
        if(ret != sizeof(dst)/sizeof(dst[0]))
        {
            ret = DDI_EINVAL;
            break;
        }

        if(trendit_atoi(dst[4]) < 2019)
        {
            TRACE_ERR("time is invalid:%s", time_info);
            ret = DDI_ERR;
            break;
        }

        ret = month_to_val(dst[1]);
        if(ret < 0)
        {
            TRACE_ERR("month is invalid:%s", time_info);
            ret = DDI_EINVAL;
            break;
        }
        
        out_time_buff[0] = trendit_asc_to_u32(dst[4]+2, strlen(dst[4]+2), MODULUS_HEX);
        out_time_buff[1] = ret;
        out_time_buff[2] = trendit_asc_to_u32(dst[2], strlen(dst[2]), MODULUS_HEX);

        memset(tmp_buff, 0, sizeof(tmp_buff));
        snprintf(tmp_buff, sizeof(tmp_buff), "%s", dst[3]);
        memset(dst, 0, sizeof(dst));
        ret = trendit_split(dst, sizeof(dst)/sizeof(dst[0]), tmp_buff, ":", SPLIT_NOIGNOR);
        if(ret != 3)
        {
            TRACE_ERR("time is invalid:%s,%d", tmp_buff, ret);
            ret = DDI_EINVAL;
            break;
        }

        out_time_buff[3] = trendit_asc_to_u32(dst[0], strlen(dst[0]), MODULUS_HEX);
        out_time_buff[4] = trendit_asc_to_u32(dst[1], strlen(dst[1]), MODULUS_HEX);
        out_time_buff[5] = trendit_asc_to_u32(dst[2], strlen(dst[2]), MODULUS_HEX);

        TRACE_INFO("%02x%02x%02x %02x:%02x:%02x", out_time_buff[0],
                                                  out_time_buff[1],
                                                  out_time_buff[2],
                                                  out_time_buff[3],
                                                  out_time_buff[4],
                                                  out_time_buff[5]);
        ret = DDI_OK;
    }while(0);

    return ret;
}
static s32 wifi_msg_handle(struct _msg *m_msg)
{
    s32 ret = MSG_KEEP;
    wifi_module_t *wifi_module = wifi_instance();
    s8 rtc[64];
    u8 sys_rtc[6];

    do{
        if(MSG_TERMINAL == m_msg->m_status)
        {
            ddi_wifi_disconnectap();
            ddi_wifi_close();
            ret = MSG_OUT;
            break;
        }
        else if(MSG_INIT == m_msg->m_status)
        {
            if(WIFI_CHECK_MODEL == m_msg->m_lParam)
            {
                ret = ddi_wifi_open();
                TRACE_DBG("wifi open:%d", ret);
                if(ret < 0)
                {
                    wifi_module->m_open_fail_try_times++;
                    if(-11 == ret)
                    {
                        TRACE_ERR("can't detect wifi model");
                        wifi_module->m_msg_status = MSG_TERMINAL;
                        ret = MSG_OUT;
                        break;
                    }
                    else
                    {
                        if(wifi_module->m_open_fail_try_times >= 3)
                        {
                            TRACE_ERR("open wifi failed:%d over time, exit", ret);
                            wifi_module->m_msg_status = MSG_TERMINAL;
                            ret = MSG_OUT;
                        }
                        else
                        {
                            TRACE_ERR("open wifi failed:%d, try %d times", ret, wifi_module->m_open_fail_try_times);
                            ret = MSG_KEEP;
                        }
                        break;
                    }
                }

                wifi_module->m_network_state = FALSE;
                wifi_module->super.m_self_check_res = TRUE;
                m_msg->m_lParam = WIFI_AP_NAME_CHECK;
                ret = MSG_KEEP;
                break;
            }
            else if(WIFI_AP_NAME_CHECK == m_msg->m_lParam)
            {
                if(!strlen(wifi_module->m_wifi_ssid))
                {
                    error_model_instance()->notify_error(ERR_WIFI_AP_DO_NOT_CONFIG);
                    ret = MSG_KEEP;
                    break;
                }
                
                ret = ddi_wifi_connectap_start(wifi_module->m_wifi_ssid, wifi_module->m_wifi_password, NULL);
                TRACE_INFO("begin to connect wifi:%s, %s", wifi_module->m_wifi_ssid, wifi_module->m_wifi_password);
                m_msg->m_status = MSG_SERVICE;
                m_msg->m_lParam = WIFI_CONNECT_AP;
                wifi_module->m_beg_con_ticks = trendit_get_ticks();
                wifi_module->m_thread_sleep_ticks = trendit_get_ticks();
                ret = MSG_KEEP;
            }
            break;
        }
        else
        {
            if(WIFI_CONNECT_AP == m_msg->m_lParam)
            {
                //休眠时间未到
                if(!trendit_query_timer(wifi_module->m_thread_sleep_ticks, WIFI_CHECK_NET_SLEEP_TIME))
                {
                    ret = MSG_KEEP;
                    break;
                }
                else
                {
                    wifi_module->m_thread_sleep_ticks = trendit_get_ticks();
                }
                
                ret = ddi_wifi_connectap_status();
                if(WIFI_STATUS_CONNECT_AP_CONNECTING == ret)
                {
                    if(trendit_query_timer(wifi_module->m_beg_con_ticks, WIFI_CONNECT_OVERTIME))
                    {
                        error_model_instance()->notify_error(ERR_WIFI_CONNECT_AP_FAIL);
                        TRACE_INFO("connect wifi overtime, then retry");
                        ddi_wifi_disconnectap();
                        ret = MSG_KEEP;
                        m_msg->m_status = MSG_INIT;
                        m_msg->m_lParam = WIFI_CHECK_MODEL;
                        break;
                    }
                    else
                    {
                        ret = MSG_KEEP;
                        break;
                    }
                }
                else if(WIFI_STATUS_CONNECT_AP_DISCONNECTED == ret)
                {
                    error_model_instance()->notify_error(ERR_WIFI_CONNECT_AP_FAIL);
                    TRACE_INFO("connect wifi fail, then retry");
                    ddi_wifi_disconnectap();
                    ddi_wifi_close();
                    ret = MSG_KEEP;
                    m_msg->m_status = MSG_INIT;
                    m_msg->m_lParam = WIFI_CHECK_MODEL;
                    break;
                }
                else if(WIFI_STATUS_CONNECT_AP_CONNECTED == ret)
                {
                    error_model_instance()->clear_error(ERR_WIFI_CONNECT_AP_FAIL);
                    TRACE_INFO("connect wifi success");
                    ret = MSG_KEEP;
                    m_msg->m_lParam = WIFI_CFG_NTP;
                    wifi_module->m_network_state = TRUE;
                    break;
                }
                ret = MSG_KEEP;
                break;
            }
            else if(WIFI_CFG_NTP == m_msg->m_lParam)
            {
                ret = ddi_wifi_ioctl(WIFI_IOCTL_CFG_NTP_SERVER, (u32)machine_instance()->m_ntp_info, sizeof(machine_instance()->m_ntp_info)/sizeof(machine_instance()->m_ntp_info[0]));
                if(ret == DDI_OK)
                {
                    m_msg->m_lParam = WIFI_GET_NTP;
                    wifi_module->m_thread_sleep_ticks = trendit_get_ticks();
                }
                ret = MSG_KEEP;
                break;
            }
            else if(WIFI_GET_NTP == m_msg->m_lParam)
            {
                if(WIFI_STATUS_CONNECT_AP_CONNECTED != ddi_wifi_connectap_status())
                {
                    TRACE_INFO("wifi disconnect, then retry");
                    error_model_instance()->notify_error(ERR_WIFI_CONNECT_AP_FAIL);
                    wifi_module->m_network_state = FALSE;
                    ddi_wifi_disconnectap();
                    ret = MSG_KEEP;
                    m_msg->m_status = MSG_INIT;
                    m_msg->m_lParam = WIFI_CHECK_MODEL;
                    break;
                }

                if(wireless_instance()->m_have_sync_time || wifi_module->m_have_sync_time)
                {
                    m_msg->m_lParam = WIFI_PING;
                    TRACE_INFO("wireless have sync success, then ignore");
                    ret = MSG_KEEP;
                    break;
                }

                //休眠时间未到
                if(!trendit_query_timer(wifi_module->m_thread_sleep_ticks, WIFI_CHECK_NET_SLEEP_TIME))
                {
                    ret = MSG_KEEP;
                    break;
                }
                else
                {
                    wifi_module->m_thread_sleep_ticks = trendit_get_ticks();
                }
                
                memset(rtc, 0, sizeof(rtc));
                ret = ddi_wifi_ioctl(WIFI_IOCTL_GET_NTP, (u32)rtc, sizeof(rtc));
                TRACE_DBG("rtc:%s", rtc);

                memset(sys_rtc, 0, sizeof(sys_rtc));
                if(format_time(rtc, sys_rtc, sizeof(sys_rtc)) == DDI_OK)
                {
                    wifi_instance()->m_have_sync_time = TRUE;
                    ddi_misc_set_time(sys_rtc);
                    m_msg->m_lParam = WIFI_PING;
                }
                else
                {
                    wifi_instance()->m_use_ntp_no_times++;
                    if(wifi_instance()->m_use_ntp_no_times > 10)
                    {
                        TRACE_ERR("get failed");
                        m_msg->m_lParam = WIFI_PING;
                        wifi_instance()->m_have_sync_time = TRUE;
                    }
                }
                ret = MSG_KEEP;
                break;
            }
            else
            {
                if(WIFI_STATUS_CONNECT_AP_CONNECTED != ddi_wifi_connectap_status())
                {
                    TRACE_INFO("wifi disconnect, then retry");
                    error_model_instance()->notify_error(ERR_WIFI_CONNECT_AP_FAIL);
                    wifi_module->m_network_state = FALSE;
                    ddi_wifi_disconnectap();
                    ret = MSG_KEEP;
                    m_msg->m_status = MSG_INIT;
                    m_msg->m_lParam = WIFI_CHECK_MODEL;
                    break;
                }
                
                //休眠时间未到
                if(!trendit_query_timer(wifi_module->m_thread_sleep_ticks, WIFI_PING_INTERVAL_SLEEP_TIME))
                {
                    ret = MSG_KEEP;
                    break;
                }
                else
                {
                    wifi_module->m_thread_sleep_ticks = trendit_get_ticks();
                }

                ret = MSG_KEEP;
                //Todo 发送ping包
            }
        }
    }while(0);

    return ret;
}

static s32 post_wifi_msg()
{
    msg_t *wifi_connect_msg = NULL;
    s32 ret = DDI_ERR;
    wifi_module_t *wifi_module = wifi_instance();

    wifi_connect_msg = (msg_t *)k_mallocapp(sizeof(msg_t));
    if(NULL == wifi_connect_msg)
    {
        TRACE_ERR("malloc failed");
    }
    else
    {
        TRACE_DBG("usbcdc:%x", wifi_connect_msg);
        memset(wifi_connect_msg, 0, sizeof(msg_t));
        wifi_module->m_msg = wifi_connect_msg;
        wifi_connect_msg->m_func = wifi_msg_handle;
        wifi_connect_msg->m_priority = MSG_NORMAL;
        wifi_connect_msg->m_lParam = 0;
        wifi_connect_msg->m_wparam = 0;
        wifi_connect_msg->m_status = MSG_INIT;
        wifi_module->m_msg_status = MSG_SERVICE;
        snprintf(wifi_connect_msg->m_msgname, sizeof(wifi_connect_msg->m_msgname), "%s", "wifi");
        trendit_postmsg(wifi_connect_msg);
    }

    return ret;
}

static void terminal_wifi_msg()
{
    wifi_module_t *wifi_module = wifi_instance();

    ddi_wifi_disconnectap();
    if(wifi_module->m_msg)
    {
        wifi_module->m_msg->m_status = MSG_TERMINAL;
        wifi_module->m_msg = NULL;
    }
}


/**
 * @brief 查询wifi ssid 信号量 连接状态
 * @param[in] ret_string -返回字符串，ret_stringlen ret-长度
 * @retval  DDI_OK 是
 * @retval  DDI_ERR 是
 */
static s32 cmd_query_wifi_info(u8 *ret_string, u16 ret_stringlen)
{
    u8 ssid[128];
    s32 ret;
    s32 connect_status = 1;
    u32 ap_num = 0;
    
    ddi_wifi_ioctl(WIFI_IOCTL_GET_SSID, (u32)ssid, 0);
    ret = ddi_wifi_get_signal(&ap_num);
    if(ret<=0)
    {
        ap_num = 0;
    }
    ret = ddi_wifi_connectap_status();
    if((ret == WIFI_STATUS_CONNECT_AP_CONNECTED) || (ret == WIFI_STATUS_CONNECT_AP_CONNECTING))
    {
        connect_status = ret;
    }
    else
    {
        connect_status = WIFI_STATUS_CONNECT_AP_DISCONNECTED;
        
    }
    TRACE_DBG("SSID:%s,ap_num:%d,connect_status:%d",ssid,ap_num,connect_status);

    snprintf(ret_string, ret_stringlen, "%s,%d,%d", GET_STR(ssid),ap_num,connect_status);
    return DDI_OK;


}

/**
 * @brief 扫描wifi热点信息
 * @param[in] value_string- 入参值  ret_string -返回字符串，ret_stringlen ret-长度
 * @retval  DDI_OK 是
 * @retval  DDI_ERR 是
 */
static s32 cmd_comand_wifi_scanap(u8 *value_string, u8 *ret_string, u16 ret_stringlen)
{
    s32 err = DDI_EIO;
    s32 i = 0;
    u32 ap_num = 0;
    wifi_apinfo_t aps[20];
    u8 ssid[64] = 0;
    u32 start_scan_time = 0;


    ap_num = sizeof(aps)/sizeof(aps[0]);

    start_scan_time = trendit_get_ticks();
    err = ddi_wifi_scanap_start(aps, ap_num);//dev_wifi_scanap_start(aps, ap_num);
    if(err == 0)
    {
        while(1)
        {
            err = ddi_wifi_scanap_status(&ap_num);//dev_wifi_scanap_status(&ap_num);
            if(err == 0)
            {
                //正在搜索
                if(trendit_get_ticks() - start_scan_time >= WIFI_SCANAP_OVERTIME)
                {
                    TRACE_INFO("scan wifi overtime, then retry");
                    ddi_wifi_disconnectap();
                    return DDI_ETIMEOUT;
                }
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

            ddi_watchdog_feed();
        }
    }
    if(err != 0)
    {
        TRACE_DBG("搜网失败\r\n");
        return DDI_EIO;
    }
    for(i = 0; i < ap_num; i++)
    {
        //TRACE_DBG("ssid:%s",aps[i].m_ssid);
        strcat(ret_string,aps[i].m_ssid);
        strcat(ret_string,",");
        sprintf(ssid, "%d", strlen(aps[i].m_ssid));
        strcat(ret_string,ssid);
        if(i < ap_num-1)
        {
            strcat(ret_string,",");
        }
        
    }
    TRACE_DBG("all ssid:%s",ret_string);
    //snprintf(ret_string, ret_stringlen, "%s,%d,%d", ssid,ap_num,connect_status);
    return DDI_OK;

}

/**
 * @brief 连接ssid
 * @param[in] value_string- 入参值 ret_string -返回字符串，ret_stringlen ret-长度
 * @retval  DDI_OK 是
 * @retval  DDI_ERR 是
 */
static s32 cmd_comand_wifi_connectap(u8 *value_string, u8 *ret_string, u16 ret_stringlen)
{

    s32 err = DDI_EIO;
    u8 ssid[64] = 0;
    u8 bssid[64] = 0;
    u8 passwd[128] = 0;
    s8 dst[10][SPLIT_PER_MAX] = {0};

    trendit_split(dst, 10, value_string, ",", SPLIT_NOIGNOR);
    TRACE_DBG("connectap:%s,%s,%s",dst[0],dst[1],dst[2]);
    err = ddi_wifi_open();
    err = ddi_wifi_connectap_start(dst[0], dst[2], NULL);

    wifi_instance()->cfg_wifi_ssid(dst[0], dst[1]);
    //err = ddi_wifi_connectap_start("Trendit", "trendit123", NULL);
    return err;

    //return ddi_wifi_connectap_start(WIFI_SSID,WIFI_PWD,NULL);

}

/**
 * @brief 断开连接
 * @param[in] value_string- 入参值  ret_string -返回字符串，ret_stringlen ret-长度
 * @retval  DDI_OK 是
 * @retval  DDI_ERR 是
 */
static s32 cmd_comand_wifi_disconnectap(u8 *value_string, u8 *ret_string, u16 ret_stringlen)
{

    s32 err = DDI_EIO;
    err = ddi_wifi_disconnectap();
    err = ddi_wifi_close();
    //TRACE_DBG("disconnectap:%d",err);
    return err;
}

static s32 exec_cfg(u16 cmd, u8 *value_string, u16 value_stringlen)
{
    s32 ret = DDI_ERR;
    
    TRACE_DBG("cmd:%04x, value_string:%s", cmd, value_string);

    return ret;
}

static s32 exec_qry(u16 cmd, u8 *ret_string, u16 ret_stringlen)
{
    s32 ret = DDI_ERR;
    
    TRACE_DBG("cmd:%04x", cmd);
    switch(cmd)
    {
        case CMD_QRY_WIFI_INFO:
            ret = cmd_query_wifi_info(ret_string, ret_stringlen);
            break;

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
        case CMD_COMAND_WIFI_SCANAP:
            ret= cmd_comand_wifi_scanap(value_string, ret_string, ret_stringlen);
            break;
        case CMD_COMAND_WIFI_CONNECTAP:
            ret = cmd_comand_wifi_connectap(value_string, ret_string, ret_stringlen);
            break;
        case CMD_COMAND_WIFI_DISCONNECTAP:
            ret = cmd_comand_wifi_disconnectap(value_string, ret_string, ret_stringlen);
            break;

    }
    return ret;
}

static void cfg_wifi_ssid(s8 *ssid, s8 *pwd)
{
    system_data_instance_t *system_data = system_data_instance();
    
    snprintf(g_model_wifi->m_wifi_ssid, sizeof(g_model_wifi->m_wifi_ssid), "%s", ssid);
    snprintf(g_model_wifi->m_wifi_password, sizeof(g_model_wifi->m_wifi_password), "%s", pwd);

    system_data->set_wifi_data(g_model_wifi->m_wifi_ssid, g_model_wifi->m_wifi_password);
}

/**
 * @brief wifi的初始化函数
 * @retval  wifi的设备结构体
 */
wifi_module_t *wifi_instance(void)
{
    system_data_instance_t *system_data = system_data_instance();
    
    if(g_model_wifi)
    {
        return g_model_wifi;
    }

    g_model_wifi = k_mallocapp(sizeof(wifi_module_t));
    if(NULL == g_model_wifi)
    {
        TRACE_ERR("malloc fail");
        return NULL;
    }
    
    memset(g_model_wifi, 0, sizeof(wifi_module_t));
    INIT_SUPER_METHOD(g_model_wifi, MODEL_WIFI, "wifi");
    g_model_wifi->check_network_connect_success = check_network_connect_success;
    g_model_wifi->post_wifi_msg = post_wifi_msg;
    g_model_wifi->terminal_wifi_msg = terminal_wifi_msg;
    g_model_wifi->cfg_wifi_ssid = cfg_wifi_ssid;
    g_model_wifi->airkiss_config = wifi_airkiss_config;
    g_model_wifi->web_config = wifi_web_config;
    g_model_wifi->m_network_state = FALSE;

    g_model_wifi->super.load_cfg(&(g_model_wifi->super), WIFI_CONFIG_FILE, exec_cfg);

    system_data->get_wifi_data(g_model_wifi->m_wifi_ssid, sizeof(g_model_wifi->m_wifi_ssid), g_model_wifi->m_wifi_password, sizeof(g_model_wifi->m_wifi_password));
    
    return g_model_wifi;
}

