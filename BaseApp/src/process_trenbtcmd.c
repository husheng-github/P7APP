#include "ddiglobal.h"
#include "app_global.h"

#define UARTDOWNLOAD_PORTNUM    PORT_CDCD_NO

void trendit_init_trendit_bt_cmd_data(void)
{

}

void process_trenbtcmd_senddata(trenbtframe_t *lp_trenbtframe, u8 *respdata, u16 respdatalen)
{
//    u8 *senddatabuf;
//    u16 maxdatalen;
    u8 sendhead[16];
    u8 sendheadcnt;
    u8 sendtail[2];
    u32 i;
    u8 lrc;

    //处理帧头
    sendhead[0] = COMMPROTOCOL_STX1;
    sendhead[1] = COMMPROTOCOL_STX2;
    //send data length
    sendhead[2] = (u8)((respdatalen+6)>>8);
    sendhead[3] = (u8)((respdatalen+6));
    sendheadcnt = 4;
    //message type
    sendhead[sendheadcnt++] = lp_trenbtframe->m_type;
    //command
    sendhead[sendheadcnt++] = (u8)((lp_trenbtframe->m_cmd)>>8);
    sendhead[sendheadcnt++] = (u8)((lp_trenbtframe->m_cmd));
    //serial no.
    sendhead[sendheadcnt++] = lp_trenbtframe->m_sn;
    //response code
    sendhead[sendheadcnt++] = (u8)((lp_trenbtframe->m_status)>>8);
    sendhead[sendheadcnt++] = (u8)((lp_trenbtframe->m_status));
    
    //处理帧尾
    //EXT(0x03)
    sendtail[0] = COMMPROTOCOL_EXT;
    //LRC
    lrc = trendit_maths_getlrc(0, &sendhead[2], sendheadcnt-2);
    if(respdatalen)
    {
        lrc = trendit_maths_getlrc(lrc, respdata, respdatalen);
    }
    lrc = trendit_maths_getlrc(lrc, sendtail, 1);
    sendtail[1] = lrc;

    //发送
    ddi_com_write(UARTDOWNLOAD_PORTNUM, sendhead, sendheadcnt);
    if(respdatalen)
    {
        ddi_com_write(UARTDOWNLOAD_PORTNUM, respdata, respdatalen);
    }
    ddi_com_write(UARTDOWNLOAD_PORTNUM, sendtail, 2);
  #if 0  
    TRENCMD_DEBUG("send=(%d,%d,%d)", sendheadcnt, respdatalen, 2);
    TRENCMD_DEBUGHEX(sendhead, sendheadcnt);
    if(respdatalen)
    {
        TRENCMD_DEBUGHEX(respdata, respdatalen);
    }
    TRENCMD_DEBUGHEX(sendtail, 2);
  #endif
}

