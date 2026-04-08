
/*Logging includes*/
#define LOG_LEVEL_LOCAL ESP_LOG_VERBOSE
#include "esp_log.h"
#define LOG_TAG "MAIN"

/*FreeRTOS includes*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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
#define GPIO_CS_ADC1 1
#define GPIO_CS_ADC2 2
#define GPIO_SCLK 12

/*SPI Configuration Variables*/
esp_err_t ret;
spi_device_handle_t spi_handle;


/*Digital GPIO Variables*/



/*Function Declarations*/
extern "C" void SPI_Init(void);
extern "C" void spi_read_data(uint8_t address);
extern "C" void spi_write_data(uint8_t address, uint8_t data);




extern "C" void app_main(void)
{

    /*Initialize SPI*/
    SPI_Init();

    if (ret == ESP_OK)
    {
        ESP_LOGI(LOG_TAG, "SPI has been initialized");
    }


    
    


}


extern "C" void SPI_Init (void)
{

    //empty struct first
    spi_bus_config_t busconfig = {};

    //Configuring the SPI bus
    busconfig.mosi_io_num = GPIO_SPI_MOSI,
    busconfig.miso_io_num = GPIO_SPI_MISO,
    busconfig.sclk_io_num = GPIO_SCLK,
    busconfig.quadwp_io_num = -1,
    busconfig.quadhd_io_num = -1,
    busconfig.max_transfer_sz = 4092;       
 

    //Initialize the SPI bus (SPI2_HOST)
    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &busconfig, SPI_DMA_CH_AUTO);

    ESP_ERROR_CHECK(ret); // check for error in intialization

    /*
    -------------------------------------------------------
    Configuring ADC1 SPI (Thermocouple, Single Analog, Differential)
    -------------------------------------------------------
    */


    //empty device struct first
    spi_device_interface_config_t devconfig_thermo = {};

    //Adding a device to the bus
    devconfig_thermo.clock_speed_hz = 4000000, // 5 MHz is max serial clock freq
    devconfig_thermo.mode = 0,
    devconfig_thermo.spics_io_num = GPIO_CS_THERMO,
    devconfig_thermo.queue_size = 3; //double check this value


    spi_device_handle_t spi_thermo_handle;
    ret = spi_bus_add_device(SPI2_HOST, &devconfig_thermo, &spi_thermo_handle);

    ESP_ERROR_CHECK(ret);

    /*
    -------------------------------------------------------
    Configuring ADC2 SPI (Current Sensing Loops, Reference Voltage)
    -------------------------------------------------------
    */


    //empty device struct first
    spi_device_interface_config_t devconfig_adc1 = {};

    //Adding a device to the bus
    devconfig_adc1.clock_speed_hz = 10000000, // ADC has around 10 MHz typical serial clock frequency
    devconfig_adc1.mode = 0,
    devconfig_adc1.spics_io_num = GPIO_CS_ADC1,
    devconfig_adc1.queue_size = 3;  //double check this value


    spi_device_handle_t spi_adc1_handle;
    ret = spi_bus_add_device(SPI2_HOST, &devconfig_adc1, &spi_adc1_handle);

    ESP_ERROR_CHECK(ret);

    /*
    -------------------------------------------------------
    Configuring Differential-Analog ADC SPI (DEVICE3)
    -------------------------------------------------------
    */


    //empty device struct first
    spi_device_interface_config_t devconfig_adc2 = {};

    //Adding a device to the bus
    devconfig_adc2.clock_speed_hz = 10000000, 
    devconfig_adc2.mode = 0,
    devconfig_adc2.spics_io_num = GPIO_CS_ADC2,
    devconfig_adc2.queue_size = 3;  //double check this value


    spi_device_handle_t spi_adc2_handle;
    ret = spi_bus_add_device(SPI2_HOST, &devconfig_adc2, &spi_adc2_handle);

    ESP_ERROR_CHECK(ret);


}