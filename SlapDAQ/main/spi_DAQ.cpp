
//includes
#include "spi_DAQ.h"
#include "esp_log.h"

static const char *SPI_TAG = "SPI";

spi_device_handle_t spi_thermo_handle = nullptr;
spi_device_handle_t spi_adc_handle = nullptr;


esp_err_t SPI_Init(void)
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

    if (SPI_ret != ESP_OK)
    {
        ESP_LOGE(SPI_TAG, "spi_bus_initialize failed: %s", esp_err_to_name(SPI_ret));
        return SPI_ret;
    }

    /*
    -------------------------------------------------------------------------------
    Configuring Thermocouple SPI 
    -------------------------------------------------------------------------------
    */

    //empty device struct first
    spi_device_interface_config_t devconfig_thermo = {};

    //configure device settings
    devconfig_thermo.clock_speed_hz = SPI_THERMO_CLOCK_HZ;
    devconfig_thermo.mode = 1;  // MAX31856 works with CPHA = 1 which is SPI 1
    devconfig_thermo.spics_io_num = GPIO_CS_THERMO;
    devconfig_thermo.queue_size = 3; // how many transactions can be queued before time out ; relevant for asynchronous, may revisit in the future

    //Add device to the bus
    SPI_ret = spi_bus_add_device(SPI2_HOST, &devconfig_thermo, &spi_thermo_handle);

    if (SPI_ret != ESP_OK)
    {
        ESP_LOGE(SPI_TAG, "adding MAX31856 to spi bus failed: %s", esp_err_to_name(SPI_ret));
        return SPI_ret;
    }



    /*
    -------------------------------------------------------------------------------
    Configuring ADC SPI
    -------------------------------------------------------------------------------
    */

    //empty device struct first
    spi_device_interface_config_t devconfig_adc = {};

    //configure device settings
    devconfig_adc.clock_speed_hz = SPI_ADC_CLOCK_HZ;
    devconfig_adc.mode = 1;                    // required to interact with adc
    devconfig_adc.spics_io_num = GPIO_CS_ADC;
    devconfig_adc.queue_size = 3;  //double check this value

    //Add device to the bus
    SPI_ret = spi_bus_add_device(SPI2_HOST, &devconfig_adc, &spi_adc_handle);

    if (SPI_ret != ESP_OK)
    {
        ESP_LOGE(SPI_TAG, "adding ADS1261 to spi failed: %s", esp_err_to_name(SPI_ret));
        return SPI_ret;
    }

    return ESP_OK;

}

esp_err_t spi_transfer(spi_device_handle_t dev, const uint8_t *tx_data, uint8_t *rx_data, size_t len_bytes)
{
    if (dev == nullptr) return ESP_ERR_INVALID_STATE;   
    
    spi_transaction_t transaction = {};
    transaction.length = len_bytes * 8; // because spi_tranaction_t length parameter is in bits
    transaction.tx_buffer = tx_data;
    transaction.rx_buffer = rx_data;
    return spi_device_transmit(dev, &transaction);
}


