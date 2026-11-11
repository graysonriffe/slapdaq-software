
#ifndef WIFI_H
#define WIFI_H

//Necessary imports
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>

#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "lwip/sockets.h"


#define WIFI_PASSWORD   "dummyPassword"
#define WIFI_SSID       "Main SLAPDAQ"
#define WIFI_TCP_PORT   15000




#ifdef __cplusplus
extern "C" {
#endif


/*Function Declaration*/
bool wifi_initialize(void);

// true while a host is connected
bool wifi_client_connected(int timeout_ms);

// sends all 'len' bytes (handles partial reads) | closes the client and returns false if there is an error
bool wifi_send(const char *data, size_t len);

// stream non-blocking receive: > 0 bytes then read, 0 nothing so wait, -1 theres no client
int wifi_receive(uint8_t *buf, size_t len);

// read function to obtain a full packet
bool wifi_receive_packet(uint8_t *pkt, size_t len);

#ifdef __cplusplus
}
#endif

#endif
