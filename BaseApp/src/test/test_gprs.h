#ifndef __TEST_GPRS_H
#define __TEST_GPRS_H

#define AT_CMD_REQ_END             "\r"
#define AT_CMD_RES_END             "\r\n"

#define AT_CMD_CODE_OK             "OK"
#define AT_CMD_CODE_ERROR          "ERROR"
#define AT_CMD_CODE_SIM_READY      "READY"
#define WIRE_AT_DEFAULT_TIMEOUT    3000
#define WIRE_AT_REG_NET_TIMEOUT    30000


s32 test_gprs(void);
s32 wireless_8960_test(void);

#endif


