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
    bool co2Enable;
    uint16_t motion;

    ChannelsCount = 0;
    uint8_t channelNumber = 1;
    size_t groupIndex = CTRL_GROUP_1;

    int16_t temperature;
    uint16_t humidity, co2, voc, noiseLevel;
    uint32_t luminance;

    // Temperature channel
    if (WbMsw->GetTemperature(temperature) && (temperature != WB_MSW_INPUT_REG_TEMPERATURE_VALUE_ERROR)) {
        Channels[ChannelsCount].SetTemperatureChannel(channelNumber, groupIndex, temperature);
        ChannelsCount++;
        channelNumber++;
        groupIndex++;
    }
    // Humidity channel
    if (WbMsw->GetHumidity(humidity) && (humidity != WB_MSW_INPUT_REG_HUMIDITY_VALUE_ERROR)) {
        Channels[ChannelsCount].SetHumidityChannel(channelNumber, groupIndex, humidity);
        ChannelsCount++;
        channelNumber++;
        groupIndex++;
    }
    // Lumen channel
    if (WbMsw->GetLuminance(luminance) && (luminance != WB_MSW_INPUT_REG_LUMEN_VALUE_ERROR)) {
        Channels[ChannelsCount].SetLuminanceChannel(channelNumber, groupIndex, luminance);
        ChannelsCount++;
        channelNumber++;
        groupIndex++;
    }
    if (WbMsw->GetCO2Status(co2Enable) && (co2Enable || WbMsw->SetCO2Status(true)) && (WbMsw->GetCO2(co2))) {
        if (co2 == WB_MSW_INPUT_REG_CO2_VALUE_ERROR) {
            co2 = 0;
        }

        Channels[ChannelsCount].SetCO2Channel(channelNumber, groupIndex, co2);
        ChannelsCount++;
        channelNumber++;
        groupIndex++;
    }
    if (WbMsw->GetVoc(voc) && (voc != WB_MSW_INPUT_REG_VOC_VALUE_ERROR)) {
        Channels[ChannelsCount].SetVocChannel(channelNumber, groupIndex, voc);
        ChannelsCount++;
        channelNumber++;
        groupIndex++;
    }
    if (WbMsw->GetNoiseLevel(noiseLevel)) {
        Channels[ChannelsCount].SetNoiseLevelChannel(channelNumber, groupIndex, noiseLevel);
        ChannelsCount++;
        channelNumber++;
        groupIndex++;
    }
    if (WbMsw->GetMotion(motion) && (motion != WB_MSW_INPUT_REG_MOTION_VALUE_ERROR)) {
        if (motion != WB_MSW_INPUT_REG_MOTION_VALUE_ERROR) {

            Channels[ChannelsCount].SetBMotionChannel(channelNumber, groupIndex, false);
            ChannelsCount++;
            channelNumber++;
            groupIndex++;
        }
    }
    return ChannelsCount;
}

