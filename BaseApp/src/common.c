#include "ddiglobal.h"
#include "app_global.h"

static s32 g_cdc_output_switch = TRUE;

/**
 * @brief 获取系统时间到字符串
 * @param[in] m_time 用来填充的字符串
 * @param[in] len 字符串长度
 * @retval  无
 */
void trendit_get_sys_time(s8 *m_time, s32 len, GET_TIME_FLAG time_flag)
{
    u8 lpTime[7] = {0};
    
    ddi_misc_get_time(lpTime);
    if(TIME_FORMAT == time_flag)
    {
        snprintf(m_time, len, "%02x%02x%02x %02x:%02x:%02x", lpTime[0], lpTime[1], lpTime[2], lpTime[3], lpTime[4], lpTime[5]);
    }
    else
    {
        snprintf(m_time, len, "%02x%02x%02x%02x%02x%02x", lpTime[0], lpTime[1], lpTime[2], lpTime[3], lpTime[4], lpTime[5]);
    }
}

/**
 * @brief 获取地址对应的文件名
 * @param[in] path_name 地址
 * @retval  根文件名
 */
s8 *trendit_basename(s8 *path_name)
{
    s8 *pstr = NULL;
    
    do{
        if(NULL == path_name)
        {
            break;
        }
        
        pstr = strrchr(path_name, '\\');
        if(NULL == pstr)
        {
            break;
        }

        pstr++;
    }while(0);

    return pstr;
}

/**
 * @brief CDC口日志输出开关
 * @param[in] onoff：TRUE:输出  FALSE:关闭
 * @retval  无
 */
s32 trendit_cdc_output_switch(s32 onoff)
{
    g_cdc_output_switch = onoff;
    ddi_misc_ioctl(MISC_IOCTL_CDC_OUTPUT_SWITCH, onoff, 0);
    return DDI_OK;
}


/**
 * @brief 格式化输出日志到控制台和文件，由TRACE_DBG、TRACE_INF、TRACE_ERR接口调用
 * @param[in] print_level 打印级别
 * @param[in] infor 指定字符串，目前固定为LOG
 * @param[in] file 代码所在的文件名
 * @param[in] line 代码所在的行
 * @param[in] func 代码所在的函数名
 * @param[in] fmt  格式
 * @retval  无
 */
void debug_Print(PRINT_LEVEL print_level, char *infor, char *file, unsigned short line, const char *func, const char *fmt, ...)
{
    va_list ap;
    s8 buffer[2048] = "";
    u16 len = 0;
    s8 time_buff[128] = {0};

    if(NULL == infor || NULL == file || NULL == func || NULL == fmt)
    {
        return;
    }

    memset(time_buff, 0, sizeof(time_buff));
    memset(buffer, 0, sizeof(buffer));
    trendit_get_sys_time(time_buff, sizeof(time_buff), TIME_FORMAT);

    switch(print_level)
    {
        case FATAL:
            snprintf((char *)(buffer+len), sizeof(buffer) - len, "%s-[%s][%s][%4d][%s][%s()]:",
                                                  time_buff, "fatal", infor, line, trendit_basename(file), func);
            break;

        case ERROR:
            snprintf((char *)(buffer+len), sizeof(buffer) - len, "%s-[%s][%s][%4d][%s][%s()]:",
                                                  time_buff, "error", infor, line, trendit_basename(file), func);
            break;

        case INFO:
            snprintf((char *)(buffer+len), sizeof(buffer) - len, "%s-[%s][%s][%4d][%s][%s()]:",
                                                  time_buff, "info", infor, line, trendit_basename(file), func);
            break;

        case DEBUG:
            snprintf((char *)(buffer+len), sizeof(buffer) - len, "%s-[%s][%s][%4d][%s][%s()]:",
                                                  time_buff, "debug", infor, line, trendit_basename(file), func);
            break;
    }
    
    len = strlen(buffer);
    va_start(ap, fmt);
    vsnprintf((char *)(buffer+len), sizeof(buffer) - len, fmt, ap);
    va_end(ap);
    
    len = strlen(buffer);
    snprintf((char *)(buffer+len), sizeof(buffer)-len,"\r\n");

#if 0
    if(g_print_level >= print_level)
    {
        write_dat(buffer, ANALYSIS_LOG_PATH);
    }
#else
    if(g_cdc_output_switch)
    {
        ddi_com_write(PORT_CDCD_NO, (u8 *)buffer, strlen(buffer));
        ddi_com_tcdrain(PORT_CDCD_NO);
    }
#endif
    return;
}

