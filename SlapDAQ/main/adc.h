
#ifndef ADC_H
#define ADC_H

//Necessary Imports
#include <stdint.h>
#include "adc_definitions.h"
#include "dig_IO.h"
#include "spi_DAQ.h"


class ADS126X
{
    public:
        // Initialization
        void begin(uint8_t chip_select);
        void begin(void);
        ADS126X(void);
        void setStartPin(uint8_t pin);      // designate pin to START

        // General ADC Commands
        void noOperation(void);
        void reset(void);
        void startADC(void);
        void stopADC(void);


        // Analog Read Functions
        int32_t readADC(uint8_t pos_pin, uint8_t neg_pin);


        // Calibration Functions
        void calibrateSysOffsetADC(uint8_t shorted1, uint8_t shorted2);
        void calibrateGainADC(uint8_t vcc_pin, uint8_t gnd_pin);
        void calibrateSelfOffset(void);


        // IDAC Functions
        void setIDAC1Pin(uint8_t pin);
        void setIDAC2Pin(uint8_t pin);
        void setIDAC1Mag(uint8_t magnitude);
        void setIDAC2Mag(uint8_t magnitude);


        // POWER Functions
        bool checkResetBit(void);
        void clearResetBit(void);
        void enableLevelShift(void);
        void disableLevelShift(void);
        void enableInternalReference(void);
        void disableInternalReference(void);


        //Checksum Functions
        void disableCheck(void);
        void setChecksumMode(void);
        void setCRCMode(void);
        bool lastChecksum(void);


        // Status Functions
        void enableStatus(void);
        void disableStatus(void);
        uint8_t lastStatus(void);       // returns entire status byte
        bool lastADCStatus(void);
        bool lastClockSource(void);
        bool lastADCLowReferenceAlarm(void);
        bool lastADCPGAOutputLowAlarm(void);
        bool lastADCPGAOutputHighAlarm(void);
        bool lastADCPGADifferentialOutputAlarm(void);
        bool lastReset(void);


        // MODE0 Functions
        void setFilter(uint8_t filter);
        void setRate(uint8_t rate);

        void setReference(uint8_t negativeReference, uint8_t positiveReference);

        // MODE1 Functions
        void setDelay(uint8_t del);
        void setContinuousMode(void);
        void setPulseMode(void);
        void setChopMode(uint8_t mode);


        // MODE2 Functions
        void gpioConnect(uint8_t pin);
        void gpioDisconnect(uint8_t pin);
        void gpioDirection(uint8_t pin, uint8_t direction);
        void gpioWrite(uint8_t pin, uint8_t val);
        bool gpioRead(uint8_t pin);


        // PGA Functions
        void enablePGA(void);
        void disablePGA(void);
        void setGain(uint8_t gain);

        //spi functions
        void spiWrite(uint8_t buffer[], uint8_t length);
        void spiRead(uint8_t buffer[], uint8_t length);


        // Main Commands
        
        /*
        Holds all values of the register. Commands are done 
        through this
        */

        ADS126X_REGISTER_Type REGISTER;

        /*
        An array to call each register by the offset.
        It has the same memory location as REGISTER, so the changes
        will reflect on both
        */

        __IO unsigned char *REGISTER_ARRAY = (uint8_t) &REGISTER.ID.reg;


        bool cs_used = false;
        uint8_t cs_pin;
        bool start_used = false;
        uint8_t start_pin;


        ADS126X_STATUS_Type STATUS;     // save last status and checksum values
        uint8_t CHECKSUM;


        void sendCommand(uint8_t command);      // sends a single command
        void readRegisters(uint8_t start_reg, uint8_t num);
        void writeRegisters(uint8_t start_reg, uint8_t num);
        uint8_t readRegister(uint8_t reg);
        void writeRegister(uint8_t reg);


        // checksum functions (reference page 52 to understand more)
        uint8_t find_checksum(uint32_t val, uint8_t byt);
        uint8_t find_crc(uint32_t val, uint8_t byt);
        uint8_t msb_pos(uint64_t val);      // returns the position of the most significant bit


};





#endif
