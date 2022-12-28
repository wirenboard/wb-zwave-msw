#include "TFastModbus.h"
#include "Arduino.h"
#include "CrcClass.h"
#include "DebugOutput.h"
#include "Status.h"

#define TIMEOUT_COUNTING_DELTA 1
#define FAST_MODBUS_DATA_BUFFER_SIZE 32

// Fast modbus packet structure
#define FAST_MODBUS_SCAN_ADDRESS 0xFD
#define FAST_MODBUS_SCAN_ADDRESS_POS 0
#define FAST_MODBUS_SCAN_COMMAND 0x60
#define FAST_MODBUS_SCAN_COMMAND_POS FAST_MODBUS_SCAN_ADDRESS_POS + 1
#define FAST_MODBUS_SCAN_SUBCOMMAND_START 0x01
#define FAST_MODBUS_SCAN_SUBCOMMAND_CONTINUE 0x02
#define FAST_MODBUS_SCAN_SUBCOMMAND_DATA 0x03
#define FAST_MODBUS_SCAN_SUBCOMMAND_END 0x04
#define FAST_MODBUS_SCAN_SUBCOMMAND_POS FAST_MODBUS_SCAN_COMMAND_POS + 1
#define FAST_MODBUS_SCAN_SERIAL_NUMBER_POS FAST_MODBUS_SCAN_SUBCOMMAND_POS + 1
#define FAST_MODBUS_SCAN_MODBUS_ADDRESS_POS FAST_MODBUS_SCAN_SERIAL_NUMBER_POS + WB_MSW_SERIAL_NUMBER_SIZE

#define FAST_MODBUS_SCAN_CRC_SIZE 2
#define FAST_MODBUS_SCAN_CRC_POS_0 FAST_MODBUS_SCAN_SUBCOMMAND_POS + 1
#define FAST_MODBUS_SCAN_CRC_POS_1 FAST_MODBUS_SCAN_CRC_POS_0 + 1
#define FAST_MODBUS_SCAN_START_PACKET_SIZE FAST_MODBUS_SCAN_CRC_POS_1 + 1
#define FAST_MODBUS_SCAN_CONTINUE_PACKET_SIZE FAST_MODBUS_SCAN_START_PACKET_SIZE

TFastModbus::TFastModbus(HardwareSerial* hardwareSerial): Serial(hardwareSerial)
{}

