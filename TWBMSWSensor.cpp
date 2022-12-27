
#include "TWBMSWSensor.h"
#include "DebugOutput.h"

#define WBMSW_REG_TEMPERATURE 0x0004
#define WBMSW_REG_HUMIDITY 0x0005
#define WBMSW_REG_LUMINANCE 0x0009
#define WBMSW_REG_CO2 0x0008
#define WBMSW_COIL_CO2_STAUS 0x0003
#define WBMSW_REG_CO2_AUTO_CALIB 0x005F
#define WBMSW_REG_VOC 0x000B
#define WBMSW_REG_NOIZE 0x0003
#define WBMSW_REG_MOTION 0x011B
#define WBMSW_REG_FW_MODE 0x0081
#define WBMSW_REG_FW_INFO 0x1000
#define WBMSW_REG_FW_DATA 0x2000
#define WBMSW_REG_FW_VERSION 0x00FA

#define WBMSW_FIRMWARE_INFO_SIZE 32
#define WBMSW_FIRMWARE_DATA_SIZE 136

#define WBMSW_REG_TEMPERATURE_AVAIL 0x0170
#define WBMSW_REG_HUMIDITY_AVAIL 0x0171
#define WBMSW_REG_LUMINANCE_AVAIL 0x0172
#define WBMSW_REG_CO2_AVAIL 0x0174
#define WBMSW_REG_VOC_AVAIL 0x0173
#define WBMSW_REG_NOIZE_AVAIL 0x0176
#define WBMSW_REG_MOTION_AVAIL 0x0175

#define WBMSW_VERSION_NUMBER_LENGTH 16

/* Public Constructors */
TWBMSWSensor::TWBMSWSensor(HardwareSerial* hardwareSerial, uint16_t timeoutMs)
    : ModBusRtuClass(hardwareSerial, timeoutMs)
{}

/* Public Methods */
bool TWBMSWSensor::OpenPort(size_t speed, uint32_t config, uint8_t rx, uint8_t tx)
{
    return ModBusRtuClass::begin(speed, config, rx, tx);
}

void TWBMSWSensor::ClosePort(void)
{
    // There is no ability to end session with ModBus Rtu, but Hardware Serial and ModBus Rtu Class have no mutex for
    // port access. Hence there no error will happen while using  non-simultaneously one physical serial port
    return;
}

void TWBMSWSensor::SetModbusAddress(uint8_t address)
{
    this->Address = address;
}

bool TWBMSWSensor::GetFwVersion(uint16_t& version)
{
    uint16_t versionStr[WBMSW_VERSION_NUMBER_LENGTH];

    if (!ModBusRtuClass::readInputRegisters(this->Address,
                                            WBMSW_REG_FW_VERSION,
                                            WBMSW_VERSION_NUMBER_LENGTH,
                                            versionStr))
    {
        return (false);
    }

    DEBUG("FW:                 ");
    for (size_t i = 0; i < WBMSW_VERSION_NUMBER_LENGTH; i++) {
        if (versionStr[i] != 0) {
            DEBUG((char)versionStr[i]);
        }
    }
    DEBUG("\n");

    size_t i = 0;
    size_t j = 0;
    uint8_t semanticVersion[2]={0};
    while ((i < WBMSW_VERSION_NUMBER_LENGTH) && (j < sizeof(semanticVersion))) {
        if (versionStr[i] == '.') {
            j++;
        } else {
            semanticVersion[j] = semanticVersion[j] * 10 + (versionStr[i] - '0');
        }
        i++;
    }
    version = (semanticVersion[0] << 8) | semanticVersion[1];
    return true;
}

bool TWBMSWSensor::GetTemperature(int64_t& temperature)
{
    int16_t temperatureTmp;
    if (readInputRegisters(Address, WBMSW_REG_TEMPERATURE, 1, &temperatureTmp)) {
        temperature = temperatureTmp;
        return true;
    }
    return false;
}

bool TWBMSWSensor::GetHumidity(int64_t& humidity)
{
    int16_t humidityTmp;
    if (readInputRegisters(Address, WBMSW_REG_HUMIDITY, 1, &humidityTmp)) {
        humidity = humidityTmp;
        return true;
    }
    return false;
}

bool TWBMSWSensor::GetLuminance(int64_t& luminance)
{
    uint16_t lumen[2];
    if (!readInputRegisters(Address, WBMSW_REG_LUMINANCE, (sizeof(lumen) / sizeof(lumen[0])), lumen)) {
        return false;
    }
    luminance = (lumen[0] << 16) | lumen[1];
    return true;
}

bool TWBMSWSensor::GetCO2(int64_t& co2)
{
    uint16_t co2Tmp;
    if (readInputRegisters(Address, WBMSW_REG_CO2, 1, &co2Tmp)) {
        co2 = co2Tmp;
        return true;
    }
    return false;
}

bool TWBMSWSensor::GetCO2Status(bool& status)
{
    uint8_t out;
    if (!readCoils(Address, WBMSW_COIL_CO2_STAUS, 1, &out)) {
        return false;
    }
    status = (out != 0);
    return true;
}

bool TWBMSWSensor::SetCO2Status(bool status)
{
    return writeSingleCoils(Address, WBMSW_COIL_CO2_STAUS, status);
}

