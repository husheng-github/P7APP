

#ifndef _DDI_MANAGE_H_
#define _DDI_MANAGE_H_


#include "ddi_common.h"
#include "ddi_misc.h"


#define POSSNFILENAME "/mtd2/posno"


s32 ddi_manage_get_hwsninfo(hwsn_info_t *lp_hwsn_info);
s32 ddi_manage_set_hwsninfo(u8 type, hwsn_info_t *lp_hwsn_info);
s32 ddi_manage_read_hwsn(u8 type, u8 *sn);

//extern s32 ddi_manage_read_dsn(SNType sntype, u8 *lpOut);
//extern s32 ddi_manage_get_firmwarever(FIRMWARETYPE firmwaretype, u8 *lpOut);
//extern s32 ddi_manage_probe_dev(u32 nDev);
extern s32 ddi_manage_download(u32 type);



#endif