void process_trenbtcmd_sendack(trenbtframe_t *lp_trenbtframe, u32 status)
{
    if(PARSEPROTOCOL_STEP_RECEIVEDATASUC == status)
    {
        lp_trenbtframe->m_status = RESPONSECODE_SUCCESS;
    }
    else
    {
        lp_trenbtframe->m_status = RESPONSECODE_LRCERR;
    }
    lp_trenbtframe->m_type = FRAME_TYPE_TRENBT_ACK;
    process_trenbtcmd_senddata(lp_trenbtframe, NULL, 0);
}
//--------------------------------------------------------------------
static void put_data_to_respdata(u8* respdata,s32* offset_resp,u8* data,s32 with_length)
{
	s32 datalen;
    
    datalen = strlen(data);
	if(with_length){
      #if 0  
		memcpy(&respdata[*offset_resp],&datalen,sizeof(datalen));
		*offset_resp +=1;
      #endif
        respdata[*offset_resp] = datalen;
		*offset_resp +=1;
	}
	memcpy(&respdata[*offset_resp],data,datalen);
	//Trace("debug","datalen=%data=%soffset_resp=%d",datalen,data,*offset_resp);
	*offset_resp += datalen; 
}
//------------------------------------------------------------------
const unsigned char rsa_preventpk[292] =
{
0x00,0x08,0x00,0x00,0xA3,0x5E,0xCD,0xA3,
0x6B,0x0D,0xBA,0x32,0x2A,0x3B,0xFD,0xE1,
0xB3,0x08,0xD8,0x32,0xAD,0x0B,0x11,0xE4,
0x91,0x12,0x52,0x3C,0x3D,0x14,0x9A,0xBA,
0xDA,0xCD,0x6A,0x3D,0xBB,0xAC,0xB8,0x33,
0x27,0x4B,0x3B,0x2A,0xC9,0x87,0x78,0x98,
0xB4,0xFC,0xD4,0x93,0xD1,0x00,0x8E,0x29,
0xEA,0xA2,0x1F,0xBC,0xED,0xC9,0x2B,0x62,
0x93,0xAE,0x08,0x4D,0x54,0x9A,0xD7,0xE3,
0x44,0x84,0xCC,0x79,0x5F,0x53,0xC4,0xAD,
0xED,0xE6,0x48,0x1E,0x98,0x22,0xF8,0x7E,
0x17,0x5C,0xBC,0xFF,0xF5,0x4E,0x87,0x7C,
0x37,0x86,0x78,0xE6,0xF1,0xC6,0x52,0x1C,
0x4A,0xE0,0x56,0x16,0x17,0x69,0x05,0x87,
0x77,0xF5,0x6D,0xA6,0x50,0xD8,0x27,0x24,
0xA1,0x66,0xA9,0x9B,0xFF,0x46,0x8A,0xF9,
0x90,0xD1,0xC1,0x0A,0xC3,0xE5,0x9F,0x56,
0x3B,0xC9,0x4C,0x99,0xD9,0xC1,0x36,0x1B,
0x1F,0xA9,0x27,0x7C,0x2C,0x65,0x35,0x94,
0x54,0x4F,0x27,0x9F,0x6F,0x70,0xBF,0x56,
0x66,0x8D,0xBC,0x21,0xF6,0x42,0x6A,0x9A,
0xAC,0x54,0x43,0x95,0x58,0xA7,0xB5,0x9A,
0xF3,0xD2,0xBE,0x0A,0x8D,0xA4,0xC8,0x92,
0xAB,0xC6,0x65,0xCE,0xE9,0x54,0xE2,0x5F,
0x7D,0x7F,0x69,0x07,0x36,0x9E,0x7E,0x16,
0xCB,0xB7,0x0C,0xC8,0xE3,0x91,0x80,0xAC,
0x3A,0x56,0xAD,0x6E,0xCC,0xA8,0xA9,0x4A,
0x50,0x6F,0xA0,0xC2,0x8D,0x6F,0x7F,0x97,
0xB0,0xC0,0xB7,0x8A,0x1A,0xAA,0x1F,0x5F,
0xB7,0x75,0x00,0x73,0xDE,0xED,0x29,0x40,
0xBA,0x0A,0x04,0x82,0x82,0x17,0xCB,0x9A,
0xC6,0x1E,0x4D,0xD9,0xCE,0xA9,0x06,0x46,
0x76,0x2B,0x6A,0x6D,0xD1,0xA9,0x61,0x4C,
0x74,0x66,0xC9,0xC3,0x36,0xD9,0x4C,0x72,
0x7F,0x0F,0xF5,0x47,0x4D,0x33,0x35,0x34,
0x2B,0xF8,0x0F,0xF7,0xDC,0xB4,0x7B,0x8B,
0xDC,0xBB,0x9F,0x35
};
//------------------------------------------------------------------
#if 0
typedef struct _strSECURITYINFO
{
    u8 m_flg;
    u8 m_opt;
    u8 m_randlen;
    u8 m_rand[16];
}strsecurityinfo_t;
//static strsecurityinfo_t g_strsecurityinfo;
#endif
s32 processcmd_trendit_requestsecureinfo(u8 *recdata,u16 recdatalen,u8 *respdata,u16 *respdatalen)
{
    s32 ret;
    u8 opstype;
    s32 offset;
    u8 tmp[64];
    u16 len;

    if(recdatalen < 1)
    {
        return RESPONSECODE_PARAMERR;
    }
    if((recdata[0] < 0x30)||(recdata[0]>0x35))
    {
        return RESPONSECODE_PARAMERR;
    }
    //opt(1B)
    respdata[0] = recdata[0];
//    g_strsecurityinfo.m_opt = recdata[0];
    //触发计数(4B)
    memset(&respdata[1], 0x0, 4);
    offset = 1+4;
    //SN号(32B)
    memset(&respdata[offset], 0, 32);
    ret = ddi_misc_read_dsn(SNTYPE_POSSN, tmp);
    if(ret == 0)
    {
        len = tmp[0];
        if(len>=32)
        {
            len = 31;
        }
        memcpy(&respdata[offset], &tmp[1], len);
    }
    offset += 32;
    //SP CPU ID(32B)
    memset(&respdata[offset], 0, 32);
    ret = ddi_misc_read_dsn(SNTYPE_APCUPID, tmp);
    if(ret == 0)
    {
        len = tmp[0];
        if(len>=32)
        {
            len = 31;
        }
        memcpy(&respdata[offset], &tmp[1], len);
    }
    offset += 32;
    //随机数(4B)
    ret = ddi_arith_getrand(tmp, 4);
//    g_strsecurityinfo.m_randlen = 4;
//    memcpy(g_strsecurityinfo.m_rand, tmp, 4);
    memcpy(&respdata[offset], tmp, 4);
//memset(&respdata[offset], 0x30, 4);
    offset += 4;
    //prevent公钥编号
    respdata[offset++] = 1;
//    g_strsecurityinfo.m_flg = 1;
    
    ret = ddi_misc_set_mcuparam(MACHINE_PARAM_SECUREAUTHENINFO, respdata,offset, 0);
    *respdatalen = offset;
    return RESPONSECODE_SUCCESS;
}
//------------------------------------------------------------------
s32 processcmd_trendit_setsecureinfo(u8 *recdata,u16 recdatalen,u8 *respdata,u16 *respdatalen)
{
    s32 ret;
    u8 output[300];
    u8 readdata[100];
    u32 outputlen;
    s32 responsecode = RESPONSECODE_DECRYPTERR;
    u16 tmpdata[16];
    u8 *tmpparamdata;
    TRENDITDEV_TEST_INFO devtestinfo;

    *respdatalen = 0;
    if(recdatalen < 256)
    {
        return RESPONSECODE_PARAMERR;
    }

    memset(readdata, 0xFF, sizeof(readdata));
    ret = ddi_misc_get_mcuparam(MACHINE_PARAM_SECUREAUTHENINFO, readdata, 74, 0);
    memset(output, 0xff, sizeof(output));
    if(memcmp(output, readdata, 74) == 0)//没有生成随机信息，直接返回
    {
        TRENCMD_DEBUG("ERR");
        return RESPONSECODE_OTHERERR;
    }
    
    //RSA解出数据
    ret = ddi_arith_rsadecrypt(output, &outputlen, recdata, 256, (u8*)rsa_preventpk, 292);
    if(ret == 0&&outputlen == 74)
    {
        if(memcmp(readdata, output, 74) == 0)
        {
            responsecode = RESPONSECODE_SUCCESS;
            recdata[0] = output[0];
            if((output[0] == 0x32)||(output[0] == 0x33))//防切机初始化  SMT初始化
            {
                ddi_innerkey_ioctl(INNERKEY_IOCTL_INITIAL, 0, 0);
                //还原到初始化状态，不做触发处理
                //pcitamper_manage_dryiceopen(0);

                //删除SN号
                responsecode = RESPONSECODE_OTHERERR;
                ret = ddi_misc_ioctl(MISC_IOCTL_CLEARHWSN, 0, 0);
                if(ret == 0)
                {
                  #if 0
                    //删除防切机绑定信息
                    tempparamdata = (u8 *)k_mallocapp(MCU_PREVENTINFO_MAX);
                    memset(tempparamdata,0xff,MCU_PREVENTINFO_MAX);
                    ret = dev_misc_machineparam_del(MCU_PREVENTINFO,tempparamdata, MCU_PREVENTINFO_MAX);
                    k_freeapp(tempparamdata);
                    if(ret == 0)
                  #endif
                    {
                      #if 0
                        //删除acquirerPK,重新下载acquirerpk
                        tempparamdata = (u8 *)k_mallocapp(ACQUIRERPKDATA_MAX);
                        memset(tempparamdata,0xff,ACQUIRERPKDATA_MAX);
                        ret = dev_misc_machineparam_del(ACQUIRERPKDATA,tempparamdata, ACQUIRERPKDATA_MAX);
                        k_free(tempparamdata);
                        if(ret == 0)
                      #endif
                        {
                            responsecode = RESPONSECODE_SUCCESS;
                            if(output[0] == 0x33)
                            {
                                //ret = ddi_misc_del_workmod();
                                memset(&devtestinfo,0xFF,sizeof(TRENDITDEV_TEST_INFO));
                                ret = ddi_misc_set_mcuparam(MACHINE_PARAM_DEVTESTINFO, (u8 *)&devtestinfo,sizeof(TRENDITDEV_TEST_INFO),0);	
                                if(ret == 0)
                                {
                                    responsecode = RESPONSECODE_SUCCESS;
                                }
                                else
                                {
                                    responsecode = RESPONSECODE_FAIL;
                                }
                            }
                        }
                    }
                }
            }
            else if(output[0] == 0x35)
            {
                //删除SN号
                //responsecode = RESPONSECODE_OTHERERR;
                ret = ddi_misc_ioctl(MISC_IOCTL_CLEARHWSN, 0, 0);
                if(ret == 0)
                {
                    responsecode = RESPONSECODE_SUCCESS;
                }
                else
                {
                    responsecode = RESPONSECODE_FAIL;
                }
            }
        }
    }
    memset(output,0xff,sizeof(output));
	if(memcmp(output,readdata,74)) //不全是0xff，要进行擦除
	{
		ddi_misc_set_mcuparam(MACHINE_PARAM_SECUREAUTHENINFO, output,74, 0);
	}
    
    return responsecode;
}
//------------------------------------------------------------------
s32 processcmd_trendit_snkey_downloadsnkey(u8 *recdata,u16 recdatalen,u8 *respdata,u16 *respdatalen)
{
    s32 retCode=RESPONSECODE_PARAMERR;
    hwsn_info_t l_hwsn_info;
    s32 len;
    s32 offset;
    s32 ret;

    *respdatalen = 0;
    if(recdata[0] == 0xF4)
    {
        //P6只支持F4
        do
        {
            memset(&l_hwsn_info, 0, sizeof(l_hwsn_info));
            ddi_misc_ioctl(MISC_IOCTL_GETHWSNANDSNKEY, 0, (u32)&l_hwsn_info);
            if(l_hwsn_info.m_snlen > 0)
            {
                retCode = RESPONSECODE_SNNOTSAME;
                break;
            }
            
            offset = 2;
            //SN号
            len = recdata[offset++];
            if(len>32)
            {
                break;      //参数错
            }
            l_hwsn_info.m_snlen = len;
            memcpy(l_hwsn_info.m_sn, &recdata[offset], len);
            offset += len;
            //密钥
            len = recdata[offset++];
            if(len>32)
            {
                break;      //参数错
            }
            l_hwsn_info.m_snkeylen = len;
            memcpy(l_hwsn_info.m_snkey, &recdata[offset], len);
            offset += len;
            //配置
            len = recdata[offset++];
            if(len>32)
            {
                break;      //参数错
            }
            l_hwsn_info.m_cfglen = len;
            memcpy(l_hwsn_info.m_cfg, &recdata[offset], len);
            offset += len;
            ret = ddi_misc_ioctl(MISC_IOCTL_SETHWSNANDSNKEY,(u32)&l_hwsn_info, 0);  //设置SN
            if(ret == 0)
            {
                retCode = RESPONSECODE_SUCCESS;
            }
            else
            {
                retCode = RESPONSECODE_OTHERERR;
            }
        }while(0);
        
    }
    return retCode;
}
//------------------------------------------------------------------

