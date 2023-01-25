#include "TZWAVESensor.h"
#include "Arduino.h"
#include "DebugOutput.h"
#include "WbMsw.h"
#include <string.h>

TZWAVESensor::TZWAVESensor(TWBMSWSensor* wbMsw): WbMsw(wbMsw)
{
    // Available device parameters description
    // Channels in the device are created dynamically, so parameters are described in "dynamic" style
    ZunoCFGParameter_t parameters[WB_MSW_MAX_CONFIG_PARAM] = {

        // Temperature channel settings
        #define WB_MSW_CONFIG_PARAMETER_TEMPERATURE_MULTIPLE			1
        ZUNO_CONFIG_PARAMETER_INFO("Temperature Report Threshold",
                                    "0 - Reports disabled. Send Report if the temperature has changed after the last report. Value in 0.01C (100 = 1C).",
                                    0, 4000, 100),//64
        ZUNO_CONFIG_PARAMETER_INFO_SIGN("Temperature Level to send Basic Set",
                                        "Send Basic Set if the temperature has crossed the level up or down + hysteresis. Value in 0.01C (100 = 1C).",
                                        -4000, 8000, 4000),//65
        ZUNO_CONFIG_PARAMETER_INFO("Temperature Hysteresis to send Basic Set",
                                    "Value in 0.01C (100 = 1C).",
                                    10, 2000, 50),//66
        ZUNO_CONFIG_PARAMETER_INFO("Temperature ON command",
                                    "Send Basic Set command.",
                                    0, 255, 255),//67
        ZUNO_CONFIG_PARAMETER_INFO("Temperature OFF command",
                                    "Send Basic Set command.",
                                    0, 255, 0),//68
        ZUNO_CONFIG_PARAMETER_INFO("Temperature ON/OFF commands rules",
                                    "1 - Send ON if the temperature is greater than Level. Send OFF if the temperature is less than Level. 2 - Send ON if the temperature is greater than Level. Do not send OFF. 3 - Do not send ON. Send OFF if the temperature is less than Level.",
                                    1, 3, 1),//69

        // Humidity sensor settings
        #define WB_MSW_CONFIG_PARAMETER_HUMIDITY_MULTIPLE				100
        ZUNO_CONFIG_PARAMETER_INFO("Humidity Report Threshold",
                                    "0 - Reports disabled. Send Report if the humidity has changed after the last report. Value in %.",
                                    0, 90, 5),//70
        ZUNO_CONFIG_PARAMETER_INFO("Humidity Level to send Basic Set",
                                    "Send Basic Set if the humidity has crossed the level up or down + hysteresis. Value in %.",
                                    5, 95, 60),//71
        ZUNO_CONFIG_PARAMETER_INFO("Humidity Hysteresis to send Basic Set",
                                    "Value in %.",
                                    1, 50, 2),//72
        ZUNO_CONFIG_PARAMETER_INFO("Humidity ON command",
                                    "Send Basic Set command.",
                                    0, 255, 255),//73
        ZUNO_CONFIG_PARAMETER_INFO("Humidity OFF command",
                                    "Send Basic Set command.",
                                    0, 255, 0),//74
        ZUNO_CONFIG_PARAMETER_INFO("Humidity ON/OFF commands rules",
                                    "1 - Send ON if the humidity is greater than Level. Send OFF if the humidity is less than Level. 2 - Send ON if the humidity is greater than Level. Do not send OFF. 3 - Do not send ON. Send OFF if the humidity is less than Level.",
                                    1, 3, 1),//75

        // Lumen sensor settings
        #define WB_MSW_CONFIG_PARAMETER_LUMEN_MULTIPLE					100
        ZUNO_CONFIG_PARAMETER_INFO("Luminance Report Threshold",
                                    "0 - Reports disabled. Send Report if the luminance has changed after the last report. Value in lux.",
                                    0, 100000, 100),//76
        ZUNO_CONFIG_PARAMETER_INFO("Luminance Level to send Basic Set",
                                    "Send Basic Set if the luminance has crossed the level up or down + hysteresis. Value in lux.",
                                    0, 100000, 200),//77
        ZUNO_CONFIG_PARAMETER_INFO("Luminance Hysteresis to send Basic Set",
                                    "Value in lux.",
                                    1, 50000, 50),//78
        ZUNO_CONFIG_PARAMETER_INFO("Luminance ON command",
                                    "Send Basic Set command.",
                                    0, 255, 255),//79
        ZUNO_CONFIG_PARAMETER_INFO("Luminance OFF command",
                                    "Send Basic Set command.",
                                    0, 255, 0),//80
        ZUNO_CONFIG_PARAMETER_INFO("Luminance ON/OFF commands rules",
                                    "1 - Send ON if the luminance is greater than Level. Send OFF if the luminance is less than Level. 2 - Send ON if the luminance is greater than Level. Do not send OFF. 3 - Do not send ON. Send OFF if the luminance is less than Level.",
                                    1, 3, 1),//81

        // CO2 sensor settings
        #define WB_MSW_CONFIG_PARAMETER_CO2_MULTIPLE					1
        ZUNO_CONFIG_PARAMETER_INFO("CO2 Report Threshold",
                                    "0 - Reports disabled. Send Report if the CO2 has changed after the last report. Value in ppm.",
                                    0, 5000, 100),//82
        ZUNO_CONFIG_PARAMETER_INFO("CO2 Level to send Basic Set",
                                    "Send Basic Set if the CO2 has crossed the level up or down + hysteresis. Value in ppm.",
                                    400, 5000, 1500),//83
        ZUNO_CONFIG_PARAMETER_INFO("CO2 Hysteresis to send Basic Set",
                                    "Value in ppm.",
                                    1, 2500, 50),//84
        ZUNO_CONFIG_PARAMETER_INFO("CO2 ON command",
                                    "Send Basic Set command.",
                                    0, 255, 255),//85
        ZUNO_CONFIG_PARAMETER_INFO("CO2 OFF command",
                                    "Send Basic Set command.",
                                    0, 255, 0),//86
        ZUNO_CONFIG_PARAMETER_INFO("CO2 ON/OFF commands rules",
                                    "1 - Send ON if the CO2 is greater than Level. Send OFF if the CO2 is less than Level. 2 - Send ON if the CO2 is greater than Level. Do not send OFF. 3 - Do not send ON. Send OFF if the CO2 is less than Level.",
                                    1, 3, 1),//87
        ZUNO_CONFIG_PARAMETER_INFO("CO2 auto",
                                    "CO2 auto",
                                    false, true, true),//88

        // VOC sensor settings
        #define WB_MSW_CONFIG_PARAMETER_VOC_MULTIPLE					1
        ZUNO_CONFIG_PARAMETER_INFO("VOC Report Threshold",
                                    "0 - Reports disabled. Send Report if the VOC has changed after the last report. Value in ppb.",
                                    0, 60000, 50),
        ZUNO_CONFIG_PARAMETER_INFO("VOC Level to send Basic Set",
                                    "Send Basic Set if the VOC has crossed the level up or down + hysteresis. Value in ppb.",
                                    0, 60000, 660),
        ZUNO_CONFIG_PARAMETER_INFO("VOC Hysteresis to send Basic Set",
                                    "Value in ppb.",
                                    1, 30000, 10),
        ZUNO_CONFIG_PARAMETER_INFO("VOC ON command",
                                    "Send Basic Set command.",
                                    0, 255, 255),
        ZUNO_CONFIG_PARAMETER_INFO("VOC OFF command",
                                    "Send Basic Set command.",
                                    0, 255, 0),
        ZUNO_CONFIG_PARAMETER_INFO("VOC ON/OFF commands rules",
                                    "1 - Send ON if the VOC is greater than Level. Send OFF if the VOC is less than Level. 2 - Send ON if the VOC is greater than Level. Do not send OFF. 3 - Do not send ON. Send OFF if the VOC is less than Level.",
                                    1, 3, 1),

        // Noise level sensor settings
        #define WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_MULTIPLE					100
        ZUNO_CONFIG_PARAMETER_INFO("Noise Report Threshold",
                                    "0 - Reports disabled. Send Report if the noise has changed after the last report. Value in dB.",
                                    0, 105, 10),
        ZUNO_CONFIG_PARAMETER_INFO("Noise Level to send Basic Set",
                                    "Send Basic Set if the noise has crossed the level up or down + hysteresis. Value in dB.",
                                    38, 105, 80),
        ZUNO_CONFIG_PARAMETER_INFO("Noise Hysteresis to send Basic Set",
                                    "Value in dB.",
                                    1, 50, 5),
        ZUNO_CONFIG_PARAMETER_INFO("Noise ON command",
                                    "Send Basic Set command.",
                                    0, 255, 255),
        ZUNO_CONFIG_PARAMETER_INFO("Noise OFF command",
                                    "Send Basic Set command.",
                                    0, 255, 0),
        ZUNO_CONFIG_PARAMETER_INFO("Noise ON/OFF commands rules",
                                    "1 - Send ON if the noise is greater than Level. Send OFF if the noise is less than Level. 2 - Send ON if the noise is greater than Level. Do not send OFF. 3 - Do not send ON. Send OFF if the noise is less than Level.",
                                    1, 3, 1),

        // Motion sensor settings
        #define WB_MSW_CONFIG_PARAMETER_MOTION_MULTIPLE							1
        #define WB_MSW_CONFIG_PARAMETER_MOTION_ON								300
        #define WB_MSW_CONFIG_PARAMETER_MOTION_OFF								250
        #define WB_MSW_CONFIG_PARAMETER_MOTION_HYSTERESIS						10
        ZUNO_CONFIG_PARAMETER_INFO("Motion delay to send OFF command",
                                    "Value in seconds.",
                                    0, 100000, 60),
        ZUNO_CONFIG_PARAMETER_INFO("Motion ON command",
                                    "Send Basic Set command.",
                                    0, 255, 255),
        ZUNO_CONFIG_PARAMETER_INFO("Motion OFF command",
                                    "Send Basic Set command.",
                                    0, 255, 0),
        ZUNO_CONFIG_PARAMETER_INFO("Motion ON/OFF commands rules",
                                    "1 - Send ON if the motion is detected. Send OFF if the motion is idle. 2 - Send ON if the motion is detected. Do not send OFF. 3 - Do not send ON. Send OFF if the motion is idle.",
                                    1, 3, 1),
    };
    memcpy(Parameters, parameters, sizeof(parameters));
    MotionLastTime = 0;
    this->_MotionLastTime_wait_Off = false;
    #ifdef LOGGING_DBG
    this->_debug_ms_next = 0x0;
    this->_debug_ms = 0x0;
    this->_debug_ms_step = 1000;
    #endif
}