/**
 * @brief 格式化输出日志到控制台和文件，由TRACE_DBG、TRACE_INF、TRACE_ERR接口调用
 * @param[in] print_level 打印级别
 * @param[in] infor 指定字符串，目前固定为LOG
 * @param[in] file 代码所在的文件名
 * @param[in] line 代码所在的行
 * @param[in] func 代码所在的函数名
 * @param[in] fmt  格式
 * @retval  无
 */
void debug_Print_hex(PRINT_LEVEL print_level, char *infor, char *file, unsigned short line, const char *func, const u8 *buff, s32 buff_len)
{
    s8 tmp_buff[128] = {0};
    s32 i = 0;

    if(NULL == infor || NULL == file || NULL == func || NULL == buff)
    {
        return;
    }

    memset(tmp_buff, 0, sizeof(tmp_buff));
    for(i=0; i<buff_len; i++)
    {
        snprintf(tmp_buff+strlen(tmp_buff), sizeof(tmp_buff)-strlen(tmp_buff), "%02x ", buff[i]);
        if(0 == (i+1)%16)
        {
            debug_Print(print_level, infor, file, line, func, "%s", tmp_buff);
            memset(tmp_buff, 0, sizeof(tmp_buff));
        }
    }

    if(strlen(tmp_buff))
    {
        debug_Print(print_level, infor, file, line, func, "%s", tmp_buff);
    }
    
    return;
}


/**
 * @brief 判断时间是否到了
 * @param[in] begin_ticks 起始时间，单位毫秒
 * @param[in] delayms 超时时间，单位毫秒
 * @retval  1 时间到
 * @retval  0 时间未到
 */
u32 trendit_query_timer(u32 begin_ticks, u32 delayms)
{     
    u32 pass_ms = 0;

    ddi_misc_get_tick(&pass_ms);

    ddi_watchdog_feed();
    if((pass_ms - begin_ticks) >= delayms)
    {
        return 1;           //时间到
    }
    else
    {
        return 0;           
    }
}

/**
 * @brief 获取系统滴答
 * @retval  返回系统滴答
 */
u32 trendit_get_ticks(void)
{
    u32 nTick = 0;
    
    ddi_misc_get_tick(&nTick);

    return nTick;
}

static void InvertUint8(u8 *dBuf, u8 *srcBuf)
{
    s32 i;
    u8 tmp[4];  

    tmp[0] = 0;  
    for(i=0;i< 8;i++)  
    {  
        if(srcBuf[0]& (1 << i))
        {
            tmp[0]|=1<<(7-i);
        }
    }  
    dBuf[0] = tmp[0];  
}

static void InvertUint16(u16 *dBuf,u16 *srcBuf)  
{  
    s32 i;  
    u16 tmp[4];  

    tmp[0] = 0;
    
    for(i=0;i< 16;i++)  
    {  
      if(srcBuf[0]& (1 << i))  
        tmp[0]|=1<<(15 - i);  
    }  
    dBuf[0] = tmp[0];  
}  

u16 trendit_Crc16CCITT_stand(u16 crcin, u8 *puchMsg, u32 usDataLen, u8 *abyCrcOut)
{  
    u16 wCRCin = 0x0000;  
    u16 wCPoly = 0x1021;  
    u8 wChar = 0;  
    s32 i = 0;

    while (usDataLen--)
    {  
        wChar = *(puchMsg++);  
        InvertUint8(&wChar,&wChar);  
        wCRCin ^= (wChar << 8);  
        for(i = 0;i < 8;i++)  
        {  
          if(wCRCin & 0x8000)  
            wCRCin = (wCRCin << 1) ^ wCPoly;  
          else  
            wCRCin = wCRCin << 1;  
        }  
    }  
    InvertUint16(&wCRCin,&wCRCin);  

    if(abyCrcOut != NULL)
    {
        abyCrcOut[0] = wCRCin/256;
        abyCrcOut[1] = wCRCin%256;
    }
    return (wCRCin) ;
}  

