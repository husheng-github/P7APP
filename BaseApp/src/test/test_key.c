#include "ddiglobal.h"
#include "app_global.h"
#include "test_global.h"



s32 test_key(void)
{
    
    u8 flg;
	u32 key;
    u8 com_key = 0;

    u8 key_code;
    u8 key_event;

    flg = 1;

	TRACE_DBG("dev_key_test\r\n"); 

    while(1)
    {
        if(flg == 1)
        {
            flg = 0;
            TRACE_DBG("================KEY Test ==============");

        }

        ddi_watchdog_feed();
        if(ddi_com_read(PORT_CDCD_NO, (u8 *)&com_key, 1) > 0)
        {
            TRACE_DBG("key:%x\r\n",key);
            switch(com_key)
            {
                case 'e':
                    return 0;
            }
        }

        //if(keypad_interface_read_beep(&key)>0)
        if(ddi_key_read(&key) > 0)
        {
            key_code = key&0xff;
            key_event = (key&0xff00)>>8;
            //TRACE_DBG("key_code:%x,key_event:%x\r\n",key_code,key_event);
            switch(key_code)
            {
                case KEY_MINUS:
                    TRACE_DBG("KEY -\r\n");
                    break;
                case KEY_PLUS:
                    TRACE_DBG("KEY +\r\n");
                    break;
                case KEY_CFG:

                    TRACE_DBG("KEY CFG\r\n");
                    break;
                case KEY_PAPER:

                    TRACE_DBG("KEY PAPER\r\n");
                    break;
                case KEY_TBD:

                    TRACE_DBG("KEY TBD\r\n");
                    break;
                case POWER:

                    TRACE_DBG("KEY POWER\r\n");
                    break;
                case KEY_CFG_PLUS:
                    TRACE_DBG("KEY_CFG KEY_+\r\n");
                    break;
                case KEY_CFG_MINUS:
                    TRACE_DBG("KEY_CFG KEY_-\r\n");
                    break;
                case KEY_CFG_PAPER:

                    TRACE_DBG("KEY_CFG KEY_PAPER\r\n");
                    break;
                case KEY_PAPER_PLUS:

                    TRACE_DBG("KEY_PAPER KEY_+\r\n");
                    break;
                case KEY_PAPER_MINUS:

                    TRACE_DBG("KEY_PAPER KEY_-\r\n");
                    break;
                case KEY_PLUS_MINUS:

                    TRACE_DBG("KEY_+ KEY_-\r\n");
                    break;
            }
            switch(key_event)
            {
                case KEY_EVENT_DOWN:

                    TRACE_DBG("KEY_DOWN\r\n");
                    break;
                case KEY_EVENT_UP:

                    TRACE_DBG("KEY_UP\r\n");
                    break;
                case KEY_LONG_PRESS:

                    TRACE_DBG("KEY_LONG\r\n");
                    break;
            }
        }
    }

    return 0;
}

s32 auto_key_test(void)
{
    u8 flg = 0;
    u8 key;
    u8 key_tested[3] = {0};
    s32 have_tested_num = 0;
    s32 ret = DDI_ERR;

    trendit_factory_test_display_msg(AUDIO_TEST_KEY_ENTER, AUDIO_PLAY_BLOCK);
    memset(key_tested, 0, sizeof(key_tested));
    ddi_key_clear();
    while(1)
    {
        if(!flg && MAX_TEST_KEY == have_tested_num)
        {
            flg = 1;
            trendit_factory_test_display_msg(AUDIO_TEST_KEY_FINISH, AUDIO_PLAY_NOBLOCK);
            ddi_key_clear();
        }

        key = trendit_get_key();
        if(key > 0)
        {
            switch(key)
            {
                case KEY_MINUS:
                    TRACE_DBG("KEY -\r\n");
                    if(!key_tested[0])
                    {
                        key_tested[0] = 1;
                        have_tested_num++;
                        trendit_factory_test_display_msg(AUDIO_SUB, AUDIO_PLAY_BLOCK);
                    }

                    if(MAX_TEST_KEY == have_tested_num && flg)
                    {
                        ret = DDI_ERR;
                        goto _out;
                    }
                    break;
                case KEY_PLUS:
                    TRACE_DBG("KEY +\r\n");
                    if(!key_tested[1])
                    {
                        key_tested[1] = 1;
                        have_tested_num++;
                        trendit_factory_test_display_msg(AUDIO_PLUS, AUDIO_PLAY_BLOCK);
                    }

                    if(MAX_TEST_KEY == have_tested_num && flg)
                    {
                        ret = DDI_ERR;
                        goto _out;
                    }
                    break;
                    
                case KEY_PAPER:
                    TRACE_DBG("KEY PAPER\r\n");
                    if(!key_tested[2])
                    {
                        key_tested[2] = 1;
                        have_tested_num++;
                        trendit_factory_test_display_msg(AUDIO_TEST_KEY_PAPER, AUDIO_PLAY_BLOCK);
                    }

                    if(MAX_TEST_KEY == have_tested_num && flg)
                    {
                        ret = DDI_ERR;
                        goto _out;
                    }
                    break;
                
                case KEY_CFG:
                    TRACE_DBG("KEY CFG\r\n");
                    if(MAX_TEST_KEY == have_tested_num)
                    {
                        ret = DDI_OK;
                        goto _out;
                    }
                    break;

                default:
                    if(MAX_TEST_KEY == have_tested_num && flg)
                    {
                        ret = DDI_ERR;
                        goto _out;
                    }
                    break;
            }
        }
    }

_out:

    return ret;
}


