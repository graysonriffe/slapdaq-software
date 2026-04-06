
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
#define GPIO_CS_SINGLE 2
#define GPIO_CS_DIFFERENTIAL 3
#define GPIO_CS_CURRENT 4
#define GPIO_SCLK 12


/*Function Declarations*/
extern "C" int SPI_Init(void);




extern "C" void app_main(void)
{

    /*Initialize SPI*/
    SPI_Init();
    
    if (SPI_Init() == 0)
    {
        printf("Device is initialized");
    }


}


extern "C" int SPI_Init (void)
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

    if (ret != 0) 
    {
        printf("Device could not be initialized");
        return -1;
    }

    /*
    -------------------------------------------------------
    Configuring Thermocouple SPI (DEVICE1)
    -------------------------------------------------------
    */


    //empty device struct first
    spi_device_interface_config_t devconfig = {};

    //Adding a device to the bus
    devconfig.clock_speed_hz = 1000000, //look up requirement for this
    devconfig.mode = 0,
    devconfig.spics_io_num = GPIO_CS_THERMO,
    devconfig.queue_size = 3;


    spi_device_handle_t spi_thermo_handle;
    ret = spi_bus_add_device(SPI2_HOST, &devconfig, &spi_thermo_handle);

    if (ret != 0) 
    {
        printf("Thermocouple could not be added");
        return -1;
    }

    /*
    -------------------------------------------------------
    Configuring Single-Analog ADC SPI (DEVICE2)
    -------------------------------------------------------
    */


    //empty device struct first
    spi_device_interface_config_t devconfig = {};

    //Adding a device to the bus
    devconfig.clock_speed_hz = 1000000, //look up requirement for this
    devconfig.mode = 0,
    devconfig.spics_io_num = GPIO_CS_SINGLE,
    devconfig.queue_size = 3;


    spi_device_handle_t spi_single_handle;
    ret = spi_bus_add_device(SPI2_HOST, &devconfig, &spi_single_handle);

    if (ret != 0) 
    {
        printf("Single-Analog Voltage could not be added");
        return -1;
    }

    /*
    -------------------------------------------------------
    Configuring Differential-Analog ADC SPI (DEVICE3)
    -------------------------------------------------------
    */


    //empty device struct first
    spi_device_interface_config_t devconfig = {};

    //Adding a device to the bus
    devconfig.clock_speed_hz = 1000000, //look up requirement for this
    devconfig.mode = 0,
    devconfig.spics_io_num = GPIO_CS_DIFFERENTIAL,
    devconfig.queue_size = 3;


    spi_device_handle_t spi_differential_handle;
    ret = spi_bus_add_device(SPI2_HOST, &devconfig, &spi_differential_handle);

    if (ret != 0) 
    {
        printf("Differential Voltage could not be added");
        return -1;
    }

    /*
    -------------------------------------------------------
    Configuring Current ADC SPI (DEVICE4)
    -------------------------------------------------------
    */


    //empty device struct first
    spi_device_interface_config_t devconfig = {};

    //Adding a device to the bus
    devconfig.clock_speed_hz = 1000000, //look up requirement for this
    devconfig.mode = 0,
    devconfig.spics_io_num = GPIO_CS_CURRENT,
    devconfig.queue_size = 3;


    spi_device_handle_t spi_current_handle;
    ret = spi_bus_add_device(SPI2_HOST, &devconfig, &spi_current_handle);

    if (ret != 0) 
    {
        printf("Current Loop could not be added");
        return -1;
    }

    return 0;

}