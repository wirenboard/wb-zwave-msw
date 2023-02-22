#include "TZWAVESensor.h"
#include "Arduino.h"
#include "DebugOutput.h"
#include "WbMsw.h"
#include <string.h>

#define WB_MSW_CONFIG_PARAMETER_TEMPERATURE_MULTIPLIER 1
#define WB_MSW_CONFIG_PARAMETER_HUMIDITY_MULTIPLIER 100
#define WB_MSW_CONFIG_PARAMETER_LUMEN_MULTIPLIER 100
#define WB_MSW_CONFIG_PARAMETER_CO2_MULTIPLIER 1
#define WB_MSW_CONFIG_PARAMETER_VOC_MULTIPLIER 1
#define WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_MULTIPLIER 100
#define WB_MSW_CONFIG_PARAMETER_MOTION_MULTIPLIER 1
#define WB_MSW_CONFIG_PARAMETER_MOTION_ON 300
#define WB_MSW_CONFIG_PARAMETER_MOTION_OFF 250
#define WB_MSW_CONFIG_PARAMETER_CO2_AUTO_VALUE true

TZWAVESensor::TZWAVESensor(TWBMSWSensor* wbMsw): WbMsw(wbMsw)
{
    // Available device parameters description
    // Channels in the device are created dynamically, so parameters are described in "dynamic" style
    ZunoCFGParameter_t parameters[WB_MSW_MAX_CONFIG_PARAM] = {

        // Motion sensor settings
        ZUNO_CONFIG_PARAMETER_INFO("Motion delay to send OFF command", "Value in seconds.", 0, 100000, 60),
        ZUNO_CONFIG_PARAMETER_INFO("Motion ON command", "Send Basic Set command.", 0, 255, 255),
        ZUNO_CONFIG_PARAMETER_INFO("Motion OFF command", "Send Basic Set command.", 0, 255, 0),
        ZUNO_CONFIG_PARAMETER_INFO(
            "Motion ON/OFF commands rules",
            "1 - Send ON if the motion is detected. Send OFF if the motion is idle. 2 - Send ON if the motion is "
            "detected. Do not send OFF. 3 - Do not send ON. Send OFF if the motion is idle.",
            1,
            3,
            1),

        // Temperature channel settings
        ZUNO_CONFIG_PARAMETER_INFO("Temperature Report Threshold",
                                   "0 - Reports disabled. Send Report if the temperature has changed after the last "
                                   "report. Value in 0.01C (100 = 1C).",
                                   0,
                                   4000,
                                   100),
        ZUNO_CONFIG_PARAMETER_INFO_SIGN("Temperature Level to send Basic Set",
                                        "Send Basic Set if the temperature has crossed the level up or down + "
                                        "hysteresis. Value in 0.01C (100 = 1C).",
                                        -4000,
                                        8000,
                                        4000),
        ZUNO_CONFIG_PARAMETER_INFO("Temperature Hysteresis to send Basic Set",
                                   "Value in 0.01C (100 = 1C).",
                                   10,
                                   2000,
                                   50),
        ZUNO_CONFIG_PARAMETER_INFO("Temperature ON command", "Send Basic Set command.", 0, 255, 255),
        ZUNO_CONFIG_PARAMETER_INFO("Temperature OFF command", "Send Basic Set command.", 0, 255, 0),
        ZUNO_CONFIG_PARAMETER_INFO("Temperature ON/OFF commands rules",
                                   "1 - Send ON if the temperature is greater than Level. Send OFF if the temperature "
                                   "is less than Level. 2 - Send ON if the temperature is greater than Level. Do not "
                                   "send OFF. 3 - Do not send ON. Send OFF if the temperature is less than Level.",
                                   1,
                                   3,
                                   1),

        // Humidity sensor settings
        ZUNO_CONFIG_PARAMETER_INFO(
            "Humidity Report Threshold",
            "0 - Reports disabled. Send Report if the humidity has changed after the last report. Value in %.",
            0,
            90,
            5),
        ZUNO_CONFIG_PARAMETER_INFO(
            "Humidity Level to send Basic Set",
            "Send Basic Set if the humidity has crossed the level up or down + hysteresis. Value in %.",
            5,
            95,
            60),
        ZUNO_CONFIG_PARAMETER_INFO("Humidity Hysteresis to send Basic Set", "Value in %.", 1, 50, 2),
        ZUNO_CONFIG_PARAMETER_INFO("Humidity ON command", "Send Basic Set command.", 0, 255, 255),
        ZUNO_CONFIG_PARAMETER_INFO("Humidity OFF command", "Send Basic Set command.", 0, 255, 0),
        ZUNO_CONFIG_PARAMETER_INFO("Humidity ON/OFF commands rules",
                                   "1 - Send ON if the humidity is greater than Level. Send OFF if the humidity is "
                                   "less than Level. 2 - Send ON if the humidity is greater than Level. Do not send "
                                   "OFF. 3 - Do not send ON. Send OFF if the humidity is less than Level.",
                                   1,
                                   3,
                                   1),

        // Lumen sensor settings
        ZUNO_CONFIG_PARAMETER_INFO(
            "Luminance Report Threshold",
            "0 - Reports disabled. Send Report if the luminance has changed after the last report. Value in lux.",
            0,
            100000,
            100),
        ZUNO_CONFIG_PARAMETER_INFO(
            "Luminance Level to send Basic Set",
            "Send Basic Set if the luminance has crossed the level up or down + hysteresis. Value in lux.",
            0,
            100000,
            200),
        ZUNO_CONFIG_PARAMETER_INFO("Luminance Hysteresis to send Basic Set", "Value in lux.", 1, 50000, 50),
        ZUNO_CONFIG_PARAMETER_INFO("Luminance ON command", "Send Basic Set command.", 0, 255, 255),
        ZUNO_CONFIG_PARAMETER_INFO("Luminance OFF command", "Send Basic Set command.", 0, 255, 0),
        ZUNO_CONFIG_PARAMETER_INFO("Luminance ON/OFF commands rules",
                                   "1 - Send ON if the luminance is greater than Level. Send OFF if the luminance is "
                                   "less than Level. 2 - Send ON if the luminance is greater than Level. Do not send "
                                   "OFF. 3 - Do not send ON. Send OFF if the luminance is less than Level.",
                                   1,
                                   3,
                                   1),

        // CO2 sensor settings
        ZUNO_CONFIG_PARAMETER_INFO(
            "CO2 Report Threshold",
            "0 - Reports disabled. Send Report if the CO2 has changed after the last report. Value in ppm.",
            0,
            5000,
            100),
        ZUNO_CONFIG_PARAMETER_INFO(
            "CO2 Level to send Basic Set",
            "Send Basic Set if the CO2 has crossed the level up or down + hysteresis. Value in ppm.",
            400,
            5000,
            1500),
        ZUNO_CONFIG_PARAMETER_INFO("CO2 Hysteresis to send Basic Set", "Value in ppm.", 1, 2500, 50),
        ZUNO_CONFIG_PARAMETER_INFO("CO2 ON command", "Send Basic Set command.", 0, 255, 255),
        ZUNO_CONFIG_PARAMETER_INFO("CO2 OFF command", "Send Basic Set command.", 0, 255, 0),
        ZUNO_CONFIG_PARAMETER_INFO(
            "CO2 ON/OFF commands rules",
            "1 - Send ON if the CO2 is greater than Level. Send OFF if the CO2 is less than Level. 2 - Send ON if the "
            "CO2 is greater than Level. Do not send OFF. 3 - Do not send ON. Send OFF if the CO2 is less than Level.",
            1,
            3,
            1),

        // VOC sensor settings
        ZUNO_CONFIG_PARAMETER_INFO(
            "VOC Report Threshold",
            "0 - Reports disabled. Send Report if the VOC has changed after the last report. Value in ppb.",
            0,
            60000,
            200),
        ZUNO_CONFIG_PARAMETER_INFO(
            "VOC Level to send Basic Set",
            "Send Basic Set if the VOC has crossed the level up or down + hysteresis. Value in ppb.",
            0,
            60000,
            660),
        ZUNO_CONFIG_PARAMETER_INFO("VOC Hysteresis to send Basic Set", "Value in ppb.", 1, 30000, 200),
        ZUNO_CONFIG_PARAMETER_INFO("VOC ON command", "Send Basic Set command.", 0, 255, 255),
        ZUNO_CONFIG_PARAMETER_INFO("VOC OFF command", "Send Basic Set command.", 0, 255, 0),
        ZUNO_CONFIG_PARAMETER_INFO(
            "VOC ON/OFF commands rules",
            "1 - Send ON if the VOC is greater than Level. Send OFF if the VOC is less than Level. 2 - Send ON if the "
            "VOC is greater than Level. Do not send OFF. 3 - Do not send ON. Send OFF if the VOC is less than Level.",
            1,
            3,
            1),

        // Noise level sensor settings
        ZUNO_CONFIG_PARAMETER_INFO(
            "Noise Report Threshold",
            "0 - Reports disabled. Send Report if the noise has changed after the last report. Value in dB.",
            0,
            105,
            10),
        ZUNO_CONFIG_PARAMETER_INFO(
            "Noise Level to send Basic Set",
            "Send Basic Set if the noise has crossed the level up or down + hysteresis. Value in dB.",
            38,
            105,
            80),
        ZUNO_CONFIG_PARAMETER_INFO("Noise Hysteresis to send Basic Set", "Value in dB.", 1, 50, 5),
        ZUNO_CONFIG_PARAMETER_INFO("Noise ON command", "Send Basic Set command.", 0, 255, 255),
        ZUNO_CONFIG_PARAMETER_INFO("Noise OFF command", "Send Basic Set command.", 0, 255, 0),
        ZUNO_CONFIG_PARAMETER_INFO("Noise ON/OFF commands rules",
                                   "1 - Send ON if the noise is greater than Level. Send OFF if the noise is less than "
                                   "Level. 2 - Send ON if the noise is greater than Level. Do not send OFF. 3 - Do not "
                                   "send ON. Send OFF if the noise is less than Level.",
                                   1,
                                   3,
                                   1),

        // Intrusion sensor settings
        ZUNO_CONFIG_PARAMETER_INFO("Intrusion Noise Level",
                                   " Send Alarm if the noise more level. Value in dB.",
                                   38,
                                   105,
                                   80),
        ZUNO_CONFIG_PARAMETER_INFO("Intrusion delay to send OFF command", "Value in seconds.", 0, 100000, 5)

    };
    memcpy(Parameters, parameters, sizeof(parameters));
    MotionLastTimeWaitOff = false;
    IntrusionLastTimeWaitOff = false;
}