// Function determines number of available Z-Wave device channels (EndPoints) and fills in the structures by channel
// type
bool TZWAVESensor::ChannelsInitialize()
{
    uint32_t startTime = millis();
    uint32_t lastTime = startTime;
    uint32_t timeout = WB_MSW_INPUT_REG_AVAILABILITY_TIMEOUT_MS;

    Channels[0].ChannelInitialize("Temperature",
                                  TZWAVEChannel::Type::TEMPERATURE,
                                  WB_MSW_INPUT_REG_TEMPERATURE_VALUE_ERROR,
                                WB_MSW_CONFIG_PARAMETER_TEMPERATURE_REPORT_THRESHOLD,
                                WB_MSW_CONFIG_PARAMETER_TEMPERATURE_LEVEL_SEND_BASIC,
                                WB_MSW_CONFIG_PARAMETER_TEMPERATURE_HYSTERESIS_SEND_BASIC,
                                WB_MSW_CONFIG_PARAMETER_TEMPERATURE_ON_COMMANDS,
                                WB_MSW_CONFIG_PARAMETER_TEMPERATURE_OFF_COMMANDS,
                                WB_MSW_CONFIG_PARAMETER_TEMPERATURE_ON_OFF_COMMANDS_RULE,
                                WB_MSW_CONFIG_PARAMETER_TEMPERATURE_MULTIPLE,
                                  WbMsw,
                                  &TWBMSWSensor::GetTemperature,
                                  &TWBMSWSensor::GetTemperatureAvailability);

    Channels[1].ChannelInitialize("Humidity",
                                  TZWAVEChannel::Type::HUMIDITY,
                                  WB_MSW_INPUT_REG_HUMIDITY_VALUE_ERROR,
                                WB_MSW_CONFIG_PARAMETER_HUMIDITY_REPORT_THRESHOLD,
                                WB_MSW_CONFIG_PARAMETER_HUMIDITY_LEVEL_SEND_BASIC,
                                WB_MSW_CONFIG_PARAMETER_HUMIDITY_HYSTERESIS_SEND_BASIC,
                                WB_MSW_CONFIG_PARAMETER_HUMIDITY_ON_COMMANDS,
                                WB_MSW_CONFIG_PARAMETER_HUMIDITY_OFF_COMMANDS,
                                WB_MSW_CONFIG_PARAMETER_HUMIDITY_ON_OFF_COMMANDS_RULE,
                                WB_MSW_CONFIG_PARAMETER_HUMIDITY_MULTIPLE,
                                  WbMsw,
                                  &TWBMSWSensor::GetHumidity,
                                  &TWBMSWSensor::GetHumidityAvailability);

    Channels[2].ChannelInitialize("Luminance",
                                  TZWAVEChannel::Type::LUMEN,
                                  WB_MSW_INPUT_REG_LUMEN_VALUE_ERROR,
                                WB_MSW_CONFIG_PARAMETER_LUMEN_REPORT_THRESHOLD,
                                WB_MSW_CONFIG_PARAMETER_LUMEN_LEVEL_SEND_BASIC,
                                WB_MSW_CONFIG_PARAMETER_LUMEN_HYSTERESIS_SEND_BASIC,
                                WB_MSW_CONFIG_PARAMETER_LUMEN_ON_COMMANDS,
                                WB_MSW_CONFIG_PARAMETER_LUMEN_OFF_COMMANDS,
                                WB_MSW_CONFIG_PARAMETER_LUMEN_ON_OFF_COMMANDS_RULE,
                                WB_MSW_CONFIG_PARAMETER_LUMEN_MULTIPLE,
                                  WbMsw,
                                  &TWBMSWSensor::GetLuminance,
                                  &TWBMSWSensor::GetLuminanceAvailability);

    Channels[3].ChannelInitialize("CO2",
                                  TZWAVEChannel::Type::CO2,
                                  WB_MSW_INPUT_REG_CO2_VALUE_ERROR,
                                WB_MSW_CONFIG_PARAMETER_CO2_REPORT_THRESHOLD,
                                WB_MSW_CONFIG_PARAMETER_CO2_LEVEL_SEND_BASIC,
                                WB_MSW_CONFIG_PARAMETER_CO2_HYSTERESIS_SEND_BASIC,
                                WB_MSW_CONFIG_PARAMETER_CO2_ON_COMMANDS,
                                WB_MSW_CONFIG_PARAMETER_CO2_OFF_COMMANDS,
                                WB_MSW_CONFIG_PARAMETER_CO2_ON_OFF_COMMANDS_RULE,
                                WB_MSW_CONFIG_PARAMETER_CO2_MULTIPLE,
                                  WbMsw,
                                  &TWBMSWSensor::GetCO2,
                                  &TWBMSWSensor::GetCO2Availability);

    Channels[4].ChannelInitialize("VOC",
                                  TZWAVEChannel::Type::VOC,
                                  WB_MSW_INPUT_REG_VOC_VALUE_ERROR,
                                WB_MSW_CONFIG_PARAMETER_VOC_REPORT_THRESHOLD,
                                WB_MSW_CONFIG_PARAMETER_VOC_LEVEL_SEND_BASIC,
                                WB_MSW_CONFIG_PARAMETER_VOC_HYSTERESIS_SEND_BASIC,
                                WB_MSW_CONFIG_PARAMETER_VOC_ON_COMMANDS,
                                WB_MSW_CONFIG_PARAMETER_VOC_OFF_COMMANDS,
                                WB_MSW_CONFIG_PARAMETER_VOC_ON_OFF_COMMANDS_RULE,
                                WB_MSW_CONFIG_PARAMETER_VOC_MULTIPLE,
                                  WbMsw,
                                  &TWBMSWSensor::GetVoc,
                                  &TWBMSWSensor::GetVocAvailability);

    Channels[5].ChannelInitialize("NoiseLevel",
                                  TZWAVEChannel::Type::NOISE_LEVEL,
                                  0,
                                WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_REPORT_THRESHOLD,
                                WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_LEVEL_SEND_BASIC,
                                WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_HYSTERESIS_SEND_BASIC,
                                WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_ON_COMMANDS,
                                WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_OFF_COMMANDS,
                                WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_ON_OFF_COMMANDS_RULE,
                                WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_MULTIPLE,
                                  WbMsw,
                                  &TWBMSWSensor::GetNoiseLevel,
                                  &TWBMSWSensor::GetNoiseLevelAvailability);

    Channels[6].ChannelInitialize("Motion",
                                  TZWAVEChannel::Type::MOTION,
                                  WB_MSW_INPUT_REG_MOTION_VALUE_ERROR,
                                0,
                                0,
                                0,
                                WB_MSW_CONFIG_PARAMETER_MOTION_ON_COMMANDS,
                                WB_MSW_CONFIG_PARAMETER_MOTION_OFF_COMMANDS,
                                WB_MSW_CONFIG_PARAMETER_MOTION_ON_OFF_COMMANDS_RULE,
                                WB_MSW_CONFIG_PARAMETER_MOTION_MULTIPLE,
                                  WbMsw,
                                  &TWBMSWSensor::GetMotion,
                                  &TWBMSWSensor::GetMotionAvailability);

    bool unknownSensorsLeft = false;

    do {
        lastTime = millis();
        for (int i = 0; i < TZWAVEChannel::CHANNEL_TYPES_COUNT; i++) {
            if (Channels[i].GetAvailability() == TWBMSWSensor::Availability::UNKNOWN) {
                Channels[i].UpdateAvailability();
                if (Channels[i].GetAvailability() == TWBMSWSensor::Availability::AVAILABLE) {

                    if ((Channels[i].GetType() == TZWAVEChannel::Type::CO2) &&
                        (!Channels[i].SetPowerOn() ||
                         !Channels[i].SetAutocalibration(GetParameterValue(WB_MSW_CONFIG_PARAMETER_CO2_AUTO))))
                    {
                        break;
                    }

                    if (Channels[i].GetType() == TZWAVEChannel::Type::MOTION) {
                        MotionChannelPtr = &Channels[i];
                    }

                    DEBUG(Channels[i].GetName());
                    DEBUG(" CHANNEL AVAILABLE\n");
                    Channels[i].Enable();
                }
            }
        }

        unknownSensorsLeft = false;
        for (uint8_t i = 0; i < TZWAVEChannel::CHANNEL_TYPES_COUNT; i++) {
            unknownSensorsLeft =
                unknownSensorsLeft || (Channels[i].GetAvailability() == TWBMSWSensor::Availability::UNKNOWN);
        }
    } while ((lastTime - startTime <= timeout) && unknownSensorsLeft);

    uint8_t channelsCount = 0;
    uint8_t channelDeviceNumber = 0;
    size_t groupIndex = CTRL_GROUP_1;
    for (int i = 0; i < TZWAVEChannel::CHANNEL_TYPES_COUNT; i++) {
        if (Channels[i].GetEnabled()) {
            Channels[i].SetChannelNumbers(channelDeviceNumber, channelDeviceNumber + 1, groupIndex);
            channelDeviceNumber++;
            groupIndex++;
            channelsCount++;
        }
    }

    return channelsCount;
}

