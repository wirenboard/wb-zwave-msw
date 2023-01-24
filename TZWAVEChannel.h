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
							uint8_t ReportThresHoldParameterNumber,
							uint8_t LevelSendBasicParameterNumber,
							uint8_t HysteresisBasicParameterNumber,
							uint8_t OnCommandsParameterNumber,
							uint8_t OffCommandsParameterNumber,
							uint8_t OnOffCommandsRuleParameterNumber,
							uint16_t multiple,
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

	inline uint8_t GetReportThresHoldParameterNumber(void) {return (this->_ReportThresHoldParameterNumber);};
	inline uint8_t GetLevelSendBasicParameterNumber(void) {return (this->_LevelSendBasicParameterNumber);};
	inline uint8_t GetHysteresisBasicParameterNumber(void) {return (this->_HysteresisBasicParameterNumber);};
	inline uint8_t GetOnCommandsParameterNumber(void) {return (this->_OnCommandsParameterNumber);};
	inline uint8_t GetOffCommandsParameterNumber(void) {return (this->_OffCommandsParameterNumber);};
	inline uint8_t GetOnOffCommandsRuleParameterNumber(void) {return (this->_OnOffCommandsRuleParameterNumber);};
	inline uint16_t GetMultiple(void) {return (this->_multiple);};

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

	uint8_t					_ReportThresHoldParameterNumber;
	uint8_t					_LevelSendBasicParameterNumber;
	uint8_t					_HysteresisBasicParameterNumber;
	uint8_t					_OnCommandsParameterNumber;
	uint8_t					_OffCommandsParameterNumber;
	uint8_t					_OnOffCommandsRuleParameterNumber;
	uint16_t				_multiple;
};