u16 trendit_Crc16CCITT(u16 crcin, const u8 *pbyDataIn, u32 dwDataLen, u8 *abyCrcOut)
{
    u16 wCrc;
    u8 byTemp;
    u16 mg_awhalfCrc16CCITT[16];
    u8 i;
    
    u16 g_awhalfCrc16CCITT[16]={ /* CRC 16bit  */
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef
    };
    wCrc = crcin;

    for(i = 0;i < 16;i++)
    {
        mg_awhalfCrc16CCITT[i] = g_awhalfCrc16CCITT[i];
    }

    while(dwDataLen-- != 0)
    {
        byTemp = ((u8)(wCrc>>8))>>4;
        wCrc <<= 4;
        wCrc ^= mg_awhalfCrc16CCITT[byTemp^(*pbyDataIn/16)];
        byTemp = ((u8)(wCrc>>8))>>4;
        wCrc <<= 4;
        wCrc ^= mg_awhalfCrc16CCITT[byTemp^(*pbyDataIn&0x0f)];
        pbyDataIn++;
    }
    if(abyCrcOut != NULL)
    {
        abyCrcOut[0] = wCrc/256;
        abyCrcOut[1] = wCrc%256;
    }
    return wCrc;
} 

/**
 * @brief 将无符号char变为字符串
 * @param[in] buf 转换后的填充buf
 * @param[in] u  需转换的数据
 * @retval  1
 */
s32 trendit_pack_u8(u8 *buf, u8 u)
{
    buf[0] = u;
    
    return 1;
}

/**
 * @brief 将无符号short数据变为字符串
 * @param[in] buf 转换后的填充buf
 * @param[in] u  需转换的数据
 * @retval  2
 */
s32 trendit_pack_u16(u8 *buf, u16 u)
{
    u16_u8_union tmp;
    s32 i = 0;
    
    tmp.num = u;
    
#ifdef USING_NETWORK_BYTE_ORDER
    for(i = 0; i < 2; i++) 
    {
        buf[i] = tmp.c[1 - i];
    }
#else
    for(i = 0; i < 2; i++) 
    {
        buf[i] = tmp.c[i];
    }
#endif
    
    return 2;
}

/**
 * @brief 将无符号整形数据变为字符串
 * @param[in] buf 转换后的填充buf
 * @param[in] u  需转换的数据
 * @retval  4
 */
s32 trendit_pack_u32(u8 *buf, u32 u)
{
    u32_u8_union tmp;
    s32 i = 0;   
    tmp.num = u;
#ifdef USING_NETWORK_BYTE_ORDER
    for(i = 0; i < 4; i++) 
    {
        buf[i] = tmp.c[3 - i];
    }
#else
    for(i = 0; i < 4; i++) 
    {
        buf[i] = tmp.c[i];
    }
#endif
    
    return 4;
}

/**
 * @brief 将字符串变无符号char型
 * @param[in] dest 转换后的无符号char型
 * @param[in] buf  需转换的数据
 * @retval  1
 */
s32 trendit_unpack_u8(u8 *dest, u8 * buf)
{
    *dest = *buf;
    return 1;
}

/**
 * @brief 将字符串变无符号short型
 * @param[in] dest 转换后的无符号short型
 * @param[in] buf  需转换的数据
 * @retval  2
 */
s32 trendit_unpack_u16(u16 *dest, u8 * buf)
{
    u16_u8_union tmp;
    uint32_t i = 0;
    
#ifdef USING_NETWORK_BYTE_ORDER 
    for(i = 0; i < 2; i++) 
    {
        tmp.c[1 - i] = buf[i];
    }
#else
    for(i = 0; i < 2; i++) 
    {
        tmp.c[i] = buf[i];
    }
#endif

	*dest = tmp.num;
    return 2;
}

