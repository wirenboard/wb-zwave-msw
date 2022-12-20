#include "TWBMSWSensor.h"
#include "TZWAVEChannel.h"
#include "WbMsw.h"

enum TZWAVEProcessResult
{
    ZWAVE_PROCESS_OK,
    ZWAVE_PROCESS_VALUE_ERROR,
    ZWAVE_PROCESS_MODBUS_ERROR
};

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

    enum TZWAVEProcessResult ProcessChannels();

private:
    TWBMSWSensor* WbMsw;
    TZWAVEChannel Channels[ZWAVE_CHANNEL_MAX];
    TZWAVEChannel* MotionChannelPtr;
    uint16_t ChannelsCount;

    ZunoCFGParameter_t Parameters[WB_MSW_MAX_CONFIG_PARAM];
    int32_t ParameterValues[WB_MSW_MAX_CONFIG_PARAM];

    bool CheckChannelAvailabilityIfUnknown(
        enum TWBMSWSensorAvailability& currentAvailability,
        bool (TWBMSWSensor::*getAvailabilityFunction)(enum TWBMSWSensorAvailability& availability));

    TZWAVEChannel* GetChannelByType(enum TZWAVEChannelType type);
    ZunoCFGParameter_t* GetParameterByNumber(size_t paramNumber);

    enum TZWAVEProcessResult ProcessTemperatureChannel(TZWAVEChannel& channel);
    enum TZWAVEProcessResult ProcessHumidityChannel(TZWAVEChannel& channel);
    enum TZWAVEProcessResult ProcessLuminanceChannel(TZWAVEChannel& channel);
    enum TZWAVEProcessResult ProcessCO2Channel(TZWAVEChannel& channel);
    enum TZWAVEProcessResult ProcessVOCChannel(TZWAVEChannel& channel);
    enum TZWAVEProcessResult ProcessNoiseLevelChannel(TZWAVEChannel& channel);
    enum TZWAVEProcessResult ProcessMotionChannel(TZWAVEChannel& channel);
    void MotionChannelReset(TZWAVEChannel* channel);
    uint32_t MotionLastTime;
    void PublishAnalogSensorValue(TZWAVEChannel& channel,
                                  int32_t value,
                                  int32_t hysteresis,
                                  bool inversion,
                                  int32_t threshold);
};