// Setting up Z-Wave channels, setting Multichannel indexes
void TZWAVESensor::ChannelsSetup()
{
    for (size_t i = 0; i < ChannelsCount; i++) {
        switch (Channels[i].GetType()) {
            case TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_TEMPERATURE:
                zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER,
                               ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE,
                               (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_CELSIUS,
                                                                      WB_MSW_INPUT_REG_TEMPERATURE_VALUE_SIZE,
                                                                      WB_MSW_INPUT_REG_TEMPERATURE_VALUE_PRECISION)));
                zunoSetZWChannel(i, i + 1);
                zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                break;
            case TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_HUMIDITY:
                zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER,
                               ZUNO_SENSOR_MULTILEVEL_TYPE_RELATIVE_HUMIDITY,
                               (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_PERCENTAGE_VALUE,
                                                                      WB_MSW_INPUT_REG_HUMIDITY_VALUE_SIZE,
                                                                      WB_MSW_INPUT_REG_HUMIDITY_VALUE_PRECISION)));
                zunoSetZWChannel(i, i + 1);
                zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                break;
            case TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_LUMEN:
                zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER,
                               ZUNO_SENSOR_MULTILEVEL_TYPE_LUMINANCE,
                               (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_LUX,
                                                                      WB_MSW_INPUT_REG_LUMEN_VALUE_SIZE,
                                                                      WB_MSW_INPUT_REG_LUMEN_VALUE_PRECISION)));
                zunoSetZWChannel(i, i + 1);
                zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                break;
            case TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_CO2:
                zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER,
                               ZUNO_SENSOR_MULTILEVEL_TYPE_CO2_LEVEL,
                               (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_PARTS_PER_MILLION,
                                                                      WB_MSW_INPUT_REG_CO2_VALUE_SIZE,
                                                                      WB_MSW_INPUT_REG_CO2_VALUE_PRECISION)));
                zunoSetZWChannel(i, i + 1);
                zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                break;
            case TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_VOC:
                zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER,
                               ZUNO_SENSOR_MULTILEVEL_TYPE_VOLATILE_ORGANIC_COMPOUND,
                               (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(1,
                                                                      WB_MSW_INPUT_REG_VOC_VALUE_SIZE,
                                                                      WB_MSW_INPUT_REG_VOC_VALUE_PRECISION)));
                zunoSetZWChannel(i, i + 1);
                zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                break;
            case TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_NOISE_LEVEL:
                zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER,
                               ZUNO_SENSOR_MULTILEVEL_TYPE_LOUDNESS,
                               (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_DECIBELS,
                                                                      WB_MSW_INPUT_REG_NOISE_LEVEL_VALUE_SIZE,
                                                                      WB_MSW_INPUT_REG_NOISE_LEVEL_PRECISION)));
                zunoSetZWChannel(i, i + 1);
                zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                break;
            case TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_MOTION:
                zunoAddChannel(ZUNO_SENSOR_BINARY_CHANNEL_NUMBER, ZUNO_SENSOR_BINARY_TYPE_MOTION, 0);
                zunoSetZWChannel(i, i + 1);
                zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                break;
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
    for (size_t i = 0; i < ChannelsCount; i++) {
        zunoAppendChannelHandler(i,
                                 WB_MSW_INPUT_REG_TEMPERATURE_VALUE_SIZE,
                                 CHANNEL_HANDLER_SINGLE_VALUEMAPPER,
                                 Channels[i].GetValuePointer());
    }
}

