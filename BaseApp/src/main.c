#include "ddiglobal.h"
#include "app_global.h"

struct  __core *core;
const PCI_CORE_VERINFO pciappver __attribute__((at(APP_PIT_START+MH1902_BASE_ADDR))) = { APP_VER };

/**
 * @brief 初始化全局变量，按文件来，每个文件最上面是对应全局变量初始化的函数
 */
static void init_data(void)
{
    trendit_cdc_output_switch(FALSE);
    trendit_init_msg_data();
    trendit_init_parsedata();

//    trendit_init_device_data();
    trendit_init_wireless_data();
    trendit_init_wifi_data();
    trendit_init_printer_data();
    trendit_init_audio_data();
    trendit_init_key_data();
    trendit_init_machine_data();
    trendit_init_usbcdc_data();
    trendit_init_tcpdata();
    trendit_init_network_strategy_data();
    trendit_init_event_data();
    trendit_init_errordata();
    trendit_init_system_data();
	trendit_init_tms_data();
    trendit_init_parsedata();
    trendit_init_tms_schedule();
    trendit_init_printport_data();
    trendit_init_trendit_bt_cmd_data();
    ddi_watchdog_open();
}

/**
 * @brief 初始化函数，负责初始化全局变量和注册每个设备
 * @retval  DDI_OK  成功
 * @retval  DDI_ERR 失败
 */
static s32 init()
{
    s32 ret = DDI_ERR;
    
    core = ( struct __core *)(CORE_DDI_ADDR + MH1902_BASE_ADDR);

    init_data();
    trendit_init_msg();
    error_model_instance();
    system_data_instance();
    trendit_reg_dev();

    return ret;
}

/**
 * @brief 处理消息节点，放在main函数最后
 */
static void handle_message()
{
    msg_t *m_msg = NULL;
    u32 begin_ticks = 0;
    s32 ret = 0;
    
    while(1)
    {
        if(DDI_OK == trendit_querymsg(&m_msg))
        {
            //TRACE_DBG("m_msgname:%s", m_msg->m_msgname);
            begin_ticks = trendit_get_ticks();
            ret = m_msg->m_func(m_msg);
            if(trendit_query_timer(begin_ticks, 7*1000))
            {
                TRACE_INFO("m_msgname use over 7s:%s %d", m_msg->m_msgname, trendit_get_ticks()-begin_ticks);
            }
            
            if(ret == MSG_KEEP)
            {
                trendit_postmsg(m_msg);
            }
            else
            {
                k_freeapp(m_msg);
                m_msg = NULL;
            }
        }

        ddi_watchdog_feed();
        //ddi_misc_msleep(100);
    }
}

int main(void)
{
    init();
   
    processtask_device_test();
    handle_message();
    
    return 0;
}

const APP_FIRMWARE_API appcore __attribute__((at(APP_CORE_START+MH1902_BASE_ADDR))) = { main };

