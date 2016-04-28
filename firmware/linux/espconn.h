#ifndef __ESPCONN_H__
#define __ESPCONN_H__

#ifdef HOST

#include "os_type.h"
#ifdef __linux__
#include <netinet/in.h>
#else
#include <windows.h>
#endif

#define ESPCONN_TCP IPPROTO_TCP
#define ESPCONN_NONE IPPROTO_NONE
typedef sint8 err_t;

typedef void *espconn_handle;
typedef void (* espconn_connect_callback)(void *arg);
typedef void (* espconn_reconnect_callback)(void *arg, sint8 err);

struct espconn_packet { uint32_t dummy; };
struct mdns_info { uint32_t dummy; };
typedef struct { uint32_t dummy; } remot_info;

/** A callback prototype to inform about events for a espconn */
typedef void (* espconn_recv_callback)(void *arg, char *pdata, unsigned short len);
typedef void (* espconn_sent_callback)(void *arg);

typedef uint32_t ip_addr_t;

typedef struct esp_tcp {
    int local_port;
} esp_tcp;


typedef struct espconn {
    int type;
    int state;
    union {
        esp_tcp *tcp;
    } proto;
    struct sockaddr_in sock;
    int sockfd;
    void*reverse;
} espconn;

sint8 espconn_connect(struct espconn *espconn);
sint8 espconn_disconnect(struct espconn *espconn);
sint8 espconn_delete(struct espconn *espconn);
sint8 espconn_accept(struct espconn *espconn);
sint8 espconn_create(struct espconn *espconn);
uint8 espconn_tcp_get_max_con(void);
sint8 espconn_tcp_set_max_con(uint8 num);
sint8 espconn_tcp_get_max_con_allow(struct espconn *espconn);
sint8 espconn_tcp_set_max_con_allow(struct espconn *espconn, uint8 num);
sint8 espconn_regist_time(struct espconn *espconn, uint32 interval, uint8 type_flag);
sint8 espconn_get_connection_info(struct espconn *pespconn, remot_info **pcon_info, uint8 typeflags);
sint8 espconn_get_packet_info(struct espconn *espconn, struct espconn_packet* infoarg);
sint8 espconn_regist_sentcb(struct espconn *espconn, espconn_sent_callback sent_cb);
sint8 espconn_regist_write_finish(struct espconn *espconn, espconn_connect_callback write_finish_fn);
sint8 espconn_sent(struct espconn *espconn, uint8 *psent, uint16 length);
sint8 espconn_regist_connectcb(struct espconn *espconn, espconn_connect_callback connect_cb);
sint8 espconn_regist_recvcb(struct espconn *espconn, espconn_recv_callback recv_cb);
sint8 espconn_regist_reconcb(struct espconn *espconn, espconn_reconnect_callback recon_cb);
sint8 espconn_regist_disconcb(struct espconn *espconn, espconn_connect_callback discon_cb);
uint32 espconn_port(void);
sint8 espconn_set_opt(struct espconn *espconn, uint8 opt);
sint8 espconn_clear_opt(struct espconn *espconn, uint8 opt);
sint8 espconn_set_keepalive(struct espconn *espconn, uint8 level, void* optarg);
sint8 espconn_get_keepalive(struct espconn *espconn, uint8 level, void *optarg);

typedef void (*dns_found_callback)(const char *name, ip_addr_t *ipaddr, void *callback_arg);

err_t espconn_gethostbyname(struct espconn *pespconn, const char *hostname, ip_addr_t *addr, dns_found_callback found);
sint8 espconn_secure_connect(struct espconn *espconn);
sint8 espconn_secure_disconnect(struct espconn *espconn);
sint8 espconn_secure_sent(struct espconn *espconn, uint8 *psent, uint16 length);
bool espconn_secure_set_size(uint8 level, uint16 size);
sint16 espconn_secure_get_size(uint8 level);
bool espconn_secure_ca_enable(uint8 level, uint8 flash_sector );
bool espconn_secure_ca_disable(uint8 level);
sint8 espconn_secure_accept(struct espconn *espconn);
sint8 espconn_igmp_join(ip_addr_t *host_ip, ip_addr_t *multicast_ip);
sint8 espconn_igmp_leave(ip_addr_t *host_ip, ip_addr_t *multicast_ip);
sint8 espconn_recv_hold(struct espconn *pespconn);
sint8 espconn_recv_unhold(struct espconn *pespconn);
void espconn_mdns_init(struct mdns_info *info);
void espconn_mdns_close(void);
void espconn_mdns_server_register(void);
void espconn_mdns_server_unregister(void);
char* espconn_mdns_get_servername(void);
void espconn_mdns_set_servername(const char *name);
void espconn_mdns_set_hostname(char *name);
char* espconn_mdns_get_hostname(void);
void espconn_mdns_disable(void);
void espconn_mdns_enable(void);
void espconn_dns_setserver(char numdns, ip_addr_t *dnsserver);



#endif

#endif
