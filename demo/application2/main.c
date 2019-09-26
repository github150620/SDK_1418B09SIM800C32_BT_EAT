
#include <string.h>

#include "eat_clib_define.h"

#include "eat_interface.h"
#include "eat_uart.h"
#include "eat_timer.h"
#include "eat_type.h"

#include "at.h"
#include "tcp.h"
#include "lbs.h"

#define EAT_UART_RX_BUF_LEN_MAX 2048

#define PERIOD_SEND_LBS      60000
#define PERIOD_SEND_BATTERY 300000

typedef void (*app_user_func)(void*);

char iccid[30] = "+ICCID: 12345678901234567890\n";

eat_bool parse_cgatt = EAT_TRUE;

eat_bool status_ate = EAT_TRUE;
eat_bool status_rdy = EAT_FALSE;
eat_bool status_call_ready = EAT_FALSE;
eat_bool status_sms_ready = EAT_FALSE;
u8 status_cgatt = 0;

extern void APP_InitRegions(void);

void app_main(void *data);
void app_func_ext1(void *data);

#pragma arm section rodata = "APP_CFG"
APP_ENTRY_FLAG
#pragma arm section rodata

#pragma arm section rodata = "APPENTRY"
const EatEntry_st AppEntry = {
	app_main,
	app_func_ext1,
	(app_user_func)EAT_NULL,  // 1
	(app_user_func)EAT_NULL,  // 2
	(app_user_func)EAT_NULL,  // 3
	(app_user_func)EAT_NULL,  // 4
	(app_user_func)EAT_NULL,  // 5
	(app_user_func)EAT_NULL,  // 6
	(app_user_func)EAT_NULL,  // 7
	(app_user_func)EAT_NULL,  // 8
	EAT_NULL,
	EAT_NULL,
	EAT_NULL,
	EAT_NULL,
	EAT_NULL,
	EAT_NULL
};
#pragma arm section rodata

void app_func_ext1(void *data) {
	/*This function can be called before Task running ,configure the GPIO,uart and etc.
	   Only these api can be used:
		 eat_uart_set_debug: set debug port
		 eat_pin_set_mode: set GPIO mode
		 eat_uart_set_at_port: set AT port
	*/
    EatUartConfig_st cfg = {
        EAT_UART_BAUD_115200,
        EAT_UART_DATA_BITS_8,
        EAT_UART_STOP_BITS_1,
        EAT_UART_PARITY_NONE
    };
	eat_uart_set_debug(EAT_UART_1);
	eat_uart_set_debug_config(EAT_UART_DEBUG_MODE_UART, &cfg);
	eat_uart_set_at_port(EAT_UART_NULL);
}

void timer_handler(EatTimer_st *timer) {
	switch (timer->timer_id)
	{
	case EAT_TIMER_1:
		eat_trace("[TRACE] EAT_TIMER_1");
		if (status_cgatt == 0 && eat_network_get_cgatt() == 1) {
			status_cgatt = 1;
			gprs_init();
		}
		eat_timer_start(timer->timer_id, 2 * 1000);
		break;
	case EAT_TIMER_2:
		eat_trace("[TRACE] EAT_TIMER_2");
		{
			u32 delay = 0;
			lbs_next(&delay);
			eat_timer_start(timer->timer_id, delay);
		}
		break;
	case EAT_TIMER_3:
		eat_trace("[TRACE] EAT_TIMER_3");
		eat_modem_write("AT+CBC\r\n", 8);
		eat_timer_start(timer->timer_id, PERIOD_SEND_BATTERY);
		break;
	case EAT_TIMER_4:
		eat_trace("[TRACE] EAT_TIMER_4");
		eat_timer_start(timer->timer_id, 30 * 1000);
		break;
	default:
		break;
	}
}

static u8 buf[2048];

void app_main(void *data) {

    u16 len = 0;

	EatEvent_st event;

    APP_InitRegions();
    APP_init_clib();

	eat_timer_start(EAT_TIMER_1, 2 * 1000);
	eat_timer_start(EAT_TIMER_2, 30 * 1000);
	eat_timer_start(EAT_TIMER_3, PERIOD_SEND_BATTERY);

	while (EAT_TRUE) {
		eat_get_event(&event);
		switch (event.event) {
		case EAT_EVENT_TIMER:
			timer_handler(&event.data.timer);
			break;
		case EAT_EVENT_MDM_READY_RD:
			len = eat_modem_read(buf, 2000);
			if(len > 0) {
				buf[len] = 0;
				eat_uart_write(EAT_UART_1, buf, strlen((char*)buf));
				at_cmd_handler(buf);
			}
			break;
		case EAT_EVENT_MDM_READY_WR:
			break;
		case EAT_EVENT_UART_READY_RD:
			if (event.data.uart.uart == EAT_UART_1) {
				len = 0;
				len = eat_uart_read(EAT_UART_1, buf, 2048);
				if(len > 0) {
					buf[len] = 0;
					eat_modem_write(buf, strlen((char*)buf));
				}
			}
			break;
		case EAT_EVENT_UART_SEND_COMPLETE:
			break;
		default:
			break;
		}
	}
}

