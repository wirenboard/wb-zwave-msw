#include "Arduino.h"
#include "TWBMSWSensor.h"

class TFWUpdater
{
public:
    TFWUpdater(TWBMSWSensor* wbMsw);
    void NewFirmwareNotification(uint32_t newFirmwareSize);
    bool GetFirmvareVersion(uint16_t& version);
    bool CheckNewFirmwareAvailable();
    bool UpdateFirmware();

private:
    TWBMSWSensor* WbMsw;
    uint32_t FirmwareSize;
    bool NewFirmware;
};