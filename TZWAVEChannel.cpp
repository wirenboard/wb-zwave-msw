#include "TZWAVEChannel.h"
#include "Arduino.h"

TZWAVEChannel::TZWAVEChannel()
{
    this->ReportedValue = 0;
    this->Triggered = false;
}

void TZWAVEChannel::SetTemperatureChannel(uint8_t channelNumber, uint8_t groupIndex, int16_t temperature)
{
    Type = TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_TEMPERATURE;
    ChannelNumber = channelNumber;
    GroupIndex = groupIndex;
    SetTemperatureValue(temperature);
}
void TZWAVEChannel::SetHumidityChannel(uint8_t channelNumber, uint8_t groupIndex, uint16_t humidity)
{
    Type = TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_HUMIDITY;
    ChannelNumber = channelNumber;
    GroupIndex = groupIndex;
    SetHumidityValue(humidity);
}
void TZWAVEChannel::SetVocChannel(uint8_t channelNumber, uint8_t groupIndex, uint16_t voc)
{
    Type = TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_VOC;
    ChannelNumber = channelNumber;
    GroupIndex = groupIndex;
    SetVocValue(voc);
}
void TZWAVEChannel::SetNoiseLevelChannel(uint8_t channelNumber, uint8_t groupIndex, uint16_t noiseLevel)
{
    Type = TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_NOISE_LEVEL;
    ChannelNumber = channelNumber;
    GroupIndex = groupIndex;
    SetNoiseLevelValue(noiseLevel);
}
void TZWAVEChannel::SetCO2Channel(uint8_t channelNumber, uint8_t groupIndex, uint16_t co2)
{
    Type = TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_CO2;
    ChannelNumber = channelNumber;
    GroupIndex = groupIndex;
    Autocalibration = false;
    SetCO2Value(co2);
}
void TZWAVEChannel::SetLuminanceChannel(uint8_t channelNumber, uint8_t groupIndex, uint32_t luminance)
{
    Type = TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_LUMEN;
    ChannelNumber = channelNumber;
    GroupIndex = groupIndex;
    SetLuminanceValue(luminance);
}
void TZWAVEChannel::SetBMotionChannel(uint8_t channelNumber, uint8_t groupIndex, uint8_t bMotion)
{
    Type = TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_MOTION;
    ChannelNumber = channelNumber;
    GroupIndex = groupIndex;
    SetBMotionValue(bMotion);
}
void TZWAVEChannel::SetTemperatureValue(int16_t temperature)
{
    if (Type == TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_TEMPERATURE) {
        Temperature = temperature;
    }
}
void TZWAVEChannel::SetHumidityValue(uint16_t humidity)
{
    if (Type == TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_HUMIDITY) {
        Humidity = humidity;
    }
}
void TZWAVEChannel::SetVocValue(uint16_t voc)
{
    if (Type == TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_VOC) {
        Voc = voc;
    }
}
void TZWAVEChannel::SetNoiseLevelValue(uint16_t noiseLevel)
{
    if (Type == TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_NOISE_LEVEL) {
        NoiseLevel = noiseLevel;
    }
}
void TZWAVEChannel::SetCO2Value(uint16_t co2)
{
    if (Type == TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_CO2) {
        CO2 = co2;
    }
}
void TZWAVEChannel::SetLuminanceValue(uint32_t luminance)
{
    if (Type == TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_LUMEN) {
        Luminance = luminance;
    }
}
void TZWAVEChannel::SetBMotionValue(uint8_t bMotion)
{
    if (Type == TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_MOTION) {
        BMotion = bMotion;
    }
}
int16_t TZWAVEChannel::GetTemperatureValue()
{
    if (Type == TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_TEMPERATURE) {
        return Temperature;
    }
}
uint16_t TZWAVEChannel::GetHumidityValue()
{
    if (Type == TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_HUMIDITY) {
        return Humidity;
    }
}
uint16_t TZWAVEChannel::GetVocValue()
{
    if (Type == TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_VOC) {
        return Voc;
    }
}
uint16_t TZWAVEChannel::GetNoiseLevelValue()
{
    if (Type == TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_NOISE_LEVEL) {
        return NoiseLevel;
    }
}
uint16_t TZWAVEChannel::GetCO2Value()
{
    if (Type == TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_CO2) {
        return CO2;
    }
}
uint32_t TZWAVEChannel::GetLuminanceValue()
{
    if (Type == TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_LUMEN) {
        return Luminance;
    }
}
uint8_t TZWAVEChannel::GetBMotionValue()
{
    if (Type == TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_MOTION) {
        return BMotion;
    }
}

enum TZWAVEChannelType TZWAVEChannel::GetType()
{
    return Type;
}

uint8_t TZWAVEChannel::GetChannelNumber()
{
    return ChannelNumber;
}

uint8_t TZWAVEChannel::GetGroupIndex()
{
    return GroupIndex;
}

int32_t TZWAVEChannel::GetReportedValue()
{
    return ReportedValue;
}

void TZWAVEChannel::SetReportedValue(int32_t reportedValue)
{
    ReportedValue = reportedValue;
}

bool TZWAVEChannel::GetTriggered()
{
    return Triggered;
}
void TZWAVEChannel::SetTriggered(bool triggered)
{
    Triggered = triggered;
}

bool TZWAVEChannel::GetAutocalibration()
{
    return Autocalibration;
}

void TZWAVEChannel::SetAutocalibration(bool autocalibration)
{
    Autocalibration == autocalibration;
}

void* TZWAVEChannel::GetValuePointer()
{
    switch (Type) {
        case TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_TEMPERATURE:
            return &Temperature;
        case TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_HUMIDITY:
            return &Humidity;
        case TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_LUMEN:
            return &Luminance;
        case TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_CO2:
            return &CO2;
        case TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_VOC:
            return &Voc;
        case TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_NOISE_LEVEL:
            return &NoiseLevel;
        case TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_MOTION:
            return &BMotion;
        default:
            return NULL;
    }
}