#include "at.h"

#include <stdio.h>
#include <string.h>

#include "eat_type.h"
#include "eat_interface.h"

#include "tcp.h"

extern char iccid[30];

static u8 buf[512];

void at_cmd_handler(u8 *buffer) {

    int i;
    int ret;
    int len;
    char *p = buffer;

    if (p[0] != '\r' || p[1] != '\n') {
        eat_trace("[ERROR] Head CR LF error.");
        return;
    }

    p = p + 2;

    for(i = 0; i < sizeof(buf) - 2; i++){
        if (p[i] == '\r' && p[i+1] == '\n') {
            break;
        }
        buf[i] = p[i];
    }

    if (i == sizeof(buf) - 2) {
        eat_trace("[ERROR] Tail CR LF error.");
        return;
    }

    buf[i] = '\n';
    buf[i+1] = '\0';

    if (strncmp(buf, "OK", 2) == 0) {

    } else if (strncmp(buf, "ERROR", 5) == 0) {

    } else if (strncmp(buf, "+CLBS:", 6) == 0) {
        //sscanf(buffer, "+CLBS: %d,%f,%f,%d,%s,%s", );
        eat_trace("[TRACE] +CLBS");
        tcp_send(buf, strlen(buf));

    } else if (strncmp(buf, "+CBC:", 5) == 0) {

        eat_trace("[TRACE] +CBC");
        tcp_send(buf, strlen(buf));

    } else if (strncmp(buf, "+CREG:", 6) == 0) {
        int param1;
        int param2;
        char param3[8];
        char param4[8];
        ret = sscanf(buf, "+CREG: %d,%d,%s,%s", &param1, &param2, param3, param4);
        if (ret == 0) {
            eat_trace("[ERROR] sscanf() return %d", ret);
            return;
        }
        eat_trace("[DEBUG] %d,%d,%s,%s", param1, param2, param3, param4);
        if (param1 == 2 && param2 == 1) {
            eat_trace("[TRACE] > AT+CLBS=4,1");
            eat_modem_write("AT+CLBS=4,1\r\n", 13);
        }

    } else if (strncmp(buf, "+CGREG:", 7) == 0) {

    } else if (strncmp(buf, "+CGATT:", 7) == 0) {

        u8 param1 = 255;
        sscanf(buf, "+CGATT: %d", &param1);
        eat_trace("[DEBUG] cgatt=%d", param1);

    } else if (strncmp(buf, "+SAPBR:", 7) == 0) {

    } else if (strncmp(buf, "+CLSCFG:", 8) == 0) {

    } else if (strncmp(buf, "+ICCID:", 7) == 0) {

        strncpy(iccid, buf, sizeof(iccid));

    } else if (strncmp(buf, "+COPS:", 6) == 0) {

    } else if (strncmp(buf, "Call Ready", 10) == 0) {

    } else if (strncmp(buf, "SMS Ready", 9) == 0) {

    } else if (strncmp(buf, "+CPIN:", 6) == 0) {

        char cpin[10];
        ret = sscanf(buf, "+CPIN: %s", cpin);
        if (strncmp(cpin, "READY", 5) != 0) {
            eat_trace("[ERROR] SIM card is NOT ready.");
            return;
        }
        eat_modem_write("AT+ICCID\r\n", 10);

    } else if (strncmp(buf, "+CFUN:", 6) == 0) {

    } else if (strncmp(buf, "RDY", 3) == 0) {
		eat_modem_write("ATE0\r\n", 6);
    } else {
        eat_trace("[INFO] else");
    }
}


