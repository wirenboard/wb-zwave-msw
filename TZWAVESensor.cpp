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

bool TZWAVESensor::CheckChannelAvailabilityIfUnknown(
    enum TWBMSWSensorAvailability& currentAvailability,
    bool (TWBMSWSensor::*getAvailabilityFunction)(enum TWBMSWSensorAvailability& availability))
{
    if (currentAvailability == TWBMSWSensorAvailability::WB_MSW_SENSOR_UNKNOWN) {
        if ((WbMsw->*getAvailabilityFunction)(currentAvailability) &&
            currentAvailability == TWBMSWSensorAvailability::WB_MSW_SENSOR_AVAILABLE)
        {
            return true;
        }
    }
    return false;
}

// Function determines number of available Z-Wave device channels (EndPoints) and fills in the structures by channel
// type
bool TZWAVESensor::ChannelsInitialize()
{

    ChannelsCount = 0;
    uint8_t channelNumber = 1;
    size_t groupIndex = CTRL_GROUP_1;

    uint32_t startTime = millis();
    uint32_t lastTime = startTime;
    uint32_t timeout = WB_MSW_INPUT_REG_AVAILABILITY_TIMEOUT_MS;

    enum TWBMSWSensorAvailability sensorAvailability[TZWAVEChannelType::ZWAVE_CHANNEL_MAX];
    memset(sensorAvailability, TWBMSWSensorAvailability::WB_MSW_SENSOR_UNKNOWN, sizeof(sensorAvailability));
    bool unknownSensorsLeft = false;

    // If channel had read unsuccessfully its value set to 0. Else, value set to channel, even if it equal to
    // VALUE_ERROR. In work sycle values won't be published if VALUE_ERROR was readed.
    do {
        lastTime = millis();
        // Temperature channel
        if (CheckChannelAvailabilityIfUnknown(sensorAvailability[TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_TEMPERATURE],
                                              &TWBMSWSensor::GetTemperatureAvailability))
        {
            DEBUG("TEMPERATURE CHANNEL AVAILABLE\n");
            Channels[ChannelsCount].SetTemperatureChannel(channelNumber, groupIndex);
            ChannelsCount++;
            channelNumber++;
            groupIndex++;
        }
        // Humidity channel
        if (CheckChannelAvailabilityIfUnknown(sensorAvailability[TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_HUMIDITY],
                                              &TWBMSWSensor::GetHumidityAvailability))
        {
            DEBUG("HUMIDITY CHANNEL AVAILABLE\n");
            Channels[ChannelsCount].SetHumidityChannel(channelNumber, groupIndex);
            ChannelsCount++;
            channelNumber++;
            groupIndex++;
        }
        // Lumen channel
        if (CheckChannelAvailabilityIfUnknown(sensorAvailability[TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_LUMEN],
                                              &TWBMSWSensor::GetLuminanceAvailability))
        {
            DEBUG("LUMINANCE CHANNEL AVAILABLE\n");
            Channels[ChannelsCount].SetLuminanceChannel(channelNumber, groupIndex);
            ChannelsCount++;
            channelNumber++;
            groupIndex++;
        }
        if (CheckChannelAvailabilityIfUnknown(sensorAvailability[TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_CO2],
                                              &TWBMSWSensor::GetCO2Availability))
        {
            // Skip channel adding if unable to turn it on
            bool co2Enable;
            if (WbMsw->GetCO2Status(co2Enable) && (co2Enable || WbMsw->SetCO2Status(true))) {
                DEBUG("CO2 CHANNEL AVAILABLE\n");
                Channels[ChannelsCount].SetCO2Channel(channelNumber, groupIndex);
                ChannelsCount++;
                channelNumber++;
                groupIndex++;
            }
        }
        if (CheckChannelAvailabilityIfUnknown(sensorAvailability[TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_VOC],
                                              &TWBMSWSensor::GetVocAvailability))
        {
            DEBUG("VOC CHANNEL AVAILABLE\n");
            Channels[ChannelsCount].SetVocChannel(channelNumber, groupIndex);
            ChannelsCount++;
            channelNumber++;
            groupIndex++;
        }
        if (CheckChannelAvailabilityIfUnknown(sensorAvailability[TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_NOISE_LEVEL],
                                              &TWBMSWSensor::GetNoiseLevelAvailability))
        {
            DEBUG("NOISE LEVEL CHANNEL AVAILABLE\n");
            Channels[ChannelsCount].SetNoiseLevelChannel(channelNumber, groupIndex);
            ChannelsCount++;
            channelNumber++;
            groupIndex++;
        }
        if (CheckChannelAvailabilityIfUnknown(sensorAvailability[TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_MOTION],
                                              &TWBMSWSensor::GetMotionAvailability))
        {
            DEBUG("MOTION CHANNEL AVAILABLE\n");
            Channels[ChannelsCount].SetBMotionChannel(channelNumber, groupIndex);
            MotionChannelPtr = &Channels[ChannelsCount];
            ChannelsCount++;
            channelNumber++;
            groupIndex++;
        }

        unknownSensorsLeft = false;
        for (uint8_t i = 0; i < sizeof(sensorAvailability); i++) {
            unknownSensorsLeft |= (sensorAvailability[i] == TWBMSWSensorAvailability::WB_MSW_SENSOR_UNKNOWN);
        }
    } while ((lastTime - startTime <= timeout) && unknownSensorsLeft);

    return ChannelsCount;
}

