#ifndef WB_MSW_SENSOR_H
#define WB_MSW_SENSOR_H

#include "ModBusRtu.h"

class TWBMSWSensor: private ModBusRtuClass
{
public:
    TWBMSWSensor(HardwareSerial* hardwareSerial, uint16_t timeoutMs, uint8_t address);
    bool OpenPort(size_t speed, uint32_t config, uint8_t rx, uint8_t tx);
    bool GetFwVersion(uint32_t* version);
    bool GetTemperature(int16_t& temperature);
    bool GetHumidity(uint16_t& humidity);
    bool GetLuminance(uint32_t& luminance);
    bool GetCO2(uint16_t& co2);
    bool GetCO2Status(bool& status);
    bool SetCO2Status(bool status);
    bool SetCO2Autocalibration(bool status);
    bool GetVoc(uint16_t& voc);
    bool GetNoiseLevel(uint16_t& noiseLevel);
    bool GetMotion(uint16_t& motion);
    bool FwMode(void);
    bool FwWriteInfo(uint8_t* info);
    bool FwWriteData(uint8_t* info);
    bool FwUpdate(const void* buffer, size_t len, uint16_t timeoutMs = 2000);

private:
    uint8_t Address;
};
#endif // WB_MSW_SENSOR_H