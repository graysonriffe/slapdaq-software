/*Logging includes*/
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_log.h>
#include <esp_timer.h>

/*Standard Library*/
#include <math.h>

/*FreeRTOS includes*/
#include <freertos/FreeRTOS.h>

/*SPI Includes*/
#include "spi_DAQ.h"

/*Wi-Fi Includes*/
#include "wifi.h"

/*Digital GPIO Includes*/
#include "dig_IO.h"

/*Thermocouple header*/
#include "thermo.h"

/*ADC Include*/
#include "adc.h"

/*Global Variables*/

static const TickType_t LOOP_PERIOD = pdMS_TO_TICKS(2); // 2 ms period = 500 Hz sample rate

constexpr size_t ADC_NUM_CHANNELS = 7;

struct Sample
{
    float adcData[ADC_NUM_CHANNELS];
    float tempData;
    float digitalIOData[8];
};

constexpr size_t SAMPLE_LEN = sizeof(Sample);

enum class ADCChannelKind : uint8_t
{
    Voltage,
    Current,
};

struct ADCChannel
{
    const char *name;
    uint8_t pos_mux;
    uint8_t neg_mux;
    ADCChannelKind kind;
};

static constexpr ADCChannel ADC_CHANNELS[] =
{
    // name  pos                neg                  kind
    { "AV1", ADS126X_MUXP_AIN5, ADS126X_MUXN_AINCOM, ADCChannelKind::Voltage },
    { "AV2", ADS126X_MUXP_AIN4, ADS126X_MUXN_AINCOM, ADCChannelKind::Voltage },
    { "AV3", ADS126X_MUXP_AIN3, ADS126X_MUXN_AINCOM, ADCChannelKind::Voltage },
    { "AV4", ADS126X_MUXP_AIN2, ADS126X_MUXN_AINCOM, ADCChannelKind::Voltage },
    { "DV",  ADS126X_MUXP_AIN6,  ADS126X_MUXN_AIN7,  ADCChannelKind::Voltage },
    { "AC1", ADS126X_MUXP_AIN9, ADS126X_MUXN_AINCOM, ADCChannelKind::Current },
    { "AC2", ADS126X_MUXP_AIN8, ADS126X_MUXN_AINCOM, ADCChannelKind::Current }
};

static const char* TAG = "MAIN";

// begin() RESETS the chips, which wipes any setting made after it.
static bool configureAdc(ADS126X &adc)
{
    esp_err_t err = adc.begin();

    // configure the default and desired settings here such as
    // data rate, filter, reference,...
    if (err == ESP_OK) err = adc.setRate(ADS126X_DR_40000);
    if (err == ESP_OK) err = adc.disablePGA();
    if (err == ESP_OK) err = adc.setReference(ADS126X_RMUXN_INTRN_AIN1, ADS126X_RMUXP_INTRN_AIN0);
    if (err == ESP_OK) err = adc.setFilter(ADS126X_FILTER_SINC1);
    if (err == ESP_OK) err = adc.startADC();
    if (err == ESP_OK) err = adc.sfocal();
    
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "ADC setup failed: %s", esp_err_to_name(err));
        return false;
    }

    return true;
}

static bool listenForDIOUpdate()
{
    uint8_t buffer[16] = {};

    int result = wifi_receive_bytes(buffer, sizeof(buffer));

    if (result < 0)
        return false; // Disconnection

    if (result == 0)
        return true; // No DIO update

    uint8_t directionMask = 0;
    uint8_t levelMask = 0;

    for (int i = 0; i < 8; i++) {
        directionMask |= (buffer[i * 2] << i);
        levelMask |= (buffer[i * 2 + 1] << i);
    }

    ESP_LOGI(TAG, "Got DIO Update");

    applyDioDirections(directionMask);
    applyDioLevels(levelMask);

    return true;
}

// measure one ADC channel and convert it to its respective units
static esp_err_t readChannel(ADS126X &adc, const ADCChannel &c, float *value)
{
    float raw = 0;
    esp_err_t err = adc.readADC(c.pos_mux, c.neg_mux, &raw);
    if (err != ESP_OK) return err;

    if (c.kind == ADCChannelKind::Current)
        raw *= 1000.0f; // Convert to mA

    *value = (float)raw;
    return ESP_OK;
}

extern "C" void app_main(void)
{
    /******************************************************** */
    /*                    INITIALIZATION                      */
    /******************************************************** */

    ESP_LOGI(TAG, "SlapDAQ initializing...");

    esp_err_t ret;

    /*Initialize SPI*/
    ret = SPI_Init();

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "SPI_Init failed: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "SPI initialized");

    /*Set GPIOs to inputs*/
    ret = set_all_pins_as_input();

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "GPIO init failed: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "GPIO initialized");

    // Initialize Wi-Fi
    if (!wifi_initialize())
    {
       ESP_LOGE(TAG, "Wifi could not be initialized");
       return;
    }

    ESP_LOGI(TAG, "Wi-Fi initialized");

    // Initialize ADC
    ADS126X adc;

    bool adc_ok = configureAdc(adc);
    if (!adc_ok)
    {
        ESP_LOGE(TAG, "configureAdc() failed");
        return;
    }

    ESP_LOGI(TAG, "ADC initialized");

    // Initialize thermocouple
    SlapDAQ_MAX31856 thermo;

    bool thermo_ok = thermo.thermoInit();
    if (!thermo_ok)
    {
        ESP_LOGE(TAG, "Thermocouple init failed");
        return;
    }

    ESP_LOGI(TAG, "Thermocouple initialized");

    /******************************************************** */
    /*                     MAIN LOOP                          */
    /******************************************************** */

    while (true) {
        wifi_wait_for_connection();

        TickType_t lastWake = xTaskGetTickCount();

        while (true)
        {
            // Check for a DIO update
            if (!listenForDIOUpdate())
                break;

            /*------------- ADC -----------*/
            // scan every channel in the table with their respective mux pair
            float adc_value[ADC_NUM_CHANNELS] = {};

            for (size_t ch = 0; ch < ADC_NUM_CHANNELS; ch++)
            {
                ret = readChannel(adc, ADC_CHANNELS[ch], &adc_value[ch]);
                if (ret != ESP_OK)
                {
                    // failure here is either the spi link or the chip
                    ESP_LOGE(TAG, "reading %s failed: %s - will re-intialize the ADC",
                                ADC_CHANNELS[ch].name, esp_err_to_name(ret));
                    for (size_t index = 0; index < ADC_NUM_CHANNELS; index++) adc_value[index] = NAN;
                    adc_ok = false;
                    break;
                }
            }
            
            /*------------- Thermocouple -----------*/
            float temp_c = thermo.readThermocoupleTemperature();

            /*----------------- DIO ----------------*/
            uint8_t digital_bits = read_all_pins();

            /*------- Send measurements to PC ------*/
            Sample sample = {};

            for (size_t ch = 0; ch < ADC_NUM_CHANNELS; ch++)
                sample.adcData[ch] = adc_value[ch];
            
            sample.tempData = temp_c;

            for (int i = 0; i < 8; i++)
                if (digital_bits & (0x01 << i))
                    sample.digitalIOData[i] = 1.0f;

            if (!wifi_send(reinterpret_cast<const char*>(&sample), SAMPLE_LEN))
                break;

            xTaskDelayUntil(&lastWake, LOOP_PERIOD);
        }
    }
}


