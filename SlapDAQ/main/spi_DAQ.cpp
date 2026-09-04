
//includes
#include "spi_DAQ.h"


void SPI_Init(void)
{
    //empty struct first
    spi_bus_config_t busconfig = {};

    //Configuring the SPI bus
    busconfig.mosi_io_num = GPIO_SPI_MOSI;
    busconfig.miso_io_num = GPIO_SPI_MISO;
    busconfig.sclk_io_num = GPIO_SCLK;
    busconfig.quadwp_io_num = -1;
    busconfig.quadhd_io_num = -1;
    busconfig.max_transfer_sz = 4092;

    //Initialize the SPI bus (SPI2_HOST)
    esp_err_t SPI_ret = spi_bus_initialize(SPI2_HOST, &busconfig, SPI_DMA_CH_AUTO);

    ESP_ERROR_CHECK(SPI_ret);  //check for error in initialization

    /*
    -------------------------------------------------------------------------------
    Configuring Thermocouple SPI 
    -------------------------------------------------------------------------------
    */

    //empty device struct first
    spi_device_interface_config_t devconfig_thermo = {};

    //configure device settings
    devconfig_thermo.clock_speed_hz = 4000000;
    devconfig_thermo.mode = 0;
    devconfig_thermo.spics_io_num = GPIO_CS_THERMO;
    devconfig_thermo.queue_size = 3; // how many transactions can be queued before time out ; relevant for asynchronous, may revisit in the future

    //Add device to the bus
    spi_device_handle_t spi_thermo_handle;
    SPI_ret = spi_bus_add_device(SPI2_HOST, &devconfig_thermo, &spi_thermo_handle);

    ESP_ERROR_CHECK(SPI_ret);



    /*
    -------------------------------------------------------------------------------
    Configuring ADC1 SPI
    -------------------------------------------------------------------------------
    */

    //empty device struct first
    spi_device_interface_config_t devconfig_adc1 = {};

    //configure device settings
    devconfig_adc1.clock_speed_hz = 10000000;
    devconfig_adc1.mode = 0;
    devconfig_adc1.spics_io_num = GPIO_CS_ADC1;
    devconfig_adc1.queue_size = 3;  //double check this value

    //Add device to the bus
    spi_device_handle_t spi_adc1_handle;
    SPI_ret = spi_bus_add_device(SPI2_HOST, &devconfig_adc1, &spi_adc1_handle);

    ESP_ERROR_CHECK(SPI_ret);



    /*
    -------------------------------------------------------------------------------
    Configuring ADC2 SPI 
    -------------------------------------------------------------------------------
    */

    //empty device struct first
    spi_device_interface_config_t devconfig_adc2 = {};

    //configure device settings
    devconfig_adc2.clock_speed_hz = 10000000;
    devconfig_adc2.mode = 0;
    devconfig_adc2.spics_io_num = GPIO_CS_ADC2;
    devconfig_adc2.queue_size = 3;  //double check this value

    //Add device to the bus
    spi_device_handle_t spi_adc2_handle;
    SPI_ret = spi_bus_add_device(SPI2_HOST, &devconfig_adc2, &spi_adc2_handle);

    ESP_ERROR_CHECK(SPI_ret);

}

esp_err_t spi_transfer(spi_device_handle_t dev, const uint8_t *tx_data, uint8_t *rx_data, size_t len_bytes)
{
    spi_transaction_t transaction = {};
    transaction.length = len_bytes * 8; // because spi_tranaction_t length parameter is in bits
    transaction.tx_buffer = tx_data;
    transaction.rx_buffer = rx_data;
    return spi_device_transmit(dev, &transaction);
}