// Function determines number of available Z-Wave device channels (EndPoints) and fills in the structures by channel
// type
bool TZWAVESensor::ChannelsInitialize()
{
    uint32_t startTime = millis();
    uint32_t lastTime = startTime;
    uint32_t timeout = WB_MSW_INPUT_REG_AVAILABILITY_TIMEOUT_MS;

    Channels[0].ChannelInitialize("Motion",
                                  TZWAVEChannel::Type::MOTION,
                                  WB_MSW_INPUT_REG_MOTION_VALUE_ERROR,
                                  0,
                                  0,
                                  0,
                                  WB_MSW_CONFIG_PARAMETER_MOTION_ON_COMMANDS,
                                  WB_MSW_CONFIG_PARAMETER_MOTION_OFF_COMMANDS,
                                  WB_MSW_CONFIG_PARAMETER_MOTION_ON_OFF_COMMANDS_RULE,
                                  WB_MSW_CONFIG_PARAMETER_MOTION_MULTIPLIER,
                                  WbMsw,
                                  &TWBMSWSensor::GetMotion,
                                  &TWBMSWSensor::GetMotionAvailability);

    Channels[1].ChannelInitialize("Intrusion",
                                  TZWAVEChannel::Type::INTRUSION,
                                  0,
                                  WB_MSW_CONFIG_PARAMETER_INTRUSION_REPORT_THRESHOLD,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_MULTIPLIER,
                                  WbMsw,
                                  &TWBMSWSensor::GetNoiseLevel,
                                  &TWBMSWSensor::GetNoiseLevelAvailability);

    Channels[2].ChannelInitialize("Temperature",
                                  TZWAVEChannel::Type::TEMPERATURE,
                                  WB_MSW_INPUT_REG_TEMPERATURE_VALUE_ERROR,
                                  WB_MSW_CONFIG_PARAMETER_TEMPERATURE_REPORT_THRESHOLD,
                                  WB_MSW_CONFIG_PARAMETER_TEMPERATURE_LEVEL_SEND_BASIC,
                                  WB_MSW_CONFIG_PARAMETER_TEMPERATURE_HYSTERESIS_SEND_BASIC,
                                  WB_MSW_CONFIG_PARAMETER_TEMPERATURE_ON_COMMANDS,
                                  WB_MSW_CONFIG_PARAMETER_TEMPERATURE_OFF_COMMANDS,
                                  WB_MSW_CONFIG_PARAMETER_TEMPERATURE_ON_OFF_COMMANDS_RULE,
                                  WB_MSW_CONFIG_PARAMETER_TEMPERATURE_MULTIPLIER,
                                  WbMsw,
                                  &TWBMSWSensor::GetTemperature,
                                  &TWBMSWSensor::GetTemperatureAvailability);

    Channels[3].ChannelInitialize("Humidity",
                                  TZWAVEChannel::Type::HUMIDITY,
                                  WB_MSW_INPUT_REG_HUMIDITY_VALUE_ERROR,
                                  WB_MSW_CONFIG_PARAMETER_HUMIDITY_REPORT_THRESHOLD,
                                  WB_MSW_CONFIG_PARAMETER_HUMIDITY_LEVEL_SEND_BASIC,
                                  WB_MSW_CONFIG_PARAMETER_HUMIDITY_HYSTERESIS_SEND_BASIC,
                                  WB_MSW_CONFIG_PARAMETER_HUMIDITY_ON_COMMANDS,
                                  WB_MSW_CONFIG_PARAMETER_HUMIDITY_OFF_COMMANDS,
                                  WB_MSW_CONFIG_PARAMETER_HUMIDITY_ON_OFF_COMMANDS_RULE,
                                  WB_MSW_CONFIG_PARAMETER_HUMIDITY_MULTIPLIER,
                                  WbMsw,
                                  &TWBMSWSensor::GetHumidity,
                                  &TWBMSWSensor::GetHumidityAvailability);

    Channels[4].ChannelInitialize("Luminance",
                                  TZWAVEChannel::Type::LUMEN,
                                  WB_MSW_INPUT_REG_LUMEN_VALUE_ERROR,
                                  WB_MSW_CONFIG_PARAMETER_LUMEN_REPORT_THRESHOLD,
                                  WB_MSW_CONFIG_PARAMETER_LUMEN_LEVEL_SEND_BASIC,
                                  WB_MSW_CONFIG_PARAMETER_LUMEN_HYSTERESIS_SEND_BASIC,
                                  WB_MSW_CONFIG_PARAMETER_LUMEN_ON_COMMANDS,
                                  WB_MSW_CONFIG_PARAMETER_LUMEN_OFF_COMMANDS,
                                  WB_MSW_CONFIG_PARAMETER_LUMEN_ON_OFF_COMMANDS_RULE,
                                  WB_MSW_CONFIG_PARAMETER_LUMEN_MULTIPLIER,
                                  WbMsw,
                                  &TWBMSWSensor::GetLuminance,
                                  &TWBMSWSensor::GetLuminanceAvailability);

    Channels[5].ChannelInitialize("CO2",
                                  TZWAVEChannel::Type::CO2,
                                  WB_MSW_INPUT_REG_CO2_VALUE_ERROR,
                                  WB_MSW_CONFIG_PARAMETER_CO2_REPORT_THRESHOLD,
                                  WB_MSW_CONFIG_PARAMETER_CO2_LEVEL_SEND_BASIC,
                                  WB_MSW_CONFIG_PARAMETER_CO2_HYSTERESIS_SEND_BASIC,
                                  WB_MSW_CONFIG_PARAMETER_CO2_ON_COMMANDS,
                                  WB_MSW_CONFIG_PARAMETER_CO2_OFF_COMMANDS,
                                  WB_MSW_CONFIG_PARAMETER_CO2_ON_OFF_COMMANDS_RULE,
                                  WB_MSW_CONFIG_PARAMETER_CO2_MULTIPLIER,
                                  WbMsw,
                                  &TWBMSWSensor::GetCO2,
                                  &TWBMSWSensor::GetCO2Availability);

    Channels[6].ChannelInitialize("VOC",
                                  TZWAVEChannel::Type::VOC,
                                  WB_MSW_INPUT_REG_VOC_VALUE_ERROR,
                                  WB_MSW_CONFIG_PARAMETER_VOC_REPORT_THRESHOLD,
                                  WB_MSW_CONFIG_PARAMETER_VOC_LEVEL_SEND_BASIC,
                                  WB_MSW_CONFIG_PARAMETER_VOC_HYSTERESIS_SEND_BASIC,
                                  WB_MSW_CONFIG_PARAMETER_VOC_ON_COMMANDS,
                                  WB_MSW_CONFIG_PARAMETER_VOC_OFF_COMMANDS,
                                  WB_MSW_CONFIG_PARAMETER_VOC_ON_OFF_COMMANDS_RULE,
                                  WB_MSW_CONFIG_PARAMETER_VOC_MULTIPLIER,
                                  WbMsw,
                                  &TWBMSWSensor::GetVoc,
                                  &TWBMSWSensor::GetVocAvailability);

    Channels[7].ChannelInitialize("NoiseLevel",
                                  TZWAVEChannel::Type::NOISE_LEVEL,
                                  0,
                                  WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_REPORT_THRESHOLD,
                                  WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_LEVEL_SEND_BASIC,
                                  WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_HYSTERESIS_SEND_BASIC,
                                  WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_ON_COMMANDS,
                                  WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_OFF_COMMANDS,
                                  WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_ON_OFF_COMMANDS_RULE,
                                  WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_MULTIPLIER,
                                  WbMsw,
                                  &TWBMSWSensor::GetNoiseLevel,
                                  &TWBMSWSensor::GetNoiseLevelAvailability);

    bool unknownSensorsLeft = false;

    do {
        lastTime = millis();
        for (int i = 0; i < TZWAVEChannel::CHANNEL_TYPES_COUNT; i++) {
            if (Channels[i].GetAvailability() == TWBMSWSensor::Availability::UNKNOWN) {
                Channels[i].UpdateAvailability();
                if (Channels[i].GetAvailability() == TWBMSWSensor::Availability::AVAILABLE) {

                    if ((Channels[i].GetType() == TZWAVEChannel::Type::CO2) &&
                        (!Channels[i].SetPowerOn() ||
                         !Channels[i].SetAutocalibration(WB_MSW_CONFIG_PARAMETER_CO2_AUTO_VALUE)))
                    {
                        break;
                    }

                    if (Channels[i].GetType() == TZWAVEChannel::Type::MOTION) {
                        MotionChannelPtr = &Channels[i];
                    }

                    if (Channels[i].GetType() == TZWAVEChannel::Type::INTRUSION) {
                        IntrusionChannelPtr = &Channels[i];
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
            if (Channels[i].GetType() == TZWAVEChannel::Type::INTRUSION) {
                Channels[i].SetChannelNumbers(channelDeviceNumber, channelDeviceNumber + 1, 0xFF);
            } else {
                Channels[i].SetChannelNumbers(channelDeviceNumber, channelDeviceNumber + 1, groupIndex);
                groupIndex++;
            }
            channelDeviceNumber++;
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
                case TZWAVEChannel::Type::INTRUSION:
                    zunoAddChannel(ZUNO_SENSOR_BINARY_CHANNEL_NUMBER, ZUNO_SENSOR_BINARY_TYPE_GENERAL_PURPOSE, 0);
                    zunoSetZWChannel(Channels[i].GetDeviceChannelNumber(), Channels[i].GetServerChannelNumber());
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

                case TZWAVEChannel::Type::INTRUSION:
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
        case WB_MSW_CONFIG_PARAMETER_INTRUSION_REPORT_THRESHOLD:
        case WB_MSW_CONFIG_PARAMETER_INTRUSION_DELAY_SEND_OFF_COMMANDS:
            if (!GetChannelByType(TZWAVEChannel::Type::INTRUSION)) {
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
                                            int32_t reportThresHold,
                                            int32_t levelSendBasic,
                                            int32_t hysteresisBasic,
                                            int32_t onCommands,
                                            int32_t offCommands,
                                            int32_t onOffCommandsRule)
{
    uint16_t multiplier;
    uint8_t groupIndex;
    bool triggered;

    multiplier = channel.GetMultiplier();
    reportThresHold = reportThresHold * multiplier;
    // Send value without condition if channels value uninitialized on server
    if ((reportThresHold != 0) && ((channel.GetState() == TZWAVEChannel::State::UNINITIALIZED) ||
                                   (abs(value - channel.GetReportedValue()) > reportThresHold)))
    {
        // DEBUG("Channel ");
        // DEBUG(channel.GetType());
        // DEBUG(" send report value ");
        // DEBUG(value);
        // DEBUG("\n");
        channel.SetReportedValue(value); // Remember last sent value
        zunoSendReport(channel.GetServerChannelNumber());
    }
    levelSendBasic = levelSendBasic * multiplier;
    hysteresisBasic = hysteresisBasic * multiplier;
    groupIndex = channel.GetGroupIndex();
    triggered = channel.GetTriggered();
    if (triggered) {
        if (value <= (levelSendBasic - hysteresisBasic)) {
            channel.SetTriggered(false);
            if (onOffCommandsRule == 1 || onOffCommandsRule == 3) {
                zunoSendToGroupSetValueCommand(groupIndex, offCommands);
            }
        }
    } else {
        if (value >= (levelSendBasic + hysteresisBasic)) {
            channel.SetTriggered(true);
            if (onOffCommandsRule == 1 || onOffCommandsRule == 2) {
                zunoSendToGroupSetValueCommand(groupIndex, onCommands);
            }
        }
    }
}

void TZWAVESensor::PublishIntrusionValue(TZWAVEChannel* channel, int64_t value)
{
    uint32_t intrusionPeriod;
    int64_t reportThresHold;
    uint32_t currentTime;

    reportThresHold = GetParameterValue(channel->GetReportThresHoldParameterNumber()) * channel->GetMultiplier();
    if (channel->GetState() == TZWAVEChannel::State::UNINITIALIZED) {
        channel->SetTriggered(false);
        channel->SetValue(false);
        channel->SetReportedValue(false); // Remember last sent value
        zunoSendReport(channel->GetServerChannelNumber());
    }
    currentTime = millis();
    if (channel->GetTriggered()) {
        if (value < (reportThresHold)) {
            channel->SetTriggered(false);
            IntrusionLastTime = currentTime;
            IntrusionLastTimeWaitOff = true;
        }
    } else {
        if (value > (reportThresHold)) {
            channel->SetTriggered(true);
            if (!channel->GetReportedValue()) {
                channel->SetValue(true);
                channel->SetReportedValue(true); // Remember last sent value
                zunoSendReport(channel->GetServerChannelNumber());
                IntrusionLastTimeWaitOff = false;
            }
        }
    }
    if (IntrusionLastTimeWaitOff) {
        intrusionPeriod = (uint32_t)GetParameterValue(WB_MSW_CONFIG_PARAMETER_INTRUSION_DELAY_SEND_OFF_COMMANDS) * 1000;
        if ((IntrusionLastTime + intrusionPeriod) <= currentTime) {
            IntrusionLastTimeWaitOff = false;
            channel->SetValue(false);
            channel->SetReportedValue(false); // Remember last sent value
            zunoSendReport(channel->GetServerChannelNumber());
        }
    }
}

void TZWAVESensor::PublishMotionValue(TZWAVEChannel* channel, int64_t value)
{
    int32_t onCommands;
    int32_t offCommands;
    int32_t onOffCommandsRule;
    uint8_t groupIndex;
    uint32_t motionPeriod;
    uint32_t currentTime;

    onOffCommandsRule = GetParameterValue(channel->GetOnOffCommandsRuleParameterNumber());
    groupIndex = channel->GetGroupIndex();
    currentTime = millis();
    if (channel->GetTriggered()) {
        if (value <= (WB_MSW_CONFIG_PARAMETER_MOTION_OFF)) {
            channel->SetTriggered(false);
            MotionLastTime = currentTime;
            MotionLastTimeWaitOff = true;
        }
    } else {
        if (value >= (WB_MSW_CONFIG_PARAMETER_MOTION_ON)) {
            channel->SetTriggered(true);
            if (!channel->GetReportedValue()) {
                channel->SetValue(true);
                channel->SetReportedValue(true); // Remember last sent value
                zunoSendReport(channel->GetServerChannelNumber());
                MotionLastTimeWaitOff = false;
                if (onOffCommandsRule == 1 || onOffCommandsRule == 2) {
                    onCommands = GetParameterValue(channel->GetOnCommandsParameterNumber());
                    zunoSendToGroupSetValueCommand(groupIndex, onCommands);
                }
            }
        }
    }
    if (MotionLastTimeWaitOff) {
        motionPeriod = (uint32_t)GetParameterValue(WB_MSW_CONFIG_PARAMETER_MOTION_DELAY_SEND_OFF_COMMANDS) * 1000;
        if ((MotionLastTime + motionPeriod) <= currentTime) {
            MotionLastTimeWaitOff = false;
            channel->SetValue(false);
            channel->SetReportedValue(false); // Remember last sent value
            zunoSendReport(channel->GetServerChannelNumber());
            if (onOffCommandsRule == 1 || onOffCommandsRule == 3) {
                offCommands = GetParameterValue(channel->GetOffCommandsParameterNumber());
                zunoSendToGroupSetValueCommand(groupIndex, offCommands);
            }
        }
    }
}

// Processing of various types of sensors
TZWAVESensor::Result TZWAVESensor::ProcessCommonChannel(TZWAVEChannel& channel)
{
    int64_t currentValue;
    int32_t reportThresHold;
    int32_t levelSendBasic;
    int32_t hysteresisBasic;
    int32_t onCommands;
    int32_t offCommands;
    int32_t onOffCommandsRule;

    if (channel.GetType() == TZWAVEChannel::Type::CO2) {
        // Check if automatic calibration is needed
        uint8_t autocalibration = WB_MSW_CONFIG_PARAMETER_CO2_AUTO_VALUE;
        if ((channel.GetAutocalibration() != autocalibration) && !channel.SetAutocalibration(autocalibration)) {
            return TZWAVESensor::Result::ZWAVE_PROCESS_MODBUS_ERROR;
        }
    }
    if (!channel.ReadValueFromSensor(currentValue)) {
        return TZWAVESensor::Result::ZWAVE_PROCESS_MODBUS_ERROR;
    }
    if ((channel.GetType() != TZWAVEChannel::Type::NOISE_LEVEL)) {
        if (currentValue == channel.GetErrorValue()) {
            return TZWAVESensor::Result::ZWAVE_PROCESS_VALUE_ERROR;
        }
    } else {
        PublishIntrusionValue(IntrusionChannelPtr, currentValue);
    }
    DEBUG(channel.GetName());
    LOG_FIXEDPOINT_VALUE("        ", currentValue, 2);
    channel.SetValue(currentValue);
    reportThresHold = GetParameterValue(channel.GetReportThresHoldParameterNumber());
    levelSendBasic = GetParameterValue(channel.GetLevelSendBasicParameterNumber());
    hysteresisBasic = GetParameterValue(channel.GetHysteresisBasicParameterNumber());
    onCommands = GetParameterValue(channel.GetOnCommandsParameterNumber());
    offCommands = GetParameterValue(channel.GetOffCommandsParameterNumber());
    onOffCommandsRule = GetParameterValue(channel.GetOnOffCommandsRuleParameterNumber());
    PublishAnalogSensorValue(channel,
                             currentValue,
                             reportThresHold,
                             levelSendBasic,
                             hysteresisBasic,
                             onCommands,
                             offCommands,
                             onOffCommandsRule);
    return TZWAVESensor::Result::ZWAVE_PROCESS_OK;
}

TZWAVESensor::Result TZWAVESensor::ProcessMotionChannel(TZWAVEChannel& channel)
{
    int64_t value;

    if (!channel.ReadValueFromSensor(value)) {
        return TZWAVESensor::Result::ZWAVE_PROCESS_MODBUS_ERROR;
    }
    if (value == WB_MSW_INPUT_REG_MOTION_VALUE_ERROR) {
        MotionChannelReset(&channel);
        return TZWAVESensor::Result::ZWAVE_PROCESS_VALUE_ERROR;
    }
    LOG_INT_VALUE("Motion:             ", (long)value);
    if ((channel.GetState() == TZWAVEChannel::State::UNINITIALIZED)) {
        // DEBUG("Channel ");
        // DEBUG(channel.GetType());
        // DEBUG(" send report value ");
        // DEBUG(value);
        // DEBUG("\n");
        channel.SetTriggered(false);
        channel.SetValue(false);
        channel.SetReportedValue(false); // Remember last sent value
        zunoSendReport(channel.GetServerChannelNumber());
    }
    PublishMotionValue(&channel, value);
    return (TZWAVESensor::Result::ZWAVE_PROCESS_OK);
}

void TZWAVESensor::MotionChannelReset(TZWAVEChannel* channel)
{
    if (channel) {
        PublishMotionValue(channel, false);
    }
}

// Device channel management and firmware data transfer
TZWAVESensor::Result TZWAVESensor::ProcessChannels()
{
    DEBUG("--------------------Measurements-----------------------\n");
    // Check all channels of available sensors
    TZWAVESensor::Result result;
    for (size_t i = 0; i < TZWAVEChannel::CHANNEL_TYPES_COUNT; i++) {
        if (Channels[i].GetEnabled()) {
            switch (Channels[i].GetType()) {
                case TZWAVEChannel::Type::INTRUSION:
                    result = TZWAVESensor::Result::ZWAVE_PROCESS_OK;
                    break;
                case TZWAVEChannel::Type::MOTION:
                    result = ProcessMotionChannel(Channels[i]);
                    break;
                default:
                    result = ProcessCommonChannel(Channels[i]);
                    break;
            }
        }
        if (result == TZWAVESensor::Result::ZWAVE_PROCESS_MODBUS_ERROR) {
            MotionChannelReset(MotionChannelPtr);
            return result;
        }
    }
    return TZWAVESensor::Result::ZWAVE_PROCESS_OK;
}