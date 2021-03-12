#ifndef __MODEL_AUDIO_H
#define __MODEL_AUDIO_H

#define AUDIO_CONFIG_FILE            "/mtd0/audio_config"

#define AUDIO_DEVICE_WIRELESS_CONECT_SUCCESS  "���������ɹ�"
#define AUDIO_DEVICE_WIFI_CONECT_SUCCESS  "wifi�����ɹ�"
#define AUDIO_UPLOADING              "�豸������,����ε���Դ"
#define AUDIO_UPLOADED               "�����ɹ�"
#define AUDIO_UPLOADED_ERR           "����ʧ��"
#define AUDIO_ENTER_WIFI_CONFIG      "�ѽ���΢������ģʽ"
#define AUDIO_ENTER_WIFI_WEB_CONFIG  "�ѽ�����ҳ����ģʽ"
#define AUDIO_ENTER_WIFI_CONFIG_OK   "�����ɹ�"
#define AUDIO_CONFIG_OVERTIME        "�豸���ó�ʱ���˳�����ģʽ"
#define AUDIO_PLUS                   "������"
#define AUDIO_MAX                    "�������"
#define AUDIO_SUB                    "������"
#define AUDIO_RECOVERY               "���ڻָ��������ã�����ε���Դ"
#define AUDIO_RECOVERY_SUCC          "�ָ������ɹ�"
#define AUDIO_POWER_ON               "��������"
#define AUDIO_POWER_OFF              "�ػ�����"
#define AUDIO_REBOOT                 "�ػ�����"
#define AUDIO_NET_DROP               "�豸�ѵ��ߣ���������"
#define AUDIO_DEVICE_OUT_PAPER       "�豸ȱֽ"
#define AUDIO_SIMCARD_NO_DATA        "ע������ʧ�ܣ�����sim������"
#define AUDIO_NO_SIMCARD             "sim��δ�壬��ػ������"
#define AUDIO_TCP_CONNECT_OVERTIME   "�豸������ʱ����������"
#define AUDIO_WIFI_CONFIG_EXIT       "�˳�����ģʽ"
#define AUDIO_WIFI_CONFIG_ERROR      "����ʧ�ܣ��˳�����ģʽ"
#define AUDIO_WIRELESS_REG_NET_DENIED "ע������ܾ�"
#define AUDIO_NO_WIRELESS_MODEL      "δ��⵽����ģ��"
#define AUDIO_NEW_VERSION            "����������°汾"
#define AUDIO_ATTACHING_WIRELESS     "��������פ�������Ժ�"
#define AUDIO_SYNCAUDIOWITHMODEM     "����ͬ�������ļ�������ε���Դ"
#define AUDIO_AUDIOFILE_SYNCERROR    "ͬ��ʧ��"
#define AUDIO_AUDIOFILE_SYNCESUCCESS "ͬ���ɹ�"
#define AUDIO_CFG_SUCCESS            "���óɹ�"
#define AUDIO_CFG_FAIL               "����ʧ��"


#define ERROR_WIRELESS_REGING         "����ע������"
#define ERROR_WIRELESS_ACTIVING       "���ڼ�������ģʽ"
#define ERROR_DES_CONNECT_AP_FAIL     "����wifi�ȵ�ʧ��"
#define AUDIO_DOWNLOAD_ENTER         "��������ģʽ"
#define AUDIO_DOWNLOAD_END           "�˳�����ģʽ"
#define ERROR_DES_WIFI_AP_DO_NOT_CONFIG "wifi�ȵ�δ����"
#define ERROR_DES_TCP_CONNECT_FAIL   "���Ӻ�̨������ʧ��"
#define WIFI_WEB_CONFIG_ANSWER_INFO  "<br><br>���յ�wifi������Ϣ<br>�ȵ���: %s<br>����: %s<br>�������ն��Ժ��������Ϣ"