/**
 * @brief 将字符串变无符号整形
 * @param[in] dest 转换后的整形数据
 * @param[in] buf  需转换的数据
 * @retval  4
 */
s32 trendit_unpack_u32(u32 *dest, u8 *buf)
{
    u32_u8_union tmp;
    uint32_t i = 0;
    
#ifdef USING_NETWORK_BYTE_ORDER 
    for(i = 0; i < 4; i++) 
    {
        tmp.c[3 - i] = buf[i];
    }
#else
    for(i = 0; i < 4; i++) 
    {
        tmp.c[i] = buf[i];
    }
#endif
    *dest = tmp.num;
    return 4;
}


/**
 * @brief 将str字符以spl分割,存于dst中，并返回子字符串数量
 * @param[in] dst 目标存储字符串
 * @param[in] dst_num 缓存个数
 * @param[in] str 需要分离的字符串
 * @param[in] spl 分隔符
 * @param[in] split_flag 长度为0的是否需忽略
 * @retval  1 时间到
 * @retval  0 时间未到
 */
s32 trendit_split(s8 dst[][SPLIT_PER_MAX], s32 dst_num,s8 *str, const s8* spl, SPLIT_FLAG split_flag)
{
    s32 n = 0;
    s8 *result = NULL;
    s32 len = 0;
    s8 *ptmp = str;

    while(1)
    {
        if(n == dst_num)
        {
            break;
        }
        
        result = strstr(ptmp, spl);
        //TRACE_DBG("result:%s, %s", result, ptmp);
        if(NULL == result)
        {   
            if(!(0 == strlen(ptmp) && split_flag == SPLIT_IGNOR_LEN0))
            {
                snprintf(dst[n++], SPLIT_PER_MAX, "%s", ptmp);
            }
            break;
        }

        memset(dst[n], 0, SPLIT_PER_MAX);
        len = result-ptmp;
        len = len > (SPLIT_PER_MAX-1) ? (SPLIT_PER_MAX-1):len;

        if(!(0 == len && split_flag == SPLIT_IGNOR_LEN0))
        {
            memcpy(dst[n++], ptmp, len);
        }
        ptmp = result + strlen(spl);
    }

    return n;
}
 
/**
 * @brief 字符串转为整形
 * @param[in] str 需转换的字符串
 * @retval  整形数据
 */
s32 trendit_atoi(const s8 *str)
{
    s32 s = 0; 
    s32 flag = 0;

    while(*str==' ') 
    {  
        str++; 
    }
    
    if(*str=='-'||*str=='+') 
    {  
        if(*str == '-')
        {
            flag = 1;
        }
        str++; 
    }
    
    while(*str >= '0' && *str <= '9')
    {
        s = s*10 + *str - '0';
        str++;
        
        if(s < 0)
        {   
            s=2147483647;
            break;
        }
    }
    
    return s*(flag?-1:1);
}

/**
 * @brief 字符串转为整形
 * @param[in] str 需转换的字符串
 * @retval  1 正在播放
 * @retval  0 未播放
 */
s32 trendit_get_audio_status()
{
    u8 audio_status = 0;
    s32 ret = DDI_ERR;

    do{
        ret = ddi_audio_ioctl(DDI_AUDIO_CTL_GET_STATUS, (u32)&audio_status, 0);

        if(DDI_OK == ret)
        {
            //TRACE_DBG("audio_status:%d", audio_status);
            ret = audio_status;
            break;
        }
    }while(0);

    return ret;
}

/**
 * @brief 去掉字符串起始的空格，注意：buff必须是可修改指针，不能是只读数据区内容
 * @param[in] buff 需去掉前面空格的字符串
 */
void trendit_ltrim(s8 *buff)
{
    s8 *pstr = buff;
    s32 ret = DDI_ERR;
    
    while(pstr)
    {
        if(' ' != *pstr)
        {
            break;
        }

        pstr++;
    }

    if(NULL != pstr)
    {
        sprintf(buff, "%s", pstr);
    }
}