// Setting up Z-Wave channels, setting Multichannel indexes
void TZWAVESensor::ChannelsSetup()
{

    for (size_t i = 0; i < TZWAVEChannel::CHANNEL_TYPES_COUNT; i++) {
        if (Channels[i].GetEnabled()) {
            switch (Channels[i].GetType()) {
                case TZWAVEChannel::Type::TEMPERATURE:
                    zunoAddChannel(
                        ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER,
                        ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE,
                        (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_CELSIUS,
                                                               WB_MSW_INPUT_REG_TEMPERATURE_VALUE_SIZE,
                                                               WB_MSW_INPUT_REG_TEMPERATURE_VALUE_PRECISION)));
                    zunoSetZWChannel(Channels[i].GetDeviceChannelNumber(), Channels[i].GetServerChannelNumber());
                    zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                    break;
                case TZWAVEChannel::Type::HUMIDITY:
                    zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER,
                                   ZUNO_SENSOR_MULTILEVEL_TYPE_RELATIVE_HUMIDITY,
                                   (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_PERCENTAGE_VALUE,
                                                                          WB_MSW_INPUT_REG_HUMIDITY_VALUE_SIZE,
                                                                          WB_MSW_INPUT_REG_HUMIDITY_VALUE_PRECISION)));
                    zunoSetZWChannel(Channels[i].GetDeviceChannelNumber(), Channels[i].GetServerChannelNumber());
                    zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                    break;
                case TZWAVEChannel::Type::LUMEN:
                    zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER,
                                   ZUNO_SENSOR_MULTILEVEL_TYPE_LUMINANCE,
                                   (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_LUX,
                                                                          WB_MSW_INPUT_REG_LUMEN_VALUE_SIZE,
                                                                          WB_MSW_INPUT_REG_LUMEN_VALUE_PRECISION)));
                    zunoSetZWChannel(Channels[i].GetDeviceChannelNumber(), Channels[i].GetServerChannelNumber());
                    zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                    break;
                case TZWAVEChannel::Type::CO2:
                    zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER,
                                   ZUNO_SENSOR_MULTILEVEL_TYPE_CO2_LEVEL,
                                   (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_PARTS_PER_MILLION,
                                                                          WB_MSW_INPUT_REG_CO2_VALUE_SIZE,
                                                                          WB_MSW_INPUT_REG_CO2_VALUE_PRECISION)));
                    zunoSetZWChannel(Channels[i].GetDeviceChannelNumber(), Channels[i].GetServerChannelNumber());
                    zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                    break;
                case TZWAVEChannel::Type::VOC:
                    zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER,
                                   ZUNO_SENSOR_MULTILEVEL_TYPE_VOLATILE_ORGANIC_COMPOUND,
                                   (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_PARTS_PER_MILLION,
                                                                          WB_MSW_INPUT_REG_VOC_VALUE_SIZE,
                                                                          WB_MSW_INPUT_REG_VOC_VALUE_PRECISION)));
                    zunoSetZWChannel(Channels[i].GetDeviceChannelNumber(), Channels[i].GetServerChannelNumber());
                    zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                    break;
                case TZWAVEChannel::Type::NOISE_LEVEL:
                    zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER,
                                   ZUNO_SENSOR_MULTILEVEL_TYPE_LOUDNESS,
                                   (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_DECIBELS,
                                                                          WB_MSW_INPUT_REG_NOISE_LEVEL_VALUE_SIZE,
                                                                          WB_MSW_INPUT_REG_NOISE_LEVEL_PRECISION)));
                    zunoSetZWChannel(Channels[i].GetDeviceChannelNumber(), Channels[i].GetServerChannelNumber());
                    zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                    break;
                case TZWAVEChannel::Type::MOTION:
                    zunoAddChannel(ZUNO_SENSOR_BINARY_CHANNEL_NUMBER, ZUNO_SENSOR_BINARY_TYPE_MOTION, 0);
                    zunoSetZWChannel(Channels[i].GetDeviceChannelNumber(), Channels[i].GetServerChannelNumber());
                    zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                    break;
                default:
                    break;
            }
        }
    }
    if (ZUNO_CFG_CHANNEL_COUNT > 1) {
        zunoSetZWChannel(0, 1 | ZWAVE_CHANNEL_MAPPED_BIT);
    }
}

