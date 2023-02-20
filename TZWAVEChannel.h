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
        INTRUSION,
    };
    static const int CHANNEL_TYPES_COUNT = 8;

    TZWAVEChannel();
    void ChannelInitialize(String name,
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

    inline uint8_t GetReportThresHoldParameterNumber(void)
    {
        return (ReportThresHoldParameterNumber);
    };
    inline uint8_t GetLevelSendBasicParameterNumber(void)
    {
        return (LevelSendBasicParameterNumber);
    };
    inline uint8_t GetHysteresisBasicParameterNumber(void)
    {
        return (HysteresisBasicParameterNumber);
    };
    inline uint8_t GetOnCommandsParameterNumber(void)
    {
        return (OnCommandsParameterNumber);
    };
    inline uint8_t GetOffCommandsParameterNumber(void)
    {
        return (OffCommandsParameterNumber);
    };
    inline uint8_t GetOnOffCommandsRuleParameterNumber(void)
    {
        return (OnOffCommandsRuleParameterNumber);
    };
    inline uint16_t GetMultiplier(void)
    {
        return (Multiplier);
    };

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

    uint8_t ReportThresHoldParameterNumber;
    uint8_t LevelSendBasicParameterNumber;
    uint8_t HysteresisBasicParameterNumber;
    uint8_t OnCommandsParameterNumber;
    uint8_t OffCommandsParameterNumber;
    uint8_t OnOffCommandsRuleParameterNumber;
    uint16_t Multiplier;
};