
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

/*Thermocouple header*/
#include "thermo.h"

/*ADC Include*/
#include "adc.h"



/*Global Variables*/
esp_err_t main_ret;

// Reference voltage ADC uses in volts. Default reference = AVDD - AVSS
static const double ADC_VREF_VOLTS = 5.0;

extern "C" void app_main(void)
{

    #if 1
    /*Initialize SPI*/
    main_ret = SPI_Init();

    if (main_ret != ESP_OK)
    {
        ESP_LOGE(LOG_TAG, "SPI_Init failed: %s", esp_err_to_name(main_ret));
        return;
    }
    ESP_LOGI(LOG_TAG, "SPI has been initialized");
    #endif

    #if 0
    /*Set GPIOs to inputs*/
    set_all_pins_as_input();

    if (main_ret == ESP_OK)
    {
        ESP_LOGI(LOG_TAG, "GPIO has been initialized");
    }

    gpio_dump_io_configuration(stdout, GPIO_SEL_BITMASK);
    #endif
    
    #if 0
    //Initialize wifi
    wifi_initialize();
    #endif

    #if 0
    /*Thermocouple instantiation*/
    SlapDAQ_MAX31856 thermo;

    //initialize thermocouple
    thermo.thermoInit();

    //read thermocouple readings
    while(true)
    {
    thermo.readThermocoupleTemperature();
    }
    #endif

    ADS126X adc;

    main_ret = adc.begin();
    if (main_ret != ESP_OK)
    {
        ESP_LOGE(LOG_TAG, "ADC begin() failed: %s", esp_err_to_name(main_ret));
        return;
    }

    ESP_LOGI(LOG_TAG, "Reading Voltages: AIN0 (+) - AIN1 (-)");

    while (true)
    {
        int32_t raw = 0;

        // Pass MUX CODES
        main_ret = adc.readADC(ADS126X_MUXP_AIN0, ADS126X_MUXN_AIN1, &raw);

        if (main_ret == ESP_OK)
        {
            ESP_LOGI(LOG_TAG, "raw = %" PRId32 "    %.6f V  (status 0x%02X)",
                    raw, adc.toVolts(raw, ADC_VREF_VOLTS), adc.lastStatus());
        }
        else
        {
            ESP_LOGE(LOG_TAG, "readADC failed: %s", esp_err_to_name(main_ret));
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    

}


