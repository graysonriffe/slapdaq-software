
#include "adc.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *ADC_TAG = "ADS1261";

// return from a function that returns an esp_err_t
#define TRY(expr) do {esp_err_t _e = (expr); if (_e != ESP_OK) return _e;} while(0)


static void delayMs(uint32_t ms)
{
    TickType_t ticks = pdMS_TO_TICKS(ms);
    vTaskDelay(ticks ? ticks : 1);
}


/****************LOW-LEVEL SPI**************/
esp_err_t ADS126X::spiTransfer(const uint8_t *tx, uint8_t *rx, size_t len)
{
    return spi_transfer(spi_adc_handle, tx, rx, len);
}

// every replay starts with FFh, then echos 1 byte of a command (datasheet Tables 17-26)
// if this function fails, the ADC didn't hear us, so check physical wiring, SPI mode 1, power, RESET/PWDN pins.
esp_err_t ADS126X::checkFrame(uint8_t sent_cmd, const uint8_t *rx)
{
    if (rx[0] != 0xFF || rx[1] != sent_cmd)
    {
        ESP_LOGE(ADC_TAG, "Bad replay to cmd 0x%02X: got 0x%02X%02X (want 0xFF%02X)",
                 sent_cmd, rx[0], rx[1], sent_cmd);
        
        // add logs for common ADC responses
        if (rx[0] == 0x00 && rx[1] == 0x00)
        {
            ESP_LOGE(ADC_TAG, " MISO reads all 0s: ADC is not powered on, RESET/PWDN pin held low, or MISO shorted to GND");
            
        } else if (rx[0] == 0xFF && rx[1] == 0xFF)
            {
                ESP_LOGE(ADC_TAG, " MISO reads all 1s: MISO floating or wrong CS");
            }
            return ESP_ERR_INVALID_RESPONSE;
    }
    return ESP_OK;
}

uint8_t &ADS126X::regByte(uint8_t addr)
{
    return reinterpret_cast<uint8_t *>(&REGISTER)[addr];
}

/*****************COMMANDS***********/
esp_err_t ADS126X::sendCommand(uint8_t command)
{
    alignas(4) uint8_t tx[2] = {command, 0x00};
    alignas(4) uint8_t rx[2] = {0, 0};

    TRY(spiTransfer(tx, rx, sizeof(tx)));
    return checkFrame(command, rx);
}

esp_err_t ADS126X::noOperation()
{
    return sendCommand(ADS126X_NOP);
}

esp_err_t ADS126X::reset(void)
{
    initialized_ = false;

    TRY(sendCommand(ADS126X_RESET));
    delayMs(5);         // restart = 512 / fCLK ~= 70us. adding more room
    return readAllRegisters();
}

esp_err_t ADS126X::startADC(void)
{
    if (start_used)
    {
        // low -> high ege on START restarts a conversion
        gpio_set_level((gpio_num_t) start_pin, 0);
        esp_rom_delay_us(5);
        gpio_set_level((gpio_num_t) start_pin, 1);
        return ESP_OK;
    }
    return sendCommand(ADS126X_START);
    
}

esp_err_t ADS126X::stopADC(void)
{
    if (start_used)
    {
        gpio_set_level((gpio_num_t) start_pin, 0);
        return ESP_OK;
    }
    return sendCommand(ADS126X_STOP);
}

esp_err_t ADS126X::setStartPin(uint8_t pin)
{
    // drive latch low before enabling the output in case pin goes high unexpectedly
    gpio_set_level((gpio_num_t) pin, 0);

    gpio_config_t cfg = {};
    cfg.pin_bit_mask = 1ULL << pin;
    cfg.mode = GPIO_MODE_OUTPUT;
    cfg.pull_up_en = GPIO_PULLUP_DISABLE;
    cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    cfg.intr_type = GPIO_INTR_DISABLE;
    TRY(gpio_config(&cfg));
    
    
    start_used = true;
    start_pin = pin;
    return ESP_OK;
}


/********Register Acess - ONE register per command******/

