#include "TZWAVESensor.h"
#include "Arduino.h"
#include "DebugOutput.h"
#include "WbMsw.h"

TZWAVESensor::TZWAVESensor(TWBMSWSensor* wbMsw): WbMsw(wbMsw)
{
    // Available device parameters description
    // Channels in the device are created dynamically, so parameters are described in "dynamic" style
    ZunoCFGParameter_t parameters[WB_MSW_MAX_CONFIG_PARAM] = {
        // Temperature channel settings
        ZUNO_CONFIG_PARAMETER_INFO("Temperature hysteresis",                    // Parameter name
                                   "If 0 reports are not sent.Value in 0.01*C", // Parameter description
                                   0,                                           // Minimum allowed value
                                   2000,                                        // Maximum allowed value
                                   100),                                        // Default value
        ZUNO_CONFIG_PARAMETER_INFO("Temperature invert",
                                   "If set device sends Basic.off instead of Basic.on",
                                   false,
                                   true,
                                   false),
        ZUNO_CONFIG_PARAMETER_INFO_SIGN("Temperature threshold", ".Value in 0.01*C", -4000, 8000, 4000),
        // Humidity sensor settings
        ZUNO_CONFIG_PARAMETER_INFO("Humidity hysteresis", "If 0 reports are not sent.Value in 0.01%", 0, 2000, 100),
        ZUNO_CONFIG_PARAMETER_INFO("Humidity invert",
                                   "If set device sends Basic.off instead of Basic.on",
                                   false,
                                   true,
                                   false),
        ZUNO_CONFIG_PARAMETER_INFO("Humidity threshold", "Value in 0.01%", 0, 10000, 5000),
        // Lumen sensor settings
        ZUNO_CONFIG_PARAMETER_INFO("Lumen hysteresis", "Value in 0.01Lux.", 0, 1000000, 200),
        ZUNO_CONFIG_PARAMETER_INFO("Lumen invert",
                                   "If set device sends Basic.off instead of Basic.on",
                                   false,
                                   true,
                                   false),
        ZUNO_CONFIG_PARAMETER_INFO("Lumen threshold", "Value in 0.01Lux.", 0, 10000000, 20000),
        // CO2 sensor settings
        ZUNO_CONFIG_PARAMETER_INFO("CO2 hysteresis", "CO2 hysteresis", 0, 200, 5),
        ZUNO_CONFIG_PARAMETER_INFO("CO2 invert",
                                   "If set device sends Basic.off instead of Basic.on",
                                   false,
                                   true,
                                   false),
        ZUNO_CONFIG_PARAMETER_INFO("CO2 threshold", "CO2 threshold", 400, 5000, 600),
        ZUNO_CONFIG_PARAMETER_INFO("CO2 auto", "CO2 auto", false, true, true),
        // VOC sensor settings
        ZUNO_CONFIG_PARAMETER_INFO("VOC hysteresis", "VOC hysteresis", 0, 200, 1),
        ZUNO_CONFIG_PARAMETER_INFO("VOC invert",
                                   "If set device sends Basic.off instead of Basic.on",
                                   false,
                                   true,
                                   false),
        ZUNO_CONFIG_PARAMETER_INFO("VOC threshold", "VOC threshold", 0, 60000, 660),
        // Noise level sensor settings
        ZUNO_CONFIG_PARAMETER_INFO("Noise level hysteresis", "Noise level hysteresis", 0, 2000, 100),
        ZUNO_CONFIG_PARAMETER_INFO("Noise level invert",
                                   "If set device sends Basic.off instead of Basic.on",
                                   false,
                                   true,
                                   false),
        ZUNO_CONFIG_PARAMETER_INFO("Noise level threshold", "Noise level threshold", 3800, 10500, 5000),
        // Motion sensor settings
        ZUNO_CONFIG_PARAMETER_INFO("Motion time", "Motion time", 0, 300, 20),
        ZUNO_CONFIG_PARAMETER_INFO("Motion invert",
                                   "If set device sends Basic.off instead of Basic.on",
                                   false,
                                   true,
                                   false),
        ZUNO_CONFIG_PARAMETER_INFO("Motion threshold", "Motion threshold", 0, 1000, 200),
    };
    memcpy(Parameters, parameters, sizeof(parameters));
    MotionLastTime = 0;
}

