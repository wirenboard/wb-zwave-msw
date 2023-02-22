#include "TZWAVEChannel.h"
#include "Arduino.h"

TZWAVEChannel::TZWAVEChannel()
{
    this->ReportedValue = 0;
    this->Triggered = false;
    this->ValueInitializationState = TZWAVEChannel::State::UNINITIALIZED;
    this->Availability = TWBMSWSensor::Availability::UNKNOWN;
    this->Enabled = false;
}

void TZWAVEChannel::ChannelInitialize(String name,
                                      TZWAVEChannel::Type type,
                                      int32_t errorValue,
                                      uint8_t reportThresHoldParameterNumber,
                                      uint8_t levelSendBasicParameterNumber,
                                      uint8_t hysteresisBasicParameterNumber,
                                      uint8_t onCommandsParameterNumber,
                                      uint8_t offCommandsParameterNumber,
                                      uint8_t onOffCommandsRuleParameterNumber,
                                      uint16_t multiplier,
                                      TWBMSWSensor* wbMsw,
                                      TWBMSWSensor::GetValueCallback readValueCallback,
                                      TWBMSWSensor::GetAvailabilityCallback readAvailabilityCallback)
{
    Name = name;
    ChannelType = type;
    ErrorValue = errorValue;
    ReportThresHoldParameterNumber = reportThresHoldParameterNumber;
    LevelSendBasicParameterNumber = levelSendBasicParameterNumber;
    HysteresisBasicParameterNumber = hysteresisBasicParameterNumber;
    OnCommandsParameterNumber = onCommandsParameterNumber;
    OffCommandsParameterNumber = offCommandsParameterNumber;
    OnOffCommandsRuleParameterNumber = onOffCommandsRuleParameterNumber;
    Multiplier = multiplier;
    WbMsw = wbMsw;
    ReadValueCallback = readValueCallback;
    ReadAvailabilityCallback = readAvailabilityCallback;
}

void TZWAVEChannel::SetChannelNumbers(uint8_t channelDeviceNumber, uint8_t channelServerNumber, uint8_t groupIndex)
{
    DeviceChannelNumber = channelDeviceNumber;
    ServerChannelNumber = channelServerNumber;
    GroupIndex = groupIndex;
}

String TZWAVEChannel::GetName() const
{
    return Name;
}

void TZWAVEChannel::SetValue(int64_t value)
{
    Value = value;
}

void* TZWAVEChannel::GetValuePointer()
{
    return &Value;
}

bool TZWAVEChannel::ReadValueFromSensor(int64_t& value)
{
    return (WbMsw->*ReadValueCallback)(value);
}

int32_t TZWAVEChannel::GetErrorValue() const
{
    return ErrorValue;
}

bool TZWAVEChannel::GetEnabled() const
{
    return Enabled;
}

void TZWAVEChannel::Enable()
{
    Enabled = true;
}

TZWAVEChannel::Type TZWAVEChannel::GetType() const
{
    return ChannelType;
}

uint8_t TZWAVEChannel::GetGroupIndex() const
{
    return GroupIndex;
}

uint8_t TZWAVEChannel::GetDeviceChannelNumber() const
{
    return DeviceChannelNumber;
}

uint8_t TZWAVEChannel::GetServerChannelNumber() const
{
    return ServerChannelNumber;
}

int64_t TZWAVEChannel::GetReportedValue() const
{
    return ReportedValue;
}

void TZWAVEChannel::SetReportedValue(int64_t reportedValue)
{
    ReportedValue = reportedValue;
    ValueInitializationState = TZWAVEChannel::State::INITIALIZED;
}

bool TZWAVEChannel::GetTriggered() const
{
    return Triggered;
}
void TZWAVEChannel::SetTriggered(bool triggered)
{
    Triggered = triggered;
}

bool TZWAVEChannel::GetAutocalibration() const
{
    return Autocalibration;
}

bool TZWAVEChannel::SetAutocalibration(bool autocalibration)
{
    Autocalibration = autocalibration;
    return WbMsw->SetCO2Autocalibration(autocalibration);
}

bool TZWAVEChannel::SetPowerOn()
{
    if (ChannelType == TZWAVEChannel::Type::CO2) {
        bool co2Enable;
        if (WbMsw->GetCO2Status(co2Enable) && (co2Enable || WbMsw->SetCO2Status(true)))
            return true;
    }
    return false;
}

TZWAVEChannel::State TZWAVEChannel::GetState() const
{
    return ValueInitializationState;
}

TWBMSWSensor::Availability TZWAVEChannel::GetAvailability()
{
    return Availability;
}

bool TZWAVEChannel::UpdateAvailability()
{
    return (WbMsw->*ReadAvailabilityCallback)(Availability);
}