/**
 * @brief 去掉字符串后面的空格，注意：buff必须是可修改指针，不能是只读数据区内容
 * @param[in] buff 需去掉后面空格的字符串
 */
void trendit_rtrim(s8 *buff)
{
    s8 *pstr = buff + strlen(buff) - 1;
    s32 ret = DDI_ERR;
    
    while(pstr)
    {
        if(' ' != *pstr)
        {
            break;
        }
        else
        {
            *pstr = 0;
        }

        pstr--;
    }
}

/**
 * @brief 去掉字符串前后的空格，注意：buff必须是可修改指针，不能是只读数据区内容
 * @param[in] buff 需去掉空格的字符串
 */
void trendit_trim(s8 *buff)
{
    trendit_ltrim(buff);
    trendit_rtrim(buff);
}

/**
 * @brief 判断字符串是否为整形
 * @param[in] buff 判断的字符串
 * @retval  DDI_OK 整形
 * @retval  DDI_ERR 不是整形
 */
s32 trendit_isdigit(s8 *buff)
{
    s32 ret = DDI_OK;
    s8 *pstr = buff;

    if(NULL == buff)
    {
        return ret;
    }
    
    while(pstr)
    {
        if(0 == *pstr)
        {
            break;
        }
        
        if(*pstr < '0' || *pstr > '9')
        {
            ret = DDI_ERR;
            break;
        }

        pstr++;
    }

    return ret;
}

/**
 * @brief 判断ip地址是否合法
 * @param[in] buff 判断的字符串
 * @retval  DDI_OK 整形
 * @retval  DDI_ERR 不是整形
 */
s32 trendit_checkip_valid(s8 *ip_addr)
{
    s8 dst[5][SPLIT_PER_MAX] = {0};
    s32 ret = DDI_ERR;
    s32 i = 0;

    do{
        memset(dst, 0, sizeof(dst));
        ret = trendit_split(dst, 5, ip_addr, ".", SPLIT_NOIGNOR);
        if(4 != ret)
        {
            TRACE_ERR("invalid ip:%s", ip_addr);
            ret = DDI_ERR;
            break;
        }

        ret = DDI_OK;
        for(i=0; i<4; i++)
        {
            if(DDI_OK != trendit_isdigit(dst[i]))
            {
                TRACE_ERR("invalid ip:%s,%s", ip_addr, dst[i]);
                ret = DDI_ERR;
                break;
            }

            if(trendit_atoi(dst[i]) < 0 || trendit_atoi(dst[i]) > 255)
            {
                TRACE_ERR("invalid range:%s,%s", ip_addr, dst[i]);
                ret = DDI_ERR;
                break;
            }
        }
    }while(0);

    return ret;
}

u32 trendit_asc_to_u32(s8 *src, u32 srclen, DATA_FORMAT_TYPE_E base)
{
    u32 i, j;
    s8 *p;
    u32 sum=0;

    p = src;
    for(i=0; i<srclen; i++)
    {
        if(base == MODULUS_HEX)
        {
            if(((*p>='0') && (*p<='9'))
             ||((*p>='A') && (*p<='F'))
             ||((*p>='a') && (*p<='f'))) 
            {
                
            }
            else
            {
                break;
            }
        }
        else
        {
            if((*p<'0') || (*p>('0'+base-1)))
            {
                break;
            }
        }
        p++;
    }
    //------------------------------
    p = src;  
    for(j=0; j<i; j++)
    {
        if(*p <= '9')
        {
            sum = (sum*base + (*p-'0'));
        }
        else if(*p <= 'F')
        {
            sum = (sum*base + (*p-'A'+10));
        }
        else
        {
            sum = (sum*base + (*p-'a'+10));
        }
        p++;        
    }
    return sum;
}

//u32转hex, 高位在前，低位在后(len<=4)
void trendit_u32_to_hex(u8 *Hex, u32 const Src,u32 Len)
{
	u32 i;
    u32 num = Src;

    memset(Hex, 0, Len);
    for(i = 0; i < Len; i--)
    {
        Hex[Len-i-1] = (num&0xFF);
        num >>= 8;
    }
}

