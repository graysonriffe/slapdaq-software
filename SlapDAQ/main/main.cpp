
/*Logging includes*/
#define LOG_LEVEL_LOCAL ESP_LOG_VERBOSE
#include "esp_log.h"
#define LOG_TAG "MAIN"

/*FreeRTOS includes*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"

/*SPI Includes*/
#include "spi_DAQ.h"

/*Wi-Fi Includes*/
#include "wifi.h"

/*Digital GPIO Includes*/
#include "dig_IO.h"

/*C Standard I/O*/
#include <stdio.h>



/*Global Variables*/
esp_err_t main_ret;


extern "C" void app_main(void)
{

    /*Initialize SPI*/
    SPI_Init();

    if (main_ret == ESP_OK)
    {
        ESP_LOGI(LOG_TAG, "SPI has been initialized");
    }


    /*Set GPIOs to inputs*/
    dig_io_config_to_input();

    if (main_ret == ESP_OK)
    {
        ESP_LOGI(LOG_TAG, "GPIO has been initialized");
    }

    gpio_dump_io_configuration(stdout, GPIO_SEL_BITMASK);

    

    //Initialize wifi
    //wifi_initialize();



}


