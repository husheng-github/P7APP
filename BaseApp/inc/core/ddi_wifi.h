#ifndef __DDI_WIFI_H__
#define __DDI_WIFI_H__


#include "ddi_common.h"




#define WIFI_WORK_MODE_STATION (1)
#define WIFI_WORK_MODE_SOFTAP (2)
#define WIFI_WORK_MODE_SOFTAT_STATION (3)

#define WIFI_TRANSFER_MODE_NORMAL (0)
#define WIFI_TRANSFER_MODE_TRANSPARENT (1)

#define WIFI_CONNECT_MODE_SINGLE (0)
#define WIFI_CONNECT_MODE_MULTIPLE (1)

#define WIFI_SLEEP_MODE_DISABLE (0)
#define WIFI_SLEEP_MODE_LIGHT (1)
#define WIFI_SLEEP_MODE_MODEM (2)

#define WIFI_SOCKET_TYPE_TCP (0)
#define WIFI_SOCKET_TYPE_UDP (1)
#define WIFI_SOCKET_TYPE_SSL (2)

#define WIFI_STATUS_CONNECTING (0)
#define WIFI_STATUS_INITIALIZED (1)
#define WIFI_STATUS_CONNECTED     (2)

#define WIFI_STATUS_CONNECT_AP_CONNECTING (0)
#define WIFI_STATUS_CONNECT_AP_DISCONNECTED (1)
#define WIFI_STATUS_CONNECT_AP_CONNECTED  (2)

#define WIFI_STATUS_SCAN_AP_PROCEEING  (0)
#define WIFI_STATUS_SCAN_AP_SUCCESSED  (1)

#define WIFI_SOCKET_STATUS_DISCONNECTED (0)
#define WIFI_SOCKET_STATUS_CONNECTING (1)
#define WIFI_SOCKET_STATUS_CONNECTED (2)


//kal events defines
#if 0
#define KAL_EVENT_AT_RESPONSE_SUCCESS (0x1 << 1)
#define KAL_EVENT_AT_READER_CLOSED (0x1 << 2)
#define KAL_EVENT_AT_READER_CLOSED_CONFIRM (0x1 << 3)
#define KAL_EVENT_AT_WRITE_SOCKET_DATA (0x1 << 4)
#define KAL_EVENT_DATA_CONSUMED (0x1 << 5)
#endif

#define WIFI_IOCTL_UPDATE    			1
#define WIFI_IOCTL_GET_FIRMWARE_VER 	2  //增加获取WIFI版本接口
#define WIFI_IOCTL_GET_SSID             3  //增加获取WIFI SSID
#define WIFI_IOCTL_GET_NET_INFO         4
#define WIFI_IOCTL_CREAT_TCP_SERVER     5
#define WIFI_IOCTL_TCP_SERVER_ACCEPT    6
#define WIFI_IOCTL_TCP_SERVER_CLOSE     7
#define WIFI_IOCTL_CFG_NTP_SERVER       8
#define WIFI_IOCTL_GET_NTP              9
#define WIFI_IOCTL_CLEAR_RF_PARAM       0xf0          //清除RF射频参数
#define WIFI_IOCTL_CMD_TRANSPARENT      0xf1          //透明传输     由应用层直接发送wifi指令
#define WIFI_IOCTL_CMD_REPOWER          0xf2          //透明传输     由应用层直接发送wifi指令







#define WIFI_VER_AT    0
#define WIFI_VER_SDK   1


typedef struct __WIFI_IOCTL_UPDATE
{
    u8 *m_filename;         //下载的文件
    u32 m_start;            //下载的起始位置
    u32 m_size;             //下载的大小
    u32 m_address;          //下载文件类型
}wifi_ioctl_update_t;

typedef struct __WIFI_APINFO{
    u32 m_ecn;
    u8 m_ssid[64];
    s32 m_rssi;
    u8 m_mac[18];
    s32 m_channel;
    s32 m_freq_offset;
    s32 m_freq_cali;
    s32 m_pairwise_cipher;
    s32 m_group_cipher;
    s32 m_bgn;
    s32 m_wps;
} wifi_apinfo_t;