//hex转u32, 高位在前，低位在后(len<=4)
u32 trendit_hex_to_u32(u8 *Hex,u32 Len)
{
  	 u32 i;
     u32 dest;
     
	 dest = 0;
	 
     for(i = 0; i < Len; i++)
     {
        dest = (dest<<8)+Hex[i];
     }
     return dest;
}
//字符替换
u8 trendit_strchr(u8 *str, u8 oldchr, u8 newchr)
{
    u32 i =0; 
    u8 ret = 0;

    while(str[i] != 0 ) 
    {
        if(str[i] == oldchr)
        {
            str[i] = newchr;
            ret = 1;
        }
        else
        {
            i++;
        }
    }
    return ret;
}
//计算文件CRC32
static const u32 CRC32TAB[256] = 
{
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433, 0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F, 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B, 0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777, 0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};
u32 trendit_calc_crc32(u32 crc, u8 *szSrc, u32 dwSrcLen)
{
    u32 len = dwSrcLen;
    u8 * buf = (u8 *)szSrc;

    while(len--) 
    {
        crc = (crc >> 8) ^ CRC32TAB[(crc & 0xFF) ^ *buf++];
    }

    return crc;
}

//计算文件的CRC
#define CALC_FILECRC32_LENMAX      4096
s32 trendit_calc_filecrc32(u8* filename, u32 calcoffset, u32 calclen, u32 *crcout)
{
    u32 crc;
    u32 i,j,k;
    u8  *ap;
    u32 addr;
    s32 ret=0;

    ap = (u8*)k_mallocapp(CALC_FILECRC32_LENMAX);
    if(ap == NULL)
    {
        //动态申请失败
        return -1;
    }
    addr = calcoffset;
    do
    {
        i = 0;
        crc = 0xFFFFFFFF;
        while(i<calclen)
        {
            if((i+CALC_FILECRC32_LENMAX)>calclen)
            {
                j = calclen-i;
            }
            else
            {
                j = CALC_FILECRC32_LENMAX;
            }
            ret = ddi_file_read(filename, ap, j, addr);
            if(ret != j)
            {
                ret = -1;    //读文件出错
                break;
            }
            crc = trendit_calc_crc32(crc, ap, j);
            addr += j;
            i += j;
        }
        
    }while(0);

    k_freeapp(ap);
    *crcout = crc ^ 0xFFFFFFFF;
    return ret;
}
//计算LRC
u8 trendit_maths_getlrc(u8 lrcini, const u8 *pdata, u16 len)
{
    u16 i;
    u8 lrc;
    lrc = lrcini;
    for(i=0; i<len; i++)
    {
        lrc ^= pdata[i];
    }
    return lrc;
}
/**
 * @brief 判断文件是否存在
 * @param[in] filename 文件名
 * @retval  DDI_OK 存在
 * @retval  DDI_ERR 不存在
 */
s32 trendit_file_access(s8 *filename)
{
    s32 fd = ddi_vfs_open(filename, "r");
    s32 ret = DDI_ERR;

    if(fd > 0)
    {
        ddi_vfs_close(fd);
        ret = DDI_OK;
    }

    return ret;
}

/**
 * @brief 获取机型码
 * @param[in] 无
 * @retval  机型码
 */
u8 trendit_get_machine_code(void)
{
    u16 ret = ddi_misc_ioctl(MISC_IOCTL_GETTMSMACHINETYPEID, 0, 0);
    u8 machine_code = 0;

    if(ret > 0)
    {
        machine_code = (ret & 0xff00) >> 8;
    }

    TRACE_DBG("ret:%x, machine_code:%x", ret, machine_code);
    return machine_code;
}

/**
 * @brief 组织返回报文
 * @param[in] in_buff:用来填充返回数据的buff
 * @param[in] data_content:正文数据
 * @param[in] data_content_len:正文数据长度
 * @param[in] pack_no:包序号
 * @param[in] cmd_type:命令类型
 * @retval  返回报文长度
 */
