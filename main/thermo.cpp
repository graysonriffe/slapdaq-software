
/*Import header file*/
#include "thermo.h"

/*Standard Library include*/
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "esp_log.h"

static const char *THERMO_TAG = "MAX31856";

/*Pre-Configure thermocouple*/
bool SlapDAQ_MAX31856::thermoInit(void)
{
    esp_err_t err = ESP_OK;

    //assert on any fault
    if (err == ESP_OK) err = writeRegister8(MAX31856_MASK_REG, 0x00);   

    // enable open circuit fault detection
    if (err == ESP_OK) err = writeRegister8(MAX31856_CR0_REG, MAX31856_CR0_OCFAULT0);
    
    // set cold junction temperature offset to zero
    if (err == ESP_OK) err = writeRegister8(MAX31856_CJTO_REG, 0x00);

    // set Type K
    if (err == ESP_OK) err = setThermocoupleType(MAX31856_TCTYPE_K);

    // set continuous conversion mode
    if (err == ESP_OK) err = setConversionMode(MAX31856_CONTINUOUS);

    if (err != ESP_OK)
    {
        ESP_LOGE(THERMO_TAG, "SPI error while configuring thermocouple: %s", esp_err_to_name(err));
        return false;
    }

    // checks to ensure that thermo is configured correctly after SPI write commands
    uint8_t cr0 = 0, cr1 = 0;
    if (readRegister8(MAX31856_CR0_REG, &cr0) != ESP_OK || readRegister8(MAX31856_CR1_REG, &cr1) != ESP_OK)
    {
        ESP_LOGE(THERMO_TAG, "SPI error during readback");
        return false;
    }

    const uint8_t want_cr0 = MAX31856_CR0_AUTOCONVERT | MAX31856_CR0_OCFAULT0;
    if (cr0 != want_cr0 || (cr1 & 0x0F) != MAX31856_TCTYPE_K)
    {
        ESP_LOGE(THERMO_TAG, "Readback mismatch: CR0 0x%02X (want 0x%02X), CR1 0x%02X (want 0x%X)"
                "Check CS pin, MISO, SPI mode, 3.3 V supply.",
                cr0, want_cr0, cr1, (unsigned) MAX31856_TCTYPE_K);
        return false;
    }

    return true;
}


/*Function to set conversion mode*/
esp_err_t SlapDAQ_MAX31856::setConversionMode(max31856_conversion_mode_t mode)
{
    uint8_t value = 0;

    esp_err_t err = readRegister8(MAX31856_CR0_REG, &value);  //get current register value
    if (err != ESP_OK) return err;  // check to make sure an unititialized byte isn't written back to IC


    if (mode == MAX31856_CONTINUOUS)
    {
        value |= MAX31856_CR0_AUTOCONVERT;  // turn on automatic
        value &= (uint8_t) ~MAX31856_CR0_1SHOT;       // turn off one-shot
    }
    else
    {
        value &= (uint8_t) ~MAX31856_CR0_AUTOCONVERT;  // turn off automatic
        value |= MAX31856_CR0_1SHOT;       // turn on one-shot
    }

    err = writeRegister8(MAX31856_CR0_REG, value);
    if (err == ESP_OK) conversionMode = mode;       // a check to ensure only a correct mode is sent to the IC 
    return err;     
}

/*Get temperature conversion mode*/
max31856_conversion_mode_t SlapDAQ_MAX31856::getConversionMode(void) const
{
    return conversionMode;
}

/*Set thermocouple type*/
esp_err_t SlapDAQ_MAX31856::setThermocoupleType(max31856_thermocoupletype_t type)
{
    uint8_t value = 0;
    
    esp_err_t err = readRegister8(MAX31856_CR1_REG, &value);
    if (err != ESP_OK) return err;

    value &= 0xF0; //mask off bottom bits
    value |= (uint8_t) type & 0x0F;
    return writeRegister8(MAX31856_CR1_REG, value);
}

/*Get thermocouple type*/
esp_err_t SlapDAQ_MAX31856::getThermocoupleType(max31856_thermocoupletype_t *type)
{
    if (type == nullptr) return ESP_ERR_INVALID_ARG;

    uint8_t value = 0;

    esp_err_t err = readRegister8(MAX31856_CR1_REG, &value);
    if (err != ESP_OK) return err;

    *type = (max31856_thermocoupletype_t) (value & 0x0F);
    return ESP_OK;
}

/*Read the fault register*/
esp_err_t SlapDAQ_MAX31856::readFault(uint8_t *fault)
{
    return readRegister8(MAX31856_SR_REG, fault);
}

/*Set the cold junction*/
esp_err_t SlapDAQ_MAX31856::setColdJunctionFaultThresholds(int8_t low, int8_t high)
{
    esp_err_t err = writeRegister8(MAX31856_CJLF_REG, (uint8_t) low);
    if (err != ESP_OK) return err;
    return writeRegister8(MAX31856_CJHF_REG, (uint8_t) high);
}

