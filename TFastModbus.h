#include "Arduino.h"

#define WB_MSW_SERIAL_NUMBER_SIZE 4

class TFastModbus
{
public:
    TFastModbus(HardwareSerial *hardwareSerial, uint16_t timeoutMs);
    bool OpenPort(size_t speed, uint32_t config, uint8_t rx, uint8_t tx);
    bool ScanBus(uint8_t *serialNumber, uint8_t *modbusAddress);
    void ClosePort(void);

private:
    bool StartScan(void);
    bool ContinueScan(uint8_t *serialNumber, uint8_t *modbusAddress);
    HardwareSerial *Serial;
    uint16_t TimeoutMs;
};