/**
 * @brief 获取下载的tts file的CRC
 * @param [in] 
               respdata 响应数据
               respdatalen 响应长度
 * @param [out] 无
 * @retval 无
 * @since 无
 */
s32 processcmd_trendit_manage_getttsfilecrc(u8 *respdata,u16 *respdatalen)
{
    s32 crc;
    u8 i;
    
    *respdatalen = 5;
    crc = ddi_audio_ioctl(DDI_AUDIO_GET_TTSFILECRC, 0, 0);
    respdata[0] = (u8)(crc>>24);
    respdata[1] = (u8)(crc>>16);
    respdata[2] = (u8)(crc>>8);
    respdata[3] = (u8)(crc);
    //获取tts文件modem同步结果
    respdata[4] = test_readttsfilesyncresult();
    return RESPONSECODE_SUCCESS;
}

s32 process_trenbtcmd_authen_getdeviceinfo(u8 *recdata,u16 recdatalen, u8 *respdata,u16 *respdatalen)
{
    s32 ret;
    s32 offset_resp = 0;
    u8 len_sn,len_imei,len_viva,len_manage;//,len_boot,len_rom,len_app;
    u8 len_spboot,len_spcore,len_spapp;//,len_hw;
    u8 result_test;         //测试结果，1为全部通过
    u8 switch_tamper;   //触发开关，1为开启
    u8 status_tamper;		//触发状态,1为已触发
    u8 no_imei[16] = {0};
    u8 sn_device[16] = {0};
    u8 ver_boot[20] = {0};
    u8 ver_rom[20] = {0};
//    u8 ver_viva[20] = {0};
//    u8 ver_manage[20] = {0};
    u8 ver_app[20] = {0};
//    u8 ver_sp_boot[30] = {0};
//    u8 ver_sp_core[30] = {0};
//    u8 ver_sp_app[30] = {0};
    u8 ver_hw[20] = {0};	
    TRENDITDEV_TEST_INFO devtestinfo;
    strSimInfo siminfo;
    hwsn_info_t l_hwsn_info;
    s8 tmp_buff[20] = "V200116001";

    memset(&devtestinfo,0,sizeof(TRENDITDEV_TEST_INFO));	
    memset(respdata,0,256);	

    //获取SN号

    //ddi_manage_read_hwsn(0,version);
    ret = ddi_misc_ioctl(MISC_IOCTL_READHWSN, 0, (u32)sn_device);
    if(ret)//SN为空或错误
    {
        len_sn = 0;
        put_data_to_respdata(respdata,&offset_resp,&len_sn,1);
    }
    else
    {
        //len_sn = sizeof(sn_device);
        //put_data_to_respdata(respdata,&offset_resp,&len_sn,1);
        put_data_to_respdata(respdata,&offset_resp,sn_device,1);	
    }

    //获取imei
    ret = ddi_gprs_get_imei(no_imei, sizeof(no_imei));
    if(ret < 0)
    {
        len_imei = 0;
    }
    else
    {
        len_imei = strlen(no_imei);
        if(len_imei>=16)
        {
            len_imei = 15;
        }
    }
    no_imei[len_imei] = 0;
    put_data_to_respdata(respdata,&offset_resp,no_imei,1);	
    //获取boot	
    ddi_misc_get_firmwareversion(FIRMWARETYPE_APBOOT,ver_boot);
    //len_boot = sizeof(ver_boot);
    //put_data_to_respdata(respdata,&offset_resp,&len_boot,1);
    put_data_to_respdata(respdata,&offset_resp,ver_boot,1);
    //获取rom, core
    ddi_misc_get_firmwareversion(FIRMWARETYPE_APCORE,ver_rom);
    //len_rom = sizeof(ver_rom);
    //put_data_to_respdata(respdata,&offset_resp,&len_rom,1);
    put_data_to_respdata(respdata,&offset_resp,ver_rom,1);

    //获取viva
    put_data_to_respdata(respdata,&offset_resp,tmp_buff,1);
    
    //获取manage版本号
    if(0 == recdatalen)    //for mes
    {
        put_data_to_respdata(respdata,&offset_resp,tmp_buff,1);
    }
    else   //下载工具时，manage位置填入app版本
    {
        //P7声音文件没有同步成功，要求重新下载
        if(MACHINE_P7_HEX == trendit_get_machine_code() && test_readttsfilesyncresult() == TTSFILE_SYNC_FAIL)
        {
            put_data_to_respdata(respdata,&offset_resp,tmp_buff,1);
        }
        else
        {
            ddi_misc_get_firmwareversion(FIRMWARETYPE_APAPP,ver_app);
            put_data_to_respdata(respdata,&offset_resp,ver_app,1);
        }
    }

    //获取app
    //P7声音文件没有同步成功，要求重新下载
    if(MACHINE_P7_HEX == trendit_get_machine_code() && test_readttsfilesyncresult() == TTSFILE_SYNC_FAIL)
    {
        put_data_to_respdata(respdata,&offset_resp,tmp_buff,1);
    }
    else
    {
        ddi_misc_get_firmwareversion(FIRMWARETYPE_APAPP,ver_app);
        put_data_to_respdata(respdata,&offset_resp,ver_app,1);
    }
    
    //获取spboot
    put_data_to_respdata(respdata,&offset_resp,tmp_buff,1);
    //获取spcore
    put_data_to_respdata(respdata,&offset_resp,tmp_buff,1);
    //获取spapp
    put_data_to_respdata(respdata,&offset_resp,tmp_buff,1);
    //获取hw版本号[sp]
    ddi_misc_get_firmwareversion(FIRMWARETYPE_HW, ver_hw);
    TRACE_DBG("ver_hw:%s", ver_hw);
    put_data_to_respdata(respdata,&offset_resp,ver_hw,1);
    
    //获取测试项是否全部通过	
    ddi_misc_get_mcuparam(MACHINE_PARAM_DEVTESTINFO, (u8 *)&devtestinfo,sizeof(TRENDITDEV_TEST_INFO),0);	
    if((devtestinfo.led==1)
    && (devtestinfo.key==1 || devtestinfo.key == 3)
    && (devtestinfo.gprs == 1)
    && (devtestinfo.wifi == 1)
    && (devtestinfo.audio == 1 || devtestinfo.audio == 3)
    && (devtestinfo.cashbox == 1 || devtestinfo.cashbox == 3)
    && (devtestinfo.printer == 1))
    {
        result_test = 1;
        //Trace("debug","test all success");	
    }
    else{
        result_test = 0;
        //Trace("debug","test has ng");	
    }		
    //获取是否开启触发
  #if 0
    if(gPciTamperManageFileInfo.tamperopenflag)
    {
        switch_tamper = 1;		
    }
    else
  #endif
    {
        switch_tamper = 0;		
    }	
    //获取是否已触发
  #if 0
    if(gPCITamperManage.gTamperDetectValue)
    {
    status_tamper = 1;
    }
    else
  #endif
    {
        status_tamper = 0;
    }
    //把result_test, switch_tamper, status_tamper填充到respdata
    memcpy(&respdata[offset_resp],&result_test,1);
    offset_resp++;
    memcpy(&respdata[offset_resp],&switch_tamper,1);
    offset_resp++;
    memcpy(&respdata[offset_resp],&status_tamper,1);
    offset_resp++;

    if(recdatalen)
    {
        if(1 == recdata[0])      //扩展增加机型ID、GPRS模块ID、硬件版本号ID
        {
            TRACE_DBG("11111");
            respdata[offset_resp++] = MACHINE_P7_HEX;              //机型ID--T1
            respdata[offset_resp++] = 0;                           //GPRS模块ID---1902\移柯

            ret = ddi_misc_ioctl(MISC_IOCTL_GETHWVER,0,0);
            if(ret >= 0)
            {
                respdata[offset_resp++] = (u8)(ret);                          //硬件版本号
            }
            else
            {
                respdata[offset_resp++] = 0xFF;
            }
        }
    }
    else
    {
        //机器配置编号
        respdata[offset_resp] = 0x02;   //带wifi，不带GPRS和摄像头
        offset_resp ++;
        //iccid 固定32字节，
        respdata[offset_resp] = 32;
        offset_resp ++;
        memset((u8*)&siminfo, 0, sizeof(strSimInfo));
        //memset(&respdata[offset_resp], 0, 32);
        ret = ddi_gprs_get_siminfo(&siminfo);
        memcpy(&respdata[offset_resp], siminfo.iccid, 32);
        offset_resp += 32;
        ret = ddi_misc_ioctl(MISC_IOCTL_GETHWSNANDSNKEY, 0, (u32)&l_hwsn_info);
        if((ret != 0) || (l_hwsn_info.m_snkeylen>32) || (l_hwsn_info.m_cfglen>32))
        {
            //密码
            respdata[offset_resp++] = 0;
            //配置码
            respdata[offset_resp++] = 0;
        }
        else
        {
            //密钥
            respdata[offset_resp++] = l_hwsn_info.m_snkeylen;
            memcpy(&respdata[offset_resp], l_hwsn_info.m_snkey, l_hwsn_info.m_snkeylen);    
            offset_resp += l_hwsn_info.m_snkeylen;
            //配置码
            respdata[offset_resp++] = l_hwsn_info.m_cfglen;
            memcpy(&respdata[offset_resp], l_hwsn_info.m_cfg, l_hwsn_info.m_cfglen);
            offset_resp += l_hwsn_info.m_cfglen;
        }
    }

    *respdatalen = offset_resp;
    #if 0
    //Trace("debug"," ver_sp_boot=%s ver_sp_core=%s  ver_sp_app=%s ver_hw=%s",ver_sp_boot,ver_sp_core,ver_sp_app,ver_hw);
    //Trace("debug","imei=%s sn=%s",no_imei,sn_device);
    //Trace("debug","ver_boot=%s ver_rom=%s ver_viva=%s ver_manage=%s ver_app=%s",ver_boot,ver_rom,ver_viva,ver_manage,ver_app);
    //Trace("debug","result_tes=%d switch_tamper=%d status_tamper=%d",result_test,switch_tamper,status_tamper);
    //Trace("debug","respdata=%slen=%d",respdata,*respdatalen);
    dev_debug_printf("sn_device = %s\r\n", sn_device);
    dev_debug_printf("ver_boot = %s\r\n", ver_boot);
    dev_debug_printf("ver_rom = %s\r\n", ver_rom);
    dev_debug_printf("ver_app = %s\r\n", ver_app);
    dev_debug_printf("ver_hw = %s\r\n", ver_hw);
    dev_debug_printf("result_test = %d\r\n", result_test);
    dev_debug_printf("switch_tamper = %d\r\n", switch_tamper);
    dev_debug_printf("status_tamper = %d\r\n", status_tamper);	
    dev_debug_printf("status_tamper = %d\r\n", status_tamper);
    dev_debug_printformat(NULL, respdata, offset_resp);
    #endif

    return RESPONSECODE_SUCCESS;
}