/*Set the noise filter*/
esp_err_t SlapDAQ_MAX31856::setNoiseFilter(max31856_noise_filter_t noiseFilter)
{
    uint8_t value = 0;
    
    esp_err_t err = readRegister8(MAX31856_CR0_REG, &value);
    if (err != ESP_OK) return err;

    // Datasheets states you can't change the 50/60 Hz during auto-conversion mode
    // add a check for this to stop conversion in order to change the frequency

    const bool was_auto = (value & MAX31856_CR0_AUTOCONVERT) != 0;
    if (was_auto)
    {
        value &= (uint8_t) ~(MAX31856_CR0_AUTOCONVERT); // shuts autoconversion off
        err = writeRegister8(MAX31856_CR0_REG, value);
        if (err != ESP_OK) return err;
    }

    if (noiseFilter == MAX31856_NOISE_FILTER_50HZ)
    {
        value |= MAX31856_CR0_50HZ;
    }
    else
    {
        value &= (uint8_t) ~MAX31856_CR0_50HZ;
    }

    err = writeRegister8(MAX31856_CR0_REG, value);
    if (err != ESP_OK) return err;

    // turn the auto-convert back on if it was changed
    if (was_auto)
    {
        value |= MAX31856_CR0_AUTOCONVERT;
        err = writeRegister8(MAX31856_CR0_REG, value);
    }
    return err;
}

/*Set temperature fault thresholds*/
esp_err_t SlapDAQ_MAX31856::setTempFaultThresholds(float flow, float fhigh)
{
    // registers hold signed 1/16 C counts in 16 bits, so set upper and lower limits to prevent overflow
    // when converting from int to uint
    if (flow < -2048.0f) flow = -2048.0f;
    if (flow > 2047.9375f) flow = 2047.9375f;
    if (fhigh < -2048.0f) fhigh = -2048.0f;
    if (fhigh > 2047.9375f) fhigh = 2047.9375f;
    
    const uint16_t low = (uint16_t) (int16_t) (flow * 16.0f);
    const uint16_t high = (uint16_t) (int16_t) (fhigh * 16.0f);


    esp_err_t err = writeRegister8(MAX31856_LTHFTH_REG, (uint8_t) (high >> 8));
    if (err == ESP_OK) err = writeRegister8(MAX31856_LTHFTL_REG, (uint8_t) high);
    if (err == ESP_OK) err = writeRegister8(MAX31856_LTLFTH_REG, (uint8_t) (low >> 8));
    if (err == ESP_OK) err = writeRegister8(MAX31856_LTLFTL_REG, (uint8_t) low);
    return err;
}

/*Returns if the conversion is complete*/
bool SlapDAQ_MAX31856::conversionComplete(void)
{
    uint8_t compareValue = 0;
    
    
    if (conversionMode == MAX31856_CONTINUOUS)
    {
        return true;
    }

    if (readRegister8(MAX31856_CR0_REG, &compareValue) != ESP_OK)
    {
        return false;
    }


    return !(compareValue & MAX31856_CR0_1SHOT);
}

/*Return cold-junction internal chip temperature*/
float SlapDAQ_MAX31856::readCJTemperature(void)
{
    //to make a 16 bit buffer: it holds both CJTH and CJTL bytes
    uint8_t b[2] = {};
    if (readRegisters(MAX31856_CJTH_REG, b, sizeof(b)) != ESP_OK) return NAN;
    
    
    // 14-bit two's complement (bits 15...2), dividing the 16-bit value by 256 gives degree in Celsius
    int16_t raw = (int16_t)(((uint16_t) b[0] << 8) | b[1]);      //adds the msb and lsb together to make 16 bits of data or 2 bytes
    
    return raw / 256.0f;
}

/*Return hot-junction thermocouple temperature*/
float SlapDAQ_MAX31856::readThermocoupleTemperature(void)
{
    // change this to function with just one read function
    // could result in incorrect readback errors from the transitions between 3
    // read commands
    uint8_t b[3] = {};
    if (readRegisters(MAX31856_LTCBH_REG, b, sizeof(b)) != ESP_OK) return NAN;
    
    // do 19-bit two's complement (bits 23...5)
    // build the 24 bits at the TOP of the 32-bit word
    // shift by 13 (8 pad bits + 5 unused bits)
    const uint32_t u = ((uint32_t) b[0] << 24) | ((uint32_t) b[1] << 16) | ((uint32_t) b[2] << 8);    //24 bit value unshifted
    const int32_t raw = (int32_t) u >> 13;

    return raw * 0.0078125f;

}

esp_err_t SlapDAQ_MAX31856::writeRegister8(uint8_t addr, uint8_t data)
{
    uint8_t tx_data[2];
    tx_data[0] = addr | 0x80;           // setting the write bit
    tx_data[1] = data;

    return spi_transfer(spi_thermo_handle, tx_data, NULL, sizeof(tx_data));
}


esp_err_t SlapDAQ_MAX31856::readRegister8(uint8_t addr, uint8_t *data)
{
    return readRegisters(addr, data, 1);
}

esp_err_t SlapDAQ_MAX31856::readRegisters(uint8_t addr, uint8_t *data, size_t count)
{
    if (data == nullptr || count == 0 || count > 16) return ESP_ERR_INVALID_ARG;

    uint8_t tx_data[17] = {};
    uint8_t rx_data[17] = {};

    tx_data[0] = addr & 0x7F;   // clears bit 7 for read command

    esp_err_t ret = spi_transfer(spi_thermo_handle, tx_data, rx_data, count + 1);

    if (ret == ESP_OK)
    {
        memcpy(data, &rx_data[1], count);   // rx_data[0] is clocked out while we send the address
    }

    return ret;
}
