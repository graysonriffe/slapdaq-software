
/*Logging includes*/
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#define LOG_TAG "MAIN"

/*Standard Library*/
#include <stdio.h>
#include <inttypes.h>
#include <math.h>

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

// Reference voltage ADC uses in volts. Default reference = AVDD - AVSS
static const double ADC_VREF_VOLTS = 5.0;

// temp scale on the wire
static const float TEMP_SCALE = 1.0f;

// retry sensor init (loops = seconds)
static const uint32_t SENSOR_RETRY_LOOPS = 5;

static const TickType_t LOOP_PERIOD = pdMS_TO_TICKS(1000);

enum class AdcKind : uint8_t
{
    Voltage,
    Current_4_20mA,
};

struct AdcChannel
{
    const char *name;
    uint8_t pos_mux;
    uint8_t neg_mux;
    bool pga_bypass;
    uint8_t pga_gain;
    AdcKind kind;
    float shunt_ohms;
};

static constexpr float SHUNT_OHMS = 1.0f;

static constexpr AdcChannel ADC_CHANNELS[] =
{
    // name        pos                 neg         bypass      gain               kind          shunt
    { "V4", ADS126X_MUXP_AIN2, ADS126X_MUXN_AINCOM, true, ADS126X_PGA_GAIN_1, AdcKind::Voltage,              0.0f},
    { "V3", ADS126X_MUXP_AIN3, ADS126X_MUXN_AINCOM, true, ADS126X_PGA_GAIN_1, AdcKind::Voltage,              0.0f},
    { "V2", ADS126X_MUXP_AIN4, ADS126X_MUXN_AINCOM, true, ADS126X_PGA_GAIN_1, AdcKind::Voltage,              0.0f},
    { "V1", ADS126X_MUXP_AIN5, ADS126X_MUXN_AINCOM, true, ADS126X_PGA_GAIN_1, AdcKind::Voltage,              0.0f},

    { "DIFF", ADS126X_MUXP_AIN6, ADS126X_MUXN_AIN7, true, ADS126X_PGA_GAIN_1, AdcKind::Voltage,              0.0f},

    { "I2", ADS126X_MUXP_AIN8, ADS126X_MUXN_AINCOM, true, ADS126X_PGA_GAIN_1, AdcKind::Current_4_20mA,      SHUNT_OHMS},
    { "I1", ADS126X_MUXP_AIN9, ADS126X_MUXN_AINCOM, true, ADS126X_PGA_GAIN_1, AdcKind::Current_4_20mA,      SHUNT_OHMS},
};

// limits set for the current loop
static const float LOOP_FAULT_LOW_MA = 3.6f;
static const float LOOP_FAULT_HIGH_MA = 21.0f;


