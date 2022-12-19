#ifndef WB_MSW_SENSOR_H
#define WB_MSW_SENSOR_H

#include "ModBusRtu.h"

enum TWBMSWSensorAvailability
{
    WB_MSW_SENSOR_AVAILABLE = 1,
    WB_MSW_SENSOR_UNAVAILABLE = 0,
    WB_MSW_SENSOR_UNKNOWN = 6
};

class TWBMSWSensor: private ModBusRtuClass
{
public:
    TWBMSWSensor(HardwareSerial* hardwareSerial, uint16_t timeoutMs);
    bool OpenPort(size_t speed, uint32_t config, uint8_t rx, uint8_t tx);
    void ClosePort(void);
    void SetModbusAddress(uint8_t address);
    bool GetFwVersion(uint16_t& version);
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
    bool FwUpdate(const void* buffer, size_t len, uint16_t timeoutMs = 2000);

    bool GetTemperatureAvailability(enum TWBMSWSensorAvailability& availability);
    bool GetHumidityAvailability(enum TWBMSWSensorAvailability& availability);
    bool GetLuminanceAvailability(enum TWBMSWSensorAvailability& availability);
    bool GetCO2Availability(enum TWBMSWSensorAvailability& availability);
    bool GetVocAvailability(enum TWBMSWSensorAvailability& availability);
    bool GetNoiseLevelAvailability(enum TWBMSWSensorAvailability& availability);
    bool GetMotionAvailability(enum TWBMSWSensorAvailability& availability);

private:
    bool SetFwMode(void);
    bool FwWriteInfo(uint8_t* info);
    bool FwWriteData(uint8_t* info);
    enum TWBMSWSensorAvailability ConvertAvailability(uint16_t availability);
    bool ReadAvailabilityRegister(enum TWBMSWSensorAvailability& availability, uint16_t registerAddress);
    uint8_t Address;
};
#endif // WB_MSW_SENSOR_H