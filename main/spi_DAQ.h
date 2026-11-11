
#ifndef SPI_DAQ_H
#define SPI_DAQ_H

//Necessary Imports
/*SPI Includes*/
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "sdkconfig.h"
#include "hal/spi_types.h"

/*General Includes*/
#include <stdint.h>
#include <stddef.h>

/*SPI Variables*/
#define GPIO_SPI_MOSI 11
#define GPIO_SPI_MISO 13
#define GPIO_CS_THERMO 8
#define GPIO_CS_ADC 10
#define GPIO_SCLK 12

#define SPI_THERMO_CLOCK_HZ     4000000     // MAX31856 max is 5 MHz
#define SPI_ADC_CLOCK_HZ       4000000     // test with 4 MHz for stability, raise to 10 MHz after tests pass

#ifdef __cplusplus
extern "C" {
#endif

/*Global Necessary Variables*/
extern spi_device_handle_t spi_thermo_handle;
extern spi_device_handle_t spi_adc_handle;



/*Function Declaration*/
esp_err_t SPI_Init(void);
esp_err_t spi_transfer(spi_device_handle_t dev, const uint8_t *tx_data, uint8_t *rx_data, size_t len_bytes);


#ifdef __cplusplus
}
#endif

#endif
