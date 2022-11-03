#include "TFastModbus.h"
#include "Status.h"
#include "CrcClass.h"
#include "Debug.h"
#include "Arduino.h"

TFastModbus::TFastModbus(HardwareSerial *hardwareSerial, uint16_t timeoutMs) : Serial(hardwareSerial), TimeoutMs(timeoutMs)
{
}

bool TFastModbus::OpenPort(size_t Speed, uint32_t Config, uint8_t Rx, uint8_t Tx)
{
    return (Serial->begin(Speed, Config, Rx, Tx) == ZunoErrorOk);
}

bool TFastModbus::StartScan()
{
    uint8_t startData[] = {0xFD, 0x60, 0x01, 0x09, 0xF0};
    bool result = Serial->write(startData, sizeof(startData)) == sizeof(startData);

    if (result)
        delay(Serial->countWaitingMs(4));
    else
    {
#ifdef LOGGING_DBG
        LOGGING_UART.print("*** ERROR sending fast modbus scan start!\n");
#endif
    }

    return result;
}

bool TFastModbus::ContinueScan(uint8_t *serialNumber, uint8_t *modbusAddress)
{
    uint8_t continueData[] = {0xFD, 0x60, 0x02, 0x49, 0xF1};
    if (Serial->write(continueData, sizeof(continueData)) != sizeof(continueData))
    {
#ifdef LOGGING_DBG
        LOGGING_UART.print("*** ERROR sending fast modbus scan continue!\n");
#endif
        return false;
    }

    uint16_t timeout = TimeoutMs;
    uint8_t scanData[32];
    size_t scanDataLength = 0;
    while (scanDataLength < sizeof(scanData) && timeout > 0x0)
    {
        int availableDataLength = Serial->available();
        if (!availableDataLength)
        {
            timeout = timeout - 0xA;
            delay(0xA);
        }
        else
        {
            for (int i = 0; i < availableDataLength; i++)
            {
                scanData[scanDataLength] = (uint8_t)Serial->read();
                scanDataLength++;
            }
        }
    }

    if (!scanDataLength)
    {
#ifdef LOGGING_DBG
        LOGGING_UART.print("*** ERROR reading fast modbus scan response!\n");
#endif
        return false;
    }

    uint8_t *fastModbusPacket = scanData;
    uint8_t fastModbusPacketLength = scanDataLength;
    while ((fastModbusPacketLength > 0) && (*fastModbusPacket == 0xFF))
    {
        fastModbusPacket += 1;
        fastModbusPacketLength -= 1;
    }

#ifdef LOGGING_DBG
    LOGGING_UART.print("Fast modbus packet length: ");
    LOGGING_UART.println(fastModbusPacketLength);
    for (int i = 0; i < fastModbusPacketLength; i++)
    {
        LOGGING_UART.print(fastModbusPacket[i], 0x10);
    }
    LOGGING_UART.println();
#endif

    uint16_t checkCrc = CrcClass::crc16_modbus(fastModbusPacket, fastModbusPacketLength - 2);
    uint16_t crc = (fastModbusPacket[fastModbusPacketLength - 1] << 8) + fastModbusPacket[fastModbusPacketLength - 2];
    if (checkCrc != crc)
    {
#ifdef LOGGING_DBG
        LOGGING_UART.print("*** ERROR crc mismatch! ");
        LOGGING_UART.print(checkCrc, 0x10);
        LOGGING_UART.print(" expected, but ");
        LOGGING_UART.print(crc, 0x10);
        LOGGING_UART.print(" actually received!\n");
#endif
        return false;
    }

    if (fastModbusPacket[0] != 0xFD || fastModbusPacket[1] != 0x60)
    {
#ifdef LOGGING_DBG
        LOGGING_UART.print("*** ERROR wrong modbus address or fast modbus command: 0xFD address and 0x60 command expected, but ");
        LOGGING_UART.print(fastModbusPacket[0], 0x10);
        LOGGING_UART.print(" and ");
        LOGGING_UART.print(fastModbusPacket[1], 0x10);
        LOGGING_UART.print(" actually received!\n");
#endif
        return false;
    }

    switch (fastModbusPacket[2])
    {
    case 0x03:
        memcpy(serialNumber, &fastModbusPacket[3], WB_MSW_SERIAL_NUMBER_SIZE);
        memcpy(modbusAddress, &fastModbusPacket[7], 1);
        return true;
    case 0x04:
        return false;
    default:
#ifdef LOGGING_DBG
        LOGGING_UART.print("*** ERROR unknown fast modbus subcommand !\n");
        LOGGING_UART.println(fastModbusPacket[2], 0x10);
#endif
        break;
    }

    return false;
}

void TFastModbus::ClosePort(void)
{
    Serial->end();
}

bool TFastModbus::ScanBus(uint8_t *serialNumber, uint8_t *modbusAddress)
{
    if (!this->StartScan())
        return false;

    uint8_t count = 0;
    uint8_t newSerialNumber[WB_MSW_SERIAL_NUMBER_SIZE];
    uint8_t newModbusAddress;
    while (this->ContinueScan(newSerialNumber, &newModbusAddress))
        count++;

    if (count == 1)
    {
        memcpy(serialNumber, newSerialNumber, WB_MSW_SERIAL_NUMBER_SIZE);
        memcpy(modbusAddress, &newModbusAddress, 1);
        return true;
    }

#ifdef LOGGING_DBG
    LOGGING_UART.print("*** ERROR fast modbus meets more than one device!\n");
#endif

    return false;
}