
#ifndef ADC_DEFINITIONS_H
#define ADC_DEFINITIONS_H

#include <stdint.h>

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

// Enable options that are used in most registers
#define ADS126X_ENABLE            0b01
#define ADS126X_DISABLE           0b00

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
//#define ADS126X_SPITIM_DISABLE             0b0
//#define ADS126X_SPITIM_ENABLE              0b1
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
//#define ADS126X_REFENB_ENABLE              0b0
//#define ADS126X_REFENB_DISABLE             0b1
#define ADS126X_RMUXP_INTRN_REFP           0b00
#define ADS126X_RMUXP_INTRN_AVDD           0b01
#define ADS126X_RMUXP_INTRN_AIN0           0b10
#define ADS126X_RMUXP_INTRN_AIN2           0b11
#define ADS126X_RMUXN_INTRN_REFN           0b00
#define ADS126X_RMUXN_INTRN_AVSS           0b01
#define ADS126X_RMUXN_INTRN_AIN1           0b10
#define ADS126X_RMUXN_INTRN_AIN3           0b11

// Pins to connect to. Input MUX
#define ADS126X_MUXP_AINCOM                0b0000
#define ADS126X_MUXP_AIN0                  0b0001
#define ADS126X_MUXP_AIN1                  0b0010
#define ADS126X_MUXP_AIN2                  0b0011
#define ADS126X_MUXP_AIN3                  0b0100
#define ADS126X_MUXP_AIN4                  0b0101
#define ADS126X_MUXP_AIN5                  0b0110
#define ADS126X_MUXP_AIN6                  0b0111
#define ADS126X_MUXP_AIN7                  0b1000
#define ADS126X_MUXP_AIN8                  0b1001
#define ADS126X_MUXP_AIN9                  0b1010
#define ADS126X_MUXP_TEMP_SENSE_P          0b1011
#define ADS126X_MUXP_AVDD_AVSS_4P          0b1100
#define ADS126X_MUXP_DVDD_P                0b1101
#define ADS126X_MUXP_INP_OPEN              0b1110
#define ADS126X_MUXP_CONN_TO_VCOM          0b1111

//IDAC Output pins
#define ADS126X_IDAC_AIN_0                     0b0000
#define ADS126X_IDAC_AIN1                      0b0001
#define ADS126X_IDAC_AIN2                      0b0010
#define ADS126X_IDAC_AIN3                      0b0011
#define ADS126X_IDAC_AIN4                      0b0100
#define ADS126X_IDAC_AIN5                      0b0101
#define ADS126X_IDAC_AIN6                      0b0110
#define ADS126X_IDAC_AIN7                      0b0111
#define ADS126X_IDAC_AIN8                      0b1000
#define ADS126X_IDAC_AIN9                      0b1001
#define ADS126X_IDAC_AIN_COM                   0b1010
#define ADS126X_IDAC_AIN_NO_CONNECT            0b1111

//IDAC Magnitude
#define ADS126X_IMAG_50_UA                     0b0001
#define ADS126X_IMAG_100_UA                    0b0010
#define ADS126X_IMAG_250_UA                    0b0011
#define ADS126X_IMAG_500_UA                    0b0100
#define ADS126X_IMAG_750_UA                    0b0101
#define ADS126X_IMAG_1000_UA                   0b0110
#define ADS126X_IMAG_1500_UA                   0b0111
#define ADS126X_IMAG_2000_UA                   0b1000
#define ADS126X_IMAG_2500_UA                   0b1001
#define ADS126X_IMAG_3000_UA                   0b1010

// PGA Macros
#define ADS126X_PGA_MODE                       0b0
#define ADS126X_PGA_BYPASS                     0b1
#define ADS126X_PGA_GAIN_1                     0b000
#define ADS126X_PGA_GAIN_2                     0b001
#define ADS126X_PGA_GAIN_4                     0b010
#define ADS126X_PGA_GAIN_8                     0b011
#define ADS126X_PGA_GAIN_16                    0b100
#define ADS126X_PGA_GAIN_32                    0b101
#define ADS126X_PGA_GAIN_64                    0b110
#define ADS126X_PGA_GAIN_128                   0b111

