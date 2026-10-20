
#include "struct_send.h"


bool sendStructOverWifi(const SensorData &data)
{
    SensorData netData{};

    // Convert multi-byte fields to network byte order (big-endian)
    netData.digitalIOData = data.digitalIOData;
    netData.tempData = htonl(static_cast<uint32_t>(data.tempData));
    netData.adcData = htonl(static_cast<uint32_t>(data.adcData));

    return wifi_send(reinterpret_cast<const char *>(&netData), sizeof(netData));
}