//DIN: 20h+reg, arbitrary, 00h
//DOUT: FFh,    echo,       register data       (datasheet Table 25)
esp_err_t ADS126X::readRegister(uint8_t reg, uint8_t *value)
{
    if (reg >= ADS126X_REG_NUM) return ESP_ERR_INVALID_ARG;

    alignas(4) uint8_t tx[3] = { (uint8_t)(ADS126X_RREG | reg), 0x00, 0x00 };
    alignas(4) uint8_t rx[3] = { 0, 0, 0};

    TRY(spiTransfer(tx, rx, sizeof(tx)));
    TRY(checkFrame(tx[0], rx));

    regByte(reg) = rx[2];       //keeps shadow in sync with the chip
    if (value) *value = rx[2];
    return ESP_OK;
}

//DIN: 40h + reg, register data         (datasheet Table 26)
//DOUT: FFh,        echo
esp_err_t ADS126X::writeRegister(uint8_t reg)
{
    if (reg >= ADS126X_REG_NUM) return ESP_ERR_INVALID_ARG;

    alignas(4) uint8_t tx[2] = { (uint8_t)(ADS126X_WREG | reg), regByte(reg) };
    alignas(4) uint8_t rx[2] = { 0, 0};

    TRY(spiTransfer(tx, rx, sizeof(tx)));
    return checkFrame(tx[0], rx);
}

esp_err_t ADS126X::readAllRegisters(void)
{
    for (uint8_t reg = 0; reg < ADS126X_REG_NUM; reg++)
    {
        TRY(readRegister(reg, nullptr));
    }
    return ESP_OK;
}

esp_err_t ADS126X::commit(uint8_t reg)
{
    if (!initialized_) return ESP_ERR_INVALID_STATE;
    return writeRegister(reg);
}


/**********Set-Up Procedure**********/
// based on datasheet Figure 101

esp_err_t ADS126X::begin(void)
{
    initialized_ = false;

    // after chip powered on, it needs 2^16 fCLK ~= 8.9 ms before it accepts commands
    delayMs(20);

    //reset adc after mcu reboot just in case
    // also checks wiring
    TRY(stopADC());
    TRY(reset());


    if (REGISTER.ID.bit.DEV_ID != ADS126X_DEV_ID)
    {
        ESP_LOGE(ADC_TAG, "Unexpected ID register 0x%02X (DEV_ID should be 0x%X). Check SPI mode 1 / MISO / CS / power.", 
                 REGISTER.ID.reg, ADS126X_DEV_ID);
        return ESP_ERR_NOT_FOUND;
    }

    // append STATUS byte to conversion reads. 
    REGISTER.MODE3.bit.STATENB = ADS126X_STATENB_STAT;
    TRY(writeRegister(ADS126X_MODE3));

    //Pulse conversions: start one conversion with current input mux to make sure 
    // there aren't any conversions from a previous channel
    REGISTER.MODE1.bit.CONVRT = ADS126X_CONVRT_ONE_SHOT;
    TRY(writeRegister(ADS126X_MODE1));

    //RESET flag is set after every flag. Clear it to check if the ADC reset unexpectly
    REGISTER.STATUS.bit.RESET = 0;
    TRY(writeRegister(ADS126X_STATUS));

    // read back what was wrote (datasheet recommends verifying)
    const uint8_t want_mode1 = REGISTER.MODE1.reg;
    const uint8_t want_mode3 = REGISTER.MODE3.reg;
    uint8_t got_mode1 = 0, got_mode3 = 0, got_status = 0;
    TRY(readRegister(ADS126X_MODE1, &got_mode1));
    TRY(readRegister(ADS126X_MODE3, &got_mode3));
    TRY(readRegister(ADS126X_STATUS, &got_status));
    if (got_mode1 != want_mode1 || got_mode3 != want_mode3 || (got_status & 0x01))
    {
        ESP_LOGE(ADC_TAG, "Register readback mismath: MODE1 0x%02X/0x%02X MODE3 0x%02X/0x%02X STATUS 0x%02X (RESET bit should be 0)",
                 got_mode1, want_mode1, got_mode3, want_mode3, got_status);
        return ESP_ERR_INVALID_RESPONSE;
    }

    initialized_ = true;
    ESP_LOGI(ADC_TAG, "ADS1261 found (ID 0x%02X), ready", REGISTER.ID.reg);
    return ESP_OK;
}

