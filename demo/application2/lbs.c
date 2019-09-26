
#include "lbs.h"

#include "eat_modem.h"
#include "eat_timer.h"
#include "eat_interface.h"

#define PERIOD_SEND_LBS 60000

typedef struct AtCmdEntityTag
{
    u8* p_atCmdStr;
    u16 cmdLen;
    u32 delay;
//    AtCmdRspCB p_atCmdCallBack;
}AtCmdEntity;

AtCmdEntity at_cmd_table[] = {
    {"AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n", 31, 1000}, // 0
    {"AT+SAPBR=3,1,\"APN\",\"CMIOT\"\r\n", 28, 1000}, // 1
    {"AT+SAPBR=1,1\r\n", 14, 3 * 1000}, // 2
    {"AT+CLBS=4,1\r\n", 13, PERIOD_SEND_LBS}, // 3
    {"AT+CLBS=4,1\r\n", 13, PERIOD_SEND_LBS}, // 4
    {"AT+CLBS=4,1\r\n", 13, PERIOD_SEND_LBS}, // 5
    {"AT+CLBS=4,1\r\n", 13, PERIOD_SEND_LBS}, // 6
    {"AT+CLBS=4,1\r\n", 13, PERIOD_SEND_LBS}, // 7
    {"AT+CLBS=4,1\r\n", 13, PERIOD_SEND_LBS}, // 8
    {"AT+CLBS=4,1\r\n", 13, PERIOD_SEND_LBS}, // 9
    {"AT+CLBS=4,1\r\n", 13, PERIOD_SEND_LBS}, // 10
    {"AT+CLBS=4,1\r\n", 13, PERIOD_SEND_LBS}, // 11
    {"AT+CLBS=4,1\r\n", 13, 1 * 1000}, // 12
    {"AT+SAPBR=0,1\r\n", 14, PERIOD_SEND_LBS} // 13
};

void lbs_next(u32 *delay) {
    static u8 index = 0;
    eat_trace("[TRACE] lbs_next() %s", at_cmd_table[index]);
    eat_modem_write(at_cmd_table[index].p_atCmdStr, at_cmd_table[index].cmdLen);
    *delay = at_cmd_table[index].delay;
    index++;
    if (index % 14 == 0) {
        index = 0;
    }
}
