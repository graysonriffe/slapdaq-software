
//necessary imports
/*GPIO Inclues*/
#include "driver/gpio.h"
#include "hal/spi_types.h"


/*Digital GPIO Variables*/
#define GPIO_21 GPIO_NUM_21
#define GPIO_7 GPIO_NUM_7
#define GPIO_6 GPIO_NUM_6
#define GPIO_5 GPIO_NUM_5
#define GPIO_4 GPIO_NUM_4
#define GPIO_2 GPIO_NUM_2
#define GPIO_1 GPIO_NUM_1
#define GPIO_14 GPIO_NUM_14

#define GPIO_SEL_BITMASK ((1ULL<<GPIO_21) | (1ULL<<GPIO_7) | (1ULL<<GPIO_6) | (1ULL<<GPIO_5) | (1ULL<<GPIO_4) | (1ULL<<GPIO_2) | (1ULL<<GPIO_1) | (1ULL<<GPIO_14))


#ifdef __cplusplus
extern "C" {
#endif

/*Function Declaration*/
void dig_io_config_to_input(void);
void update_dig_io_config(void);

#ifdef __cplusplus
}
#endif