s32 trendit_pack_answer_data(u8 *in_buff, u8 *data_content, u16 data_content_len, u16 pack_no, CMD_TYPE cmd_type)
{
    u16 len = 0;
    u8 crc[2] = {0};
    u16 tmp_len = 0;
    
    in_buff[len++] = STX;
    memcpy(in_buff+len, KEY_WORDS, strlen(KEY_WORDS));
    len += strlen(KEY_WORDS);
    trendit_pack_u16(in_buff+len, pack_no);
    len += 2;
    ddi_misc_get_time(in_buff+len);
    len += PARSE_STEP4_LEN;
    in_buff[len++] = PROTOCOL_VERSION;
    in_buff[len++] = cmd_type;
    trendit_pack_u16(in_buff+len, data_content_len);
    len += 2;
    memcpy(in_buff+len, data_content, data_content_len);
    len += data_content_len;

    tmp_len = PARSE_STEP1_LEN+PARSE_STEP2_LEN+PARSE_STEP3_LEN+PARSE_STEP4_LEN;
    #ifdef IBOX
    trendit_Crc16CCITT_stand(0, in_buff+tmp_len, len-tmp_len, crc);
    #else
    trendit_Crc16CCITT(0, in_buff+tmp_len, len-tmp_len, crc);
    #endif
    in_buff[len++] = ETX;
    in_buff[len++] = crc[0];
    in_buff[len++] = crc[1];

    return len;
}

/**
 * @brief 去掉字符串后的换行符
 * @param[in] buff 需去掉空格的字符串
 */
s32 trendit_trim_enter(s8 * buff)
{
    s8 *pstr = buff + strlen(buff) - 1;
    s32 ret = DDI_ERR;
    
    while(pstr)
    {
        if('\r' != *pstr && '\n' != *pstr)
        {
            break;
        }
        else
        {
            *pstr = 0;
        }

        pstr--;
    }
}

/**
 * @brief 屏蔽前4后4字节以外的数据
 * @param[in] buff 
 * @retval buff  
 */
s8 *trendit_hide_word(s8 * buff)
{
    if(strlen(buff) > 8)
    {
        memset(buff+4, '*', strlen(buff)-8);
    }

    return buff;
}

/**
 * @brief 生产测试相关的测试菜单的提示信息，smt和集成模式，直接语音播报，其它打印出来
 * @param[in] msg:提示信息 
 * @retval buff  
 */
void trendit_factory_test_display_msg(s8 * msg, AUDIO_PLAY_TYPE audio_play_type)
{
    s8 buff[128] = {0};

    if(0 == dev_misc_get_workmod() && MACHINE_P7_HEX != trendit_get_machine_code())
    {
        snprintf(buff, sizeof(buff), "%s\r\n\r\n\r\n", msg);
        printer_instance()->direct_print(buff, strlen(buff));
    }
    else
    {
        audio_instance()->audio_play(msg, audio_play_type);
    }
}

s32 sdk_misc_set_mcuparam(u16 sn, u8 *param, u16 paramlen, u16 offset)
{
    
    strMCUparam_ioctl_t l_mcuparam;
    
    l_mcuparam.m_sn = sn;
    l_mcuparam.m_paramlen = paramlen;
    l_mcuparam.m_param = param;
    l_mcuparam.m_offset = offset;
    return ddi_misc_ioctl(MISC_IOCTL_SETMCUPARAM, (u32)&l_mcuparam, 0);
    
}

s32 sdk_misc_get_mcuparam(u16 sn, u8 *param, u16 paramlen, u16 offset)
{
    strMCUparam_ioctl_t l_mcuparam;
    
    l_mcuparam.m_sn = sn;
    l_mcuparam.m_paramlen = paramlen;
    l_mcuparam.m_param = param;
    l_mcuparam.m_offset = offset;
    return ddi_misc_ioctl(MISC_IOCTL_GETMCUPARAM, (u32)&l_mcuparam, 0);
    
}