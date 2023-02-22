#include "TWBMSWSensor.h"
#include "TZWAVEChannel.h"
#include "WbMsw.h"

class TZWAVESensor
{
public:
    enum class Result
    {
        ZWAVE_PROCESS_OK,
        ZWAVE_PROCESS_VALUE_ERROR,
        ZWAVE_PROCESS_MODBUS_ERROR
    };
    TZWAVESensor(TWBMSWSensor* wbMsw);
    bool ChannelsInitialize();
    void ChannelsSetup();
    void SetChannelHandlers();
    const char* GetGroupNameByIndex(uint8_t groupIndex);

    void ParametersInitialize(void);
    const ZunoCFGParameter_t* GetParameterIfChannelExists(size_t paramNumber);
    void SetParameterValue(size_t paramNumber, int32_t value);
    int32_t GetParameterValue(size_t paramNumber);

    TZWAVESensor::Result ProcessChannels();
    ZunoCFGParameter_t* GetParameterByNumber(size_t paramNumber);

private:
    TWBMSWSensor* WbMsw;
    TZWAVEChannel Channels[TZWAVEChannel::CHANNEL_TYPES_COUNT];
    TZWAVEChannel* MotionChannelPtr;
    TZWAVEChannel* IntrusionChannelPtr;

    ZunoCFGParameter_t Parameters[WB_MSW_MAX_CONFIG_PARAM];
    int32_t ParameterValues[WB_MSW_MAX_CONFIG_PARAM];

    TZWAVEChannel* GetChannelByType(TZWAVEChannel::Type type);

    TZWAVESensor::Result ProcessCommonChannel(TZWAVEChannel& channel);
    TZWAVESensor::Result ProcessMotionChannel(TZWAVEChannel& channel);
    void MotionChannelReset(TZWAVEChannel* channel);
    uint32_t MotionLastTime;
    uint32_t IntrusionLastTime;
    bool MotionLastTimeWaitOff;
    bool IntrusionLastTimeWaitOff;
    void PublishAnalogSensorValue(TZWAVEChannel& channel,
                                  int64_t value,
                                  int32_t reportThresHold,
                                  int32_t levelSendBasic,
                                  int32_t hysteresisBasic,
                                  int32_t onCommands,
                                  int32_t offCommands,
                                  int32_t onOffCommandsRule);
    void PublishMotionValue(TZWAVEChannel* channel, int64_t value);
    void PublishIntrusionValue(TZWAVEChannel* channel, int64_t value);
};