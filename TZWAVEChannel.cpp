#include "TZWAVEChannel.h"
#include "Arduino.h"

TZWAVEChannel::TZWAVEChannel()
{
    this->ReportedValue = 0;
    this->Triggered = false;
}

void TZWAVEChannel::SetTemperatureChannel(uint8_t channelNumber, uint8_t groupIndex, int16_t temperature)
{
    Type = TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_TEMPERATURE;
    ChannelNumber = channelNumber;
    GroupIndex = groupIndex;
    Temperature = temperature;
}
void TZWAVEChannel::SetHumidityChannel(uint8_t channelNumber, uint8_t groupIndex, uint16_t humidity)
{
    Type = TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_HUMIDITY;
    ChannelNumber = channelNumber;
    GroupIndex = groupIndex;
    Humidity = humidity;
}
void TZWAVEChannel::SetVocChannel(uint8_t channelNumber, uint8_t groupIndex, uint16_t voc)
{
    Type = TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_VOC;
    ChannelNumber = channelNumber;
    GroupIndex = groupIndex;
    Voc = voc;
}
void TZWAVEChannel::SetNoiseLevelChannel(uint8_t channelNumber, uint8_t groupIndex, uint16_t noiseLevel)
{
    Type = TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_NOISE_LEVEL;
    ChannelNumber = channelNumber;
    GroupIndex = groupIndex;
    NoiseLevel = noiseLevel;
}
void TZWAVEChannel::SetCO2Channel(uint8_t channelNumber, uint8_t groupIndex, uint16_t co2)
{
    Type = TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_CO2;
    ChannelNumber = channelNumber;
    GroupIndex = groupIndex;
    Autocalibration = false;
    CO2 = co2;
}
void TZWAVEChannel::SetLuminanceChannel(uint8_t channelNumber, uint8_t groupIndex, uint32_t luminance)
{
    Type = TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_LUMEN;
    ChannelNumber = channelNumber;
    GroupIndex = groupIndex;
    Luminance = luminance;
}
void TZWAVEChannel::SetBMotionChannel(uint8_t channelNumber, uint8_t groupIndex, uint8_t bMotion)
{
    Type = TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_MOTION;
    ChannelNumber = channelNumber;
    GroupIndex = groupIndex;
    BMotion = bMotion;
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
        case TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_TEMPERATURE:
            return &Temperature;
            break;
        case TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_HUMIDITY:
            return &Humidity;
            break;
        case TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_LUMEN:
            return &Luminance;
            break;
        case TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_CO2:
            return &CO2;
            break;
        case TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_VOC:
            return &Voc;
            break;
        case TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_NOISE_LEVEL:
            return &NoiseLevel;
            break;
        case TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_MOTION:
            return &BMotion;
            break;
        default:
            return NULL;
            break;
    }
}