/***********Reading Conversions***********/
esp_err_t ADS126X::readADC(uint8_t pos_mux, uint8_t neg_mux, int32_t *raw, uint32_t timeout_ms)
{
    if (!initialized_)                      return ESP_ERR_INVALID_STATE;
    if (raw == nullptr)                       return ESP_ERR_INVALID_ARG;
    if (pos_mux > 0x0F || neg_mux > 0x0F)    return ESP_ERR_INVALID_ARG;
    
    //1. Select the inputs (only touch the chip if something changed)
    if (REGISTER.INPMUX.bit.MUXP != pos_mux || REGISTER.INPMUX.bit.MUXN != neg_mux)
    {
        REGISTER.INPMUX.bit.MUXP = pos_mux;
        REGISTER.INPMUX.bit.MUXN = neg_mux;
        TRY(writeRegister(ADS126X_INPMUX));
    }

    // RDATA with the STATUS byte is 6 bytes
    //      DIN: 12h, arbitrary, 00h,   00h,        00h,        00h
    //      DOUT: FFh, echo,    STATUS, data MSB, data MID, data LSB        datasheet table 21
    alignas(4) uint8_t tx[6] = { ADS126X_RDATA, 0x00, 0x00, 0x00, 0x00, 0x00};
    alignas(4) uint8_t rx[6] = {};


    //2. clear any stale data from an earlier transfer
    TRY(spiTransfer(tx, rx, sizeof(tx)));
    TRY(checkFrame(ADS126X_RDATA, rx));
    
    // 3. Start ONE conversion (pulse mode)
    TRY(startADC());

    // 4. Poll with RDATA until status byte    
    const int64_t deadline_us = esp_timer_get_time() + (int64_t) timeout_ms * 1000;

    while (true)
    {
        for (uint8_t &b : rx) b = 0;
        TRY(spiTransfer(tx, rx, sizeof(tx)));
        TRY(checkFrame(ADS126X_RDATA, rx));

        STATUS.reg = rx[2];
        if (STATUS.bit.DRDY) break;     // new conversion results

        if (esp_timer_get_time() > deadline_us)
        {
            ESP_LOGE(ADC_TAG, "Timed out waiting for a conversion (is the clock running? is START pin held low?)");
            return ESP_ERR_TIMEOUT;
        }
        vTaskDelay(1);
    }

    //5. Assemble the 24-bit two's complement result and sign-extend it to 32 bits
    const uint32_t u = ((uint32_t) rx[3] << 16) | ((uint32_t) rx[4] << 8) | (uint32_t) rx[5];
    *raw = (int32_t) ((u ^ 0x800000u) - 0x800000u);
    return ESP_OK;
}

// One LSB = VREF / (Gain * 2^23)  (datasheet Table 15)
double ADS126X::toVolts(int32_t raw, double vref_volts) const
{
    const double gain = REGISTER.PGA.bit.BYPASS ? 1.0 : (double) (1u << REGISTER.PGA.bit.GAIN);
    return (double) raw * vref_volts / (gain * 8388608.0);
}


/**************IDAC************/
esp_err_t ADS126X::setIDAC1Pin(uint8_t pin)
{
    if (pin > 0x0F) return ESP_ERR_INVALID_ARG;
    REGISTER.IMUX.bit.IMUX1 = pin;
    return commit(ADS126X_IMUX);
}

esp_err_t ADS126X::setIDAC2Pin(uint8_t pin)
{
    if (pin > 0x0F) return ESP_ERR_INVALID_ARG;
    REGISTER.IMUX.bit.IMUX2 = pin;
    return commit(ADS126X_IMUX);
}

esp_err_t ADS126X::setIDAC1Mag(uint8_t magnitude)
{
    if (magnitude > ADS126X_IMAG_3000_UA) return ESP_ERR_INVALID_ARG;
    REGISTER.IMAG.bit.IMAG1 = magnitude;
    return commit(ADS126X_IMAG);
}

