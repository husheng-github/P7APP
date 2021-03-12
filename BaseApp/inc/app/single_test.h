#ifndef __SINGLE_TEST_H
#define __SINGLE_TEST_H

#define TIMEOUT_TIME 1000*60 //60S��ʱ�˳�

typedef struct{
    s32 result;
    s8 des[24];
}auto_test_res_t;

typedef struct{
    u8 flag[2];
    u8 led;  //0 δ���ԣ�1���Գɹ���2 ����ʧ�ܡ�3��֧�֡�
    u8 lcd;
    u8 beeper;
    u8 key;
    u8 magcard;    
    u8 iccard;
    u8 rfcard;
    u8 spiflash;
    u8 bt;
    u8 wifi;
    u8 printer;
    u8 gprs;
    u8 rtc;
    u8 camera;
    u8 tp;
    u8 sleep;
    u8 audio;
    u8 cashbox;
    u8 sencond_test;
    u8 audiofilesyncresult;
    u8 reserved[56-8-22];
    u8 pcbatestflag[8];  //�ܹ�֧��56���ֽڣ����ܳ���56���ֽ�
}TRENDITDEV_TEST_INFO;

typedef enum
{
    DEVICE_TEST_MODE_INTEGER = 0,
    DEVICE_TEST_MODE_SMT,
    DEVICE_TEST_MODE_AGING,
    DEVICE_TEST_MODE_SINGLE
}DEVICE_TEST_MODE;

typedef enum
{
    TTSFILE_SYNC_NONE,
    TTSFILE_SYNC_SUCCESS,
    TTSFILE_SYNC_FAIL
}TTSFILE_SYNC_RESULT;

#define DEVICE_TEST_SUCCESS         "���Գɹ�"
#define DEVICE_TEST_FAIL            "����ʧ��"


#define TEST_HANDLER_MAX    4
typedef s32 (*single_test_handler)(TRENDITDEV_TEST_INFO *devtestinfo, u8 testmode);
typedef s32 (*test_handler)(TRENDITDEV_TEST_INFO *devtestinfo, u8 testmode);


s32 single_test_led(TRENDITDEV_TEST_INFO *devtestinfo, u8 testmode);

s32 single_test_audio(TRENDITDEV_TEST_INFO *devtestinfo, u8 testmode);
s32 single_test_key(TRENDITDEV_TEST_INFO *devtestinfo, u8 testmode);
s32 single_test_gprs(TRENDITDEV_TEST_INFO *devtestinfo, u8 testmode);
s32 single_test_wifi(TRENDITDEV_TEST_INFO *devtestinfo, u8 testmode);
s32 single_test_printer(TRENDITDEV_TEST_INFO *devtestinfo, u8 testmode);

void trendit_factory_test(void);
void test_savettsfilesyncresult(TTSFILE_SYNC_RESULT ttsfilesyncresult);
u8 test_readttsfilesyncresult(void);


#endif