// Input Bias Macros
#define ADS126X_BOCSP_PULLUP                   0b0
#define ADS126X_BOCSP_PULLDOWN                 0b1
#define ADS126X_BOCS_OFF                       0b000
#define ADS126X_BOCS_50_NA                     0b001
#define ADS126X_BOCS_200_NA                    0b010
#define ADS126X_BOCS_1_UA                      0b011
#define ADS126X_BOCS_10_UA                     0b100


/* Make register maps using structs */

// ID Register
typedef union 
{
    struct 
    {
        uint8_t REV_ID:4;               //bit: 0...4 Revision ID
        uint8_t DEV_ID:4;               //bit: 5...8 Device ID
    } bit;
    uint8_t reg;
} ADS126X_ID_Type;

// Status Register
typedef union 
{
    struct
    {
        uint8_t RESET:1;                //bit: 0 Reset
        uint8_t CLOCK:1;                //bit: 1 Clock
        uint8_t DRDY:1;                 //bit: 2 Data Ready
        uint8_t REFL_ALM:1;             //bit: 3 Reference Low Alarm
        uint8_t PGAH_ALM:1;             //bit: 4 PGA High Alarm
        uint8_t PGAL_ALM:1;             //bit: 5 PGA Low Alarm
        uint8_t CRCERR:1;               //bit: 6 CRC Error
        uint8_t LOCK:1;                 //bit: 7 Register Lock Status
    } bit;
} ADS126X_STATUS_Type;

// MODE0 Register
typedef union
{
    struct
    {
        uint8_t FILTER:3;               //bit: 0...2 Digital Filter
        uint8_t DR:5;                   //bit: 3...7 Data Rate
    } bit;
} ADS126X_MODE0_Type;

// MODE1 Register
typedef union
{
    struct
    {
        uint8_t DELAY:4;                //bit: 0...3 Conversion Start Delay
        uint8_t CONVRT:1;               //bit: 4     ADC Conversion Delay
        uint8_t CHOP:2;                 //bit: 5...6 Chop and AC-Excitation Modes
        uint8_t :1;                     //bit: 7     Reserved
    } bit;
} ADS126X_MODE1_Type;

// MODE2 Register
typedef union
{
    struct
    {
        uint8_t GPIO_DIR:4;             //bit: 0...3 GPIO Pin Direction
        uint8_t GPIO_CON:4;             //bit: 4...7 GPIO Pin Connection
    } bit;
} ADS126X_MODE2_Type;

// MODE3 Register
typedef union
{
    struct
    {
        uint8_t GPIO_DAT:4;             //bit: 0...3 GPIO Data
        uint8_t SPITIM:1;               //bit: 4 SPI Auto-Reset Function
        uint8_t CRCENB:1;               //bit: 5 CRC Data Verification
        uint8_t STATENB:1;              //bit: 6 STATUS Byte
        uint8_t PWDN:1;                 //bit: 7 Software Power-Down Mode
    } bit;
} ADS126X_MODE3_Type;

// REF Register
typedef union
{
    struct
    {
        uint8_t RMUXN:2;                //bit: 0...1 Reference Negative Input
        uint8_t RMUXP:2;                //bit: 2...3 Reference Positive Input
        uint8_t REFENB:1;               //bit: 4     Internal Reference Enable
        uint8_t :3;                     //bit: 5...7 Reserved
    } bit;
} ADS126X_REF_Type;

// Offset Register
typedef union
{
    struct
    {
        uint8_t OFC:8;                  //bit: 0...7 Offset Calibration
    } bit;
} ADS126X_OFCAL_Type;

// FSCAL Register
typedef union
{
    struct
    {
        uint8_t FSCAL:8;                //bit: 0...7 Full-Scale Calibration
    } bit;
} ADS126X_FSCAL_Type;

// IMUX Register
typedef union
{
    struct
    {
        uint8_t IMUX1:4;                //bit: 0...3 IDAC1 Output Multiplexer
        uint8_t IMUX2:4;                //bit: 4...7 IDAC2 Output Multiplexer
    } bit;
} ADS126X_IMUX_Type;

// IMAG Register
typedef union
{
    struct
    {
        uint8_t IMAG1:4;                //bit: 0...3 IDAC1 Current Magnitude
        uint8_t IMAG2:4;                //bit: 4...7 IDAC2 Current Magnitude
    } bit;
} ADS126X_IMAG_Type;

// RESERVED Register
typedef union
{
    struct
    {
        uint8_t :8;                     //bit: 0...7 Reserved
    } bit;
} ADS126X_RESERVED_Type;

// PGA Register
typedef union
{
    struct
    {
        uint8_t GAIN:3;                 //bit: 0...2 Gain
        uint8_t :4;                     //bit: 3...6 Reserved
        uint8_t BYPASS:1;               //bit: 7     PGA Bypass Mode
    } bit;
} ADS126X_PGA_Type;

// INPMUX Register
typedef union
{
    struct
    {
        uint8_t MUXN:4;                 //bit: 0...3 Negative Input Multiplexer
        uint8_t MUXP:4;                 //bit: 4...7 Positive Input Multiplexer
    } bit;
} ADS126X_INPMUX_Type;

// INPBIAS Register
typedef union
{
    struct
    {
        uint8_t BOCS:3;                 //bit: 0...2 Burn-Out Current Source Magnitude
        uint8_t BOCSP:1;                //bit: 3     Burn-Out Current Source Polarity
        uint8_t VBIAS:1;                //bit: 4     VBIAS
        uint8_t :3;                     //bit: 5...7 Reserved
    } bit;
} ADS126X_INPBIAS_Type;


/* The entire register map structure */
typedef struct
{
    __IO ADS126X_ID_Type            ID;         // Offset: 0x00 (R/W 8) Device Identification
    __IO ADS126X_STATUS_Type        STATUS;     // Offset: 0x01 (R/W 8) Device Status
    __IO ADS126X_MODE0_Type         MODE0;      // Offset: 0x02 (R/W 8) Mode 0
    __IO ADS126X_MODE1_Type         MODE1;      // Offset: 0x03 (R/W 8) Mode 1
    __IO ADS126X_MODE2_Type         MODE2;      // Offset: 0x04 (R/W 8) Mode 2
    __IO ADS126X_MODE3_Type         MODE3;      // Offset: 0x05 (R/W 8) Mode 3
    __IO ADS126X_REF_Type           REFMUX;     // Offset: 0x06 (R/W 8) Reference Configuration
    __IO ADS126X_OFCAL_Type         OFCAL0;     // Offset: 0x07 (R/W 8) Offset Calibration 0
    __IO ADS126X_OFCAL_Type         OFCAL1;     // Offset: 0x08 (R/W 8) Offset Calibration 1
    __IO ADS126X_OFCAL_Type         OFCAL2;     // Offset: 0x09 (R/W 8) Offset Calibration 2
    __IO ADS126X_FSCAL_Type         FSCAL0;     // Offset: 0x0A (R/W 8) Full-Scale Calibration 0
    __IO ADS126X_FSCAL_Type         FSCAL1;     // Offset: 0x0B (R/W 8) Full-Scale Calibration 1
    __IO ADS126X_FSCAL_Type         FSCAL2;     // Offset: 0x0C (R/W 8) Full-Scale Calibration 2
    __IO ADS126X_IMUX_Type          IMUX;       // Offset: 0x0D (R/W 8) IDAC Multiplexer
    __IO ADS126X_IMAG_Type          IMAG;       // Offset: 0x0E (R/W 8) IDAC Magnitude
    __IO ADS126X_RESERVED_Type      RESERVED;   // Offset: 0x0F (R/W 8) Reserved
    __IO ADS126X_PGA_Type           PGA;        // Offset: 0x10 (R/W 8) PGA Configuration
    __IO ADS126X_INPMUX_Type        INPMUX;     // Offset: 0x11 (R/W 8) Input Multiplexer
    __IO ADS126X_INPBIAS_Type       INPBIAS;    // Offset: 0x12 (R/W 8) Input Bias
} ADS126X_REGISTER_Type;



#ifdef __cplusplus
}
#endif

#endif
