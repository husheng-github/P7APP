#ifndef __HTTP_DOWNLOAD_H
#define __HTTP_DOWNLOAD_H

//���������ļ�����
typedef enum{
    FT_BIN_FILE,    //�������ļ�����core��app
    FT_PHOTO,       //ͼƬ
    FT_AUDIO,       //����
    FT_FONT,        //�ֿ�
    FT_TEXT
}UPGRDE_FILE_TYPE_E;

//������������
typedef enum{
    US_CHECK_ENV,           //�������
    US_DOWNLOAD_RESULT,     //����   �����0�ɹ���-1ʧ��
    US_UPGRADE_RESULT,      //����   �����0�ɹ���-1ʧ��
}UPGRADE_STEP_E;

//���ز���
typedef enum{
    DOWNLOADING,
    DOWNLOADED,
    DOWNLOADERR
}DOWNLOAD_STEP_E;

typedef struct{
    u16 m_backend_cmd;                               //����Э��ָ��
    u16 m_packno;                                    //��̨�·�����Э����İ����
    void *m_lparam;                                  //�û��Զ������ݣ������malloc�ڴ棬�ɵ��ò�ȥ�ͷ��ⲿ���ڴ�
}upgrade_callback_info_t;

typedef struct {
    UPGRDE_FILE_TYPE_E  m_download_file_type;           //�����ļ�����
    s8 *m_store_filepath;                               //�����flash·�������ΪNULL���ɵ��ò��Լ��ڻص�����self_save_cb�ﴦ����ΪNULL��������/mtd0��/mtd2·����
    s32 (*self_save_cb)(u8 *data, u32 data_len, DOWNLOAD_STEP_E download_step, upgrade_callback_info_t *upgrade_callback_info);
    s8 *m_download_url;                                 //���ص�url
    upgrade_callback_info_t *m_upgrade_callback_info;   //callback�����õ�����Ϣ������÷�����
    void (*upgrade_result_callback)(upgrade_callback_info_t *callback_info, UPGRADE_STEP_E upgrade_step, s32 result);  //���µĻص�����
    s8 m_default[32];
}upgrade_info_t;

s32 trendit_post_download_msg(upgrade_info_t *upgrade_info);
s32 upgrade_result_callback(upgrade_callback_info_t *callback_info, UPGRADE_STEP_E upgrade_step, s32 result);

#endif