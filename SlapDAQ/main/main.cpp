
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

/*Struct Function Includes*/
#include "struct_send.h"


/*Global Variables*/
esp_err_t main_ret;



// Reference voltage ADC uses in volts. Default reference = AVDD - AVSS
static const double ADC_VREF_VOLTS = 5.0;

extern "C" void app_main(void)
{

    /******************************************************** */
    /*                INITIALIZATION                          */
    /******************************************************** */


    /*Initialize SPI*/
    main_ret = SPI_Init();

    if (main_ret != ESP_OK)
    {
        ESP_LOGE(LOG_TAG, "SPI_Init failed: %s", esp_err_to_name(main_ret));
        return;
    }
    ESP_LOGI(LOG_TAG, "SPI has been initialized");


    /*Set GPIOs to inputs*/
    set_all_pins_as_input();

    if (main_ret == ESP_OK)
    {
        ESP_LOGI(LOG_TAG, "GPIO has been initialized");
    }


    //Initialize wifi
    if (!wifi_initialize())
    {
        ESP_LOGE(LOG_TAG, "Wifi could not be initialized");
        return;
    }

    /*Thermocouple instantiation*/
    SlapDAQ_MAX31856 thermo;

    //initialize thermocouple
    if (thermo.thermoInit() == true)
    {
        ESP_LOGI(LOG_TAG, "Thermocouple has been initialized!");
    }
    


    ADS126X adc;

    main_ret = adc.begin();
    if (main_ret != ESP_OK)
    {
        ESP_LOGE(LOG_TAG, "ADC begin() failed: %s", esp_err_to_name(main_ret));
        return;
    }




    /******************************************************** */
    /*                MAIN LOOP                               */
    /******************************************************** */


    bool was_connected = false;
    TickType_t lastWake = xTaskGetTickCount();


    while (true)
    {

        bool connected = wifi_client_connected(0);

        if (connected != was_connected)
        {
            ESP_LOGI(LOG_TAG, "Client %s", connected ? "connected" : "disconnected");
            was_connected = connected;
        }

        // update digital IO from user
        uint8_t rx[2];
        int n = connected ? wifi_receive(rx, sizeof(rx)) : 0;

        if (n > 0)
        {
            uint8_t type, mask;
            if (parseDioConfigPacket(rx, n, &type, &mask))
            {
                switch (type)
                {
                    case MSG_SET_DIO_DIR: applyDioDirections(mask); break;
                    case MSG_SET_DIO_LEVEL: applyDioDirections(mask); break;
                    default: break;
                }
            }
        }

        uint8_t digital_bits = read_all_pins();



        // read thermocouple data
        float thermo_temps_to_send;

        ESP_LOGI(LOG_TAG, "Reading temperatures from thermocouple!");

        thermo_temps_to_send = thermo.readThermocoupleTemperature();
        


        //read ADC data

        ESP_LOGI(LOG_TAG, "Reading Voltages: AIN0 (+) - AIN1 (-)");

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



        // send measurements to host PC
        if (connected)
        {
            SensorData measurements;
            measurements.digitalIOData = digital_bits;
            measurements.tempData = static_cast<int32_t>(thermo_temps_to_send);
            measurements.adcData = raw;
            sendStructOverWifi(measurements);
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(1000));
        
    
    }


}


