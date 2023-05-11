#ifndef WB_MSW_SENSOR_H
#define WB_MSW_SENSOR_H

#include "ModBusRtu.h"

class TWBMSWSensor: private ModBusRtuClass
{
public:
    enum class Availability
    {
        AVAILABLE,
        UNAVAILABLE,
        UNKNOWN
    };

    enum class LedStatus
    {
        LED_STATUS_ON,
        LED_STATUS_OFF,
        LED_STATUS_UNKNOWN
    };

    typedef bool (TWBMSWSensor::*GetAvailabilityCallback)(TWBMSWSensor::Availability& availability);
    typedef bool (TWBMSWSensor::*GetValueCallback)(int64_t& value);

    TWBMSWSensor(HardwareSerial* hardwareSerial, uint16_t timeoutMs);
    bool OpenPort(size_t speed, uint32_t config, uint8_t rx, uint8_t tx);
    void ClosePort(void);
    void SetModbusAddress(uint8_t address);
    bool GetFwVersion(uint16_t& version);
    bool GetTemperature(int64_t& temperature);
    bool GetHumidity(int64_t& humidity);
    bool GetLuminance(int64_t& luminance);
    bool GetCO2(int64_t& co2);
    bool GetCO2Status(bool& status);
    bool SetCO2Status(bool status);
    bool SetCO2Autocalibration(bool status);
    bool GetVoc(int64_t& voc);
    bool GetNoiseLevel(int64_t& noiseLevel);
    bool GetMotion(int64_t& motion);
    bool FwUpdate(uint16_t* buffer, size_t length, uint16_t timeoutMs = 2000);

    bool GetTemperatureAvailability(TWBMSWSensor::Availability& availability);
    bool GetHumidityAvailability(TWBMSWSensor::Availability& availability);
    bool GetLuminanceAvailability(TWBMSWSensor::Availability& availability);
    bool GetCO2Availability(TWBMSWSensor::Availability& availability);
    bool GetVocAvailability(TWBMSWSensor::Availability& availability);
    bool GetNoiseLevelAvailability(TWBMSWSensor::Availability& availability);
    bool GetMotionAvailability(TWBMSWSensor::Availability& availability);

    bool BuzzerAvailable(TWBMSWSensor::Availability& availability);
    bool BuzzerStart(void);
    bool BuzzerStop(void);

    bool SetLedFlashDuration(uint8_t ms);
    bool SetLedFlashTimout(uint8_t sec);
    bool SetLedRedOn(void);
    bool SetLedRedOff(void);
    bool SetLedGreenOn(void);
    bool SetLedGreenOff(void);
    LedStatus GetLedRedStatus(void);
    LedStatus GetLedGreenStatus(void);

private:
    bool SetFwMode(void);
    bool FwWriteInfo(uint16_t* info);
    bool FwWriteData(uint16_t* info);
    TWBMSWSensor::Availability ConvertAvailability(uint16_t availability) const;
    bool ReadAvailabilityRegister(TWBMSWSensor::Availability& availability, uint16_t registerAddress);
    uint8_t Address;
    LedStatus LedStatusRed;
    LedStatus LedStatusGreen;
};
#endif // WB_MSW_SENSOR_H