
#include "tcp.h"

#include "eat_type.h"
#include "eat_network.h"
#include "eat_socket.h"
#include "eat_interface.h"

// 123.56.139.45:9999
#define SERVER_ADDR1  123
#define SERVER_ADDR2  56
#define SERVER_ADDR3  139
#define SERVER_ADDR4  45
#define SERVER_PORT 9999

extern char iccid[30];

static s8 socket_id = -1;

eat_bear_notify bear_notify_cb(cbm_bearer_state_enum state, u8 ip_addr[4]) {
    switch (state)
    {
    case CBM_DEACTIVATED:
        eat_trace("[TRACE] CBM_DEACTIVATED");
        break;
    case CBM_ACTIVATING:
        eat_trace("[TRACE] CBM_ACTIVATING");
        break;
    case CBM_ACTIVATED:
        eat_trace("[TRACE] CBM_ACTIVATED");
        eat_trace("[INFO] IP: %d.%d.%d.%d", ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
        tcp_connect();
        break;
    case CBM_DEACTIVATING:
        eat_trace("[TRACE] CBM_DEACTIVATING");
        break;
    case CBM_CSD_AUTO_DISC_TIMEOUT:
        eat_trace("[TRACE] CBM_CSD_AUTO_DISC_TIMEOUT");
        break;
    case CBM_GPRS_AUTO_DISC_TIMEOUT:
        eat_trace("[TRACE] CBM_GPRS_AUTO_DISC_TIMEOUT");
        break;    
    case CBM_NWK_NEG_QOS_MODIFY:
        eat_trace("[TRACE] CBM_NWK_NEG_QOS_MODIFY");
        break;    
    case CBM_WIFI_STA_INFO_MODIFY:
        eat_trace("[TRACE] CBM_WIFI_STA_INFO_MODIFY");
        break;    
    default:
        eat_trace("[DEBUG] state: 0x%x", state);
        break;
    }
}

static u8 buf[1024];

eat_soc_notify soc_notify_cb(s8 s,soc_event_enum event,eat_bool result, u16 ack_size) {
    s32 ret;
    switch (event)
    {
    case SOC_CONNECT:
        eat_trace("[TRACE] SOC_CONNECT");
        if (result == EAT_FALSE) {
            eat_trace("connect failed.");
            eat_soc_close(s);
            socket_id = -1;
            break;
        }
        eat_soc_send(s, iccid, strlen(iccid));
        break;
    case SOC_READ:
        eat_trace("[TRACE] SOC_READ");
        ret = eat_soc_recv(s, buf, sizeof(buf));
        if (ret <= 0) {
            eat_soc_close(s);
            socket_id = -1;
        } else {
            buf[ret] = '\0';
            eat_trace("eat_soc_recv: %s", buf);
        }
        break;
    case SOC_WRITE:
        eat_trace("[TRACE] SOC_WRITE");
        if (result == EAT_FALSE) {
            eat_soc_close(s);
            socket_id = -1;
            break;
        }    
        break;
    case SOC_CLOSE:
        eat_trace("[TRACE] SOC_CLOSE");
        eat_soc_close(s);
        socket_id = -1;
        break;
    default:
        break;
    }
}

void gprs_init(void) {
    s8 ret;

    eat_trace("[TRACE] eat_gprs_bearer_open()...");
    ret = eat_gprs_bearer_open("CMIOT", NULL, NULL, bear_notify_cb);
    if (ret != CBM_OK && ret != CBM_WOULDBLOCK) {
        eat_trace("[ERROR] eat_gprs_bearer_open() return %d", ret);
        eat_sleep(10 * 1000);
        return;
    }

    eat_trace("[TRACE] eat_gprs_bearer_hold()...");
    ret = eat_gprs_bearer_hold();
    if (ret != CBM_OK) {
        eat_trace("[ERROR] eat_gprs_bearer_hold() return %d", ret);
        eat_sleep(10 * 1000);
        return;
    }
}

s8 tcp_connect(void) {

    s8 ret;

    u8 val;
    sockaddr_struct address={0};

    eat_soc_notify_register(soc_notify_cb);

    socket_id = eat_soc_create(SOC_SOCK_STREAM, 0);
    if (socket_id < 0) {
        eat_trace("[ERROR] eat_soc_create() return %d", socket_id);
        return -1;
    }

    val = TRUE;
    ret = eat_soc_setsockopt(socket_id, SOC_NBIO, &val, sizeof(val));
    if (ret != SOC_SUCCESS) {
        eat_trace("[ERROR] eat_soc_setsockopt() SOC_NBIO return %d",ret);
        eat_soc_close(socket_id);
        socket_id = -1;
        return -1;
    }

    val = (SOC_READ | SOC_WRITE | SOC_CLOSE | SOC_CONNECT);
    ret = eat_soc_setsockopt(socket_id,SOC_ASYNC,&val,sizeof(val));
    if (ret != SOC_SUCCESS) {
        eat_trace("[ERROR] eat_soc_setsockopt SOC_ASYNC return %d",ret);
        eat_soc_close(socket_id);
        socket_id = -1;
        return -1;
    }

    address.sock_type = SOC_SOCK_STREAM;
    address.addr_len = 4;
    address.port = SERVER_PORT;
    address.addr[0] = SERVER_ADDR1;
    address.addr[1] = SERVER_ADDR2;
    address.addr[2] = SERVER_ADDR3;
    address.addr[3] = SERVER_ADDR4;
    ret = eat_soc_connect(socket_id, &address);
    if (ret < 0 && ret != SOC_WOULDBLOCK) {
        eat_trace("[WARN] eat_soc_connect() return %d", ret);
        eat_soc_close(socket_id);
        socket_id = -1;
        return -1;
    }
    return 0;
}

s8 tcp_send(u8 *buf, s32 len) {
    s32 ret;
    eat_trace("[TRACE] tcp_send()");
    if (socket_id < 0) {
        eat_trace("[TRACE] socket_id < 0 , tcp_connect()");
        tcp_connect();
        return -1;
    }
    ret = eat_soc_send(socket_id, buf, len);
    if (ret < 0 && ret != SOC_WOULDBLOCK) {
        eat_trace("[TRACE] socket_id < 0 , tcp_connect()");
        return -1;
    }
    return 0;
}