typedef enum{
    WIFI_AIRKISS_FREE = 0,       //空闲
    WIFI_AIRKISS_SEARCHING,      //正在配网中
    WIFI_AIRKISS_GETCONFIGING,   //正在获取wifi ssid、password
    WIFI_AIRKISS_GETCONFIG,      //已经获取到 wifi ssid、password
    WIFI_AIRKISS_CONNECTING,     //正在连接网络中
    WIFI_AIRKISS_DHCP_TIMEOUT,   //网络获取IP超时
    WIFI_AIRKISS_CONNECT_OK,     //网络连接成功
    WIFI_AIRKISS_CONNECT_ERR,    //网络连接失败
}WIFI_AIRKISS_STEP_E;

typedef struct 
 {
    s8 m_localip[16]; //本地 IP
    s8 m_gateway [16]; //本地网关
    s8 m_mask [16]; //子网掩码
    s8 m_dns1 [16]; //默认 dns
    s8 m_dns2 [16]; //备用 dns
    s8 m_default[32];
 }wifi_net_info_t;

//=====================================================
//对外函数声明
s32 ddi_wifi_open(void);
s32 ddi_wifi_close(void);
s32 ddi_wifi_scanap_start(wifi_apinfo_t *lp_scaninfo, u32 apmax);
s32 ddi_wifi_scanap_status(u32 *ap_num);
s32 ddi_wifi_connectap_start(u8 *ssid, u8 *psw, u8 *bssid);
s32 ddi_wifi_connectap_status (void);
s32 ddi_wifi_disconnectap(void);
s32 ddi_wifi_socket_create_start(u8 type, u8 mode, u8 *param, u16 port);
s32 ddi_wifi_socket_get_status(s32 socketid);
s32 ddi_wifi_socket_send(s32 socketid, u8 *wbuf, s32 wlen);
s32 ddi_wifi_socket_recv(s32 socketid, u8 *rbuf, s32 rlen);
s32 ddi_wifi_socket_close(s32 socketid);
s32 ddi_wifi_get_signal(s32 *rssi);
s32 ddi_wifi_ioctl(u32 nCmd, u32 lParam, u32 wParam);
s32 ddi_wifi_start_airkiss(void);
s32 ddi_wifi_get_airkiss_status(void);
s32 ddi_wifi_get_airkiss_config(    u8 *ssid, u8 *password);
s32 ddi_wifi_stop_airkiss(void);

//===============================================
typedef    s32 (*core_ddi_wifi_open)(void);
typedef    s32 (*core_ddi_wifi_close)(void);
typedef s32 (*core_ddi_wifi_scanap_start)(wifi_apinfo_t *lp_apinfo, u32 ap_num);
typedef s32 (*core_ddi_wifi_scanap_status) (u32 *ap_num);
typedef    s32 (*core_ddi_wifi_connectap_start)(u8 *ssid, u8 *psw, u8 *bssid);
typedef s32 (*core_ddi_wifi_connectap_status)(void);
typedef    s32 (*core_ddi_wifi_disconnectap)(void);
typedef    s32 (*core_ddi_wifi_socket_create_start)(u8 type, u8 mode, u8 *param, u16 port);
typedef s32 (*core_ddi_wifi_socket_get_status)(s32 socketid);
typedef    s32 (*core_ddi_wifi_socket_send)(s32 socketid, u8 *wbuf, s32 wlen);
typedef    s32 (*core_ddi_wifi_socket_recv)(s32 socketid, u8 *rbuf, s32 rlen);
typedef    s32 (*core_ddi_wifi_socket_close)(s32 socketid);
typedef    s32 (*core_ddi_wifi_get_signal)(s32 *rssi);
typedef s32 (*core_ddi_wifi_ioctl)(u32 nCmd, u32 lParam, u32 wParam);
typedef	s32 (*core_ddi_wifi_start_airkiss)(void);
typedef s32 (*core_ddi_wifi_get_airkiss_status)(void);
typedef s32 (*core_ddi_wifi_get_airkiss_config)(    u8 *ssid, u8 *password);
typedef s32 (*core_ddi_wifi_stop_airkiss)(void);


#endif /* __DDI_WIFI_H__ */