// Function that displays the thermocouple faults to the serial monitor for debugging
static void logThermoFault(uint8_t fault)
{
    if (fault == 0)
    {
        ESP_LOGI(LOG_TAG, "Thermocouple faults cleared");
        return;
    }

    ESP_LOGW(LOG_TAG, "Thermocouple fault 0x%02X:%s%s%s%s%s%s%s%s", fault,
             (fault & MAX31856_FAULT_CJRANGE) ? " CJ-out-of-range" : "",
             (fault & MAX31856_FAULT_TCRANGE) ? " TC-out-of-range" : "",
             (fault & MAX31856_FAULT_CJHIGH) ? " CJ-high" : "",
             (fault & MAX31856_FAULT_CJLOW) ? " CJ-low" : "",
             (fault & MAX31856_FAULT_TCHIGH) ? " TC-high" : "",
             (fault & MAX31856_FAULT_TCLOW) ? " TC-low" : "",
             (fault & MAX31856_FAULT_OVUV) ? " over/under-voltage" : "",
             (fault & MAX31856_FAULT_OPEN) ? " OPEN-circuit" : "");
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

// function to convert temp in C to int32 when sending to host
static int32_t tempToWire(float temp_c)
{
    if (isnan(temp_c)) return 0;

    float scaled = temp_c * TEMP_SCALE;
    if (scaled > 2.0e9f) scaled = 2.0e9f;
    if (scaled < -2.0e9f) scaled = -2.0e9f;

    // static_cast trucated to zero, rouding is more accurate
    return (int32_t) lround(scaled);
}

// begin() RESETS the chips, which wipes any setting made after it.
static bool configureAdc(ADS126X &adc)
{
    esp_err_t err = adc.begin();

    // configure the default and desired settings here such as
    // data rate, filter, reference,...
    if (err == ESP_OK) err = adc.setRate(ADS126X_DR_20);
    if (err == ESP_OK) err = adc.setFilter(ADS126X_FILTER_FIR);
    if (err == ESP_OK) err = adc.setDelay(ADS126X_DELAY_50_US);
    if (err == ESP_OK) err = adc.disableInternalReference();
    

    if (err != ESP_OK)
    {
        ESP_LOGE(LOG_TAG, "ADC setup failed: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}

// measure one ADC channel and convert it to its respective units
static esp_err_t readChannel(ADS126X &adc, const AdcChannel &c, float *value)
{
    // 1. this channel's PGA setting
    esp_err_t err = adc.setPGA(c.pga_bypass, c.pga_gain);
    if (err != ESP_OK) return err;

    // 2. one conversion on this channel's input pair
    int32_t raw = 0;
    err = adc.readADC(c.pos_mux, c.neg_mux, &raw);
    if (err != ESP_OK) return err;

    // 3. convert code to volts
    const double volts = adc.toVolts(raw, ADC_VREF_VOLTS);

    // 4. volts to channel's unit
    if (c.kind == AdcKind::Current_4_20mA)
    {
        *value = (float) (volts / (double) c.shunt_ohms * 1000.0);
    }
    else
    {
        *value = (float) volts;
    }
    return ESP_OK;
}

extern "C" void app_main(void)
{

    /******************************************************** */
    /*                INITIALIZATION                          */
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

    /*Thermocouple instantiation*/
    SlapDAQ_MAX31856 thermo;

    //initialize thermocouple
    bool thermo_ok = thermo.thermoInit();
    if (thermo_ok)
    {
        ESP_LOGI(LOG_TAG, "Thermocouple has been initialized!");
    }
    else
    {
        ESP_LOGE(LOG_TAG, "Thermocouple init failed - retrying every %" PRIu32 " s", SENSOR_RETRY_LOOPS);
    }
    
    /*ADC instantiation*/
    ADS126X adc;

    bool adc_ok = configureAdc(adc);
    if (!adc_ok)
    {
        ESP_LOGE(LOG_TAG, "ADC begin() failed - retrying every %" PRIu32 " s", SENSOR_RETRY_LOOPS);
    }





    /******************************************************** */
    /*                MAIN LOOP                               */
    /******************************************************** */


    bool was_connected = false;
    uint8_t last_fault = 0;
    uint32_t loop_count = 0;
    TickType_t lastWake = xTaskGetTickCount();


    while (true)
    {
        loop_count++;

        /*------------- host connection + commands -----------*/
        bool connected = wifi_client_connected(0);

        if (connected != was_connected)
        {
            ESP_LOGI(LOG_TAG, "Client %s", connected ? "connected" : "disconnected");
            was_connected = connected;
        }

        // update digital IO from user
        uint8_t pkt[DIO_PACKET_LEN];
        while (connected && wifi_receive_packet(pkt, sizeof(pkt)))
        {
            handleHostCommand(pkt);
        }

        uint8_t digital_bits = read_all_pins();



        /*------------- thermocouple -----------*/
        if (!thermo_ok && (loop_count % SENSOR_RETRY_LOOPS == 0))
        {
            thermo_ok = thermo.thermoInit();
            if (thermo_ok) ESP_LOGI(LOG_TAG, "Thermocouple initialized on rety");
        }
        
        float temp_c = NAN;
        if (thermo_ok)
        {
            temp_c = thermo.readThermocoupleTemperature();

            uint8_t fault = 0;
            if (thermo.readFault(&fault) == ESP_OK && fault != last_fault)
            {
                logThermoFault(fault);
                last_fault = fault;
            }
        }
        

        /*------------- ADC -----------*/
        if (!adc_ok && (loop_count % SENSOR_RETRY_LOOPS == 0))
        {
            adc_ok = configureAdc(adc);
            if (adc_ok) ESP_LOGI(LOG_TAG, "ADC initialized on retry");
        }

        if (adc_ok)
        {
            bool adc_was_reset = false;
            if (adc.checkResetBit(&adc_was_reset) == ESP_OK && adc_was_reset)
            {
                ESP_LOGW(LOG_TAG, "ADC reset unexpectedly - re-initializing");
                adc_ok = configureAdc(adc);
            }
        }

        // scan every channel in the table with their respective mux pair
        float adc_value[ADC_NUM_CHANNELS];
        for (size_t ch = 0; ch < ADC_NUM_CHANNELS; ch++) adc_value[ch] = NAN;   // NaN = no reading

        if (adc_ok)
        {
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
        }
        
        // two summary lines per loop: DIO + thermocouple, then every ADC measurement
        ESP_LOGI(LOG_TAG, "DIO 0x%02X | TC %.2f C", digital_bits, (double) temp_c);

        char adc_line[256];
        int pos = 0;
        for (size_t ch = 0; ch < ADC_NUM_CHANNELS; ch++)
        {
            const AdcChannel &c = ADC_CHANNELS[ch];
            const bool is_current = (c.kind == AdcKind::Current_4_20mA);
            const float v = adc_value[ch];

            // check if 4-20 mA loops are open
            const char *flag = "";
            if (is_current && !isnan(v))
            {
                if (v < LOOP_FAULT_LOW_MA)      flag = " [LOOP OPEN?]";
                else if (v > LOOP_FAULT_HIGH_MA) flag = " [LOOP FAULT]";
            }

            int n = snprintf(adc_line + pos, sizeof(adc_line) - pos, "%s %s %.4f %s%s",
                             (ch == 0) ? "" : " |", c.name, (double) v, is_current ? "mA" : "V", flag);
            if (n < 0 || pos + n >= (int) sizeof(adc_line)) break;
            pos += n;
        }
        ESP_LOGI(LOG_TAG, "ADC%s%s", adc_line, adc_ok ? "" : " [ADC offline]");



        /*------------- send measurements to PC -----------*/
        if (connected)
        {
            SensorData measurements = {};
            measurements.digitalIOData = digital_bits;
            measurements.tempData = tempToWire(temp_c);
            for (size_t ch = 0; ch < ADC_NUM_CHANNELS; ch++)
            {
                measurements.adcData[ch] = adc_value[ch];
            }
            sendStructOverWifi(measurements);
        }

        if (xTaskDelayUntil(&lastWake, LOOP_PERIOD) == pdFALSE)
        {
            lastWake = xTaskGetTickCount();
        }
    }
}


