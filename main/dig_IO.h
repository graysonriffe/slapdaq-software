
#ifndef DIG_IO_H
#define DIG_IO_H

//necessary imports
/*GPIO Inclues*/
#include "driver/gpio.h"
#include "hal/spi_types.h"

/*Relevant includes*/
#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

/*Digital GPIO Variables*/
#define GPIO_4 GPIO_NUM_4
#define GPIO_5 GPIO_NUM_5
#define GPIO_6 GPIO_NUM_6
#define GPIO_7 GPIO_NUM_7
#define GPIO_15 GPIO_NUM_15
#define GPIO_16 GPIO_NUM_16
#define GPIO_17 GPIO_NUM_17
#define GPIO_18 GPIO_NUM_18

#define GPIO_SEL_BITMASK ((1ULL<<GPIO_15) | (1ULL<<GPIO_7) | (1ULL<<GPIO_6) | (1ULL<<GPIO_5) | (1ULL<<GPIO_4) | (1ULL<<GPIO_16) | (1ULL<<GPIO_17) | (1ULL<<GPIO_18))

/*Create a look-up table for pin-to-bit mapping*/
static const gpio_num_t DIO_PINS[8] =
{
    GPIO_NUM_4, GPIO_NUM_5, GPIO_NUM_6, GPIO_NUM_7,
    GPIO_NUM_15, GPIO_NUM_16, GPIO_NUM_17,
    GPIO_NUM_18
};

/*

    TCP byte streams are like this:

    [type][mask]

    type 0x01 (MSG_SET_DIO_DIR):    bit n = 1 -> DIO_PINS[n] becomes an output, 0 -> input
    type 0x02 (MSG_SET_DIO_LEVEL):    bit n = 1 -> DIO_PINS[n] high, 0 -> low (only affects outputs)

*/

constexpr uint8_t MSG_SET_DIO_DIR = 0x01;
constexpr uint8_t MSG_SET_DIO_LEVEL = 0x02;
constexpr size_t DIO_PACKET_LEN = 2;


#ifdef __cplusplus
extern "C" {
#endif

/*Function Declaration*/
esp_err_t set_all_pins_as_input(void);
bool parseDioConfigPacket(const uint8_t *buf, size_t len, uint8_t *type, uint8_t *mask);
esp_err_t applyDioDirections(uint8_t dir_mask);
esp_err_t applyDioLevels(uint8_t level_mask);
uint8_t read_all_pins(void);

#ifdef __cplusplus
}
#endif

#endif
