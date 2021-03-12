#ifndef __MODEL_AUDIO_H
#define __MODEL_AUDIO_H

#define AUDIO_CONFIG_FILE            "/mtd0/audio_config"

#define AUDIO_DEVICE_WIRELESS_CONECT_SUCCESS  "无线联网成功"
#define AUDIO_DEVICE_WIFI_CONECT_SUCCESS  "wifi联网成功"
#define AUDIO_UPLOADING              "设备升级中,请勿拔掉电源"
#define AUDIO_UPLOADED               "升级成功"
#define AUDIO_UPLOADED_ERR           "升级失败"
#define AUDIO_ENTER_WIFI_CONFIG      "已进入微信配网模式"
#define AUDIO_ENTER_WIFI_WEB_CONFIG  "已进入网页配网模式"
#define AUDIO_ENTER_WIFI_CONFIG_OK   "配网成功"
#define AUDIO_CONFIG_OVERTIME        "设备配置超时，退出配置模式"
#define AUDIO_PLUS                   "音量加"
#define AUDIO_MAX                    "音量最大"
#define AUDIO_SUB                    "音量减"
#define AUDIO_RECOVERY               "正在恢复出厂设置，请勿拔掉电源"
#define AUDIO_RECOVERY_SUCC          "恢复出厂成功"
#define AUDIO_POWER_ON               "开机声音"
#define AUDIO_POWER_OFF              "关机声音"
#define AUDIO_REBOOT                 "关机声音"
#define AUDIO_NET_DROP               "设备已掉线，请检查网络"
#define AUDIO_DEVICE_OUT_PAPER       "设备缺纸"
#define AUDIO_SIMCARD_NO_DATA        "注册网络失败，请检查sim卡流量"
#define AUDIO_NO_SIMCARD             "sim卡未插，请关机后插入"
#define AUDIO_TCP_CONNECT_OVERTIME   "设备联网超时，请检查网络"
#define AUDIO_WIFI_CONFIG_EXIT       "退出配置模式"
#define AUDIO_WIFI_CONFIG_ERROR      "配网失败，退出配置模式"
#define AUDIO_WIRELESS_REG_NET_DENIED "注册网络拒绝"
#define AUDIO_NO_WIRELESS_MODEL      "未检测到无线模块"
#define AUDIO_NEW_VERSION            "软件已是最新版本"
#define AUDIO_ATTACHING_WIRELESS     "无线正在驻网，请稍后"
#define AUDIO_SYNCAUDIOWITHMODEM     "正在同步声音文件，请勿拔掉电源"
#define AUDIO_AUDIOFILE_SYNCERROR    "同步失败"
#define AUDIO_AUDIOFILE_SYNCESUCCESS "同步成功"
#define AUDIO_CFG_SUCCESS            "配置成功"
#define AUDIO_CFG_FAIL               "配置失败"


#define ERROR_WIRELESS_REGING         "正在注册网络"
#define ERROR_WIRELESS_ACTIVING       "正在激活数据模式"
#define ERROR_DES_CONNECT_AP_FAIL     "连接wifi热点失败"
#define AUDIO_DOWNLOAD_ENTER         "进入下载模式"
#define AUDIO_DOWNLOAD_END           "退出下载模式"
#define ERROR_DES_WIFI_AP_DO_NOT_CONFIG "wifi热点未配置"
#define ERROR_DES_TCP_CONNECT_FAIL   "连接后台服务器失败"
#define WIFI_WEB_CONFIG_ANSWER_INFO  "<br><br>已收到wifi配置信息<br>热点名: %s<br>密码: %s<br>请留意终端稍后的语音信息"



//测试模式提示
#define AUDIO_SMT_TEST_ENTER         "已进入SMT测试模式"
#define AUDIO_ASSEMBLY_TEST_ENTER    "已进入组装测试模式，按任意键开始测试"
#define AUDIO_TEST_ENTER             "已进入测试模式"
#define AUDIO_TEST_ENTER_WAIT_KEY    "已进入测试模式，按任意键"
#define AUDIO_TEST_KEY_ENTER         "按键测试，请按键"
#define AUDIO_TEST_KEY_PAPER         "走纸键"
#define AUDIO_TEST_KEY_FINISH        "按功能键测试成功，按其他键测试失败"
#define AUDIO_TEST_LED_ENTER         "led测试"
#define AUDIO_TEST_LED_STATUS_B      "状态灯蓝灯"
#define AUDIO_TEST_LED_STATUS_R      "状态灯红灯"
#define AUDIO_TEST_LED_SIGNAL_Y      "信号灯黄灯"
#define AUDIO_TEST_LED_SIGNAL_B      "信号灯蓝灯"
#define AUDIO_TEST_FINISH            "测试完成"
#define AUDIO_TEST_PASS              "通过"
#define AUDIO_TEST_FAIL              "不通过"
#define AUDIO_TEST_CASHBOX           "钱箱测试"
#define AUDIO_TEST_SINGLE            "单项测试"
#define AUDIO_TEST_AGING             "老化测试"
#define AUDIO_TEST_AUTO              "自动测试"
#define AUDIO_TEST_8960              "捌玖陆零测试"
#define AUDIO_TEST_FACTORY           "已进入生产测试，按音量加减键"
#define AUDIO_TEST_KEY_EXIT          "按任意键，退出"
#define AUDIO_TEST                   "音频测试"
#define AUDIO_TEST_SINGLE_ENTER      "单项测试，按音量加减键"
#define AUDIO_TEST_KEY_PLUS          "按音量加键测试"