// Setting up handlers for all sensor cannels. Handler is used when requesting channel data from controller
// Function binds i-channel directly to value
void TZWAVESensor::SetChannelHandlers()
{
    for (size_t i = 0; i < TZWAVEChannel::CHANNEL_TYPES_COUNT; i++) {
        if (Channels[i].GetEnabled()) {
            uint8_t dataSize;
            switch (Channels[i].GetType()) {
                case TZWAVEChannel::Type::LUMEN:
                    dataSize = SENSOR_MULTILEVEL_SIZE_FOUR_BYTES;
                    break;

                case TZWAVEChannel::Type::MOTION:
                    dataSize = SENSOR_MULTILEVEL_SIZE_ONE_BYTE;
                    break;

                default:
                    dataSize = SENSOR_MULTILEVEL_SIZE_TWO_BYTES;
                    break;
            }
            zunoAppendChannelHandler(Channels[i].GetDeviceChannelNumber(),
                                     dataSize,
                                     CHANNEL_HANDLER_SINGLE_VALUEMAPPER,
                                     Channels[i].GetValuePointer());
        }
    }
}

// Sets by return value name of group by its index
const char* TZWAVESensor::GetGroupNameByIndex(uint8_t groupIndex)
{
    for (uint8_t i = 0; i < TZWAVEChannel::CHANNEL_TYPES_COUNT; i++) {
        if (Channels[i].GetEnabled()) {
            if (Channels[i].GetGroupIndex() == groupIndex) {
                switch (Channels[i].GetType()) {
                    case TZWAVEChannel::Type::TEMPERATURE:
                        return "Temperature Basic On/Off";
                    case TZWAVEChannel::Type::HUMIDITY:
                        return "Humidity Basic On/Off";
                    case TZWAVEChannel::Type::LUMEN:
                        return "Lumen Basic On/Off";
                    case TZWAVEChannel::Type::CO2:
                        return "CO2 Basic On/Off";
                    case TZWAVEChannel::Type::VOC:
                        return "VOC Basic On/Off";
                    case TZWAVEChannel::Type::NOISE_LEVEL:
                        return "Noise level Basic On/Off";
                    case TZWAVEChannel::Type::MOTION:
                        return "Motion Basic On/Off";
                    default:
                        return NULL;
                }
            }
        }
    }
    return NULL;
}

