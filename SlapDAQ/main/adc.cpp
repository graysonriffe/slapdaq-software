
#include "adc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Initial Setup*/

ADS126X::ADS126X() {}

void ADS126X::begin(uint8_t chip_select)
{
    cs_used = true;
    cs_pin = chip_select;
    gpio_set_direction((gpio_num_t) chip_select, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t) chip_select, 1);
    ADS126X::begin();
}

void ADS126X::begin()
{
    ADS126X::reset();
}

void ADS126X::setStartPin(uint8_t pin)
{
    start_used = true;
    start_pin = pin;
    gpio_set_direction((gpio_num_t) pin, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t) pin, 0);
}


/* Regular ADC Commands */

void ADS126X::noOperation()
{
    ADS126X::sendCommand(ADS126X_NOP);
}

void ADS126X::startADC()
{
    if (start_used)
    {
        gpio_set_level((gpio_num_t) start_pin, 0);
        
        // in ms
        vTaskDelay(pdMS_TO_TICKS(2));
        gpio_set_level((gpio_num_t) start_pin, 1);
    }
    else
    {
        ADS126X::sendCommand(ADS126X_START);
    }
}

void ADS126X::stopADC()
{
    ADS126X::sendCommand(ADS126X_STOP);
}

int32_t ADS126X::readADC(uint8_t pos_pin, uint8_t neg_pin)
{
    if (cs_used)
    {
        gpio_set_level((gpio_num_t) cs_pin, 0);
    }

    // create buffer to hold transmission
    uint8_t buff[10] = {0};     // lots of room with all zeros
    
}