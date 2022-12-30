#include "Arduino.h"
#include "TWBMSWSensor.h"

class TZWAVEChannel
{
public:
    enum class State
    {
        UNINITIALIZED,
        INITIALIZED
    };

    enum class Type
    {
        TEMPERATURE,
        HUMIDITY,
        LUMEN,
        CO2,
        VOC,
        NOISE_LEVEL,
        MOTION,
    };
    static const int CHANNEL_TYPES_COUNT = 7;

    TZWAVEChannel();
    void ChannelInitialize(String name,
                           TZWAVEChannel::Type type,
                           int32_t errorValue,
                           uint8_t hysteresisParameterNumber,
                           uint8_t thresholdParameterNumber,
                           uint8_t inversionParameterNumber,
                           TWBMSWSensor* wbMsw,
                           TWBMSWSensor::GetValueCallback readValueCallback,
                           TWBMSWSensor::GetAvailabilityCallback readAvailabilityCallback);

    void SetChannelNumbers(uint8_t channelDeviceNumber, uint8_t channelServerNumber, uint8_t groupIndex);

    String GetName() const;
    void SetValue(int64_t value);
    void* GetValuePointer();
    bool ReadValueFromSensor(int64_t& value);
    int32_t GetErrorValue() const;
    bool GetEnabled() const;
    void Enable();

    TZWAVEChannel::Type GetType() const;
    uint8_t GetGroupIndex() const;
    uint8_t GetDeviceChannelNumber() const;
    uint8_t GetServerChannelNumber() const;

    uint8_t GetHysteresisParameterNumber() const;
    uint8_t GetThresholdParameterNumber() const;
    uint8_t GetInversionParameterNumber() const;

    int64_t GetReportedValue() const;
    void SetReportedValue(int64_t reportedValue);

    bool GetTriggered() const;
    void SetTriggered(bool triggered);

    bool GetAutocalibration() const;
    bool SetAutocalibration(bool autocalibration);
    bool SetPowerOn();

    TZWAVEChannel::State GetState() const;
    TWBMSWSensor::Availability GetAvailability();
    bool UpdateAvailability();

private:
    String Name;
    int64_t Value;
    int32_t ErrorValue;
    TZWAVEChannel::State ValueInitializationState;
    TWBMSWSensor::Availability Availability;
    bool Enabled;
    TZWAVEChannel::Type ChannelType;
    uint8_t DeviceChannelNumber;
    uint8_t ServerChannelNumber;
    uint8_t GroupIndex;

    int64_t ReportedValue; // A value sent to the controller
    bool Triggered;        // Threshold exceeding trigger flag
    bool Autocalibration;  // For CO2 channel type

    TWBMSWSensor* WbMsw;
    TWBMSWSensor::GetAvailabilityCallback ReadAvailabilityCallback;
    TWBMSWSensor::GetValueCallback ReadValueCallback;
    uint8_t HysteresisParameterNumber, ThresholdParameterNumber, InversionParameterNumber;
};