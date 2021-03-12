#ifndef __COMMON_H
#define __COMMON_H

#define WIFI_FIXED_FREQ_TEST
#undef WIFI_FIXED_FREQ_TEST

#define PRINT_AGING
#undef PRINT_AGING

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define MH1902_BASE_ADDR    0x1000000
#define BOOT_START_ADDR     (0x1000)
#define BOOT_SIZE           0x16000
#define CORE_START_ADDR     (0x18000)
#define CORE_SIZE           0x60000
#define CORE_DDI_ADDR       (CORE_START_ADDR+0x1000)
#define APP_START_ADDR      (CORE_START_ADDR+CORE_SIZE)
#define APP_SIZE            0x48000
#define APP_PIT_START       APP_START_ADDR
#define APP_CORE_START     (APP_START_ADDR+0x200)

typedef int (*core_app_start)(void);

typedef struct _APP_FIRMWARE_API
{
    core_app_start  app_start;
}APP_FIRMWARE_API;

typedef enum{
    FATAL=1,
    ERROR=2,
    INFO=3,
    DEBUG=4
}PRINT_LEVEL;

typedef union 
{
    u16 num;
    u8  c[2];
}u16_u8_union;

typedef union 
{
    u32 num;
    u8 c[4];
} u32_u8_union;

//trendit_split函数最后一个参数
typedef enum{
    SPLIT_NOIGNOR,          //默认值
    SPLIT_IGNOR_LEN0        //split后长度为0，直接忽略
}SPLIT_FLAG;

typedef enum{
    TIME_NO_FORMAT,      //无格式
    TIME_FORMAT     //带格式
}GET_TIME_FLAG;

typedef enum{
    MODULUS_BIN = 2,     //二进制
    MODULUS_OCT = 8,     //八进制
    MODULUS_DEC = 10,    //十进制
    MODULUS_HEX = 16     //十六进制
}DATA_FORMAT_TYPE_E;

#define MACHINE_P6  "56"
#define MACHINE_P6_HEX  0x56
#define MACHINE_P7  "57"
#define MACHINE_P7_HEX  0x57

#define APP_VER "V" MACHINE_P7 "A" "RZ1S"  "210112" "004"
#define TRACE_DBG(arg...)  debug_Print(DEBUG,"APP",__FILE__,__LINE__,__FUNCTION__, ##arg);
#define TRACE_INFO(arg...) debug_Print(INFO,"APP",__FILE__,__LINE__,__FUNCTION__, ##arg);
#define TRACE_ERR(arg...)  debug_Print(ERROR,"APP",__FILE__,__LINE__,__FUNCTION__, ##arg);
#define TRACE_DBG_HEX(buff, buff_len)  debug_Print_hex(DEBUG,"APP",__FILE__,__LINE__,__FUNCTION__, buff, buff_len);
#define TRACE_INFO_HEX(buff, buff_len) debug_Print_hex(INFO,"APP",__FILE__,__LINE__,__FUNCTION__, buff, buff_len);
#define TRACE_ERR_HEX(buff, buff_len)  debug_Print_hex(ERROR,"APP",__FILE__,__LINE__,__FUNCTION__, buff, buff_len);
#define USING_NETWORK_BYTE_ORDER
#define SPLIT_PER_MAX      80         //TLV value数据每个数据最大长度
#define GET_STR(str)      ((strlen(str) == 0) ?"null":str)


void trendit_get_sys_time(s8 *m_time, s32 len, GET_TIME_FLAG time_flag);
s8 *trendit_basename(s8 *path_name);
void debug_Print(PRINT_LEVEL print_level, char *infor, char *file, unsigned short line, const char *func, const char *fmt, ...);
u32 trendit_query_timer(u32 begin_ticks, u32 delayms);
u32 trendit_get_ticks(void);
u16 trendit_Crc16CCITT(u16 crcin, const u8 *pbyDataIn, u32 dwDataLen, u8 *abyCrcOut);
s32 trendit_pack_u8(u8 *buf, u8 u);
s32 trendit_pack_u16(u8 *buf, u16 u);
s32 trendit_pack_u32(u8 *buf, u32 u);
s32 trendit_unpack_u8(u8 *dest, u8 * buf);
s32 trendit_unpack_u16(u16 *dest, u8 * buf);
s32 trendit_unpack_u32(u32 *dest, u8 *buf);
s32 trendit_split(s8 dst[][SPLIT_PER_MAX], s32 dst_num,s8 *str, const s8* spl, SPLIT_FLAG split_flag);
s32 trendit_atoi(const s8 *str);
s32 trendit_get_audio_status(void);
void trendit_ltrim(s8 *buff);
void trendit_rtrim(s8 *buff);
void trendit_trim(s8 *buff);
s32 trendit_isdigit(s8 *buff);
s32 trendit_checkip_valid(s8 *ip_addr);
void trendit_u32_to_hex(u8 *Hex, u32 const Src,u32 Len);
u32 trendit_hex_to_u32(u8 *Hex,u32 Len);
s32 trendit_calc_filecrc32(u8* filename, u32 calcoffset, u32 calclen, u32 *crcout);
u8 trendit_maths_getlrc(u8 lrcini, const u8 *pdata, u16 len);
s32 trendit_file_access(s8 *filename);
s32 trendit_cdc_output_switch(s32 onoff);
u8 trendit_get_machine_code(void);
s32 sdk_misc_set_mcuparam(u16 sn, u8 *param, u16 paramlen, u16 offset);
s32 sdk_misc_get_mcuparam(u16 sn, u8 *param, u16 paramlen, u16 offset);


#endif
