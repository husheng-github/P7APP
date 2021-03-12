#ifndef __USBCDC_DOWNLOAD_H
#define __USBCDC_DOWNLOAD_H

#if 1
#define DL_DEBUG     TRACE_DBG
#define DL_DEBUGHEX  TRACE_DBG_HEX
#else
#define DL_DEBUG     
#define DL_DEBUGHEX   
#endif

typedef enum _DOWNLOAD_FILE_TYPE
{

    DOWNLOAD_FILE_TYPE_BOOTBIN,
    DOWNLOAD_FILE_TYPE_COREBIN,
    DOWNLOAD_FILE_TYPE_PCICOREBIN,
    DOWNLOAD_FILE_TYPE_APPBIN,
    DOWNLOAD_FILE_TYPE_TREDNITPK,
    DOWNLOAD_FILE_TYPE_ACQUIRERPK,
    DOWNLOAD_FILE_TYPE_AUTHEN1PK,
    DOWNLOAD_FILE_TYPE_AUTHEN2PK,
    DOWNLOAD_FILE_TYPE_AUTHEN3PK,
    DOWNLOAD_FILE_TYPE_AUTHEN4PK,
    DOWNLOAD_FILE_TYPE_PREVENTPK,
    DOWNLOAD_FILE_TYPE_VFSCONFIG,
    DOWNLOAD_FILE_TYPE_RESOURCE,
    DOWNLOAD_FILE_TYPE_RESERVED,
    DOWNLOAD_FILE_TYPE_UPDATEBAK,
    DOWNLOAD_FILE_TYPE_NULL,

}BOOTDOWNLOAD_FILE_TYPE;


#define MAXFILE_NUM  6   //一次最多可以下载30个文件
#define MAXFILENAMELEN  20
typedef struct _CANDOWNFILEINFO
{
    u8  m_filename[MAXFILENAMELEN];//文件名
    u8  m_filetype;
    s32 m_fileaddr;       //下载的地址
    u32 m_maxfilesize;
}CANDOWNFILEINFO_t;

typedef struct _DOWNFILEINFO
{
    u32 m_filelen;
    u32 m_flg;
    u32 m_crc;
    u32 m_addr;
    u32 m_failflg;      //出错标志，0：没错,
    u8 m_filetype;
    u8 m_filename[51];
}DOWNFILEINFO_t;

typedef struct _FILEBAGINFO
{
    u8 m_filenum;       //file number need to download
    u8 m_bagno;         //current download file bag NO.
    u8 m_bagnosum;      //current download file total bag number
    u8 m_machtype;      //machine type
    u8 m_hardver;       //hardware version
    u8 m_filename[51];  //file name must not overflow 50 bytes
    u8 m_filetype;      //file type
    u32 m_filelen;       //current download file length
    u32 m_crc;            //file crc
    u32 m_offset;         //offset of the current download bag in file
    u8  *m_p;           //pointer to download file data
}FILEBAGINFO_t;



#define PROMPT_DOWNLOADING         "DOWNLOADING..."



#define PROMPT_EXE_OTHER           "Other fault"
#define EXECUTION_MEMORY           1
#define PROMPT_EXE_MEMORY          "Memory not enough"
#define EXECUTION_HARDWAREVERSION  2
#define PROMPT_EXE_HARDWAREVERSION "Hardware not match!"
#define EXECUTION_DOWNFILENUM      3
#define PROMPT_EXE_DOWNFILENUM     "file number overflow!"
#define EXECUTION_DOWNFILEOPEN     4
#define PROMPT_EXE_DOWNFILEOPEN    "file open fail!"
#define EXECUTION_DOWNFILEOPS      5
#define PROMPT_EXE_DOWNFILEOPS     "operate file fail!"
#define EXECUTION_MACHINEMODE      6
#define PROMPT_EXE_MACHINEMODE     "model  not match!"
#define EXECUTION_DISKNOTENOUGH    7
#define PROMPT_EXE_DISKNOTENOUGH   "space not enough"
#define EXECUTION_DOWNFILENAME     8
#define PROMPT_EXE_DOWNFILENAME    "file name err!"
#define EXECUTION_CRCERR            9
#define PROMPT_EXE_CRCERR          "CRC err!"
#define DOWNLOADFILESUCCESS         10
#define PROMPT_DOWNLOADFILESUCCESS  "Download success!"
#define DOWNLOADAUTHENTICATE        11
#define PROMPT_DOWNLOADAUTHENTICATE "Authenticate..."
#define AUTHENTICATEFAIL            12
#define PROMPT_AUTHENTICATEFAIL     "Authenticate fail"
#define EXECUTION_SAVEINFOERROR     13
#define PROMPT_SAVEINFOERROR        "save file err"
#define EXECUTION_TRENDITPKNOTEXIST    14
#define PROMPT_TRENDITPKNOTEXIST        "TREN PK not exist"
#define EXECUTION_ACQUIRERPKAUTHENFAIL       15
#define PROMPT_ACQUIRERPKAUTHENFAIL          "acquirer pk fail"
#define EXECUTION_APPVERIFYFAIL     16
#define PROMPT_APPVERIFYFAIL        "app verify fail!"
#define EXECUTION_PKNOTSUPPORT      17
#define PROMPT_PKNOTSUPPORT         "PK file not support"
#define EXECUTION_FILEVERSIONERROR  18
#define PROMPT_FILEVERSIONERROR     "file version older"


#define EXECUTION_PREVENTTHELFTFAIL            18
#define EXECUTION_PREVENTTHELFTFAIL_CPUID      19
#define EXECUTION_PREVENTTHELFTFAIL_VERSION    20
#define EXECUTION_HANDSHAKEKEYNOTEXIST         21
#define EXECUTION_WAITINTODOWNLOAD             22


typedef enum
{
    UPDATE_STATUS_SUCCESS,   // 0
    UPDATE_STATUS_SAVEFAIL,   // 1
    UPDATE_STATUS_AUTHENFAIL,  // 2
    UPDATE_STATUS_CRCERR,     // 3
    UPDATE_STATUS_WRITEFAIL,  // 4
    UPDATE_STATUS_FILECRCERR,  // 5
    UPDATE_STATUS_FILELENERR,  // 6
    UPDATE_STATUS_FILENOTEXIST,  // 7
    UPDATE_STATUS_VERSIONOLDER,   // 8
    UPDATE_STATUS_SPCOMMUERR,    //  9
    UPDATE_STATUS_SPFIRMWARENOTEXIST,  // 10
    UPDATE_STATUS_BOOTUPDATEFAIL,     // 11
    UPDATE_STATUS_OTHERERR,          // 12
    UPDATE_STATUS_MEMERROR,          // 13
    UPDATE_STATUS_RESETUPDATE = 0xFF
}UPDATE_STATUS;



#endif