s32 process_trenbtcmd(void)
{
    s32 ret;
    frameinfo_t *lp_frameinfo;
    trenbtframe_t l_trenbtframe;
    u16 respdatalen;
    u8 *respdata;
    
    lp_frameinfo = usb_frame_parse_getinfo();
    if(lp_frameinfo->m_protocoltype != PROTOCOL_TYPE_TRENBT)
    {
        //
        TRENCMD_DEBUG("Protocoltype(%d) Err\r\n", lp_frameinfo->m_protocoltype);
        return DDI_EINVAL;          //参数不正确
    }

    //解析失败的不做处理
    if(PARSEPROTOCOL_STEP_RECEIVEDATAERR == lp_frameinfo->m_rxvalid)
    {
        return DDI_OK;
    }
    
    l_trenbtframe.m_sn = lp_frameinfo->m_sn;
    l_trenbtframe.m_cmd = lp_frameinfo->m_cmd;
    l_trenbtframe.m_len = lp_frameinfo->m_rxlen;
    l_trenbtframe.m_type = lp_frameinfo->m_frametype;
    l_trenbtframe.m_data = lp_frameinfo->m_rxbuf;

    if(FRAME_TYPE_TRENBT_REQ == l_trenbtframe.m_type)
    {
        process_trenbtcmd_sendack(&l_trenbtframe, lp_frameinfo->m_rxvalid);
    }
    else
    {
        return 0;
    }

    l_trenbtframe.m_type = FRAME_TYPE_TRENBT_RSP;
    respdata = (u8*)k_mallocapp(COREMANAGE_MAXSENDBUFSIZE);
    if(respdata == NULL)
    {
        TRENCMD_DEBUG("malloc err!");
        return DDI_ERR;
    }
    switch(l_trenbtframe.m_cmd)
    {
    case COMAND_SECUREMANAGE_REQUESTINFO:       //0x3003       //PC工具向POS终端请求安全处理信息
        ret = processcmd_trendit_requestsecureinfo(l_trenbtframe.m_data, l_trenbtframe.m_len, respdata, &respdatalen);
        break;
    case COMAND_SECUREMANAGE_AUTHEN:            //0x3004       //PC工具向POS终端发送安全数据处理结果
        ret = processcmd_trendit_setsecureinfo(l_trenbtframe.m_data, l_trenbtframe.m_len, respdata, &respdatalen);
        break;
    case COMAND_SECUREMANAGE_GETENCRYPTEDRAND:
        break;
    case COMMAND_PCIKEY_LOADTMK:
        ret = processcmd_trendit_snkey_downloadsnkey(l_trenbtframe.m_data, l_trenbtframe.m_len, respdata, &respdatalen);
        break;
    case COMMAND_AUTHEN_GETDEVINFO:
        ret = process_trenbtcmd_authen_getdeviceinfo(l_trenbtframe.m_data, l_trenbtframe.m_len, respdata, &respdatalen);
        break;

    case COMMAND_MANAGE_DISPFIRMWARENEW:
        if(trendit_query_timer(usbcdc_instance()->m_promptnewversiontimer, 8000))  //8S提示一次当前已是最新版本
        {
            usbcdc_instance()->m_promptnewversiontimer = trendit_get_ticks();
            audio_instance()->audio_play(AUDIO_NEW_VERSION, AUDIO_PLAY_NOBLOCK);
            respdatalen = 0;
            ret = RESPONSECODE_SUCCESS;
        }
        break;

    case COMMAND_MANAGE_GETTTSFILECRC:
        ret = processcmd_trendit_manage_getttsfilecrc(respdata, &respdatalen);
        break;
    
    default:
        ret = RESPONSECODE_CMDNOTSUPPORT;
        respdatalen = 0;
        break;
    }
    if(ret != RESPONSECODE_WAIT)
    {
        l_trenbtframe.m_status = ret;
        process_trenbtcmd_senddata(&l_trenbtframe, respdata, respdatalen);
    }
    k_freeapp(respdata);
    return 0;
}