typedef enum
{
    AUDIO_PALY_1 = 1,
    AUDIO_PALY_2,
    AUDIO_PALY_3,
} AUDIO_PLAY_CNT;

typedef enum{
    AUDIO_PLAY_BLOCK,        //阻塞等待播放完
    AUDIO_PLAY_NOBLOCK,      //非阻塞
}AUDIO_PLAY_TYPE;

typedef enum{
    VOL_CFG_SOURCE_WEB,    //后台配置
    VOL_CFG_SOURCE_KEY     //按键配置
}AUDIO_VOL_CFG_SOURCE_E;

typedef enum{
    AP_STEP_NEW_INSERT=1,        //外部有播放数据插入
    AP_STEP_CHECK_PLAY_RES,    //检测播放结果
    AP_STEP_SLEEP_CHECK,       //休眠结果确认
    AP_STEP_SLEEP_REPLAY,      //循环播放
    AP_STEP_IDLE               //任务空闲
}AUDIO_PLAY_STEP_E;

typedef struct{
    s8 m_play_string[128];     //播放的字符串
    s32 m_playtimes;            //播放的次数
    s32 m_have_playtimes;
    s32 m_interval_time;           //播放间隔时间，单位秒
    u32 m_cur_ticks;
    AUDIO_PLAY_STEP_E m_play_step;
}audio_play_info_t;

typedef struct{
    model_device_t super;
    s32 m_vol;                      //声音大小
    s32 m_load_cfg;
    s32 (*audio_play)(s8 *play_string, AUDIO_PLAY_TYPE audio_play_type);
    s32 (*cfg_vol)(AUDIO_VOL_CFG_SOURCE_E vol_cfg_source, s32 key_flag, s32 vol);
    void (*play_poweron_audio)(void);
    void (*insert_play_data)(s8 *play_string, s32 play_time, s32 interval_times);
    void (*clear_play_data)(void);
    audio_play_info_t m_play_info;
}audio_module_t;


#define CMD_PLAY_MAX_LEN            128

#define CMD_CFG_AUDIO_VOLUME        0x0401
#define CMD_COMAND_AUDIO_PLAY       0x04c0

#define TRENDIT_AUDIO_DOWNLOAD      1

#ifdef TRENDIT_AUDIO_DOWNLOAD
//存放音频资源的文件
typedef struct{
    u8 m_file_name[128];//download 文件名
    u32 m_file_index;//download 文件第几个音源
    u8  m_file_flg;  //0-下载，1-更新
    u32 m_file_offset;//download 文件音源位置offset
}audio_file_config;

#define AUDIO_SOURCE_PATH      "/mtd0/audio" //客户定制音源

#define AUDIO_MAX_TEXT_NAME_LEN 128 //客户文件名长度
#define AUDIO_MAX_TEXT_DOWNLOAD_FLG 1//文件下载状态
#define AUDIO_MAX_TEXT_FILE_CNT 50  //50k
#define AUDIO_MAX_TEXT_FILE_LEN AUDIO_MAX_TEXT_FILE_CNT*1024//客户文件内容长度
#define AUDIO_MAX_NUM   10  //客户最大Audio数量


#define audio_filename_addr(n)      (n*(AUDIO_MAX_TEXT_NAME_LEN+AUDIO_MAX_TEXT_DOWNLOAD_FLG+AUDIO_MAX_TEXT_FILE_LEN))
#define audio_filedownflg_addr(n)   (audio_filename_addr(n)+AUDIO_MAX_TEXT_NAME_LEN)
#define audio_filedata_addr(n)      (audio_filedownflg_addr(n)+AUDIO_MAX_TEXT_DOWNLOAD_FLG)


#endif

audio_module_t *audio_instance(void);
void trendit_init_audio_data(void);



#endif

