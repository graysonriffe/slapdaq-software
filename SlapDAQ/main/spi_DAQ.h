
//Necessary Imports
/*SPI Includes*/
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "sdkconfig.h"
#include "hal/spi_types.h"

/*SPI Variables*/
#define GPIO_SPI_MOSI 11
#define GPIO_SPI_MISO 13
#define GPIO_CS_THERMO 10
#define GPIO_CS_ADC1 8
#define GPIO_CS_ADC2 9
#define GPIO_SCLK 12



#ifdef __cplusplus
extern "C" {
#endif

/*Global Necessary Variables*/
extern spi_device_handle_t spi_thermo_handle;
extern spi_device_handle_t spi_adc1_handle;
extern spi_device_handle_t spi_adc2_handle;


/*Function Declaration*/
void SPI_Init(void);
esp_err_t spi_transfer(spi_device_handle_t dev, const uint8_t *tx_data, uint8_t *rx_data, size_t len_bytes);
void thermo_read();

#ifdef __cplusplus
}
#endif