// Setting up Z-Wave channels, setting Multichannel indexes
void TZWAVESensor::ChannelsSetup()
{
    for (size_t i = 0; i < ChannelsCount; i++) {
        switch (Channels[i].GetType()) {
            case TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_TEMPERATURE:
                zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER,
                               ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE,
                               (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_CELSIUS,
                                                                      WB_MSW_INPUT_REG_TEMPERATURE_VALUE_SIZE,
                                                                      WB_MSW_INPUT_REG_TEMPERATURE_VALUE_PRECISION)));
                zunoSetZWChannel(i, i + 1);
                zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                break;
            case TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_HUMIDITY:
                zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER,
                               ZUNO_SENSOR_MULTILEVEL_TYPE_RELATIVE_HUMIDITY,
                               (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_PERCENTAGE_VALUE,
                                                                      WB_MSW_INPUT_REG_HUMIDITY_VALUE_SIZE,
                                                                      WB_MSW_INPUT_REG_HUMIDITY_VALUE_PRECISION)));
                zunoSetZWChannel(i, i + 1);
                zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                break;
            case TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_LUMEN:
                zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER,
                               ZUNO_SENSOR_MULTILEVEL_TYPE_LUMINANCE,
                               (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_LUX,
                                                                      WB_MSW_INPUT_REG_LUMEN_VALUE_SIZE,
                                                                      WB_MSW_INPUT_REG_LUMEN_VALUE_PRECISION)));
                zunoSetZWChannel(i, i + 1);
                zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                break;
            case TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_CO2:
                zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER,
                               ZUNO_SENSOR_MULTILEVEL_TYPE_CO2_LEVEL,
                               (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_PARTS_PER_MILLION,
                                                                      WB_MSW_INPUT_REG_CO2_VALUE_SIZE,
                                                                      WB_MSW_INPUT_REG_CO2_VALUE_PRECISION)));
                zunoSetZWChannel(i, i + 1);
                zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                break;
            case TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_VOC:
                zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER,
                               ZUNO_SENSOR_MULTILEVEL_TYPE_VOLATILE_ORGANIC_COMPOUND,
                               (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_PARTS_PER_MILLION,
                                                                      WB_MSW_INPUT_REG_VOC_VALUE_SIZE,
                                                                      WB_MSW_INPUT_REG_VOC_VALUE_PRECISION)));
                zunoSetZWChannel(i, i + 1);
                zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                break;
            case TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_NOISE_LEVEL:
                zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER,
                               ZUNO_SENSOR_MULTILEVEL_TYPE_LOUDNESS,
                               (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_DECIBELS,
                                                                      WB_MSW_INPUT_REG_NOISE_LEVEL_VALUE_SIZE,
                                                                      WB_MSW_INPUT_REG_NOISE_LEVEL_PRECISION)));
                zunoSetZWChannel(i, i + 1);
                zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                break;
            case TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_MOTION:
                zunoAddChannel(ZUNO_SENSOR_BINARY_CHANNEL_NUMBER, ZUNO_SENSOR_BINARY_TYPE_MOTION, 0);
                zunoSetZWChannel(i, i + 1);
                zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
                break;
            default:
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
                case TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_TEMPERATURE:
                    return "Temperature Basic On/Off";
                case TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_HUMIDITY:
                    return "Humidity Basic On/Off";
                case TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_LUMEN:
                    return "Lumen Basic On/Off";
                case TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_CO2:
                    return "CO2 Basic On/Off";
                case TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_VOC:
                    return "VOC Basic On/Off";
                case TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_NOISE_LEVEL:
                    return "Noise level Basic On/Off";
                case TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_MOTION:
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
            if (!GetChannelByType(TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_TEMPERATURE)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
        case WB_MSW_CONFIG_PARAMETER_HUMIDITY_HYSTERESIS:
        case WB_MSW_CONFIG_PARAMETER_HUMIDITY_INVERT:
        case WB_MSW_CONFIG_PARAMETER_HUMIDITY_THRESHOLD:
            if (!GetChannelByType(TZWAVEChannelType ::ZWAVE_CHANNEL_TYPE_HUMIDITY)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
        case WB_MSW_CONFIG_PARAMETER_LUMEN_HYSTERESIS:
        case WB_MSW_CONFIG_PARAMETER_LUMEN_INVERT:
        case WB_MSW_CONFIG_PARAMETER_LUMEN_THRESHOLD:
            if (!GetChannelByType(TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_LUMEN)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
        case WB_MSW_CONFIG_PARAMETER_CO2_HYSTERESIS:
        case WB_MSW_CONFIG_PARAMETER_CO2_INVERT:
        case WB_MSW_CONFIG_PARAMETER_CO2_THRESHOLD:
        case WB_MSW_CONFIG_PARAMETER_CO2_AUTO:
            if (!GetChannelByType(TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_CO2)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
        case WB_MSW_CONFIG_PARAMETER_VOC_HYSTERESIS:
        case WB_MSW_CONFIG_PARAMETER_VOC_INVERT:
        case WB_MSW_CONFIG_PARAMETER_VOC_THRESHOLD:
            if (!GetChannelByType(TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_VOC)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
        case WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_HYSTERESIS:
        case WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_INVERT:
        case WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_THRESHOLD:
            if (!GetChannelByType(TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_NOISE_LEVEL)) {
                return (ZUNO_CFG_PARAMETER_UNKNOWN);
            }
            break;
        case WB_MSW_CONFIG_PARAMETER_MOTION_TIME:
        case WB_MSW_CONFIG_PARAMETER_MOTION_INVERT:
        case WB_MSW_CONFIG_PARAMETER_MOTION_THRESHOLD:
            if (!GetChannelByType(TZWAVEChannelType::ZWAVE_CHANNEL_TYPE_MOTION)) {
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
    // Send value without condition if channels value uninitialized on server
    if ((channel.GetState() == TZWAVEChannelState::ZWAVE_CHANNEL_UNINITIALIZED) ||
        ((hysteresis != 0) && (abs(value - channel.GetReportedValue()) > hysteresis)))
    {
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
enum TZWAVEProcessResult TZWAVESensor::ProcessTemperatureChannel(TZWAVEChannel& channel)
{
    int16_t currentTemperature;
    if (!WbMsw->GetTemperature(currentTemperature)) {
        return TZWAVEProcessResult::ZWAVE_PROCESS_MODBUS_ERROR;
    }
    if (currentTemperature == WB_MSW_INPUT_REG_TEMPERATURE_VALUE_ERROR) {
        return TZWAVEProcessResult::ZWAVE_PROCESS_VALUE_ERROR;
    }
    LOG_FIXEDPOINT_VALUE("Temperature:        ", currentTemperature, 2);
    channel.SetTemperatureValue(currentTemperature);

    int32_t hysteresis = GetParameterValue(WB_MSW_CONFIG_PARAMETER_TEMPERATURE_HYSTERESIS);
    bool inversion = (bool)GetParameterValue(WB_MSW_CONFIG_PARAMETER_TEMPERATURE_INVERT);
    int32_t threshold = GetParameterValue(WB_MSW_CONFIG_PARAMETER_TEMPERATURE_THRESHOLD);
    PublishAnalogSensorValue(channel, currentTemperature, hysteresis, inversion, threshold);
    return TZWAVEProcessResult::ZWAVE_PROCESS_OK;
}

enum TZWAVEProcessResult TZWAVESensor::ProcessHumidityChannel(TZWAVEChannel& channel)
{
    uint16_t currentHumidity;
    if (!WbMsw->GetHumidity(currentHumidity)) {
        return TZWAVEProcessResult::ZWAVE_PROCESS_MODBUS_ERROR;
    }
    if (currentHumidity == WB_MSW_INPUT_REG_HUMIDITY_VALUE_ERROR) {
        return TZWAVEProcessResult::ZWAVE_PROCESS_VALUE_ERROR;
    }
    LOG_FIXEDPOINT_VALUE("Humidity:           ", currentHumidity, 2);
    channel.SetHumidityValue(currentHumidity);

    int32_t hysteresis = GetParameterValue(WB_MSW_CONFIG_PARAMETER_HUMIDITY_HYSTERESIS);
    bool inversion = (bool)GetParameterValue(WB_MSW_CONFIG_PARAMETER_HUMIDITY_INVERT);
    int32_t threshold = GetParameterValue(WB_MSW_CONFIG_PARAMETER_HUMIDITY_THRESHOLD);
    PublishAnalogSensorValue(channel, currentHumidity, hysteresis, inversion, threshold);
    return TZWAVEProcessResult::ZWAVE_PROCESS_OK;
}

enum TZWAVEProcessResult TZWAVESensor::ProcessLuminanceChannel(TZWAVEChannel& channel)
{
    uint32_t currentLumen;
    if (!WbMsw->GetLuminance(currentLumen)) {
        return TZWAVEProcessResult::ZWAVE_PROCESS_MODBUS_ERROR;
    }
    if (currentLumen == WB_MSW_INPUT_REG_LUMEN_VALUE_ERROR) {
        return TZWAVEProcessResult::ZWAVE_PROCESS_VALUE_ERROR;
    }
    LOG_FIXEDPOINT_VALUE("Lumen:              ", currentLumen, 2);
    channel.SetLuminanceValue(currentLumen);

    int32_t hysteresis = GetParameterValue(WB_MSW_CONFIG_PARAMETER_LUMEN_HYSTERESIS);
    bool inversion = (bool)GetParameterValue(WB_MSW_CONFIG_PARAMETER_LUMEN_INVERT);
    int32_t threshold = GetParameterValue(WB_MSW_CONFIG_PARAMETER_LUMEN_THRESHOLD);
    PublishAnalogSensorValue(channel, currentLumen, hysteresis, inversion, threshold);
    return TZWAVEProcessResult::ZWAVE_PROCESS_OK;
}

enum TZWAVEProcessResult TZWAVESensor::ProcessCO2Channel(TZWAVEChannel& channel)
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
        return TZWAVEProcessResult::ZWAVE_PROCESS_MODBUS_ERROR;
    }
    if (currentCO2 == WB_MSW_INPUT_REG_CO2_VALUE_ERROR) {
        return TZWAVEProcessResult::ZWAVE_PROCESS_VALUE_ERROR;
    }
    LOG_INT_VALUE("CO2:                ", currentCO2);
    channel.SetCO2Value(currentCO2);

    int32_t hysteresis = GetParameterValue(WB_MSW_CONFIG_PARAMETER_CO2_HYSTERESIS);
    bool inversion = (bool)GetParameterValue(WB_MSW_CONFIG_PARAMETER_CO2_INVERT);
    int32_t threshold = GetParameterValue(WB_MSW_CONFIG_PARAMETER_CO2_THRESHOLD);
    PublishAnalogSensorValue(channel, currentCO2, hysteresis, inversion, threshold);
    return TZWAVEProcessResult::ZWAVE_PROCESS_OK;
}

enum TZWAVEProcessResult TZWAVESensor::ProcessVOCChannel(TZWAVEChannel& channel)
{
    uint16_t currentVoc;
    if (!WbMsw->GetVoc(currentVoc)) {
        return TZWAVEProcessResult::ZWAVE_PROCESS_MODBUS_ERROR;
    }
    if (currentVoc == WB_MSW_INPUT_REG_VOC_VALUE_ERROR) {
        return TZWAVEProcessResult::ZWAVE_PROCESS_VALUE_ERROR;
    }
    LOG_INT_VALUE("VOC:                ", currentVoc);
    channel.SetVocValue(currentVoc);

    int32_t hysteresis = GetParameterValue(WB_MSW_CONFIG_PARAMETER_VOC_HYSTERESIS);
    bool inversion = (bool)GetParameterValue(WB_MSW_CONFIG_PARAMETER_VOC_INVERT);
    int32_t threshold = GetParameterValue(WB_MSW_CONFIG_PARAMETER_VOC_THRESHOLD);
    PublishAnalogSensorValue(channel, currentVoc, hysteresis, inversion, threshold);
    return TZWAVEProcessResult::ZWAVE_PROCESS_OK;
}

enum TZWAVEProcessResult TZWAVESensor::ProcessNoiseLevelChannel(TZWAVEChannel& channel)
{
    uint16_t currentNoiseLevel;
    if (!WbMsw->GetNoiseLevel(currentNoiseLevel)) {
        return TZWAVEProcessResult::ZWAVE_PROCESS_MODBUS_ERROR;
    }
    LOG_FIXEDPOINT_VALUE("Noise Level:        ", currentNoiseLevel, 2);
    channel.SetNoiseLevelValue(currentNoiseLevel);

    int32_t hysteresis = GetParameterValue(WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_HYSTERESIS);
    bool inversion = (bool)GetParameterValue(WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_INVERT);
    int32_t threshold = GetParameterValue(WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_THRESHOLD);
    PublishAnalogSensorValue(channel, currentNoiseLevel, hysteresis, inversion, threshold);
    return TZWAVEProcessResult::ZWAVE_PROCESS_OK;
}

enum TZWAVEProcessResult TZWAVESensor::ProcessMotionChannel(TZWAVEChannel& channel)
{
    bool inverting = GetParameterValue(WB_MSW_CONFIG_PARAMETER_MOTION_INVERT);
    uint32_t motionPeriod = (uint32_t)GetParameterValue(WB_MSW_CONFIG_PARAMETER_MOTION_TIME) * 1000;

    uint16_t currentMotion;
    if (!WbMsw->GetMotion(currentMotion)) {
        channel.SetBMotionValue(false);
        channel.SetReportedValue(false);
        zunoSendReport(channel.GetChannelNumber());
        zunoSendToGroupSetValueCommand(channel.GetGroupIndex(), (!inverting) ? WB_MSW_OFF : WB_MSW_ON);
        return TZWAVEProcessResult::ZWAVE_PROCESS_MODBUS_ERROR;
    }

    if (currentMotion == WB_MSW_INPUT_REG_MOTION_VALUE_ERROR) {
        channel.SetBMotionValue(false);
        channel.SetReportedValue(false);
        zunoSendReport(channel.GetChannelNumber());
        zunoSendToGroupSetValueCommand(channel.GetGroupIndex(), (!inverting) ? WB_MSW_OFF : WB_MSW_ON);
        return TZWAVEProcessResult::ZWAVE_PROCESS_VALUE_ERROR;
    }

    LOG_INT_VALUE("Motion:             ", currentMotion);
    bool newMotionValue =
        currentMotion >= (size_t)GetParameterValue(WB_MSW_CONFIG_PARAMETER_MOTION_THRESHOLD) ? true : false;
    channel.SetBMotionValue(newMotionValue);

    uint32_t currentTime = millis();
    if (newMotionValue) {
        if ((channel.GetState() == TZWAVEChannelState::ZWAVE_CHANNEL_UNINITIALIZED) || (!channel.GetReportedValue())) {
            MotionLastTime = currentTime;
            channel.SetReportedValue(newMotionValue);
            // DEBUG("Channel Motion send report value ");
            // DEBUG(channel.GetReportedValue());
            // DEBUG("\n");
            zunoSendReport(channel.GetChannelNumber());
            zunoSendToGroupSetValueCommand(channel.GetGroupIndex(), (!inverting) ? WB_MSW_ON : WB_MSW_OFF);
        }
    } else {
        if ((channel.GetState() == TZWAVEChannelState::ZWAVE_CHANNEL_UNINITIALIZED) ||
            (channel.GetReportedValue() && ((currentTime - MotionLastTime) >= motionPeriod)))
        {
            channel.SetReportedValue(newMotionValue);
            // DEBUG("Channel Motion send report value ");
            // DEBUG(channel.GetReportedValue());
            // DEBUG("\n");
            zunoSendReport(channel.GetChannelNumber());
            zunoSendToGroupSetValueCommand(channel.GetGroupIndex(), (!inverting) ? WB_MSW_OFF : WB_MSW_ON);
        }
    }
    return TZWAVEProcessResult::ZWAVE_PROCESS_OK;
}

void TZWAVESensor::MotionChannelReset(TZWAVEChannel* channel)
{
    if (channel) {
        bool inverting = GetParameterValue(WB_MSW_CONFIG_PARAMETER_MOTION_INVERT);

        channel->SetBMotionValue(false);
        channel->SetReportedValue(false);
        zunoSendReport(channel->GetChannelNumber());
        zunoSendToGroupSetValueCommand(channel->GetGroupIndex(), (!inverting) ? WB_MSW_OFF : WB_MSW_ON);
    }
}

// Device channel management and firmware data transfer
enum TZWAVEProcessResult TZWAVESensor::ProcessChannels()
{
    DEBUG("--------------------Measurements-----------------------\n");
    // Check all channels of available sensors
    enum TZWAVEProcessResult result;
    for (size_t i = 0; i < ZUNO_CFG_CHANNEL_COUNT; i++) {
        switch (Channels[i].GetType()) {
            case ZWAVE_CHANNEL_TYPE_TEMPERATURE:
                result = ProcessTemperatureChannel(Channels[i]);
                break;
            case ZWAVE_CHANNEL_TYPE_HUMIDITY:
                result = ProcessHumidityChannel(Channels[i]);
                break;
            case ZWAVE_CHANNEL_TYPE_LUMEN:
                result = ProcessLuminanceChannel(Channels[i]);
                break;
            case ZWAVE_CHANNEL_TYPE_CO2:
                result = ProcessCO2Channel(Channels[i]);
                break;
            case ZWAVE_CHANNEL_TYPE_VOC:
                result = ProcessVOCChannel(Channels[i]);
                break;
            case ZWAVE_CHANNEL_TYPE_NOISE_LEVEL:
                result = ProcessNoiseLevelChannel(Channels[i]);
                break;
            case ZWAVE_CHANNEL_TYPE_MOTION:
                result = ProcessMotionChannel(Channels[i]);
                break;
            default:
                result = TZWAVEProcessResult::ZWAVE_PROCESS_OK;
                break;
        }
        if (result == TZWAVEProcessResult::ZWAVE_PROCESS_MODBUS_ERROR) {
            MotionChannelReset(MotionChannelPtr);
            return result;
        }
    }
    return TZWAVEProcessResult::ZWAVE_PROCESS_OK;
}