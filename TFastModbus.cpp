#include "TFastModbus.h"
#include "Arduino.h"
#include "CrcClass.h"
#include "DebugOutput.h"
#include "Status.h"

#define TIMEOUT_COUNTING_DELTA 1

TFastModbus::TFastModbus(HardwareSerial* hardwareSerial): Serial(hardwareSerial)
{}

bool TFastModbus::OpenPort(size_t speed, uint32_t config, uint8_t rx, uint8_t tx)
{
    return (Serial->begin(speed, config, rx, tx) == ZunoErrorOk);
}

// Starts scan cycle
bool TFastModbus::StartScan()
{
    // Fast modbus packet with start scan command
    // 0xFD - address for scan, 0x60 command, 0x01 - start subcommand
    uint8_t startData[] = {0xFD, 0x60, 0x01, 0x09, 0xF0};
    bool result = Serial->write(startData, sizeof(startData)) == sizeof(startData);

    if (result) {
        delay(Serial->countWaitingMs(4));
    } else {
        DEBUG("*** ERROR Sending fast modbus scan start!\n");
    }

    return result;
}

uint8_t TFastModbus::ReadBytes(uint8_t* buffer, uint8_t bufferSize, uint16_t timeoutMs)
{
    uint16_t timeLeft = timeoutMs;
    bool timeoutFlag = false;
    uint8_t dataLength = 0;

    while (dataLength < bufferSize && !timeoutFlag) {
        uint8_t availableDataLength = Serial->available();
        if (!availableDataLength) {
            if (timeLeft >= TIMEOUT_COUNTING_DELTA) {
                timeLeft = timeLeft - TIMEOUT_COUNTING_DELTA;
                delay(TIMEOUT_COUNTING_DELTA);
            } else {
                timeoutFlag = true;
            }
        } else {
            if ((dataLength + availableDataLength) > bufferSize) {
                DEBUG("ERROR Fast modbus scan buffer too short\n");
                return 0;
            }
            for (int i = 0; i < availableDataLength; i++) {
                buffer[dataLength] = (uint8_t)Serial->read();
                dataLength++;
            }
        }
    }
    return dataLength;
}

// Continues scan cycle until 0x04 end scan subcommand would reach
bool TFastModbus::ContinueScan(uint8_t* serialNumber, uint8_t* modbusAddress, uint16_t timeoutMs)
{
    // Fast modbus packet with continue scan command
    // 0xFD - address for scan, 0x60 command, 0x02 - continue subcommand
    uint8_t continueData[] = {0xFD, 0x60, 0x02, 0x49, 0xF1};
    if (Serial->write(continueData, sizeof(continueData)) != sizeof(continueData)) {
        DEBUG("*** ERROR Sending fast modbus scan continue!\n");
        return false;
    }

    uint8_t scanData[32];
    uint8_t scanDataLength = ReadBytes(scanData, sizeof(scanData), timeoutMs);
    if (!scanDataLength) {
        DEBUG("*** ERROR Reading fast modbus scan response!\n");
        return false;
    }

    uint8_t* fastModbusPacket = scanData;
    uint8_t fastModbusPacketLength = scanDataLength;
    while ((fastModbusPacketLength > 0) && (*fastModbusPacket == 0xFF)) {
        fastModbusPacket += 1;
        fastModbusPacketLength -= 1;
    }

    DEBUG("Fast modbus packet length: ");
    DEBUG(fastModbusPacketLength);
    DEBUG("\n");
    for (int i = 0; i < fastModbusPacketLength; i++) {
        DEBUG(fastModbusPacket[i], 16);
    }
    DEBUG("\n");

    uint16_t checkCrc = CrcClass::crc16_modbus(fastModbusPacket, fastModbusPacketLength - 2);
    uint16_t crc = (fastModbusPacket[fastModbusPacketLength - 1] << 8) + fastModbusPacket[fastModbusPacketLength - 2];
    if (checkCrc != crc) {
        DEBUG("*** ERROR CRC mismatch! ");
        DEBUG(checkCrc, 16);
        DEBUG(" expected, but ");
        DEBUG(crc, 16);
        DEBUG(" actually received!\n");
        return false;
    }

    // Check if packet has wrong address and fast modbus command
    if (fastModbusPacket[0] != 0xFD || fastModbusPacket[1] != 0x60) {
        DEBUG("*** ERROR Wrong modbus address or fast modbus command: 0xFD address and 0x60 command expected, but ");
        DEBUG(fastModbusPacket[0], 16);
        DEBUG(" and ");
        DEBUG(fastModbusPacket[1], 16);
        DEBUG(" actually received!\n");
        return false;
    }

    switch (fastModbusPacket[2]) {
        // If fast modbus subcommand==0x03 the response contain device data
        case 0x03:
            memcpy(serialNumber, &fastModbusPacket[3], WB_MSW_SERIAL_NUMBER_SIZE);
            memcpy(modbusAddress, &fastModbusPacket[7], 1);
            return true;
        case 0x04:
            // If fast modbus subcommand==0x04, there are no unquestioned devices left on fast modbus bus
            return false;
        default:
            DEBUG("*** ERROR Unknown fast modbus subcommand !\n");
            DEBUG(fastModbusPacket[2], 16);
            DEBUG("\n");
            break;
    }

    return false;
}

void TFastModbus::ClosePort(void)
{
    Serial->end();
}

bool TFastModbus::ScanBus(uint8_t* serialNumber,
                          uint8_t serialNumberSize,
                          uint8_t* modbusAddress,
                          uint8_t modbusAddressSize,
                          uint16_t timeoutMs)
{
    uint8_t newSerialNumber[WB_MSW_SERIAL_NUMBER_SIZE];
    uint8_t newModbusAddress;

    if ((serialNumberSize != sizeof(newSerialNumber)) || (modbusAddressSize != sizeof(newModbusAddress))) {
        DEBUG("ERROR wrong buffer size for serial number or modbus address\n");
        return false;
    }

    if (!this->StartScan()) {
        return false;
    }

    uint8_t count = 0;
    while (this->ContinueScan(newSerialNumber, &newModbusAddress, timeoutMs)) {
        count++;
    }

    if (count == 1) {
        memcpy(serialNumber, newSerialNumber, WB_MSW_SERIAL_NUMBER_SIZE);
        memcpy(modbusAddress, &newModbusAddress, 1);
        return true;
    }

    DEBUG("*** ERROR Fast modbus meets zero or more than one device!\n");

    return false;
}