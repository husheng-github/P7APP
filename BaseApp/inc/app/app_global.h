#ifndef __APP_GLOBAL_H
#define __APP_GLOBAL_H

#include <stdarg.h>
#include "common.h"
#include "dlist.h"
#include "queue.h"
#include "postmessage.h"
#include "parse_data.h"
#include "tcp.h"
#include "event.h"
#include "machine_error.h"

#include "model_device.h"
#include "model_wireless.h"
#include "model_wifi.h"
#include "model_audio.h"
#include "model_printer.h"
#include "model_key.h"
#include "model_usbcdc.h"
#include "model_printport.h"
#include "model_tms.h"
#include "network_control_strategy.h"
#include "model_machine.h"
#include "usbcdc_download.h"
#include "process_trenbtcmd.h"
#include "device_config.h"
#include "single_test.h"
#include "http_download.h"
#include "tms_common.h"
#include "tms_schedule.h"
#include "tms_download.h"
#include "tms_upload.h"
#include "tms_upgrade.h"

#define IBOX
//#undef IBOX

#endif
