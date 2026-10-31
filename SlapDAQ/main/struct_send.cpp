
#include "struct_send.h"
#include "wifi.h"


// write a 32 bit value to the MSB for network byte configurations
static void put_be32(uint8_t *dst, uint32_t v)
{
    dst[0] = (uint8_t) (v >> 24);
    dst[1] = (uint8_t) (v >> 16);
    dst[2] = (uint8_t) (v >> 8);
    dst[3] = (uint8_t) (v);
}

static uint32_t float_bits(float f)
{
    uint32_t u;
    memcpy(&u, &f, sizeof(u));
    return u;
}

void serializeSensorData(const SensorData &data, uint8_t out[SENSOR_PACKET_LEN])
{
    out[0] = data.digitalIOData;
    put_be32(&out[1], (uint32_t) data.tempData);    // int32 -> uint32 keesp the two's complement bits
    
    for (size_t ch = 0; ch < ADC_NUM_CHANNELS; ch++)
    {
        put_be32(&out[5 + 4 * ch], float_bits(data.adcData[ch]));
    }
}

bool sendStructOverWifi(const SensorData &data)
{
    uint8_t pkt[SENSOR_PACKET_LEN];
    serializeSensorData(data, pkt);

    return wifi_send(reinterpret_cast<const char *>(pkt), sizeof(pkt));
}