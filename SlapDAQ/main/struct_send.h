
#ifndef STRUCT_SEND_H
#define STRUCT_SEND_H

//Necessary Imports
#include <stdint.h>
#include "wifi.h"



#ifdef __cplusplus
extern "C" {
#endif

/*Global Necessary Variables*/
static const char* STRUCT_TAG = "struct-test";

/*Struct to Send Data*/
#pragma pack(push, 1)
struct SensorData
{
    uint8_t digitalIOData;
    uint32_t tempData;
    uint32_t adcData;
};
#pragma pack(pop)

static_assert(sizeof(SensorData) == 9, "SensorData must be 9 bytes");

/*Function Declaration*/
bool sendStructOverWifi(const SensorData &data);

#ifdef __cplusplus
}
#endif

#endif
