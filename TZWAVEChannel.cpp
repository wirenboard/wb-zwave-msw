#include "Arduino.h"
#include "TZWAVEChannel.h"

TZWAVEChannel::TZWAVEChannel()
{
    this->ReportedValue = 0;
    this->Triggered = false;
}

void TZWAVEChannel::SetTemperatureChannel(enum TZWAVEChannelType type,
                                          uint8_t channelNumber,
                                          uint8_t groupIndex,
                                          int16_t temperature)
{
    Type = type;
    ChannelNumber = channelNumber;
    GroupIndex = groupIndex;
    Temperature = temperature;
}
void TZWAVEChannel::SetHumidityChannel(enum TZWAVEChannelType type,
                                       uint8_t channelNumber,
                                       uint8_t groupIndex,
                                       uint16_t humidity)
{
    Type = type;
    ChannelNumber = channelNumber;
    GroupIndex = groupIndex;
    Humidity = humidity;
}
void TZWAVEChannel::SetVocChannel(enum TZWAVEChannelType type, uint8_t channelNumber, uint8_t groupIndex, uint16_t voc)
{
    Type = type;
    ChannelNumber = channelNumber;
    GroupIndex = groupIndex;
    Voc = voc;
}
void TZWAVEChannel::SetNoiseLevelChannel(enum TZWAVEChannelType type,
                                         uint8_t channelNumber,
                                         uint8_t groupIndex,
                                         uint16_t noiseLevel)
{
    Type = type;
    ChannelNumber = channelNumber;
    GroupIndex = groupIndex;
    NoiseLevel = noiseLevel;
}
void TZWAVEChannel::SetCO2Channel(enum TZWAVEChannelType type, uint8_t channelNumber, uint8_t groupIndex, uint16_t co2)
{
    Type = type;
    ChannelNumber = channelNumber;
    GroupIndex = groupIndex;
    Autocalibration = false;
    CO2 = co2;
}
void TZWAVEChannel::SetLuminanceChannel(enum TZWAVEChannelType type,
                                        uint8_t channelNumber,
                                        uint8_t groupIndex,
                                        uint32_t luminance)
{
    Type = type;
    ChannelNumber = channelNumber;
    GroupIndex = groupIndex;
    Luminance = luminance;
}
void TZWAVEChannel::SetBMotionChannel(enum TZWAVEChannelType type,
                                      uint8_t channelNumber,
                                      uint8_t groupIndex,
                                      uint8_t bMotion)
{
    Type = type;
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