//����ģʽ��ʾ
#define AUDIO_SMT_TEST_ENTER         "�ѽ���SMT����ģʽ"
#define AUDIO_ASSEMBLY_TEST_ENTER    "�ѽ�����װ����ģʽ�����������ʼ����"
#define AUDIO_TEST_ENTER             "�ѽ������ģʽ"
#define AUDIO_TEST_ENTER_WAIT_KEY    "�ѽ������ģʽ���������"
#define AUDIO_TEST_KEY_ENTER         "�������ԣ��밴��"
#define AUDIO_TEST_KEY_PAPER         "��ֽ��"
#define AUDIO_TEST_KEY_FINISH        "�����ܼ����Գɹ���������������ʧ��"
#define AUDIO_TEST_LED_ENTER         "led����"
#define AUDIO_TEST_LED_STATUS_B      "״̬������"
#define AUDIO_TEST_LED_STATUS_R      "״̬�ƺ��"
#define AUDIO_TEST_LED_SIGNAL_Y      "�źŵƻƵ�"
#define AUDIO_TEST_LED_SIGNAL_B      "�źŵ�����"
#define AUDIO_TEST_FINISH            "�������"
#define AUDIO_TEST_PASS              "ͨ��"
#define AUDIO_TEST_FAIL              "��ͨ��"
#define AUDIO_TEST_CASHBOX           "Ǯ�����"
#define AUDIO_TEST_SINGLE            "�������"
#define AUDIO_TEST_AGING             "�ϻ�����"
#define AUDIO_TEST_AUTO              "�Զ�����"
#define AUDIO_TEST_8960              "�ƾ�½�����"
#define AUDIO_TEST_FACTORY           "�ѽ����������ԣ��������Ӽ���"
#define AUDIO_TEST_KEY_EXIT          "����������˳�"
#define AUDIO_TEST                   "��Ƶ����"
#define AUDIO_TEST_SINGLE_ENTER      "������ԣ��������Ӽ���"
#define AUDIO_TEST_KEY_PLUS          "�������Ӽ�����"

typedef enum
{
    AUDIO_PALY_1 = 1,
    AUDIO_PALY_2,
    AUDIO_PALY_3,
} AUDIO_PLAY_CNT;

typedef enum{
    AUDIO_PLAY_BLOCK,        //�����ȴ�������
    AUDIO_PLAY_NOBLOCK,      //������
}AUDIO_PLAY_TYPE;

typedef enum{
    VOL_CFG_SOURCE_WEB,    //��̨����
    VOL_CFG_SOURCE_KEY     //��������
}AUDIO_VOL_CFG_SOURCE_E;

typedef enum{
    AP_STEP_NEW_INSERT=1,        //�ⲿ�в������ݲ���
    AP_STEP_CHECK_PLAY_RES,    //��ⲥ�Ž��
    AP_STEP_SLEEP_CHECK,       //���߽��ȷ��
    AP_STEP_SLEEP_REPLAY,      //ѭ������
    AP_STEP_IDLE               //�������
}AUDIO_PLAY_STEP_E;

typedef struct{
    s8 m_play_string[128];     //���ŵ��ַ���
    s32 m_playtimes;            //���ŵĴ���
    s32 m_have_playtimes;
    s32 m_interval_time;           //���ż��ʱ�䣬��λ��
    u32 m_cur_ticks;
    AUDIO_PLAY_STEP_E m_play_step;
}audio_play_info_t;

typedef struct{
    model_device_t super;
    s32 m_vol;                      //������С
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
//�����Ƶ��Դ���ļ�
typedef struct{
    u8 m_file_name[128];//download �ļ���
    u32 m_file_index;//download �ļ��ڼ�����Դ
    u8  m_file_flg;  //0-���أ�1-����
    u32 m_file_offset;//download �ļ���Դλ��offset
}audio_file_config;

#define AUDIO_SOURCE_PATH      "/mtd0/audio" //�ͻ�������Դ

#define AUDIO_MAX_TEXT_NAME_LEN 128 //�ͻ��ļ�������
#define AUDIO_MAX_TEXT_DOWNLOAD_FLG 1//�ļ�����״̬
#define AUDIO_MAX_TEXT_FILE_CNT 50  //50k
#define AUDIO_MAX_TEXT_FILE_LEN AUDIO_MAX_TEXT_FILE_CNT*1024//�ͻ��ļ����ݳ���
#define AUDIO_MAX_NUM   10  //�ͻ����Audio����


#define audio_filename_addr(n)      (n*(AUDIO_MAX_TEXT_NAME_LEN+AUDIO_MAX_TEXT_DOWNLOAD_FLG+AUDIO_MAX_TEXT_FILE_LEN))
#define audio_filedownflg_addr(n)   (audio_filename_addr(n)+AUDIO_MAX_TEXT_NAME_LEN)
#define audio_filedata_addr(n)      (audio_filedownflg_addr(n)+AUDIO_MAX_TEXT_DOWNLOAD_FLG)


#endif

audio_module_t *audio_instance(void);
void trendit_init_audio_data(void);



#endif

