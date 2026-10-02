
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

    // a structure to hold all the data
    union
    {
        struct
        {
            uint32_t DATA3:8;   // bits: 0...7
            uint32_t DATA2:8;   // bits: 8...15
            uint32_t DATA1:8;   // bits: 16...23
            uint32_t :8;        // bits: 24...31 (all don't care)
        } bit;
        uint32_t reg;
    } ADC_BYTES;

    ADC_BYTES.reg = 0;  // clear the ram just in case

    // check if desired pins are different than old pins
    if((REGISTER.INPMUX.bit.MUXN != neg_pin) || (REGISTER.INPMUX.bit.MUXP != pos_pin))
    {
        REGISTER.INPMUX.bit.MUXN = neg_pin;
        REGISTER.INPMUX.bit.MUXP = pos_pin;
        ADS126X::writeRegister(ADS126X_INPMUX);     // replace the old pins
    }


    uint8_t i = 0;      // buffer index
    buff[i] = ADS126X_RDATA;    // the read adc command
    i++;


    if (REGISTER.MODE3.bit.STATENB) i++;    // place to hold status byte
    i += 3;     //place to hold ADC data
    i++;        //plot to hold empty byte

    if (REGISTER.MODE3.bit.CRCENB>0) i++;   // place to hold crc byte
    spiWrite(buff, i);      // send buffer to the ADC

    uint8_t j = 1;      // starts a byte 1, either status or first adc value

    if (REGISTER.MODE3.bit.STATENB)
    {
        STATUS
    }


}

void ADS126X::spiWrite(uint8_t buffer[], uint8_t length)
{
    spi_transaction_t t = {};
    t.length = 8*length;
    t.tx_buffer = buffer;
    t.rx_buffer = NULL;

    spi_device_transmit(spi_adc1_handle, &t);
}