// Sets by return value name of group by its index
const char* TZWAVESensor::GetGroupNameByIndex(uint8_t groupIndex)
{
    for (uint8_t i = 0; i < ZUNO_CFG_CHANNEL_COUNT; i++) {
        if (Channels[i].GetGroupIndex() == groupIndex) {
            switch (Channels[i].GetType()) {
                case TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_TEMPERATURE:
                    return "Temperature Basic On/Off";
                case TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_HUMIDITY:
                    return "Humidity Basic On/Off";
                case TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_LUMEN:
                    return "Lumen Basic On/Off";
                case TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_CO2:
                    return "CO2 Basic On/Off";
                case TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_VOC:
                    return "VOC Basic On/Off";
                case TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_NOISE_LEVEL:
                    return "Noise level Basic On/Off";
                case TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_MOTION:
                    return "Motion Basic On/Off";
                default:
                    return NULL;
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
TZWAVEChannel* TZWAVESensor::GetChannelByType(enum TZWAVEChannelType type)
{
    for (size_t i = 0; i < ZUNO_CFG_CHANNEL_COUNT; i++) {
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
            if (!GetChannelByType(TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_TEMPERATURE)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
        case WB_MSW_CONFIG_PARAMETER_HUMIDITY_HYSTERESIS:
        case WB_MSW_CONFIG_PARAMETER_HUMIDITY_INVERT:
        case WB_MSW_CONFIG_PARAMETER_HUMIDITY_THRESHOLD:
            if (!GetChannelByType(TZWAVEChannelType ::WB_MSW_CHANNEL_TYPE_HUMIDITY)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
        case WB_MSW_CONFIG_PARAMETER_LUMEN_HYSTERESIS:
        case WB_MSW_CONFIG_PARAMETER_LUMEN_INVERT:
        case WB_MSW_CONFIG_PARAMETER_LUMEN_THRESHOLD:
            if (!GetChannelByType(TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_LUMEN)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
        case WB_MSW_CONFIG_PARAMETER_CO2_HYSTERESIS:
        case WB_MSW_CONFIG_PARAMETER_CO2_INVERT:
        case WB_MSW_CONFIG_PARAMETER_CO2_THRESHOLD:
        case WB_MSW_CONFIG_PARAMETER_CO2_AUTO:
            if (!GetChannelByType(TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_CO2)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
        case WB_MSW_CONFIG_PARAMETER_VOC_HYSTERESIS:
        case WB_MSW_CONFIG_PARAMETER_VOC_INVERT:
        case WB_MSW_CONFIG_PARAMETER_VOC_THRESHOLD:
            if (!GetChannelByType(TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_VOC)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
        case WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_HYSTERESIS:
        case WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_INVERT:
        case WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_THRESHOLD:
            if (!GetChannelByType(TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_NOISE_LEVEL)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
        case WB_MSW_CONFIG_PARAMETER_MOTION_TIME:
        case WB_MSW_CONFIG_PARAMETER_MOTION_INVERT:
        case WB_MSW_CONFIG_PARAMETER_MOTION_THRESHOLD:
            if (!GetChannelByType(TZWAVEChannelType::WB_MSW_CHANNEL_TYPE_MOTION)) {
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
                                            int32_t value,
                                            int32_t hysteresis,
                                            bool inversion,
                                            int32_t threshold)
{
    if ((hysteresis != 0) && (abs(value - channel.GetReportedValue()) > hysteresis)) {
        // DEBUG("Channel ");
        // DEBUG(channel.GetType());
        // DEBUG(" send report value ");
        // DEBUG(value);
        // DEBUG("\n");
        channel.SetReportedValue(value); // Remember last sent value
        zunoSendReport(channel.GetChannelNumber());
    }
    // Is the threshold exceeded?
    if (channel.GetTriggered()) {
        if ((value + hysteresis) < threshold) {
            channel.SetTriggered(false); // No exceed
            // Sent to the Basic.Off group (On with inverted logic)
            zunoSendToGroupSetValueCommand(channel.GetGroupIndex(), (!inversion) ? WB_MSW_OFF : WB_MSW_ON);
        }
    } else {
        if ((value - hysteresis) > threshold) {
            channel.SetTriggered(true); // Exceed
            // Sent to the Basic.On group (Off with inverted logic)
            zunoSendToGroupSetValueCommand(channel.GetGroupIndex(), (!inversion) ? WB_MSW_ON : WB_MSW_OFF);
        }
    }
}

// Processing of various types of sensors
void TZWAVESensor::ProcessTemperatureChannel(TZWAVEChannel& channel)
{
    int16_t currentTemperature;
    if (!WbMsw->GetTemperature(currentTemperature)) {
        return;
    }
    if (currentTemperature == WB_MSW_INPUT_REG_TEMPERATURE_VALUE_ERROR) {
        return;
    }
    LOG_FIXEDPOINT_VALUE("Temperature:        ", currentTemperature, 2);
    channel.Temperature = currentTemperature;

    int32_t hysteresis = GetParameterValue(WB_MSW_CONFIG_PARAMETER_TEMPERATURE_HYSTERESIS);
    bool inversion = (bool)GetParameterValue(WB_MSW_CONFIG_PARAMETER_TEMPERATURE_INVERT);
    int32_t threshold = GetParameterValue(WB_MSW_CONFIG_PARAMETER_TEMPERATURE_THRESHOLD);
    PublishAnalogSensorValue(channel, currentTemperature, hysteresis, inversion, threshold);
}

void TZWAVESensor::ProcessHumidityChannel(TZWAVEChannel& channel)
{
    uint16_t currentHumidity;
    if (!WbMsw->GetHumidity(currentHumidity)) {
        return;
    }
    if (currentHumidity == WB_MSW_INPUT_REG_HUMIDITY_VALUE_ERROR) {
        return;
    }
    LOG_FIXEDPOINT_VALUE("Humidity:           ", currentHumidity, 2);
    channel.Humidity = currentHumidity;

    int32_t hysteresis = GetParameterValue(WB_MSW_CONFIG_PARAMETER_HUMIDITY_HYSTERESIS);
    bool inversion = (bool)GetParameterValue(WB_MSW_CONFIG_PARAMETER_HUMIDITY_INVERT);
    int32_t threshold = GetParameterValue(WB_MSW_CONFIG_PARAMETER_HUMIDITY_THRESHOLD);
    PublishAnalogSensorValue(channel, currentHumidity, hysteresis, inversion, threshold);
}

void TZWAVESensor::ProcessLuminanceChannel(TZWAVEChannel& channel)
{
    uint32_t currentLumen;
    if (!WbMsw->GetLuminance(currentLumen)) {
        return;
    }
    if (currentLumen == WB_MSW_INPUT_REG_LUMEN_VALUE_ERROR) {
        return;
    }
    LOG_FIXEDPOINT_VALUE("Lumen:              ", currentLumen, 2);
    channel.Luminance = currentLumen;

    int32_t hysteresis = GetParameterValue(WB_MSW_CONFIG_PARAMETER_LUMEN_HYSTERESIS);
    bool inversion = (bool)GetParameterValue(WB_MSW_CONFIG_PARAMETER_LUMEN_INVERT);
    int32_t threshold = GetParameterValue(WB_MSW_CONFIG_PARAMETER_LUMEN_THRESHOLD);
    PublishAnalogSensorValue(channel, currentLumen, hysteresis, inversion, threshold);
}

void TZWAVESensor::ProcessCO2Channel(TZWAVEChannel& channel)
{
    uint16_t currentCO2;
    // Check if automatic calibration is needed
    // Default autocalibration is false;
    uint8_t autocalibration = GetParameterValue(WB_MSW_CONFIG_PARAMETER_CO2_AUTO);
    if (channel.GetAutocalibration() != autocalibration) {
        channel.SetAutocalibration(autocalibration);
        WbMsw->SetCO2Autocalibration(autocalibration);
    }
    if (!WbMsw->GetCO2(currentCO2)) {
        return;
    }
    if (currentCO2 == WB_MSW_INPUT_REG_CO2_VALUE_ERROR) {
        return;
    }
    LOG_INT_VALUE("CO2:                ", currentCO2);
    channel.CO2 = currentCO2;

    int32_t hysteresis = GetParameterValue(WB_MSW_CONFIG_PARAMETER_CO2_HYSTERESIS);
    bool inversion = (bool)GetParameterValue(WB_MSW_CONFIG_PARAMETER_CO2_INVERT);
    int32_t threshold = GetParameterValue(WB_MSW_CONFIG_PARAMETER_CO2_THRESHOLD);
    PublishAnalogSensorValue(channel, currentCO2, hysteresis, inversion, threshold);
}

void TZWAVESensor::ProcessVOCChannel(TZWAVEChannel& channel)
{
    uint16_t currentVoc;
    if (!WbMsw->GetVoc(currentVoc)) {
        return;
    }
    if (currentVoc == WB_MSW_INPUT_REG_VOC_VALUE_ERROR) {
        return;
    }
    LOG_INT_VALUE("VOC:                ", currentVoc);
    channel.Voc = currentVoc;

    int32_t hysteresis = GetParameterValue(WB_MSW_CONFIG_PARAMETER_VOC_HYSTERESIS);
    bool inversion = (bool)GetParameterValue(WB_MSW_CONFIG_PARAMETER_VOC_INVERT);
    int32_t threshold = GetParameterValue(WB_MSW_CONFIG_PARAMETER_VOC_THRESHOLD);
    PublishAnalogSensorValue(channel, currentVoc, hysteresis, inversion, threshold);
}

void TZWAVESensor::ProcessNoiseLevelChannel(TZWAVEChannel& channel)
{
    uint16_t currentNoiseLevel;
    if (!WbMsw->GetNoiseLevel(currentNoiseLevel)) {
        return;
    }
    LOG_FIXEDPOINT_VALUE("Noise Level:        ", currentNoiseLevel, 2);
    channel.NoiseLevel = currentNoiseLevel;

    int32_t hysteresis = GetParameterValue(WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_HYSTERESIS);
    bool inversion = (bool)GetParameterValue(WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_INVERT);
    int32_t threshold = GetParameterValue(WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_THRESHOLD);
    PublishAnalogSensorValue(channel, currentNoiseLevel, hysteresis, inversion, threshold);
}

void TZWAVESensor::ProcessMotionChannel(TZWAVEChannel& channel)
{
    uint8_t bMotion;

    bool inverting = GetParameterValue(WB_MSW_CONFIG_PARAMETER_MOTION_INVERT);
    uint32_t motionPeriod = (uint32_t)GetParameterValue(WB_MSW_CONFIG_PARAMETER_MOTION_TIME) * 1000;
    uint32_t currentTime = millis();

    bMotion = channel.BMotion;
    if (channel.BMotion && ((currentTime - MotionLastTime) >= motionPeriod)) {
        channel.BMotion = false;
    }

    if (!channel.BMotion) {
        uint16_t currentMotion;
        if (!WbMsw->GetMotion(currentMotion)) {
            return;
        }

        if (currentMotion == WB_MSW_INPUT_REG_MOTION_VALUE_ERROR) {
            return;
        }

        LOG_INT_VALUE("Motion:             ", currentMotion);
        if (currentMotion >= (size_t)GetParameterValue(WB_MSW_CONFIG_PARAMETER_MOTION_THRESHOLD)) {
            MotionLastTime = currentTime;
            channel.BMotion = true;
            // DEBUG("Channel Motion send report value ");
            // DEBUG(channel.BMotion);
            // DEBUG("\n");
            zunoSendReport(channel.GetChannelNumber());
            zunoSendToGroupSetValueCommand(channel.GetGroupIndex(), (!inverting) ? WB_MSW_ON : WB_MSW_OFF);
        } else if (bMotion) {
            // DEBUG("Channel Motion send report value ");
            // DEBUG(channel.BMotion);
            // DEBUG("\n");
            zunoSendReport(channel.GetChannelNumber());
            zunoSendToGroupSetValueCommand(channel.GetGroupIndex(), (!inverting) ? WB_MSW_OFF : WB_MSW_ON);
        }
    }
}

// Device channel management and firmware data transfer
void TZWAVESensor::ProcessChannels()
{
    DEBUG("--------------------Measurements-----------------------\n");
    // Check all channels of available sensors
    for (size_t i = 0; i < ZUNO_CFG_CHANNEL_COUNT; i++)
        switch (Channels[i].GetType()) {
            case WB_MSW_CHANNEL_TYPE_TEMPERATURE:
                ProcessTemperatureChannel(Channels[i]);
                break;
            case WB_MSW_CHANNEL_TYPE_HUMIDITY:
                ProcessHumidityChannel(Channels[i]);
                break;
            case WB_MSW_CHANNEL_TYPE_LUMEN:
                ProcessLuminanceChannel(Channels[i]);
                break;
            case WB_MSW_CHANNEL_TYPE_CO2:
                ProcessCO2Channel(Channels[i]);
                break;
            case WB_MSW_CHANNEL_TYPE_VOC:
                ProcessVOCChannel(Channels[i]);
                break;
            case WB_MSW_CHANNEL_TYPE_NOISE_LEVEL:
                ProcessNoiseLevelChannel(Channels[i]);
                break;
            case WB_MSW_CHANNEL_TYPE_MOTION:
                ProcessMotionChannel(Channels[i]);
                break;
            default:
                break;
        }
}