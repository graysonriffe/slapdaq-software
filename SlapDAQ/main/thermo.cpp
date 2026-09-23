
/*Import header file*/
#include "thermo.h"

/*Standard Library include*/
#include <stdlib.h>


/*Pre-Configure thermocouple*/
bool SlapDAQ_MAX31856::thermoInit(void)
{
    //assert on any fault
    writeRegister8(MAX31856_MASK_REG, 0x0);

    // enable open circuit fault detection
    writeRegister8(MAX31856_CR0_REG, MAX31856_CR0_OCFAULT0);

    // set cold junction temperature offset to zero
    writeRegister8(MAX31856_CJTO_REG, 0x0);

    // set Type K
    setThermocoupleType(MAX31856_TCTYPE_K);

    // set continuous conversion mode
    setConversionMode(MAX31856_CONTINUOUS);

    return true;
}

// don't need this, find a way to get rid of it.
SlapDAQ_MAX31856::SlapDAQ_MAX31856() {}

/*Function to set conversion mode*/
void SlapDAQ_MAX31856::setConversionMode(max31856_conversion_mode_t mode)
{
    conversionMode = mode;

    uint8_t value;
    

    readRegister8(MAX31856_CR0_REG, &value);  //get current register value




    if (conversionMode == MAX31856_CONTINUOUS)
    {
        value |= MAX31856_CR0_AUTOCONVERT;  // turn on automatic
        value &= ~MAX31856_CR0_1SHOT;       // turn off one-shot
    }
    else
    {
        value &= ~MAX31856_CR0_AUTOCONVERT;  // turn off automatic
        value |= MAX31856_CR0_1SHOT;       // turn on one-shot
    }

    writeRegister8(MAX31856_CR0_REG, value);
}

/*Get temperature conversion mode*/
max31856_conversion_mode_t SlapDAQ_MAX31856::getConversionMode(void)
{
    return conversionMode;
}

/*Set thermocouple type*/
void SlapDAQ_MAX31856::setThermocoupleType(max31856_thermocoupletype_t type)
{
    uint8_t value;
    
    readRegister8(MAX31856_CR1_REG, &value);


    value &= 0xF0; //mask off bottom bits
    value |= (uint8_t) type & 0x0F;
    writeRegister8(MAX31856_CR1_REG, value);
}

/*Get thermocouple type*/
max31856_thermocoupletype_t SlapDAQ_MAX31856::getThermocoupleType(uint8_t *type)
{
    
    readRegister8(MAX31856_CR1_REG, type);

    *type &= 0x0F;

    return (max31856_thermocoupletype_t) (*type);
}

/*Read the fault register*/
esp_err_t SlapDAQ_MAX31856::readFault(uint8_t *fault)
{
    return readRegister8(MAX31856_SR_REG, fault);
}

/*Set the cold junction*/
void SlapDAQ_MAX31856::setColdJunctionFaultThresholds(int8_t low, int8_t high)
{
    writeRegister8(MAX31856_CJLF_REG, low);
    writeRegister8(MAX31856_CJHF_REG, high);
}

/*Set the noise filter*/
void SlapDAQ_MAX31856::setNoiseFilter(max31856_noise_filter_t noiseFilter)
{
    uint8_t value;
    

    readRegister8(MAX31856_CR0_REG, &value);

    if (noiseFilter == MAX31856_NOISE_FILTER_50HZ)
    {
        value |= 0x01;
    }
    else
    {
        value &= ~0xFE;
    }

    writeRegister8(MAX31856_CR0_REG, value);
}

/*Set temperature fault thresholds*/
void SlapDAQ_MAX31856::setTempFaultThresholds(float flow, float fhigh)
{
    int16_t low, high;

    flow *= 16;
    low = flow;

    fhigh *= 16;
    high = fhigh;

    writeRegister8(MAX31856_LTHFTH_REG, high >> 8);
    writeRegister8(MAX31856_LTHFTL_REG, high);

    writeRegister8(MAX31856_LTLFTH_REG, low >> 8);
    writeRegister8(MAX31856_LTLFTL_REG, low);
}

/*Returns if the conversion is complete*/
bool SlapDAQ_MAX31856::conversionComplete(void)
{
    uint8_t compareValue;
    
    
    if (conversionMode == MAX31856_CONTINUOUS)
    {
        return true;
    }

    readRegister8(MAX31856_CR0_REG, &compareValue);


    return !(compareValue & MAX31856_CR0_1SHOT);
}

/*Return cold-junction internal chip temperature*/
float SlapDAQ_MAX31856::readCJTemperature(void)
{
    //to make a 16 bit buffer
    uint8_t msb, lsb;
    
    //call readRegister twice to read in 16 bits of data
    readRegister8(MAX31856_CJTH_REG, &msb);
    readRegister8(MAX31856_CJTL_REG, &lsb);
    
    int16_t raw = (int16_t)((msb << 8) | lsb);      //adds the msb and lsb together to make 16 bits of data or 2 bytes
    float tempC = raw / 256.0f;
    
    return tempC;

}


/*Return hot-junction thermocouple temperature*/
float SlapDAQ_MAX31856::readThermocoupleTemperature(void)
{
    //to make a 24 bit buffer
    uint8_t msb, midb, lsb;
    
    // have to repeat 3 times to get the full 3 byte value
    readRegister8(MAX31856_LTCBH_REG, &msb);
    readRegister8(MAX31856_LTCBM_REG, &midb);
    readRegister8(MAX31856_LTCBL_REG, &lsb);


    int32_t raw = ((int32_t)msb << 16) | ((int32_t)midb << 8) | lsb;    //24 bit value unshifted
    
    raw <<= 8;      // slide sign bit from bit 23 to bit 31
    raw >>= 13;     // arithemtic sign shift: sign-extends and removes the 8-bit pad + 5 unused bits (8+5 = 13)

    return raw * 0.0078125f;

}



esp_err_t SlapDAQ_MAX31856::writeRegister8(uint8_t addr, uint8_t data)
{
    uint8_t tx_data[2];
    tx_data[0] = addr | 0x80;           // setting the write bit
    tx_data[1] = data;

    spi_transaction_t t = {};
    t.length = 8*2;
    t.tx_buffer = tx_data;
    t.rx_buffer = NULL;                 //we don't care about the response

    esp_err_t ret = spi_device_transmit(spi_thermo_handle, &t);
    return ret;
}


esp_err_t SlapDAQ_MAX31856::readRegister8(uint8_t addr, uint8_t *data)
{
    uint8_t tx_data[2];
    uint8_t rx_data[2];

    tx_data[0] = addr & 0x7F;           //clears bit 7 for read command
    tx_data[1] = 0x00;

    
    spi_transaction_t t = {};
    t.length = 8*2;
    t.tx_buffer = tx_data;
    t.rx_buffer = rx_data;              
    
    esp_err_t ret = spi_device_transmit(spi_thermo_handle, &t);
    
    if (ret == ESP_OK)
    {
        *data = rx_data[1];                 //only rx_data[1] has data bytes we need
    }
        
    
    return ret;

}


