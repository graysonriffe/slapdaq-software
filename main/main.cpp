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

static const char* LOG_TAG = "MAIN";

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
        ESP_LOGE(LOG_TAG, "ADC setup failed: %s", esp_err_to_name(err));
        return false;
    }

    return true;
}

static void handleHostCommand(const uint8_t *pkt)
{
    uint8_t type = 0, mask = 0;
    if (!parseDioConfigPacket(pkt, DIO_PACKET_LEN, &type, &mask))
    {
        ESP_LOGW(LOG_TAG, "Ignoring unknown command packet %02X %02X", pkt[0], pkt[1]);
        return;
    }

    esp_err_t err = ESP_OK;
    switch (type)
    {
        case MSG_SET_DIO_DIR:   err = applyDioDirections(mask); break;
        case MSG_SET_DIO_LEVEL: err = applyDioLevels(mask);     break;
        default: break;
    }

    if (err != ESP_OK)
    {
        ESP_LOGE(LOG_TAG, "DIO command 0x%02X failed: %s", type, esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(LOG_TAG, "DIO %s mask set to 0x%02X", (type == MSG_SET_DIO_DIR) ? "direction" : "level", mask);
    }
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

    esp_err_t ret;

    /*Initialize SPI*/
    ret = SPI_Init();

    if (ret != ESP_OK)
    {
        ESP_LOGE(LOG_TAG, "SPI_Init failed: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(LOG_TAG, "SPI has been initialized");

    /*Set GPIOs to inputs*/
    ret = set_all_pins_as_input();

    if (ret != ESP_OK)
    {
        ESP_LOGE(LOG_TAG, "GPIO init failed: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(LOG_TAG, "GPIO has been initialized");

    //Initialize wifi
    if (!wifi_initialize())
    {
       ESP_LOGE(LOG_TAG, "Wifi could not be initialized");
       return;
    }

    /*ADC instantiation*/
    ADS126X adc;

    bool adc_ok = configureAdc(adc);
    if (!adc_ok)
    {
        ESP_LOGE(LOG_TAG, "configureAdc() failed");
        return;
    }

    /*Thermocouple instantiation*/
    SlapDAQ_MAX31856 thermo;

    //initialize thermocouple
    bool thermo_ok = thermo.thermoInit();
    if (!thermo_ok)
    {
        ESP_LOGE(LOG_TAG, "Thermocouple init failed");
        return;
    }

    /******************************************************** */
    /*                MAIN LOOP                               */
    /******************************************************** */

    while (true) {
        wifi_wait_for_connection();

        uint32_t loop_count = 0;
        TickType_t lastWake = xTaskGetTickCount();

        while (true)
        {
            int64_t startTime = esp_timer_get_time();

            // if (connected != was_connected)
            // {
            //     ESP_LOGI(LOG_TAG, "Client %s", connected ? "connected" : "disconnected");
            //     was_connected = connected;
            // }

            // update digital IO from user
            // uint8_t pkt[DIO_PACKET_LEN];
            // while (wifi_receive_packet(pkt, sizeof(pkt)))
            // {
            //     handleHostCommand(pkt);
            // }

            // Handle disconnect here when trying to receive

            /*------------- ADC -----------*/
            // scan every channel in the table with their respective mux pair
            float adc_value[ADC_NUM_CHANNELS];
            for (size_t ch = 0; ch < ADC_NUM_CHANNELS; ch++) adc_value[ch] = NAN;   // NaN = no reading

            for (size_t ch = 0; ch < ADC_NUM_CHANNELS; ch++)
            {
                ret = readChannel(adc, ADC_CHANNELS[ch], &adc_value[ch]);
                if (ret != ESP_OK)
                {
                    // failure here is either the spi link or the chip
                    ESP_LOGE(LOG_TAG, "reading %s failed: %s - will re-intialize the ADC",
                                ADC_CHANNELS[ch].name, esp_err_to_name(ret));
                    for (size_t index = 0; index < ADC_NUM_CHANNELS; index++) adc_value[index] = NAN;
                    adc_ok = false;
                    break;
                }
            }
            
            // two summary lines per loop: DIO + thermocouple, then every ADC measurement
            // ESP_LOGI(LOG_TAG, "DIO 0x%02X | TC %.2f C", digital_bits, (double) temp_c);


            // char adc_line[256];
            // int pos = 0;
            // for (size_t ch = 0; ch < ADC_NUM_CHANNELS; ch++)
            // {
            //     const ADCChannel &c = ADC_CHANNELS[ch];
            //     const bool is_current = (c.kind == ADCChannelKind::Current);
            //     const float v = adc_value[ch];

            //     int n = snprintf(adc_line + pos, sizeof(adc_line) - pos, "%s %s %.4f %s",
            //                      (ch == 0) ? "" : " |", c.name, (double) v, is_current ? "mA" : "V");
            //     if (n < 0 || pos + n >= (int) sizeof(adc_line)) break;
            //     pos += n;
            // }
            // ESP_LOGI(LOG_TAG, "ADC%s%s", adc_line, adc_ok ? "" : " [ADC offline]");

            /*------------- thermocouple -----------*/
            float temp_c = NAN;
            temp_c = thermo.readThermocoupleTemperature();

            /*----------------- DIO ----------------*/
            uint8_t digital_bits = read_all_pins();

            /*------------- send measurements to PC -----------*/
            //measurements.digitalIOData = digital_bits;
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

            loop_count++;

            int64_t loopDuration = esp_timer_get_time() - startTime;

            if (loop_count % 100 == 0)
                ESP_LOGI(LOG_TAG, "Elapsed time: %lld", loopDuration);
        }
    }
}