esp_err_t ADS126X::setIDAC2Mag(uint8_t magnitude)
{
    if (magnitude > ADS126X_IMAG_3000_UA) return ESP_ERR_INVALID_ARG;
    REGISTER.IMAG.bit.IMAG2 = magnitude;
    return commit(ADS126X_IMAG);
}

/*************POWER REGISTER / REFERENCE / BIAS************/

esp_err_t ADS126X::checkResetBit(bool *was_reset)
{
    if (was_reset == nullptr) return ESP_ERR_INVALID_ARG;
    TRY(readRegister(ADS126X_STATUS, nullptr));
    *was_reset = REGISTER.STATUS.bit.RESET;
    return ESP_OK;
}

esp_err_t ADS126X::clearResetBit(void)
{
    REGISTER.STATUS.bit.RESET = 0;
    return commit(ADS126X_STATUS);
}

esp_err_t ADS126X::enableLevelShift(void)
{
    REGISTER.INPBIAS.bit.VBIAS = 1;
    return commit(ADS126X_INPBIAS);
}

esp_err_t ADS126X::disableLevelShift(void)
{
    REGISTER.INPBIAS.bit.VBIAS = 0;
    return commit(ADS126X_INPBIAS);
}

esp_err_t ADS126X::enableInternalReference(void)
{
    REGISTER.REFMUX.bit.REFENB = 1;
    return commit(ADS126X_REFMUX);
}

esp_err_t ADS126X::disableInternalReference(void)
{
    REGISTER.REFMUX.bit.REFENB = 0;
    return commit(ADS126X_REFMUX);
}

esp_err_t ADS126X::setReference(uint8_t negativeReference, uint8_t positiveReference)
{
    if (negativeReference > 3 || positiveReference > 3) return ESP_ERR_INVALID_ARG;
    REGISTER.REFMUX.bit.RMUXN = negativeReference;
    REGISTER.REFMUX.bit.RMUXP = positiveReference;
    return commit(ADS126X_REFMUX);
}

/***********Status byte from last readADC()*********/


uint8_t ADS126X::lastStatus(void) const     {return STATUS.reg;}
bool ADS126X::lastDataReady(void) const     {return STATUS.bit.DRDY;}
bool ADS126X::lastClockSource(void) const   {return STATUS.bit.CLOCK;}
bool ADS126X::lastADCLowReferenceAlarm(void) const  {return STATUS.bit.REFL_ALM;}
bool ADS126X::lastADCPGAOutputLowAlarm(void) const  {return STATUS.bit.PGAL_ALM;}
bool ADS126X::lastADCPGAOutputHighAlarm(void) const {return STATUS.bit.PGAH_ALM;}
bool ADS126X::lastReset(void) const         {return STATUS.bit.RESET;}


/***********MODE0 / MODE1 (writing either will restart a conversion in progress) **************/

esp_err_t ADS126X::setFilter(uint8_t filter)
{
    if (filter > ADS126X_FILTER_FIR) return ESP_ERR_INVALID_ARG;
    REGISTER.MODE0.bit.FILTER = filter;
    return commit(ADS126X_MODE0);
}

esp_err_t ADS126X::setRate(uint8_t rate)
{
    if (rate > ADS126X_DR_40000) return ESP_ERR_INVALID_ARG;
    REGISTER.MODE0.bit.DR = rate;
    return commit(ADS126X_MODE0);
}

esp_err_t ADS126X::setDelay(uint8_t del)
{
    if (del > ADS126X_DELAY_17_8_MS) return ESP_ERR_INVALID_ARG;
    REGISTER.MODE1.bit.DELAY = del;
    return commit(ADS126X_MODE1);
}

esp_err_t ADS126X::setContinuousMode(void)
{
    REGISTER.MODE1.bit.CONVRT = ADS126X_CONVRT_CONT;
    return commit(ADS126X_MODE1);
}

esp_err_t ADS126X::setPulseMode(void)
{
    REGISTER.MODE1.bit.CONVRT = ADS126X_CONVRT_ONE_SHOT;
    return commit(ADS126X_MODE1);
}