// Function determines number of available Z-Wave device channels (EndPoints) and fills in the structures by channel
// type
bool TZWAVESensor::ChannelsInitialize()
{
    uint32_t startTime = millis();
    uint32_t lastTime = startTime;
    uint32_t timeout = WB_MSW_INPUT_REG_AVAILABILITY_TIMEOUT_MS;

    Channels[0].ChannelInitialize("Temperature",
                                  TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_TEMPERATURE,
                                  WB_MSW_INPUT_REG_TEMPERATURE_VALUE_ERROR,
                                  WB_MSW_CONFIG_PARAMETER_TEMPERATURE_HYSTERESIS,
                                  WB_MSW_CONFIG_PARAMETER_TEMPERATURE_THRESHOLD,
                                  WB_MSW_CONFIG_PARAMETER_TEMPERATURE_INVERT,
                                  WbMsw,
                                  &TWBMSWSensor::GetTemperature,
                                  &TWBMSWSensor::GetTemperatureAvailability);

    Channels[1].ChannelInitialize("Humidity",
                                  TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_HUMIDITY,
                                  WB_MSW_INPUT_REG_HUMIDITY_VALUE_ERROR,
                                  WB_MSW_CONFIG_PARAMETER_HUMIDITY_HYSTERESIS,
                                  WB_MSW_CONFIG_PARAMETER_HUMIDITY_THRESHOLD,
                                  WB_MSW_CONFIG_PARAMETER_HUMIDITY_INVERT,
                                  WbMsw,
                                  &TWBMSWSensor::GetHumidity,
                                  &TWBMSWSensor::GetHumidityAvailability);

    Channels[2].ChannelInitialize("Luminance",
                                  TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_LUMEN,
                                  WB_MSW_INPUT_REG_LUMEN_VALUE_ERROR,
                                  WB_MSW_CONFIG_PARAMETER_LUMEN_HYSTERESIS,
                                  WB_MSW_CONFIG_PARAMETER_LUMEN_THRESHOLD,
                                  WB_MSW_CONFIG_PARAMETER_LUMEN_INVERT,
                                  WbMsw,
                                  &TWBMSWSensor::GetLuminance,
                                  &TWBMSWSensor::GetLuminanceAvailability);

    Channels[3].ChannelInitialize("CO2",
                                  TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_CO2,
                                  WB_MSW_INPUT_REG_CO2_VALUE_ERROR,
                                  WB_MSW_CONFIG_PARAMETER_CO2_HYSTERESIS,
                                  WB_MSW_CONFIG_PARAMETER_CO2_THRESHOLD,
                                  WB_MSW_CONFIG_PARAMETER_CO2_INVERT,
                                  WbMsw,
                                  &TWBMSWSensor::GetCO2,
                                  &TWBMSWSensor::GetCO2Availability);

    Channels[4].ChannelInitialize("VOC",
                                  TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_VOC,
                                  WB_MSW_INPUT_REG_VOC_VALUE_ERROR,
                                  WB_MSW_CONFIG_PARAMETER_VOC_HYSTERESIS,
                                  WB_MSW_CONFIG_PARAMETER_VOC_THRESHOLD,
                                  WB_MSW_CONFIG_PARAMETER_VOC_INVERT,
                                  WbMsw,
                                  &TWBMSWSensor::GetVoc,
                                  &TWBMSWSensor::GetVocAvailability);

    Channels[5].ChannelInitialize("NoiseLevel",
                                  TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_NOISE_LEVEL,
                                  0,
                                  WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_HYSTERESIS,
                                  WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_THRESHOLD,
                                  WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_INVERT,
                                  WbMsw,
                                  &TWBMSWSensor::GetNoiseLevel,
                                  &TWBMSWSensor::GetNoiseLevelAvailability);

    Channels[6].ChannelInitialize("Motion",
                                  TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_MOTION,
                                  WB_MSW_INPUT_REG_MOTION_VALUE_ERROR,
                                  0,
                                  WB_MSW_CONFIG_PARAMETER_MOTION_THRESHOLD,
                                  WB_MSW_CONFIG_PARAMETER_MOTION_INVERT,
                                  WbMsw,
                                  &TWBMSWSensor::GetNoiseLevel,
                                  &TWBMSWSensor::GetNoiseLevelAvailability);

    bool unknownSensorsLeft = false;

    do {
        lastTime = millis();
        for (int i = 0; i < TZWAVEChannel::CHANNEL_TYPES_COUNT; i++) {
            if (Channels[i].GetAvailability() == TWBMSWSensor::Availability::WB_MSW_SENSOR_UNKNOWN) {
                Channels[i].UpdateAvailability();
                if (Channels[i].GetAvailability() == TWBMSWSensor::Availability::WB_MSW_SENSOR_AVAILABLE) {
                    if (Channels[i].GetType() == TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_CO2) {
                        bool co2Enable;
                        if (!WbMsw->GetCO2Status(co2Enable) || (!co2Enable && (!WbMsw->SetCO2Status(true))))
                            break;
                    }
                    if (Channels[i].GetType() == TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_MOTION) {
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
            unknownSensorsLeft = unknownSensorsLeft ||
                                 (Channels[i].GetAvailability() == TWBMSWSensor::Availability::WB_MSW_SENSOR_UNKNOWN);
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
                case TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_TEMPERATURE:
                    zunoAddChannel(
                        ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER,
                        ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE,
                        (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_CELSIUS,
                                                               WB_MSW_INPUT_REG_TEMPERATURE_VALUE_SIZE,
                                                               WB_MSW_INPUT_REG_TEMPERATURE_VALUE_PRECISION)));
                    zunoSetZWChannel(Channels[i].GetDeviceChannelNumber(), Channels[i].GetServerChannelNumber());
                    zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                    break;
                case TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_HUMIDITY:
                    zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER,
                                   ZUNO_SENSOR_MULTILEVEL_TYPE_RELATIVE_HUMIDITY,
                                   (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_PERCENTAGE_VALUE,
                                                                          WB_MSW_INPUT_REG_HUMIDITY_VALUE_SIZE,
                                                                          WB_MSW_INPUT_REG_HUMIDITY_VALUE_PRECISION)));
                    zunoSetZWChannel(Channels[i].GetDeviceChannelNumber(), Channels[i].GetServerChannelNumber());
                    zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                    break;
                case TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_LUMEN:
                    zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER,
                                   ZUNO_SENSOR_MULTILEVEL_TYPE_LUMINANCE,
                                   (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_LUX,
                                                                          WB_MSW_INPUT_REG_LUMEN_VALUE_SIZE,
                                                                          WB_MSW_INPUT_REG_LUMEN_VALUE_PRECISION)));
                    zunoSetZWChannel(Channels[i].GetDeviceChannelNumber(), Channels[i].GetServerChannelNumber());
                    zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                    break;
                case TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_CO2:
                    zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER,
                                   ZUNO_SENSOR_MULTILEVEL_TYPE_CO2_LEVEL,
                                   (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_PARTS_PER_MILLION,
                                                                          WB_MSW_INPUT_REG_CO2_VALUE_SIZE,
                                                                          WB_MSW_INPUT_REG_CO2_VALUE_PRECISION)));
                    zunoSetZWChannel(Channels[i].GetDeviceChannelNumber(), Channels[i].GetServerChannelNumber());
                    zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                    break;
                case TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_VOC:
                    zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER,
                                   ZUNO_SENSOR_MULTILEVEL_TYPE_VOLATILE_ORGANIC_COMPOUND,
                                   (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_PARTS_PER_MILLION,
                                                                          WB_MSW_INPUT_REG_VOC_VALUE_SIZE,
                                                                          WB_MSW_INPUT_REG_VOC_VALUE_PRECISION)));
                    zunoSetZWChannel(Channels[i].GetDeviceChannelNumber(), Channels[i].GetServerChannelNumber());
                    zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                    break;
                case TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_NOISE_LEVEL:
                    zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER,
                                   ZUNO_SENSOR_MULTILEVEL_TYPE_LOUDNESS,
                                   (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_DECIBELS,
                                                                          WB_MSW_INPUT_REG_NOISE_LEVEL_VALUE_SIZE,
                                                                          WB_MSW_INPUT_REG_NOISE_LEVEL_PRECISION)));
                    zunoSetZWChannel(Channels[i].GetDeviceChannelNumber(), Channels[i].GetServerChannelNumber());
                    zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                    break;
                case TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_MOTION:
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
                case (TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_LUMEN):
                    dataSize = SENSOR_MULTILEVEL_SIZE_FOUR_BYTES;
                    break;

                case (TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_MOTION):
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
                    case TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_TEMPERATURE:
                        return "Temperature Basic On/Off";
                    case TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_HUMIDITY:
                        return "Humidity Basic On/Off";
                    case TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_LUMEN:
                        return "Lumen Basic On/Off";
                    case TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_CO2:
                        return "CO2 Basic On/Off";
                    case TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_VOC:
                        return "VOC Basic On/Off";
                    case TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_NOISE_LEVEL:
                        return "Noise level Basic On/Off";
                    case TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_MOTION:
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
        case WB_MSW_CONFIG_PARAMETER_TEMPERATURE_HYSTERESIS:
        case WB_MSW_CONFIG_PARAMETER_TEMPERATURE_INVERT:
        case WB_MSW_CONFIG_PARAMETER_TEMPERATURE_THRESHOLD:
            if (!GetChannelByType(TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_TEMPERATURE)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
        case WB_MSW_CONFIG_PARAMETER_HUMIDITY_HYSTERESIS:
        case WB_MSW_CONFIG_PARAMETER_HUMIDITY_INVERT:
        case WB_MSW_CONFIG_PARAMETER_HUMIDITY_THRESHOLD:
            if (!GetChannelByType(TZWAVEChannel::Type ::ZWAVE_CHANNEL_TYPE_HUMIDITY)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
        case WB_MSW_CONFIG_PARAMETER_LUMEN_HYSTERESIS:
        case WB_MSW_CONFIG_PARAMETER_LUMEN_INVERT:
        case WB_MSW_CONFIG_PARAMETER_LUMEN_THRESHOLD:
            if (!GetChannelByType(TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_LUMEN)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
        case WB_MSW_CONFIG_PARAMETER_CO2_HYSTERESIS:
        case WB_MSW_CONFIG_PARAMETER_CO2_INVERT:
        case WB_MSW_CONFIG_PARAMETER_CO2_THRESHOLD:
        case WB_MSW_CONFIG_PARAMETER_CO2_AUTO:
            if (!GetChannelByType(TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_CO2)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
        case WB_MSW_CONFIG_PARAMETER_VOC_HYSTERESIS:
        case WB_MSW_CONFIG_PARAMETER_VOC_INVERT:
        case WB_MSW_CONFIG_PARAMETER_VOC_THRESHOLD:
            if (!GetChannelByType(TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_VOC)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
        case WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_HYSTERESIS:
        case WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_INVERT:
        case WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_THRESHOLD:
            if (!GetChannelByType(TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_NOISE_LEVEL)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
        case WB_MSW_CONFIG_PARAMETER_MOTION_TIME:
        case WB_MSW_CONFIG_PARAMETER_MOTION_INVERT:
        case WB_MSW_CONFIG_PARAMETER_MOTION_THRESHOLD:
            if (!GetChannelByType(TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_MOTION)) {
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
                                            int32_t hysteresis,
                                            int32_t threshold,
                                            bool inversion)
{
    // Send value without condition if channels value uninitialized on server
    if ((channel.GetState() == TZWAVEChannel::State::ZWAVE_CHANNEL_UNINITIALIZED) ||
        ((hysteresis != 0) && (abs(value - channel.GetReportedValue()) > hysteresis)))
    {
        // DEBUG("Channel ");
        // DEBUG(channel.GetType());
        // DEBUG(" send report value ");
        // DEBUG(value);
        // DEBUG("\n");
        channel.SetReportedValue(value); // Remember last sent value
        zunoSendReport(channel.GetServerChannelNumber());
    }
    // Is the threshold exceeded?
    if (channel.GetTriggered()) {
        if ((value + hysteresis) < threshold) {
            channel.SetTriggered(false); // No exceed
            // Sent to the Basic.Off group (On with inverted logic)
            zunoSendToGroupSetValueCommand(channel.GetGroupIndex(), (inversion) ? WB_MSW_ON : WB_MSW_OFF);
        }
    } else {
        if ((value - hysteresis) > threshold) {
            channel.SetTriggered(true); // Exceed
            // Sent to the Basic.On group (Off with inverted logic)
            zunoSendToGroupSetValueCommand(channel.GetGroupIndex(), (inversion) ? WB_MSW_OFF : WB_MSW_ON);
        }
    }
}

// Processing of various types of sensors
TZWAVESensor::Result TZWAVESensor::ProcessCommonChannel(TZWAVEChannel& channel)
{
    int64_t currentValue;

    if (channel.GetType() == TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_CO2) {
        // Check if automatic calibration is needed
        // Default autocalibration is false;
        uint8_t autocalibration = GetParameterValue(WB_MSW_CONFIG_PARAMETER_CO2_AUTO);
        if (channel.GetAutocalibration() != autocalibration) {
            channel.SetAutocalibration(autocalibration);
            WbMsw->SetCO2Autocalibration(autocalibration);
        }
    }

    if (!channel.ReadValueFromSensor(currentValue)) {
        return TZWAVESensor::Result::ZWAVE_PROCESS_MODBUS_ERROR;
    }
    if ((channel.GetType() != TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_NOISE_LEVEL) &&
        (currentValue == channel.GetErrorValue()))
    {
        return TZWAVESensor::Result::ZWAVE_PROCESS_VALUE_ERROR;
    }

    DEBUG(channel.GetName());
    LOG_FIXEDPOINT_VALUE("        ", currentValue, 2);
    channel.SetValue(currentValue);
    int32_t hysteresis = GetParameterValue(channel.GetHysteresisParameterNumber());
    int32_t threshold = GetParameterValue(channel.GetThresholdParameterNumber());
    bool inversion = (bool)GetParameterValue(channel.GetInversionParameterNumber());
    PublishAnalogSensorValue(channel, currentValue, hysteresis, threshold, inversion);
    return TZWAVESensor::Result::ZWAVE_PROCESS_OK;
}

TZWAVESensor::Result TZWAVESensor::ProcessMotionChannel(TZWAVEChannel& channel)
{
    bool inverting = GetParameterValue(WB_MSW_CONFIG_PARAMETER_MOTION_INVERT);
    uint32_t motionPeriod = (uint32_t)GetParameterValue(WB_MSW_CONFIG_PARAMETER_MOTION_TIME) * 1000;

    int64_t currentMotion;
    if (!WbMsw->GetMotion(currentMotion)) {
        return TZWAVESensor::Result::ZWAVE_PROCESS_MODBUS_ERROR;
    }

    if (currentMotion == WB_MSW_INPUT_REG_MOTION_VALUE_ERROR) {
        MotionChannelReset(&channel);
        return TZWAVESensor::Result::ZWAVE_PROCESS_VALUE_ERROR;
    }

    LOG_INT_VALUE("Motion:             ", (long)currentMotion);
    bool newMotionValue = (currentMotion >= (size_t)GetParameterValue(WB_MSW_CONFIG_PARAMETER_MOTION_THRESHOLD));
    channel.SetValue(newMotionValue);

    uint32_t currentTime = millis();
    if (newMotionValue) {
        if ((channel.GetState() == TZWAVEChannel::State::ZWAVE_CHANNEL_UNINITIALIZED) || (!channel.GetReportedValue()))
        {
            MotionLastTime = currentTime;
            channel.SetReportedValue(newMotionValue);
            // DEBUG("Channel Motion send report value ");
            // DEBUG(channel.GetReportedValue());
            // DEBUG("\n");
            zunoSendReport(channel.GetServerChannelNumber());
            zunoSendToGroupSetValueCommand(channel.GetGroupIndex(), (inverting) ? WB_MSW_OFF : WB_MSW_ON);
        }
    } else if ((channel.GetState() == TZWAVEChannel::State::ZWAVE_CHANNEL_UNINITIALIZED) ||
               (channel.GetReportedValue() && ((currentTime - MotionLastTime) >= motionPeriod)))
    {
        channel.SetReportedValue(newMotionValue);
        // DEBUG("Channel Motion send report value ");
        // DEBUG(channel.GetReportedValue());
        // DEBUG("\n");
        zunoSendReport(channel.GetServerChannelNumber());
        zunoSendToGroupSetValueCommand(channel.GetGroupIndex(), (inverting) ? WB_MSW_ON : WB_MSW_OFF);
    }

    return TZWAVESensor::Result::ZWAVE_PROCESS_OK;
}

void TZWAVESensor::MotionChannelReset(TZWAVEChannel* channel)
{
    if (channel) {
        bool inverting = GetParameterValue(WB_MSW_CONFIG_PARAMETER_MOTION_INVERT);

        channel->SetValue(false);
        channel->SetReportedValue(false);
        zunoSendReport(channel->GetServerChannelNumber());
        zunoSendToGroupSetValueCommand(channel->GetGroupIndex(), (!inverting) ? WB_MSW_OFF : WB_MSW_ON);
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
                case TZWAVEChannel::Type::ZWAVE_CHANNEL_TYPE_MOTION:
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