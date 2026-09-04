
//Necessary imports
#include <iostream>


#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "lwip/sockets.h"

#include "dig_IO.h"
#include "spi_DAQ.h"


#define WIFI_PASSWORD   "dummyPassword"
#define WIFI_SSID  "Main SLAPDAQ"

static const char* TAG = "esp-test";



#ifdef __cplusplus
extern "C" {
#endif


/*Function Declaration*/
void wifi_initialize(void);

#ifdef __cplusplus
}
#endif