esp_err_t ADS126X::setChopMode(uint8_t mode)
{
    if (mode > 3) return ESP_ERR_INVALID_ARG;
    REGISTER.MODE1.bit.CHOP = mode;
    return commit(ADS126X_MODE1);
}


/*********MODE2 = connection + direction; MODE3 = data*************/

esp_err_t ADS126X::gpioConnect(uint8_t pin)
{
    if (pin > 3) return ESP_ERR_INVALID_ARG;
    REGISTER.MODE2.bit.GPIO_CON = REGISTER.MODE2.bit.GPIO_CON | (1u << pin);
    return commit(ADS126X_MODE2);
}

esp_err_t ADS126X::gpioDisconnect(uint8_t pin)
{
    if (pin > 3) return ESP_ERR_INVALID_ARG;
    REGISTER.MODE2.bit.GPIO_CON = REGISTER.MODE2.bit.GPIO_CON & ~(1u << pin);;
    return commit(ADS126X_MODE2);
}

//NOTE: datasheet polarity GPIO_DIR bit 0 = OUTPUT, 1 = INPUT
esp_err_t ADS126X::gpioDirection(uint8_t pin, uint8_t direction)
{
    if (pin > 3) return ESP_ERR_INVALID_ARG;
    if (direction != ADS126X_GPIO_OUTPUT && direction != ADS126X_GPIO_INPUT) return ESP_ERR_INVALID_ARG;
    
    if (direction == ADS126X_GPIO_OUTPUT)
    {
        REGISTER.MODE2.bit.GPIO_DIR = REGISTER.MODE2.bit.GPIO_DIR & ~(1u << pin);
    }
    else
    {
        REGISTER.MODE2.bit.GPIO_DIR = REGISTER.MODE2.bit.GPIO_DIR | (1u << pin);
    }
    return commit(ADS126X_MODE2);
}

esp_err_t ADS126X::gpioWrite(uint8_t pin, uint8_t val)
{
    if (pin > 3) return ESP_ERR_INVALID_ARG;
    if (val)
    {
        REGISTER.MODE3.bit.GPIO_DAT = REGISTER.MODE3.bit.GPIO_DAT | (1u << pin);
    }
    else
    {
        REGISTER.MODE3.bit.GPIO_DAT = REGISTER.MODE3.bit.GPIO_DAT & ~(1u << pin);
    }
    return commit(ADS126X_MODE3);
}

esp_err_t ADS126X::gpioRead(uint8_t pin, bool *level)
{
    if (pin > 3 || level == nullptr) return ESP_ERR_INVALID_ARG;
    TRY(readRegister(ADS126X_MODE3, nullptr));
    *level = (REGISTER.MODE3.bit.GPIO_DAT >> pin) & 1u;
    return ESP_OK;
}


/************PGA**********/

esp_err_t ADS126X::enablePGA(void)
{
    REGISTER.PGA.bit.BYPASS = ADS126X_PGA_MODE;
    return commit(ADS126X_PGA);
}

esp_err_t ADS126X::disablePGA(void)
{
    REGISTER.PGA.bit.BYPASS = ADS126X_PGA_BYPASS;
    return commit(ADS126X_PGA);
}

esp_err_t ADS126X::setGain(uint8_t gain)
{
    if (gain > ADS126X_PGA_GAIN_128) return ESP_ERR_INVALID_ARG;
    REGISTER.PGA.bit.GAIN = gain;
    return commit(ADS126X_PGA);
}

esp_err_t ADS126X::setPGA(bool bypass, uint8_t gain)
{
    if (!initialized_) return ESP_ERR_INVALID_STATE;
    if (gain > ADS126X_PGA_GAIN_128) return ESP_ERR_INVALID_ARG;

    ADS126X_PGA_Type want = REGISTER.PGA;
    want.bit.BYPASS = bypass ? ADS126X_PGA_BYPASS : ADS126X_PGA_MODE;
    want.bit.GAIN = bypass ? ADS126X_PGA_GAIN_1 : gain;

    if (want.reg == REGISTER.PGA.reg) return ESP_OK;

    REGISTER.PGA = want;
    return writeRegister(ADS126X_PGA);
}
