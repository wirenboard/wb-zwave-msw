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

    void SetTemperatureValue(int16_t temperature);
    void SetHumidityValue(uint16_t humidity);
    void SetVocValue(uint16_t voc);
    void SetNoiseLevelValue(uint16_t noiseLevel);
    void SetCO2Value(uint16_t co2);
    void SetLuminanceValue(uint32_t luminance);
    void SetBMotionValue(uint8_t bMotion);

    int16_t GetTemperatureValue();
    uint16_t GetHumidityValue();
    uint16_t GetVocValue();
    uint16_t GetNoiseLevelValue();
    uint16_t GetCO2Value();
    uint32_t GetLuminanceValue();
    uint8_t GetBMotionValue();
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
    enum TZWAVEChannelType Type;
    uint8_t ChannelNumber;
    uint8_t GroupIndex;
    int32_t ReportedValue; // A value sent to the controller
    bool Triggered;        // Threshold exceeding trigger flag
    bool Autocalibration;  // For CO2 channel type
};