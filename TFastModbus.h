#include "Arduino.h"

#define WB_MSW_SERIAL_NUMBER_SIZE 4

class TFastModbus
{
public:
    TFastModbus(HardwareSerial* hardwareSerial);
    bool OpenPort(size_t speed, uint32_t config, uint8_t rx, uint8_t tx);
    bool ScanBus(uint8_t* serialNumber,
                 uint8_t serialNumberSize,
                 uint8_t* modbusAddress,
                 uint8_t modbusAddressSize,
                 uint16_t timeoutMs);
    void ClosePort(void);

private:
    bool StartScan(void);
    uint8_t ReadBytes(uint8_t* buffer, uint8_t bufferSize, uint16_t timeoutMs);
    bool ContinueScan(uint8_t* serialNumber, uint8_t* modbusAddress, uint16_t timeoutMs);
    HardwareSerial* Serial;
};