void TZWAVESensor::ParametersInitialize()
{
    // Load configuration parameters from FLASH memory
    for (size_t i = 0; i < WB_MSW_MAX_CONFIG_PARAM; i++) {
        ParameterValues[i] = zunoLoadCFGParam(i + WB_MSW_CONFIG_PARAMETER_FIRST);
        DEBUG("Parameter ");
        DEBUG(i);
        DEBUG(" value ");
        DEBUG(ParameterValues[i]);
        DEBUG("\n");
    }
}

// The handler is called when a configuration parameter value updated from Z-Wave controller
void TZWAVESensor::SetParameterValue(size_t paramNumber, int32_t value)
{
    // DEBUG("Set parameter ");
    // DEBUG(paramNumber);
    // DEBUG(" with value ");
    // DEBUG(value);
    // DEBUG("\n");
    if (paramNumber < WB_MSW_CONFIG_PARAMETER_FIRST || paramNumber > WB_MSW_CONFIG_PARAMETER_LAST) {
        DEBUG("***ERROR Wrong parameter number!\n");
        return;
    }
    ParameterValues[paramNumber - WB_MSW_CONFIG_PARAMETER_FIRST] = value;
}

int32_t TZWAVESensor::GetParameterValue(size_t paramNumber)
{
    // DEBUG("Get parameter ");
    // DEBUG(paramNumber);
    if (paramNumber < WB_MSW_CONFIG_PARAMETER_FIRST || paramNumber > WB_MSW_CONFIG_PARAMETER_LAST) {
        DEBUG("***ERROR Wrong parameter value number!\n");
        return 0;
    }
    // DEBUG(" value ");
    // DEBUG(ParameterValues[paramNumber - WB_MSW_CONFIG_PARAMETER_FIRST]);
    // DEBUG("\n");
    return ParameterValues[paramNumber - WB_MSW_CONFIG_PARAMETER_FIRST];
}

ZunoCFGParameter_t* TZWAVESensor::GetParameterByNumber(size_t paramNumber)
{
    // DEBUG("Get parameter ");
    // DEBUG(paramNumber);
    // DEBUG("\n");
    if (paramNumber < WB_MSW_CONFIG_PARAMETER_FIRST || paramNumber > WB_MSW_CONFIG_PARAMETER_LAST) {
        DEBUG("***ERROR Wrong parameter number \n");
        return NULL;
    }
    return &Parameters[paramNumber - WB_MSW_CONFIG_PARAMETER_FIRST];
}