bool TFastModbus::OpenPort(size_t speed, uint32_t config, uint8_t rx, uint8_t tx)
{
    return (Serial->begin(speed, config, rx, tx) == ZunoErrorOk);
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

uint8_t TFastModbus::ReadFastModbusPacket(uint8_t* buffer, uint8_t bufferLength, uint16_t timeoutMs)
{
    uint8_t scanData[FAST_MODBUS_DATA_BUFFER_SIZE];
    uint8_t scanDataLength = ReadBytes(scanData, sizeof(scanData), timeoutMs);
    if (!scanDataLength) {
        DEBUG("*** ERROR Reading fast modbus scan response!\n");
        return 0;
    }

    uint8_t* fastModbusPacket = scanData;
    uint8_t fastModbusPacketLength = scanDataLength;
    while ((fastModbusPacketLength > 0) && (*fastModbusPacket == 0xFF)) {
        fastModbusPacket += 1;
        fastModbusPacketLength -= 1;
    }

    if (fastModbusPacketLength > bufferLength) {
        DEBUG("*** ERROR Not enough buffer length for fast modbus packet length");
        return 0;
    }

    memcpy(buffer, fastModbusPacket, fastModbusPacketLength);

    DEBUG("Fast modbus packet length: ");
    DEBUG(fastModbusPacketLength);
    DEBUG("\n");
    for (int i = 0; i < fastModbusPacketLength; i++) {
        DEBUG(fastModbusPacket[i], 16);
    }
    DEBUG("\n");

    return fastModbusPacketLength;
}

bool TFastModbus::CheckFastModbusPacket(const uint8_t* packet, uint8_t packetlength) const
{
    uint16_t checkCrc = CrcClass::crc16_modbus(packet, packetlength - 2);
    uint16_t crc = (packet[packetlength - 1] << 8) + packet[packetlength - 2];
    if (checkCrc != crc) {
        DEBUG("*** ERROR CRC mismatch! ");
        DEBUG(checkCrc, 16);
        DEBUG(" expected, but ");
        DEBUG(crc, 16);
        DEBUG(" actually received!\n");
        return false;
    }

    // Check if packet has wrong address and fast modbus command
    if (packet[FAST_MODBUS_SCAN_ADDRESS_POS] != FAST_MODBUS_SCAN_ADDRESS ||
        packet[FAST_MODBUS_SCAN_COMMAND_POS] != FAST_MODBUS_SCAN_COMMAND)
    {
        DEBUG("*** ERROR Wrong modbus address or fast modbus command: ");
        DEBUG(FAST_MODBUS_SCAN_ADDRESS);
        DEBUG(" address and ");
        DEBUG(FAST_MODBUS_SCAN_COMMAND);
        DEBUG(" command expected, but ");
        DEBUG(packet[FAST_MODBUS_SCAN_ADDRESS_POS], 16);
        DEBUG(" and ");
        DEBUG(packet[FAST_MODBUS_SCAN_COMMAND_POS], 16);
        DEBUG(" actually received!\n");
        return false;
    }

    return true;
}

bool TFastModbus::ParseFastModbusPacket(const uint8_t* packet,
                                        uint8_t packetlength,
                                        uint8_t* serialNumber,
                                        uint8_t* modbusAddress) const
{
    // Check packet subcommand
    switch (packet[FAST_MODBUS_SCAN_SUBCOMMAND_POS]) {
        case FAST_MODBUS_SCAN_SUBCOMMAND_DATA:
            // Fast modbus response contain device data
            memcpy(serialNumber, &packet[FAST_MODBUS_SCAN_SERIAL_NUMBER_POS], WB_MSW_SERIAL_NUMBER_SIZE);
            memcpy(modbusAddress, &packet[FAST_MODBUS_SCAN_MODBUS_ADDRESS_POS], 1);
            return true;
        case FAST_MODBUS_SCAN_SUBCOMMAND_END:
            // There are no unquestioned devices left on fast modbus bus
            return false;
        default:
            DEBUG("*** ERROR Unknown fast modbus subcommand !\n");
            DEBUG(packet[FAST_MODBUS_SCAN_SUBCOMMAND_POS], 16);
            DEBUG("\n");
            return false;
    }
}

bool TFastModbus::ReadNewDeviceData(uint8_t* serialNumber, uint8_t* modbusAddress, uint16_t timeoutMs)
{
    uint8_t fastModbusPacket[FAST_MODBUS_DATA_BUFFER_SIZE];
    uint8_t fastModbusPacketLength = ReadFastModbusPacket(fastModbusPacket, sizeof(fastModbusPacket), timeoutMs);
    if (!fastModbusPacketLength) {
        return false;
    }

    if (!CheckFastModbusPacket(fastModbusPacket, fastModbusPacketLength)) {
        return false;
    }

    if (!ParseFastModbusPacket(fastModbusPacket, fastModbusPacketLength, serialNumber, modbusAddress)) {
        return false;
    }

    return true;
}

// Starts scan cycle
bool TFastModbus::StartScan(uint8_t* serialNumber, uint8_t* modbusAddress, uint16_t timeoutMs)
{
    // Fast modbus packet with start scan command
    uint8_t startData[FAST_MODBUS_DATA_BUFFER_SIZE];
    startData[FAST_MODBUS_SCAN_ADDRESS_POS] = FAST_MODBUS_SCAN_ADDRESS;
    startData[FAST_MODBUS_SCAN_COMMAND_POS] = FAST_MODBUS_SCAN_COMMAND;
    startData[FAST_MODBUS_SCAN_SUBCOMMAND_POS] = FAST_MODBUS_SCAN_SUBCOMMAND_START;
    uint16_t checkCrc = CrcClass::crc16_modbus(startData, FAST_MODBUS_SCAN_SUBCOMMAND_POS + 1);
    startData[FAST_MODBUS_SCAN_CRC_POS_0] = checkCrc & 0x00FF;
    startData[FAST_MODBUS_SCAN_CRC_POS_1] = checkCrc >> 8;

    if (Serial->write(startData, FAST_MODBUS_SCAN_START_PACKET_SIZE) != FAST_MODBUS_SCAN_START_PACKET_SIZE) {
        DEBUG("*** ERROR Sending fast modbus scan start!\n");
        return false;
    }

    return ReadNewDeviceData(serialNumber, modbusAddress, timeoutMs);
}

// Continues scan cycle until 0x04 end scan subcommand would reach
bool TFastModbus::ContinueScan(uint8_t* serialNumber, uint8_t* modbusAddress, uint16_t timeoutMs)
{
    // Fast modbus packet with continue scan command
    uint8_t continueData[FAST_MODBUS_DATA_BUFFER_SIZE];
    continueData[FAST_MODBUS_SCAN_ADDRESS_POS] = FAST_MODBUS_SCAN_ADDRESS;
    continueData[FAST_MODBUS_SCAN_COMMAND_POS] = FAST_MODBUS_SCAN_COMMAND;
    continueData[FAST_MODBUS_SCAN_SUBCOMMAND_POS] = FAST_MODBUS_SCAN_SUBCOMMAND_CONTINUE;
    uint16_t checkCrc = CrcClass::crc16_modbus(continueData, FAST_MODBUS_SCAN_SUBCOMMAND_POS + 1);
    continueData[FAST_MODBUS_SCAN_CRC_POS_0] = checkCrc & 0x00FF;
    continueData[FAST_MODBUS_SCAN_CRC_POS_1] = checkCrc >> 8;

    if (Serial->write(continueData, FAST_MODBUS_SCAN_CONTINUE_PACKET_SIZE) != FAST_MODBUS_SCAN_CONTINUE_PACKET_SIZE) {
        DEBUG("*** ERROR Sending fast modbus scan continue!\n");
        return false;
    }

    return ReadNewDeviceData(serialNumber, modbusAddress, timeoutMs);
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

    uint8_t count = 0;
    if (!this->StartScan(newSerialNumber, &newModbusAddress, timeoutMs)) {
        return false;
    }
    count++;

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