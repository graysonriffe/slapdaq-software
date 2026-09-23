
#ifndef DIG_IO_H
#define DIG_IO_H

//necessary imports
/*GPIO Inclues*/
#include "driver/gpio.h"
#include "hal/spi_types.h"


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


#ifdef __cplusplus
extern "C" {
#endif

/*Function Declaration*/
void dig_io_config_to_input(void);
void update_dig_io_config(void);

#ifdef __cplusplus
}
#endif

#endif
