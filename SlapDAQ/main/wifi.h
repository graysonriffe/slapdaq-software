
#ifndef WIFI_H
#define WIFI_H

//Necessary imports
#include <iostream>


#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
//#include "esp_mac.h"
#include "esp_netif.h"
#include "lwip/sockets.h"

#include "dig_IO.h"
#include "spi_DAQ.h"


#define WIFI_PASSWORD   "dummyPassword"
#define WIFI_SSID  "Main SLAPDAQ"

static const char* WIFI_TAG = "esp-test";



#ifdef __cplusplus
extern "C" {
#endif


/*Function Declaration*/
bool wifi_initialize(void);
bool wifi_client_connected(int timeout_ms);
bool wifi_send(const char *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif
