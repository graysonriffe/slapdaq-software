
#ifndef STRUCT_SEND_H
#define STRUCT_SEND_H

//Necessary Imports
#include <stdint.h>
#include <stddef.h>
#include <string.h>

// 4 single-ended voltage + 1 differential voltage + 2 (4-20 mA) = 7 total channels
constexpr size_t ADC_NUM_CHANNELS = 7;

struct SensorData
{
    uint8_t digitalIOData;
    int32_t tempData;           // temps and ADC codes can be negative so this needs to be int32 intead of uint32
    float adcData[ADC_NUM_CHANNELS];
};

constexpr size_t SENSOR_PACKET_LEN = 1 + 4 + 4 * ADC_NUM_CHANNELS;  // 33

/*Function Declaration*/
void serializeSensorData(const SensorData &data, uint8_t out[SENSOR_PACKET_LEN]);
bool sendStructOverWifi(const SensorData &data);


#endif
