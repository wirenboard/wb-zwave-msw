#include "Arduino.h"

enum TZWAVEChannelType
{
    ZWAVE_CHANNEL_TYPE_TEMPERATURE,
    ZWAVE_CHANNEL_TYPE_HUMIDITY,
    ZWAVE_CHANNEL_TYPE_LUMEN,
    ZWAVE_CHANNEL_TYPE_CO2,
    ZWAVE_CHANNEL_TYPE_VOC,
    ZWAVE_CHANNEL_TYPE_NOISE_LEVEL,
    ZWAVE_CHANNEL_TYPE_MOTION,
    ZWAVE_CHANNEL_MAX
};

enum TZWAVEChannelState
{
    ZWAVE_CHANNEL_UNINITIALIZED,
    ZWAVE_CHANNEL_INITIALIZED
};

class TZWAVEChannel
{
public:
    TZWAVEChannel();
    void SetTemperatureChannel(uint8_t channelNumber, uint8_t groupIndex);
    void SetHumidityChannel(uint8_t channelNumber, uint8_t groupIndex);
    void SetVocChannel(uint8_t channelNumber, uint8_t groupIndex);
    void SetNoiseLevelChannel(uint8_t channelNumber, uint8_t groupIndex);
    void SetCO2Channel(uint8_t channelNumber, uint8_t groupIndex);
    void SetLuminanceChannel(uint8_t channelNumber, uint8_t groupIndex);
    void SetBMotionChannel(uint8_t channelNumber, uint8_t groupIndex);

    void SetTemperatureValue(int16_t temperature);
    void SetHumidityValue(uint16_t humidity);
    void SetVocValue(uint16_t voc);
    void SetNoiseLevelValue(uint16_t noiseLevel);
    void SetCO2Value(uint16_t co2);
    void SetLuminanceValue(uint32_t luminance);
    void SetBMotionValue(uint8_t bMotion);

    void* GetValuePointer();

    enum TZWAVEChannelType GetType();
    uint8_t GetGroupIndex();
    uint8_t GetChannelNumber();

    int32_t GetReportedValue();
    void SetReportedValue(int32_t reportedValue);

    bool GetTriggered();
    void SetTriggered(bool triggered);

    bool GetAutocalibration();
    void SetAutocalibration(bool autocalibration);

    enum TZWAVEChannelState GetState();

private:
    union
    {
        int16_t Temperature;
        uint16_t Humidity;
        uint16_t Voc;
        uint16_t NoiseLevel;
        uint16_t CO2;
        uint32_t Luminance;
        uint8_t BMotion;
    };
    enum TZWAVEChannelState State;
    enum TZWAVEChannelType Type;
    uint8_t ChannelNumber;
    uint8_t GroupIndex;
    int32_t ReportedValue; // A value sent to the controller
    bool Triggered;        // Threshold exceeding trigger flag
    bool Autocalibration;  // For CO2 channel type
};