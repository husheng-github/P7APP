#include "ddiglobal.h"
#include "app_global.h"

/**
 * @brief http更新文件的回调函数
 * @param[in] callback_info 包含后台下发的包序号、命令字
 * @param[in] upgrade_step 步骤：下载或更新
 * @param[in] result 对应的结果
 * @retval  DDI_OK  成功
 * @retval  DDI_EINVAL  参数错误
 */
s32 upgrade_result_callback(upgrade_callback_info_t *callback_info, UPGRADE_STEP_E upgrade_step, s32 result)
{
    s32 len = 0;
    s32 ret = DDI_ERR;
    u8 ret_buff[RETURN_CONTENT_MAXSIZE] = {0};          //指令返回内容
    u8 ret_content_buff[RETURN_CONTENT_MAXSIZE/2] = {0};
    u16 ret_content_len = 0;
    s8 sn[64] = {0};

    do{
        if(!callback_info)
        {
            TRACE_ERR("arg is NULL");
            ret = DDI_EINVAL;
            break;
        }

        memset(ret_content_buff, 0, sizeof(ret_content_buff));
        ret_content_buff[ret_content_len++] = trendit_get_machine_code();
        
        memset(sn, 0, sizeof(sn));
        ddi_misc_read_dsn(SNTYPE_POSSN, sn);
        ret_content_buff[ret_content_len++] = sn[0];
        snprintf(ret_content_buff+ret_content_len, sizeof(ret_content_buff), "%s", sn+1);
        ret_content_len += strlen(sn+1);

        ret_content_buff[ret_content_len++] = -result;
        TRACE_DBG("cmd:%04x", callback_info->m_backend_cmd);
        if(callback_info->m_backend_cmd & 0xff00 != 0x1000)   //Add by xiaohonghui 2020.8.3 开机tms升级时处理
        {
            trendit_pack_u16(ret_content_buff+ret_content_len, 0x10C0);
        }
        else
        {
            trendit_pack_u16(ret_content_buff+ret_content_len, callback_info->m_backend_cmd);
        }
        ret_content_len += 2;
        trendit_pack_u16(ret_content_buff+ret_content_len, 1);  //len
        ret_content_len += 2;
        ret_content_buff[ret_content_len++] = upgrade_step+'0';
    
        memset(ret_buff, 0, sizeof(ret_buff));
        len = trendit_pack_answer_data(ret_buff, ret_content_buff, ret_content_len, callback_info->m_packno, CMD_TYPE_EVENT);
        ret = DDI_OK;
    }while(0);

    TRACE_INFO("upgrade step:%d, result:%d", upgrade_step, result);
    tcp_protocol_instance()->send(ret_buff, len);

    return ret;
}

