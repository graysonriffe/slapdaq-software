
#ifndef ADC_DEFINITIONS_H
#define ADC_DEFINITIONS_H



#ifdef __cplusplus
extern "C" {
#endif

#ifndef __IO
#define __IO volatile
#endif

// command addresses byte
#define ADS126X_NOP               0x00
#define ADS126X_RESET             0x06
#define ADS126X_START             0x08
#define ADS126X_STOP              0x0A

#define ADS126X_RDATA             0x12

#define ADS126X_SYOCAL            0x16
#define ADS126X_GANCAL            0x17   // called SFOCAL in reference code
#define ADS126X_SFOCAL            0x19

#define ADS126X_RREG              0x20  // +rrh = 5-bit register address
#define ADS126X_WREG              0x40  // +rrh = 5-bit register address

//register addresses 
#define ADS126X_ID                0x00
#define ADS126X_STATUS            0x01
#define ADS126X_MODE0             0x02
#define ADS126X_MODE1             0x03
#define ADS126X_MODE2             0x04
#define ADS126X_MODE3             0x05
#define ADS126X_OFCAL0            0x07
#define ADS126X_OFCAL1            0x08
#define ADS126X_OFCAL2            0x09
#define ADS126X_FSCAL0            0x0A
#define ADS126X_FSCAL1            0x0B
#define ADS126X_FSCAL2            0x0C
#define ADS126X_IMUX              0x0D
#define ADS126X_IMAG              0x0E
#define ADS126X_RESERVED          0x0F
#define ADS126X_PGA               0x10
#define ADS126X_INPMUX            0x11
#define ADS126X_INPBIAS           0x12

#define ADS126X_REG_NUM           0x13 // number of registers

/**************************UNCLEAR******************************/

//checksum macros - not sure if we are using CRC yet
#define ADS126X_CRCEN             0b1 // used to enable CRC
#define ADS126X_CRCDIS            0b0

/**************************UNCLEAR******************************/


//Status bit options
#define ADS126X_LOCK_UNLOCK                0b0 // Indicates register lock status
#define ADS126X_LOCK_LOCK                  0b1
#define ADS126X_CRCERR_NOCRC               0b0 // If you want ADC to detect CRC
#define ADS126X_CRCERR_CRC                 0b1
#define ADS126X_PGAL_ALM_NOALM             0b0 // Indicates PGA output voltage is below low limit
#define ADS126X_PGAL_ALM_ALM               0b1
#define ADS126X_PGAH_ALM_NOALM             0b0 // Indicates PGA output voltage is above high limit
#define ADS126X_PGAH_ALM_ALM               0b1
#define ADS126X_REFL_ALM_NOALM             0b0 // Indicates reference voltage is below low limit
#define ADS126X_REFL_ALM_ALM               0b1
#define ADS126X_DRDY_NOTNEW                0b0 // Indicates conversion data ready
#define ADS126X_DRDY_NEW                   0b1
#define ADS126X_CLOCK_INT                  0b0 // Indicates if clock is external or internal
#define ADS126X_CLOCK_EXT                  0b1
#define ADS126X_RESET_NO                   0b0 
#define ADS126X_RESET_YES                  0b1

//Mode0 bit options                        all in SPS aka Hz
#define ADS126X_DR_2_5                     0b00000
#define ADS126X_DR_5                       0b00001
#define ADS126X_DR_10                      0b00010
#define ADS126X_DR_16_6                    0b00011
#define ADS126X_DR_20                      0b00100
#define ADS126X_DR_50                      0b00101
#define ADS126X_DR_60                      0b00110
#define ADS126X_DR_100                     0b00111
#define ADS126X_DR_400                     0b01000
#define ADS126X_DR_1200                    0b01001
#define ADS126X_DR_2400                    0b01010
#define ADS126X_DR_4800                    0b01011
#define ADS126X_DR_7200                    0b01100
#define ADS126X_DR_14400                   0b01101
#define ADS126X_DR_19200                   0b01110
#define ADS126X_DR_25600                   0b01111
#define ADS126X_DR_40000                   0b10000

//Mode1 bit options
#define ADS126X_ZERO_BIT                   0b0
#define ADS126X_CHOP_NORM                  0b00
#define ADS126X_CHOP_CHOP                  0b01
#define ADS126X_CHOP_2_WIRE                0b10
#define ADS126X_CHOP_4_WIRE                0b11
#define ADS126X_CONVRT_CONT                0b0
#define ADS126X_CONVRT_ONE_SHOT            0b1
#define ADS126X_DELAY_0_US                 0b0000
#define ADS126X_DELAY_50_US                0b0001
#define ADS126X_DELAY_59_US                0b0010
#define ADS126X_DELAY_67US                 0b0011
#define ADS126X_DELAY_85_US                0b0100
#define ADS126X_DELAY_119_US               0b0101
#define ADS126X_DELAY_189_US               0b0110
#define ADS126X_DELAY_328_US               0b0111
#define ADS126X_DELAY_605_US               0b1000
#define ADS126X_DELAY_1_16_MS              0b1001
#define ADS126X_DELAY_2_27_MS              0b1010
#define ADS126X_DELAY_4_49_MS              0b1011
#define ADS126X_DELAY_8_93_MS              0b1100
#define ADS126X_DELAY_17_8_MS              0b1101

//Mode2 bit options
#define ADS126X_GPIO_CON_3_OFF             0b0      //AIN5
#define ADS126X_GPIO_CON_3_ON              0b1
#define ADS126X_GPIO_CON_2_OFF             0b0      //AIN4
#define ADS126X_GPIO_CON_2_ON              0b1
#define ADS126X_GPIO_CON_1_OFF             0b0      //AIN3
#define ADS126X_GPIO_CON_1_ON              0b1
#define ADS126X_GPIO_CON_0_OFF             0b0      //AIN2
#define ADS126X_GPIO_CON_0_ON              0b1
#define ADS126X_GPIO_DIR_3_OUT             0b0      //AIN5
#define ADS126X_GPIO_DIR_3_IN              0b1
#define ADS126X_GPIO_DIR_2_OUT             0b0      //AIN4
#define ADS126X_GPIO_DIR_2_IN              0b1
#define ADS126X_GPIO_DIR_1_OUT             0b0      //AIN3
#define ADS126X_GPIO_DIR_1_IN              0b1
#define ADS126X_GPIO_DIR_0_OUT             0b0      //AIN2
#define ADS126X_GPIO_DIR_0_IN              0b1

//Mode3 bit options
#define ADS126X_PWDN_NORM                  0b0
#define ADS126X_PWDN_SOFT                  0b1
#define ADS126X_STATENB_NO_STAT            0b0
#define ADS126X_STATENB_STAT               0b1
#define ADS126X_CRCENB_NO_CRC              0b0
#define ADS126X_CRCENB_CRC                 0b1
#define ADS126X_SPITIM_DISABLE             0b0
#define ADS126X_SPITIM_ENABLE              0b1
#define ADS126X_GPIO_DAT_3_LOW             0b0      //read
#define ADS126X_GPIO_DAT_3_HIGH            0b1      //write
#define ADS126X_GPIO_DAT_2_LOW             0b0
#define ADS126X_GPIO_DAT_2_HIGH            0b1
#define ADS126X_GPIO_DAT_1_LOW             0b0
#define ADS126X_GPIO_DAT_1_HIGH            0b1
#define ADS126X_GPIO_DAT_0_LOW             0b0
#define ADS126X_GPIO_DAT_0_HIGH            0b1

/****************** */
//ADD REFERENCE MACROS!!!!!



//Pins to connect for outputs!!!!
#define ADS126X_AIN_0                      0b0000
#define ADS126X_AIN_1                      0b0001
#define ADS126X_AIN_2                      0b0010
#define ADS126X_AIN_3                      0b0011
#define ADS126X_AIN_4                      0b0100
#define ADS126X_AIN_5                      0b0101
#define ADS126X_AIN_6                      0b0110
#define ADS126X_AIN_7                      0b0111
#define ADS126X_AIN_8                      0b1000
#define ADS126X_AIN_9                      0b1001
#define ADS126X_AIN_COM                    0b1010
#define ADS126X_AIN_NO_CONNECT             0b1111

//Pins to connect for outputs












#ifdef __cplusplus
}
#endif

#endif
