#include "ddiglobal.h"
#include "app_global.h"
#include "test_global.h"

s32 test_printer(void)
{
    unsigned char buf[] = "大趋智能打印机测试，qwertyuiopASDFGHJKL,1234567890wewewewewwrfedvvxcvcvcvxxxxxxxxxxxxxxxxxxxxxx    eeeeeeeeEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE";
    unsigned char buf1[] = {0x1B, 0x01};
    unsigned char buf2[] = {0x1B, 0x05};
    s32 ret;
    str_audio_tts audio_tts;
    u32 print_len;

    TRACE_DBG("enter");

    printer_instance()->direct_print(buf, strlen(buf));
    
    return 0;
}

s32 test_print_block(void)
{
    u32 print_len;

    ddi_thmprn_esc_init();
    ddi_thmprn_open();

    ddi_thmprn_ioctl(DDI_THMPRN_CTL_BLOCK_TEST, 0, 0);
    ddi_thmprn_ioctl(DDI_THMPRN_GET_PRINT_LEN, (u32)&print_len, 0);
    printer_instance()->m_print_len += print_len;
    ddi_thmprn_close();
    
}

s32 auto_printer_test(void)
{
    unsigned char buf[] = "国国国国国国国国国国国国国国国国\r\n国国国国国国国国国国国国国国国国\r\n\r\n";

    return printer_instance()->direct_print(buf, strlen(buf));
}
s32 test_printer_twoline(TRENDITDEV_TEST_INFO *devtestinfo,u8 testmode)
{
    //unsigned char buf[] = "国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国国";
    unsigned char buf[] = "国国国国国国国国国国国国国国国国\r\n国国国国国国国国国国国国国国国国\r\n\r\n\r\n";
    s32 ret;
    u32 print_len;

    ret = printer_instance()->direct_print(buf, strlen(buf));

    return ret;
}
