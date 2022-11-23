#include "TWBMSWSensor.h"
#include "TZWAVEChannel.h"
#include "WbMsw.h"

class TZWAVESensor
{
public:
    TZWAVESensor(TWBMSWSensor* wbMsw);
    bool ChannelsInitialize();
    void ChannelsSetup();
    void SetChannelHandlers();
    const char* GetGroupNameByIndex(uint8_t groupIndex);

    void ParametersInitialize(void);
    const ZunoCFGParameter_t* GetParameterIfChannelExists(size_t paramNumber);
    void SetParameterValue(size_t paramNumber, int32_t value);
    int32_t GetParameterValue(size_t paramNumber);

    void ProcessChannels();

private:
    TWBMSWSensor* WbMsw;
    TZWAVEChannel Channels[WB_MSW_CHANNEL_MAX];
    uint16_t ChannelsCount;

    ZunoCFGParameter_t Parameters[WB_MSW_MAX_CONFIG_PARAM];
    int32_t ParameterValues[WB_MSW_MAX_CONFIG_PARAM];

    TZWAVEChannel* GetChannelByType(enum TZWAVEChannelType type);
    ZunoCFGParameter_t* GetParameterByNumber(size_t paramNumber);

    void ProcessTemperatureChannel(TZWAVEChannel& channel);
    void ProcessHumidityChannel(TZWAVEChannel& channel);
    void ProcessLuminanceChannel(TZWAVEChannel& channel);
    void ProcessCO2Channel(TZWAVEChannel& channel);
    void ProcessVOCChannel(TZWAVEChannel& channel);
    void ProcessNoiseLevelChannel(TZWAVEChannel& channel);
    void ProcessMotionChannel(TZWAVEChannel& channel);
    uint32_t MotionLastTime;
    void PublishAnalogSensorValue(TZWAVEChannel& channel,
                                  int32_t value,
                                  int32_t hysteresis,
                                  bool inversion,
                                  int32_t threshold);
};