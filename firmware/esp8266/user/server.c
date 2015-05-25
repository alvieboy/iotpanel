#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "driver/gpio16.h"
#include "driver/spi_master.h"
#include "user_interface.h"
#include "espconn.h"
#include <stdio.h>

LOCAL void ICACHE_FLASH_ATTR server_recv(void *arg, char *pusrdata, unsigned short length)
{
    struct espconn *pesp_conn = arg;
}

LOCAL ICACHE_FLASH_ATTR void server_recon(void *arg, sint8 err)
{
    struct espconn *pesp_conn = arg;
}

LOCAL ICACHE_FLASH_ATTR void server_discon(void *arg)
{
    struct espconn *pesp_conn = arg;
}



LOCAL void ICACHE_FLASH_ATTR server_listen(void *arg)
{
    struct espconn *pesp_conn = arg;

    espconn_regist_recvcb(pesp_conn, server_recv);
    espconn_regist_reconcb(pesp_conn, server_recon);
    espconn_regist_disconcb(pesp_conn, server_discon);
}
void ICACHE_FLASH_ATTR user_server_init(uint32 port)
{
     struct espconn esp_conn;
     esp_tcp esptcp;

    esp_conn.type = ESPCONN_TCP;
    esp_conn.state = ESPCONN_NONE;
    esp_conn.proto.tcp = &esptcp;
    esp_conn.proto.tcp->local_port = port;
    espconn_regist_connectcb(&esp_conn, server_listen);

#ifdef SERVER_SSL_ENABLE
    espconn_secure_accept(&esp_conn);
#else
    espconn_accept(&esp_conn);
#endif
}
