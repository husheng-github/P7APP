#ifndef __HTTP_DOWNLOAD_H
#define __HTTP_DOWNLOAD_H

//程序升级文件类型
typedef enum{
    FT_BIN_FILE,    //二进制文件，如core、app
    FT_PHOTO,       //图片
    FT_AUDIO,       //声音
    FT_FONT,        //字库
    FT_TEXT
}UPGRDE_FILE_TYPE_E;

//程序升级步骤
typedef enum{
    US_CHECK_ENV,           //条件检查
    US_DOWNLOAD_RESULT,     //下载   结果：0成功，-1失败
    US_UPGRADE_RESULT,      //更新   结果：0成功，-1失败
}UPGRADE_STEP_E;

//下载步骤
typedef enum{
    DOWNLOADING,
    DOWNLOADED,
    DOWNLOADERR
}DOWNLOAD_STEP_E;

typedef struct{
    u16 m_backend_cmd;                               //空中协议指令
    u16 m_packno;                                    //后台下发空中协议里的包序号
    void *m_lparam;                                  //用户自定义数据，如果是malloc内存，由调用层去释放这部分内存
}upgrade_callback_info_t;

typedef struct {
    UPGRDE_FILE_TYPE_E  m_download_file_type;           //升级文件类型
    s8 *m_store_filepath;                               //存入的flash路径，如果为NULL，由调用层自己在回调函数self_save_cb里处理，不为NULL，必须是/mtd0或/mtd2路径下
    s32 (*self_save_cb)(u8 *data, u32 data_len, DOWNLOAD_STEP_E download_step, upgrade_callback_info_t *upgrade_callback_info);
    s8 *m_download_url;                                 //下载的url
    upgrade_callback_info_t *m_upgrade_callback_info;   //callback函数用到的信息，需调用放填入
    void (*upgrade_result_callback)(upgrade_callback_info_t *callback_info, UPGRADE_STEP_E upgrade_step, s32 result);  //更新的回调函数
    s8 m_default[32];
}upgrade_info_t;

s32 trendit_post_download_msg(upgrade_info_t *upgrade_info);
s32 upgrade_result_callback(upgrade_callback_info_t *callback_info, UPGRADE_STEP_E upgrade_step, s32 result);

#endif