// Finds channel of needed type
TZWAVEChannel* TZWAVESensor::GetChannelByType(TZWAVEChannel::Type type)
{
    for (size_t i = 0; i < TZWAVEChannel::CHANNEL_TYPES_COUNT; i++) {
        if (Channels[i].GetType() == type) {
            return (&Channels[i]);
        }
    }
    return NULL;
}

// Returns parameter's description if corresponding channel exists
const ZunoCFGParameter_t* TZWAVESensor::GetParameterIfChannelExists(size_t paramNumber)
{
    switch (paramNumber) {
		case WB_MSW_CONFIG_PARAMETER_TEMPERATURE_REPORT_THRESHOLD:
		case WB_MSW_CONFIG_PARAMETER_TEMPERATURE_LEVEL_SEND_BASIC:
		case WB_MSW_CONFIG_PARAMETER_TEMPERATURE_HYSTERESIS_SEND_BASIC:
		case WB_MSW_CONFIG_PARAMETER_TEMPERATURE_ON_COMMANDS:
		case WB_MSW_CONFIG_PARAMETER_TEMPERATURE_OFF_COMMANDS:
		case WB_MSW_CONFIG_PARAMETER_TEMPERATURE_ON_OFF_COMMANDS_RULE:
            if (!GetChannelByType(TZWAVEChannel::Type::TEMPERATURE)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
		case WB_MSW_CONFIG_PARAMETER_HUMIDITY_REPORT_THRESHOLD:
		case WB_MSW_CONFIG_PARAMETER_HUMIDITY_LEVEL_SEND_BASIC:
		case WB_MSW_CONFIG_PARAMETER_HUMIDITY_HYSTERESIS_SEND_BASIC:
		case WB_MSW_CONFIG_PARAMETER_HUMIDITY_ON_COMMANDS:
		case WB_MSW_CONFIG_PARAMETER_HUMIDITY_OFF_COMMANDS:
		case WB_MSW_CONFIG_PARAMETER_HUMIDITY_ON_OFF_COMMANDS_RULE:
            if (!GetChannelByType(TZWAVEChannel::Type ::HUMIDITY)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
		case WB_MSW_CONFIG_PARAMETER_LUMEN_REPORT_THRESHOLD:
		case WB_MSW_CONFIG_PARAMETER_LUMEN_LEVEL_SEND_BASIC:
		case WB_MSW_CONFIG_PARAMETER_LUMEN_HYSTERESIS_SEND_BASIC:
		case WB_MSW_CONFIG_PARAMETER_LUMEN_ON_COMMANDS:
		case WB_MSW_CONFIG_PARAMETER_LUMEN_OFF_COMMANDS:
		case WB_MSW_CONFIG_PARAMETER_LUMEN_ON_OFF_COMMANDS_RULE:
            if (!GetChannelByType(TZWAVEChannel::Type::LUMEN)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
		case WB_MSW_CONFIG_PARAMETER_CO2_REPORT_THRESHOLD:
		case WB_MSW_CONFIG_PARAMETER_CO2_LEVEL_SEND_BASIC:
		case WB_MSW_CONFIG_PARAMETER_CO2_HYSTERESIS_SEND_BASIC:
		case WB_MSW_CONFIG_PARAMETER_CO2_ON_COMMANDS:
		case WB_MSW_CONFIG_PARAMETER_CO2_OFF_COMMANDS:
		case WB_MSW_CONFIG_PARAMETER_CO2_ON_OFF_COMMANDS_RULE:
		case WB_MSW_CONFIG_PARAMETER_CO2_AUTO:
            if (!GetChannelByType(TZWAVEChannel::Type::CO2)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
		case WB_MSW_CONFIG_PARAMETER_VOC_REPORT_THRESHOLD:
		case WB_MSW_CONFIG_PARAMETER_VOC_LEVEL_SEND_BASIC:
		case WB_MSW_CONFIG_PARAMETER_VOC_HYSTERESIS_SEND_BASIC:
		case WB_MSW_CONFIG_PARAMETER_VOC_ON_COMMANDS:
		case WB_MSW_CONFIG_PARAMETER_VOC_OFF_COMMANDS:
		case WB_MSW_CONFIG_PARAMETER_VOC_ON_OFF_COMMANDS_RULE:
            if (!GetChannelByType(TZWAVEChannel::Type::VOC)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
		case WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_REPORT_THRESHOLD:
		case WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_LEVEL_SEND_BASIC:
		case WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_HYSTERESIS_SEND_BASIC:
		case WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_ON_COMMANDS:
		case WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_OFF_COMMANDS:
		case WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_ON_OFF_COMMANDS_RULE:
            if (!GetChannelByType(TZWAVEChannel::Type::NOISE_LEVEL)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
		case WB_MSW_CONFIG_PARAMETER_MOTION_DELAY_SEND_OFF_COMMANDS:
		case WB_MSW_CONFIG_PARAMETER_MOTION_ON_COMMANDS:
		case WB_MSW_CONFIG_PARAMETER_MOTION_OFF_COMMANDS:
		case WB_MSW_CONFIG_PARAMETER_MOTION_ON_OFF_COMMANDS_RULE:
            if (!GetChannelByType(TZWAVEChannel::Type::MOTION)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
        default:
            return (ZUNO_CFG_PARAMETER_UNKNOWN);
            break;
    }
    return GetParameterByNumber(paramNumber);
}

void TZWAVESensor::PublishAnalogSensorValue(TZWAVEChannel& channel,
                                            int64_t value,
											int32_t Report_ThresHold,
											int32_t Level_Send_Basic,
											int32_t Hysteresis_Basic,
											int32_t On_Commands,
											int32_t Off_Commands,
											int32_t On_Off_Commands_Rule)
{
	uint16_t								multiple;
	uint8_t									group_index;
	bool									triggered;

	multiple = channel.GetMultiple();
	Report_ThresHold = Report_ThresHold * multiple;
	// Send value without condition if channels value uninitialized on server
	if ((channel.GetState() == TZWAVEChannel::State::UNINITIALIZED) ||
		((Report_ThresHold != 0) && (abs(value - channel.GetReportedValue()) > Report_ThresHold)))
	{
		// DEBUG("Channel ");
		// DEBUG(channel.GetType());
		// DEBUG(" send report value ");
		// DEBUG(value);
		// DEBUG("\n");
		channel.SetReportedValue(value); // Remember last sent value
		zunoSendReport(channel.GetServerChannelNumber());
	}
	Level_Send_Basic = Level_Send_Basic * multiple;
	Hysteresis_Basic = Hysteresis_Basic * multiple;
	group_index = channel.GetGroupIndex();
	triggered = channel.GetTriggered();
	switch (On_Off_Commands_Rule) {
		case 0x1:
			if ((Level_Send_Basic + Hysteresis_Basic) >= value && triggered == false) {
				channel.SetTriggered(true);
				zunoSendToGroupSetValueCommand(group_index, On_Commands);
			}
			else if ((Level_Send_Basic - Hysteresis_Basic) <= value && triggered == true) {
				channel.SetTriggered(false);
				zunoSendToGroupSetValueCommand(group_index, Off_Commands);
			}
			break ;
		case 0x2:
			if ((Level_Send_Basic + Hysteresis_Basic) >= value && triggered == false) {
				channel.SetTriggered(true);
				zunoSendToGroupSetValueCommand(group_index, On_Commands);
			}
			else if ((Level_Send_Basic - Hysteresis_Basic) <= value && triggered == true) {
				channel.SetTriggered(false);
			}
			break ;
		case 0x3:
			if ((Level_Send_Basic + Hysteresis_Basic) >= value && triggered == false) {
				channel.SetTriggered(true);
			}
			else if ((Level_Send_Basic - Hysteresis_Basic) <= value && triggered == true) {
				channel.SetTriggered(false);
				zunoSendToGroupSetValueCommand(group_index, Off_Commands);
			}
			break ;
	}
}

#ifdef LOGGING_DBG
static void _debugMessage(TZWAVEChannel& channel, int64_t currentValue) {
	const char									space[] = "                         ";
	size_t										len;
	String										str1;

	str1 = channel.GetName();
	len = str1.length();
	if (len >= (sizeof(space) - 0x1))
		len = 0x1;
	else
		len =  (sizeof(space) - 0x1) - len;
	LOGGING_UART.print(str1);
	LOGGING_UART.write((uint8_t *)&space[0x0], len);
	if (channel.GetType() == TZWAVEChannel::Type::VOC || channel.GetType() == TZWAVEChannel::Type::CO2 || channel.GetType() == TZWAVEChannel::Type::MOTION)
		LOGGING_UART.fixPrint( currentValue, 0);
	else
		LOGGING_UART.fixPrint( currentValue, 2);
	LOGGING_UART.print("\n");
}
#endif

// Processing of various types of sensors
TZWAVESensor::Result TZWAVESensor::ProcessCommonChannel(TZWAVEChannel& channel)
{
    int64_t currentValue;
	int32_t					Report_ThresHold;
	int32_t					Level_Send_Basic;
	int32_t					Hysteresis_Basic;
	int32_t					On_Commands;
	int32_t					Off_Commands;
	int32_t					On_Off_Commands_Rule;

    if (channel.GetType() == TZWAVEChannel::Type::CO2) {
        // Check if automatic calibration is needed
        uint8_t autocalibration = GetParameterValue(WB_MSW_CONFIG_PARAMETER_CO2_AUTO);
        if ((channel.GetAutocalibration() != autocalibration) && !channel.SetAutocalibration(autocalibration)) {
            return TZWAVESensor::Result::ZWAVE_PROCESS_MODBUS_ERROR;
        }
    }

    if (!channel.ReadValueFromSensor(currentValue)) {
        return TZWAVESensor::Result::ZWAVE_PROCESS_MODBUS_ERROR;
    }
    if ((channel.GetType() != TZWAVEChannel::Type::NOISE_LEVEL) && (currentValue == channel.GetErrorValue())) {
        return TZWAVESensor::Result::ZWAVE_PROCESS_VALUE_ERROR;
    }
	#ifdef LOGGING_DBG
	if (this->_debug_ms > this->_debug_ms_next)
		_debugMessage(channel, currentValue);
	#endif
    channel.SetValue(currentValue);
	Report_ThresHold = GetParameterValue(channel.GetReportThresHoldParameterNumber());
	Level_Send_Basic = GetParameterValue(channel.GetLevelSendBasicParameterNumber());
	Hysteresis_Basic = GetParameterValue(channel.GetHysteresisBasicParameterNumber());
	On_Commands = GetParameterValue(channel.GetOnCommandsParameterNumber());
	Off_Commands = GetParameterValue(channel.GetOffCommandsParameterNumber());
	On_Off_Commands_Rule = GetParameterValue(channel.GetOnOffCommandsRuleParameterNumber());
    PublishAnalogSensorValue(channel, currentValue, Report_ThresHold, Level_Send_Basic, Hysteresis_Basic, On_Commands, Off_Commands, On_Off_Commands_Rule);
    return TZWAVESensor::Result::ZWAVE_PROCESS_OK;
}

TZWAVESensor::Result TZWAVESensor::ProcessMotionChannel(TZWAVEChannel& channel)
{
	int64_t									value;
	int32_t									On_Commands;
	int32_t									Off_Commands;
	int32_t									On_Off_Commands_Rule;
	bool									triggered;
	uint8_t									group_index;
	uint32_t								motionPeriod;
	uint32_t								currentTime;

    if (!channel.ReadValueFromSensor(value)) {
        return TZWAVESensor::Result::ZWAVE_PROCESS_MODBUS_ERROR;
    }
    if (value == WB_MSW_INPUT_REG_MOTION_VALUE_ERROR) {
        // MotionChannelReset(&channel);
        return TZWAVESensor::Result::ZWAVE_PROCESS_VALUE_ERROR;
    }
	#ifdef LOGGING_DBG
	if (this->_debug_ms > this->_debug_ms_next)
		_debugMessage(channel, value);
	#endif
	if ((channel.GetState() == TZWAVEChannel::State::UNINITIALIZED) ||
		((abs(value - channel.GetReportedValue()) > WB_MSW_CONFIG_PARAMETER_MOTION_HYSTERESIS)))
	{
		// DEBUG("Channel ");
		// DEBUG(channel.GetType());
		// DEBUG(" send report value ");
		// DEBUG(value);
		// DEBUG("\n");
		channel.SetReportedValue(value); // Remember last sent value
		zunoSendReport(channel.GetServerChannelNumber());
	}
	On_Commands = GetParameterValue(channel.GetOnCommandsParameterNumber());
	On_Off_Commands_Rule = GetParameterValue(channel.GetOnOffCommandsRuleParameterNumber());
	triggered = channel.GetTriggered();
	group_index =channel.GetGroupIndex();
	currentTime = millis();
	switch (On_Off_Commands_Rule) {
		case 0x1:
			if ((WB_MSW_CONFIG_PARAMETER_MOTION_ON) >= value && triggered == false) {
				channel.SetTriggered(true);
				zunoSendToGroupSetValueCommand(group_index, On_Commands);
			}
			else if ((WB_MSW_CONFIG_PARAMETER_MOTION_OFF) <= value && triggered == true) {
				channel.SetTriggered(false);
				this->MotionLastTime = currentTime;
				this->_MotionLastTime_wait_Off = true;
			}
			break ;
		case 0x2:
			if ((WB_MSW_CONFIG_PARAMETER_MOTION_ON) >= value && triggered == false) {
				channel.SetTriggered(true);
				zunoSendToGroupSetValueCommand(group_index, On_Commands);
			}
			else if ((WB_MSW_CONFIG_PARAMETER_MOTION_OFF) <= value && triggered == true) {
				channel.SetTriggered(false);
			}
			break ;
		case 0x3:
			if ((WB_MSW_CONFIG_PARAMETER_MOTION_ON) >= value && triggered == false) {
				channel.SetTriggered(true);
			}
			else if ((WB_MSW_CONFIG_PARAMETER_MOTION_OFF) <= value && triggered == true) {
				channel.SetTriggered(false);
				this->MotionLastTime = currentTime;
				this->_MotionLastTime_wait_Off = true;
			}
			break ;
	}
	switch (On_Off_Commands_Rule) {
		case 0x1:
		case 0x3:
			if (this->_MotionLastTime_wait_Off == true)
			{
				motionPeriod = (uint32_t)GetParameterValue(WB_MSW_CONFIG_PARAMETER_MOTION_DELAY_SEND_OFF_COMMANDS) * 1000;
				if ((this->MotionLastTime + motionPeriod) <= currentTime)
				{
					this->_MotionLastTime_wait_Off = false;
					Off_Commands = GetParameterValue(channel.GetOffCommandsParameterNumber());
					zunoSendToGroupSetValueCommand(group_index, Off_Commands);
				}
			}
			break ;
	}
	return (TZWAVESensor::Result::ZWAVE_PROCESS_OK);
}

// void TZWAVESensor::MotionChannelReset(TZWAVEChannel* channel)
// {
//     if (channel) {
//         bool inverting = GetParameterValue(WB_MSW_CONFIG_PARAMETER_MOTION_INVERT);

//         channel->SetValue(false);
//         channel->SetReportedValue(false);
//         zunoSendReport(channel->GetServerChannelNumber());
//         zunoSendToGroupSetValueCommand(channel->GetGroupIndex(), (!inverting) ? WB_MSW_OFF : WB_MSW_ON);
//     }
// }

// Device channel management and firmware data transfer
TZWAVESensor::Result TZWAVESensor::ProcessChannels()
{
    #ifdef LOGGING_DBG
    this->_debug_ms = millis();
    if (this->_debug_ms > this->_debug_ms_next)
        DEBUG("--------------------Measurements-----------------------\n");
    #endif
    // Check all channels of available sensors
    TZWAVESensor::Result result;
    for (size_t i = 0; i < TZWAVEChannel::CHANNEL_TYPES_COUNT; i++) {
        if (Channels[i].GetEnabled()) {
            switch (Channels[i].GetType()) {
                case TZWAVEChannel::Type::MOTION:
                    result = ProcessMotionChannel(Channels[i]);
                    break;
                default:
                    result = ProcessCommonChannel(Channels[i]);
                    break;
            }
        }
        if (result == TZWAVESensor::Result::ZWAVE_PROCESS_MODBUS_ERROR) {
            // MotionChannelReset(MotionChannelPtr);
            #ifdef LOGGING_DBG
            if (this->_debug_ms > this->_debug_ms_next)
                this->_debug_ms_next = this->_debug_ms + this->_debug_ms_step;
            #endif
            return result;
        }
    }
    #ifdef LOGGING_DBG
    if (this->_debug_ms > this->_debug_ms_next)
        this->_debug_ms_next = this->_debug_ms + this->_debug_ms_step;
    #endif
    return TZWAVESensor::Result::ZWAVE_PROCESS_OK;
}