bool TWBMSWSensor::SetCO2Autocalibration(bool status)
{
    uint16_t value = status ? 1 : 0;
    return writeSingleRegisters(Address, WBMSW_REG_CO2_AUTO_CALIB, value);
}

bool TWBMSWSensor::GetVoc(int64_t& voc)
{
    uint16_t vocTmp;
    if (readInputRegisters(Address, WBMSW_REG_VOC, 1, &vocTmp)) {
        voc = vocTmp;
        return true;
    }
    return false;
}

bool TWBMSWSensor::GetNoiseLevel(int64_t& noiseLevel)
{
    int16_t noiseLevelTmp;
    if (readInputRegisters(Address, WBMSW_REG_NOIZE, 1, &noiseLevelTmp)) {
        noiseLevel = noiseLevelTmp;
        return true;
    }
    return false;
}

bool TWBMSWSensor::GetMotion(int64_t& motion)
{
    uint16_t motionTmp;
    if (readInputRegisters(Address, WBMSW_REG_MOTION, 1, &motionTmp)) { // 0x0118 - max
        motion = motionTmp;
        return true;
    }
    return false;
}

bool TWBMSWSensor::SetFwMode(void)
{
    return writeSingleRegisters(Address, WBMSW_REG_FW_MODE, 1);
}

bool TWBMSWSensor::FwWriteInfo(uint8_t* info)
{
    return writeMultipleRegisters(Address, WBMSW_REG_FW_INFO, WBMSW_FIRMWARE_INFO_SIZE / 2, info);
}

bool TWBMSWSensor::FwWriteData(uint8_t* data)
{
    return writeMultipleRegisters(Address, WBMSW_REG_FW_DATA, WBMSW_FIRMWARE_DATA_SIZE / 2, data);
}

bool TWBMSWSensor::FwUpdate(const void* buffer, size_t len, uint16_t timeoutMs)
{
    uint8_t* data;
    if (len < WBMSW_FIRMWARE_INFO_SIZE) {
        return false;
    }
    DEBUG("FW size: ");
    DEBUG(len);
    DEBUG("\n");

    if (!this->SetFwMode()) {
        return false;
    }
    DEBUG("Wait ");
    DEBUG(timeoutMs);
    DEBUG(" ms\n");

    delay(timeoutMs);
    data = (uint8_t*)buffer;
    if (!this->FwWriteInfo(data)) {
        return false;
    }
    DEBUG("Write info\n");
    data += WBMSW_FIRMWARE_INFO_SIZE;
    len -= WBMSW_FIRMWARE_INFO_SIZE;

    DEBUG("Write data\n");
    while (len) {
        if (!FwWriteData(data)) {
            return false;
        }
        data += WBMSW_FIRMWARE_DATA_SIZE;
        len -= WBMSW_FIRMWARE_DATA_SIZE;
    }

    DEBUG("Write finish\n");
    return true;
}

TWBMSWSensor::Availability TWBMSWSensor::ConvertAvailability(uint16_t availability) const
{
    switch (availability) {
        case 0:
            return TWBMSWSensor::Availability::WB_MSW_SENSOR_UNAVAILABLE;
        case 1:
            return TWBMSWSensor::Availability::WB_MSW_SENSOR_AVAILABLE;
        default:
            return TWBMSWSensor::Availability::WB_MSW_SENSOR_UNKNOWN;
    }
}

bool TWBMSWSensor::ReadAvailabilityRegister(TWBMSWSensor::Availability& availability, uint16_t registerAddress)
{
    uint16_t availabilityFlag;
    if (readInputRegisters(Address, registerAddress, 1, &availabilityFlag)) {
        availability = ConvertAvailability(availabilityFlag);
        return true;
    }
    return false;
}

bool TWBMSWSensor::GetTemperatureAvailability(TWBMSWSensor::Availability& availability)
{
    return ReadAvailabilityRegister(availability, WBMSW_REG_TEMPERATURE_AVAIL);
}
bool TWBMSWSensor::GetHumidityAvailability(TWBMSWSensor::Availability& availability)
{
    return ReadAvailabilityRegister(availability, WBMSW_REG_HUMIDITY_AVAIL);
}
bool TWBMSWSensor::GetLuminanceAvailability(TWBMSWSensor::Availability& availability)
{
    return ReadAvailabilityRegister(availability, WBMSW_REG_LUMINANCE_AVAIL);
}
bool TWBMSWSensor::GetCO2Availability(TWBMSWSensor::Availability& availability)
{
    return ReadAvailabilityRegister(availability, WBMSW_REG_CO2_AVAIL);
}
bool TWBMSWSensor::GetVocAvailability(TWBMSWSensor::Availability& availability)
{
    return ReadAvailabilityRegister(availability, WBMSW_REG_VOC_AVAIL);
}
bool TWBMSWSensor::GetNoiseLevelAvailability(TWBMSWSensor::Availability& availability)
{
    return ReadAvailabilityRegister(availability, WBMSW_REG_NOIZE_AVAIL);
}
bool TWBMSWSensor::GetMotionAvailability(TWBMSWSensor::Availability& availability)
{
    return ReadAvailabilityRegister(availability, WBMSW_REG_MOTION_AVAIL);
}
