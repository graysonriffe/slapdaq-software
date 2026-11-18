#pragma once

#include <cstdint>

#include "driver/gpio.h"

#define WIFI_PASSWORD   "dummyPassword"
#define WIFI_SSID       "Main SLAPDAQ"
#define WIFI_TCP_PORT   15000

#define WIFI_LED_PIN    GPIO_NUM_1

/*Function Declaration*/
bool wifi_initialize(void);

// true while a host is connected
bool wifi_wait_for_connection();

// sends all 'len' bytes (handles partial reads) | closes the client and returns false if there is an error
bool wifi_send(const char *data, size_t len);

// stream non-blocking receive: > 0 bytes then read, 0 nothing so wait, -1 theres no client
int wifi_receive(uint8_t *buf, size_t len);

// read function to obtain a full packet
bool wifi_receive_packet(uint8_t *pkt, size_t len);
