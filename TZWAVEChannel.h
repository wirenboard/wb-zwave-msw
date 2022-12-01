#include "Arduino.h"

enum TZWAVEChannelType
{
    WB_MSW_CHANNEL_TYPE_TEMPERATURE,
    WB_MSW_CHANNEL_TYPE_HUMIDITY,
    WB_MSW_CHANNEL_TYPE_LUMEN,
    WB_MSW_CHANNEL_TYPE_CO2,
    WB_MSW_CHANNEL_TYPE_VOC,
    WB_MSW_CHANNEL_TYPE_NOISE_LEVEL,
    WB_MSW_CHANNEL_TYPE_MOTION,
    WB_MSW_CHANNEL_MAX
};

class TZWAVEChannel
{
public:
    TZWAVEChannel();
    void SetTemperatureChannel(uint8_t channelNumber, uint8_t groupIndex, int16_t temperature);
    void SetHumidityChannel(uint8_t channelNumber, uint8_t groupIndex, uint16_t humidity);
    void SetVocChannel(uint8_t channelNumber, uint8_t groupIndex, uint16_t voc);
    void SetNoiseLevelChannel(uint8_t channelNumber, uint8_t groupIndex, uint16_t noiseLevel);
    void SetCO2Channel(uint8_t channelNumber, uint8_t groupIndex, uint16_t co2);
    void SetLuminanceChannel(uint8_t channelNumber, uint8_t groupIndex, uint32_t luminance);
    void SetBMotionChannel(uint8_t channelNumber, uint8_t groupIndex, uint8_t bMotion);
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

    // It's nessesary to make this union public, CHANNEL_HANDLER_SINGLE_VALUEMAPPER takes only direct access to the
    // field
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

private:
    enum TZWAVEChannelType Type;
    uint8_t ChannelNumber;
    uint8_t GroupIndex;
    int32_t ReportedValue; // A value sent to the controller
    bool Triggered;        // Threshold exceeding trigger flag
    bool Autocalibration;  // For CO2 channel type
};