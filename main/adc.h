
#ifndef ADC_H
#define ADC_H

//Necessary Imports
#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"
#include "adc_definitions.h"
#include "spi_DAQ.h"


// library: the ADS1261 has no multi-register burst read/write, and every command is 2 bytes long.
//
// Conventions
//   * Every function that talks to the chip returns esp_err_t (ESP_OK on success) - same style as readRegister8.
//   * The driver keeps a RAM "shadow" copy of the 19 registers in REGISTER. Setters change the shadow, then write
//     that one register to the chip. begin() loads the shadow from the real chip, so the two start out in sync.
//   * CRC mode is NOT supported (it changes the framing of every command). Leave MODE3.CRCENB = 0.
//   * Chip select is handled by the ESP-IDF SPI driver (spics_io_num in spi_DAQ.cpp) - one CS frame per call to
//     spi_device_transmit(). Do not toggle CS with gpio_set_level() as well.
//   * The SPI device for this ADC must be configured for SPI MODE 1 (CPOL = 0, CPHA = 1). See datasheet 9.5.1.

class ADS126X
{
    public:
        ADS126X() = default;

        // Initialization
        // reset, verify the chip answers using the ID register, load the shadow registers, then configure it.
        esp_err_t begin(void);

        // tie START pin to ground as its unused
        esp_err_t setStartPin(uint8_t pin);

        // General ADC Commands
        esp_err_t noOperation(void);
        esp_err_t reset(void);
        esp_err_t startADC(void);
        esp_err_t stopADC(void);


        // Analog Read Functions
        esp_err_t readADC(uint8_t pos_mux, uint8_t neg_mux, int32_t *raw, uint32_t timeout_ms = 2000);

        // code to volts
        double toVolts(int32_t raw, double vref_volts) const;

        // IDAC Functions
        esp_err_t setIDAC1Pin(uint8_t pin);
        esp_err_t setIDAC2Pin(uint8_t pin);
        esp_err_t setIDAC1Mag(uint8_t magnitude);
        esp_err_t setIDAC2Mag(uint8_t magnitude);

        // POWER Functions
        esp_err_t checkResetBit(bool *was_reset);
        esp_err_t clearResetBit(void);
        esp_err_t enableLevelShift(void);
        esp_err_t disableLevelShift(void);
        esp_err_t enableInternalReference(void);
        esp_err_t disableInternalReference(void);
        esp_err_t setReference(uint8_t negativeReference, uint8_t positiveReference);

        // Status byte from most recent readADC()
        uint8_t lastStatus(void) const;
        bool lastDataReady(void) const;
        bool lastClockSource(void) const;
        bool lastADCLowReferenceAlarm(void) const;
        bool lastADCPGAOutputLowAlarm(void) const;
        bool lastADCPGAOutputHighAlarm(void) const;
        bool lastReset(void) const;

        // MODE0 / MODE1 -----------------------------------------------------------------
        esp_err_t setFilter(uint8_t filter);
        esp_err_t setRate(uint8_t rate);
        esp_err_t setDelay(uint8_t del);
        esp_err_t setContinuousMode(void);      // note: readADC() expects pulse mode
        esp_err_t setPulseMode(void);
        esp_err_t setChopMode(uint8_t mode);

        // MODE2 / MODE3: the four ADC GPIO0...3 share pins with AIN2...AIN5
        esp_err_t gpioConnect(uint8_t pin);
        esp_err_t gpioDisconnect(uint8_t pin);
        esp_err_t gpioDirection(uint8_t pin, uint8_t direction);    // ADS126X_GPIO_OUTPUT / _INPUT
        esp_err_t gpioWrite(uint8_t pin, uint8_t val);
        esp_err_t gpioRead(uint8_t pin, bool *level);

        // PGA ------------------------------------------------------
        esp_err_t enablePGA(void);
        esp_err_t disablePGA(void);         // = PGA bypass
        esp_err_t setGain(uint8_t gain);
        esp_err_t setPGA(bool bypass, uint8_t gain);

        //----Rgister-level access
        esp_err_t sendCommand(uint8_t command);
        esp_err_t readRegister(uint8_t reg, uint8_t *value);      // also refreshes REGISTER
        esp_err_t writeRegister(uint8_t reg);                        // write REGISTER's copy of reg to the chip
        esp_err_t readAllRegisters(void);

        ADS126X_REGISTER_Type REGISTER = {};        // shadow copy of chip's registers
        ADS126X_STATUS_Type   STATUS   = {};        // returned by the last readADC()


    private:
        esp_err_t spiTransfer(const uint8_t *tx, uint8_t *rx, size_t len);
        static esp_err_t checkFrame(uint8_t sent_cmd, const uint8_t *rx);
        uint8_t &regByte(uint8_t addr);     // shadow register by address
        esp_err_t commit(uint8_t reg);      // setter helper

        bool initialized_ = false;
        bool start_used = false;
        uint8_t start_pin = 0;

};





#endif
