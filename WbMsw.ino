
#include "WbMsw.h"
#include "WBMSWSensor.h"
#include "Debug.h"

//  Project global macros definitions for accurate protocol configuration
ZUNO_ENABLE(
		/* Commands class support. Defined here since device channels being created dynamically */
		WITH_CC_MULTICHANNEL
		WITH_CC_CONFIGURATION
		WITH_CC_SENSOR_MULTILEVEL
		WITH_CC_NOTIFICATION
		// SKETCH_FLAGS=(HEADER_FLAGS_NOSKETCH_OTA)
		ZUNO_CUSTOM_OTA_OFFSET=0x10000 // 64 kB
		/* Additional OTA firmwares count*/
		ZUNO_EXT_FIRMWARES_COUNT=1
		/* Firmware descriptor pointer */
		ZUNO_EXT_FIRMWARES_DESCR_PTR=&g_OtaDesriptor
		LOGGING_DBG // Comment out if debugging information is not needed
					// Debugging information being printed with RTOS system console output to UART0 (TX0) by default
		//LOGGING_UART=Serial

	);
	//
// Debug output of values
// DBG
#ifdef LOGGING_DBG
#define LOG_INT_VALUE(TEXT, VALUE) LOGGING_UART.print(TEXT); LOGGING_UART.print(VALUE); LOGGING_UART.print("\n");
#define LOG_FIXEDPOINT_VALUE(TEXT, VALUE, PREC) LOGGING_UART.print(TEXT); LOGGING_UART.fixPrint(VALUE, PREC); LOGGING_UART.print("\n");
#else
#define LOG_INT_VALUE(TEXT, VALUE)
#define LOG_FIXEDPOINT_VALUE(TEXT, VALUE, PREC)
#endif
/* ZUNO_DECLARE defines global EXTERN for whole project, descriptor must be visible in all project files to make build right*/
ZUNO_DECLARE(ZUNOOTAFWDescr_t g_OtaDesriptor);

/* WB chip firmware descriptor*/
// ZUNOOTAFWDescr_t g_OtaDesriptor = {0x010A, 0x0103};
ZUNOOTAFWDescr_t g_OtaDesriptor = {0x0101, 0x0103};
WBMSWSensor* wb_msw;
static WbMswChannel_t _channel[WB_MSW_CHANNEL_MAX];
static int32_t _config_parameter[WB_MSW_MAX_CONFIG_PARAM];
static uint8_t _c02_auto = false;
// New firmware image values
static WbMswFw_t_t _fw = {
	.size = 0,
	.bUpdate = false
};

enum TZUnoState{
	ZUNO_OK = 0,
	ZUNO_OPEN_PORT_ERROR = -1,
	ZUNO_WB_SENSOR_NOT_FOUND = -2,
	ZUNO_WB_SENSOR_NO_CHANNELS = -3
};

enum TZUnoState ZUnoState;

// Available device parameters description
// Channels in the device are created dynamically, so parameters are described in "dynamic" style
static const ZunoCFGParameter_t config_parameter_init[WB_MSW_MAX_CONFIG_PARAM] = {
	// Temperature channel settings
	ZUNO_CONFIG_PARAMETER_INFO("Temperature hysteresis",  // Parameter name
							  "If 0 reports are not sent.Value in 0.01*C",   // Parameter description
							  0,                          // Minimum allowed value
							  2000,                         // Maximum allowed value
							  100),                         // Default value
	ZUNO_CONFIG_PARAMETER_INFO("Temperature invert", "If set device sends Basic.off instead of Basic.on", false, true, false),
	ZUNO_CONFIG_PARAMETER_INFO_SIGN("Temperature threshold", ".Value in 0.01*C", -4000, 8000, 4000),
	// Humidity sensor settings
	ZUNO_CONFIG_PARAMETER_INFO("Humidity hysteresis", "If 0 reports are not sent.Value in 0.01%", 0, 2000, 100),
	ZUNO_CONFIG_PARAMETER_INFO("Humidity invert", "If set device sends Basic.off instead of Basic.on", false, true, false),
	ZUNO_CONFIG_PARAMETER_INFO("Humidity threshold", "Value in 0.01%", 0, 10000, 5000),
    // Lumen sensor settings
	ZUNO_CONFIG_PARAMETER_INFO("Lumen hysteresis", "Value in 0.01Lux.", 0, 1000000, 200),
	ZUNO_CONFIG_PARAMETER_INFO("Lumen invert", "If set device sends Basic.off instead of Basic.on", false, true, false),
	ZUNO_CONFIG_PARAMETER_INFO("Lumen threshold", "Value in 0.01Lux.", 0, 10000000, 20000),
	// CO2 sensor settings
	ZUNO_CONFIG_PARAMETER_INFO("C02 hysteresis", "C02 hysteresis", 0, 200, 5),
	ZUNO_CONFIG_PARAMETER_INFO("C02 invert", "If set device sends Basic.off instead of Basic.on", false, true, false),
	ZUNO_CONFIG_PARAMETER_INFO("C02 threshold", "C02 threshold", 400, 5000, 600),
	ZUNO_CONFIG_PARAMETER_INFO("C02 auto", "C02 auto", false, true, true),
	// VOC sensor settings
	ZUNO_CONFIG_PARAMETER_INFO("VOC hysteresis", "VOC hysteresis", 0, 200, 1),
	ZUNO_CONFIG_PARAMETER_INFO("VOC invert", "If set device sends Basic.off instead of Basic.on", false, true, false),
	ZUNO_CONFIG_PARAMETER_INFO("VOC threshold", "VOC threshold", 0, 60000, 660),
    // Noise level sensor settings
	ZUNO_CONFIG_PARAMETER_INFO("Noise level hysteresis", "Noise level hysteresis", 0, 2000, 100),
	ZUNO_CONFIG_PARAMETER_INFO("Noise level invert", "If set device sends Basic.off instead of Basic.on", false, true, false),
	ZUNO_CONFIG_PARAMETER_INFO("Noise level threshold", "Noise level threshold", 3800, 10500, 5000),
	// Motion sensor settings
	ZUNO_CONFIG_PARAMETER_INFO("Motion time", "Motion time", 0, 300, 20),
	ZUNO_CONFIG_PARAMETER_INFO("Motion invert", "If set device sends Basic.off instead of Basic.on", false, true, false),
	ZUNO_CONFIG_PARAMETER_INFO("Motion threshold", "Motion threshold", 0, 1000, 200),
};
// Function return group names. "Dynamic" style is used also
// Only those groups for which there are corresponding channels are created in the device
const char *zunoAssociationGroupName(uint8_t groupIndex) {
	for (uint8_t channel = 0; channel < ZUNO_CFG_CHANNEL_COUNT; channel++)
		if (_channel[channel].groupIndex == groupIndex) {
			switch (_channel[channel].type) {
				case WB_MSW_CHANNEL_TYPE_TEMPERATURE:
					return "Temperature Basic On/Off";
				case WB_MSW_CHANNEL_TYPE_HUMIDITY:
					return "Humidity Basic On/Off";
				case WB_MSW_CHANNEL_TYPE_LUMEN:
					return  "Lumen Basic On/Off";
				case WB_MSW_CHANNEL_TYPE_C02:
					return "CO2 Basic On/Off";
				case WB_MSW_CHANNEL_TYPE_VOC:
					return "VOC Basic On/Off";
				case WB_MSW_CHANNEL_TYPE_NOISE_LEVEL:
					return "Noise level Basic On/Off";
				case WB_MSW_CHANNEL_TYPE_MOTION:
					return "Motion Basic On/Off";
				default:
					return NULL;
			}
		}
	return NULL;
}
// Finds channel of needed type
static WbMswChannel_t *_channelFindType(size_t type) {
	for (size_t channel = 0;channel < ZUNO_CFG_CHANNEL_COUNT; channel++)
		if (_channel[channel].type == type)
			return (&_channel[channel]);
	return 0;
}
//  Function checks whether the device has a specified configuration parameter
const ZunoCFGParameter_t *zunoCFGParameter(size_t param) {
	switch (param) {
		case WB_MSW_CONFIG_PARAMETER_TEMPERATURE_HYSTERESIS:
		case WB_MSW_CONFIG_PARAMETER_TEMPERATURE_INVERT:
		case WB_MSW_CONFIG_PARAMETER_TEMPERATURE_THRESHOLD:
			if (!_channelFindType(WB_MSW_CHANNEL_TYPE_TEMPERATURE))
				return (ZUNO_CFG_PARAMETER_UNKNOWN);
			break ;
		case WB_MSW_CONFIG_PARAMETER_HUMIDITY_HYSTERESIS:
		case WB_MSW_CONFIG_PARAMETER_HUMIDITY_INVERT:
		case WB_MSW_CONFIG_PARAMETER_HUMIDITY_THRESHOLD:
			if (!_channelFindType(WB_MSW_CHANNEL_TYPE_HUMIDITY))
				return (ZUNO_CFG_PARAMETER_UNKNOWN);
			break ;
		case WB_MSW_CONFIG_PARAMETER_LUMEN_HYSTERESIS:
		case WB_MSW_CONFIG_PARAMETER_LUMEN_INVERT:
		case WB_MSW_CONFIG_PARAMETER_LUMEN_THRESHOLD:
			if (!_channelFindType(WB_MSW_CHANNEL_TYPE_LUMEN))
				return (ZUNO_CFG_PARAMETER_UNKNOWN);
			break ;
		case WB_MSW_CONFIG_PARAMETER_C02_HYSTERESIS:
		case WB_MSW_CONFIG_PARAMETER_C02_INVERT:
		case WB_MSW_CONFIG_PARAMETER_C02_THRESHOLD:
		case WB_MSW_CONFIG_PARAMETER_C02_AUTO:
			if (!_channelFindType(WB_MSW_CHANNEL_TYPE_C02))
				return (ZUNO_CFG_PARAMETER_UNKNOWN);
			break ;
		case WB_MSW_CONFIG_PARAMETER_VOC_HYSTERESIS:
		case WB_MSW_CONFIG_PARAMETER_VOC_INVERT:
		case WB_MSW_CONFIG_PARAMETER_VOC_THRESHOLD:
			if (!_channelFindType(WB_MSW_CHANNEL_TYPE_VOC))
				return (ZUNO_CFG_PARAMETER_UNKNOWN);
			break ;
		case WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_HYSTERESIS:
		case WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_INVERT:
		case WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_THRESHOLD:
			if (!_channelFindType(WB_MSW_CHANNEL_TYPE_NOISE_LEVEL))
				return (ZUNO_CFG_PARAMETER_UNKNOWN);
			break ;
		case WB_MSW_CONFIG_PARAMETER_MOTION_TIME:
		case WB_MSW_CONFIG_PARAMETER_MOTION_INVERT:
		case WB_MSW_CONFIG_PARAMETER_MOTION_THRESHOLD:
			if (!_channelFindType(WB_MSW_CHANNEL_TYPE_MOTION))
				return (ZUNO_CFG_PARAMETER_UNKNOWN);
			break ;
		default:
			return (ZUNO_CFG_PARAMETER_UNKNOWN);
			break ;
	}
	return (&config_parameter_init[param - WB_MSW_CONFIG_PARAMETER_FIRST]);
}
// The handler is called when a configuration parameter value updated from Z-Wave controller
static void _configParameterChanged(size_t param, int32_t value) {
	if (param < WB_MSW_CONFIG_PARAMETER_FIRST || param > WB_MSW_CONFIG_PARAMETER_LAST)
		return;
	_config_parameter[param - WB_MSW_CONFIG_PARAMETER_FIRST] = value;
}
// Device configuration parameters initialization
static void _configParameterInit(void) {
	// Load configuration parameters from FLASH memory
	for (size_t i =0; i < WB_MSW_MAX_CONFIG_PARAM; i++)
		_config_parameter[i] = zunoLoadCFGParam(i + WB_MSW_CONFIG_PARAMETER_FIRST);
	// Handler installing
	zunoAttachSysHandler(ZUNO_HANDLER_ZW_CFG, 0, (void*) _configParameterChanged);
}
// Function determines number of available Z-Wave device channels (EndPoints) and fills in the structures by channel type
static size_t _channelInit(WBMSWSensor* wb_msw) {
	size_t							channel;
	size_t							groupIndex;
	bool							C02_enable;
	uint16_t						motion;

	channel = 0;
	groupIndex = CTRL_GROUP_1;
	_channel[channel].triggered = false;
	_channel[channel].reported_value = 0;
	// Temperature channel
	if (wb_msw->getTemperature(_channel[channel].temperature)) {
		// Such channel exists
		if (_channel[channel].temperature != WB_MSW_INPUT_REG_TEMPERATURE_VALUE_ERROR) {
			// Value is valid
			_channel[channel].type = WB_MSW_CHANNEL_TYPE_TEMPERATURE;
			_channel[channel].groupIndex = groupIndex;
			channel++;
			groupIndex++;
		}
	}
	// Humidity channel
	if (wb_msw->getHumidity(_channel[channel].humidity)) {
		if (_channel[channel].humidity != WB_MSW_INPUT_REG_HUMIDITY_VALUE_ERROR) {
			_channel[channel].type = WB_MSW_CHANNEL_TYPE_HUMIDITY;
			_channel[channel].groupIndex = groupIndex;
			channel++;
			groupIndex++;
		}
	}
	// Lumen channel
	if (wb_msw->getLumminance(_channel[channel].lumen)) {
		if (_channel[channel].lumen != WB_MSW_INPUT_REG_LUMEN_VALUE_ERROR) {
			_channel[channel].type = WB_MSW_CHANNEL_TYPE_LUMEN;
			_channel[channel].groupIndex = groupIndex;
			channel++;
			groupIndex++;
		}
	}
	if (wb_msw->getC02Status(C02_enable)) {
		if (C02_enable || wb_msw->setC02Status(true)) {
			if (wb_msw->getC02(_channel[channel].c02)) {
				if (_channel[channel].c02 == WB_MSW_INPUT_REG_C02_VALUE_ERROR)
					_channel[channel].c02 = 0;
				_channel[channel].type = WB_MSW_CHANNEL_TYPE_C02;
				_channel[channel].groupIndex = groupIndex;
				channel++;
				groupIndex++;
			}
		}
	}
	if (wb_msw->getVoc(_channel[channel].voc)) {
		if (_channel[channel].voc != WB_MSW_INPUT_REG_VOC_VALUE_ERROR) {
			_channel[channel].type = WB_MSW_CHANNEL_TYPE_VOC;
			_channel[channel].groupIndex = groupIndex;
			channel++;
			groupIndex++;
		}
	}
	if (wb_msw->getNoiseLevel(_channel[channel].noise_level)) {
		_channel[channel].type = WB_MSW_CHANNEL_TYPE_NOISE_LEVEL;
		_channel[channel].groupIndex = groupIndex;
		channel++;
		groupIndex++;
	}
	if (wb_msw->getMotion(motion)) {
		if (motion != WB_MSW_INPUT_REG_MOTION_VALUE_ERROR) {
			_channel[channel].bMotion = false;
			_channel[channel].type = WB_MSW_CHANNEL_TYPE_MOTION;
			_channel[channel].groupIndex = groupIndex;
			channel++;
			groupIndex++;
		}
	}
	return channel;
}
// Setting up Z-Wave channels, setting Multichannel indexes
static void _channelSet(size_t channel_max) {
	for (size_t channel = 0; channel < channel_max; channel++)
		switch (_channel[channel].type) {
			case WB_MSW_CHANNEL_TYPE_TEMPERATURE:
				zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER, ZUNO_SENSOR_MULTILEVEL_TYPE_TEMPERATURE, (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_CELSIUS,
				WB_MSW_INPUT_REG_TEMPERATURE_VALUE_SIZE, WB_MSW_INPUT_REG_TEMPERATURE_VALUE_PRECISION)));
				zunoSetZWChannel(channel, channel + 1);
				zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
				break ;
			case WB_MSW_CHANNEL_TYPE_HUMIDITY:
				zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER, ZUNO_SENSOR_MULTILEVEL_TYPE_RELATIVE_HUMIDITY, (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_PERCENTAGE_VALUE,
				WB_MSW_INPUT_REG_HUMIDITY_VALUE_SIZE, WB_MSW_INPUT_REG_HUMIDITY_VALUE_PRECISION)));
				zunoSetZWChannel(channel, channel + 1);
				zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
				break ;
			case WB_MSW_CHANNEL_TYPE_LUMEN:
				zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER, ZUNO_SENSOR_MULTILEVEL_TYPE_LUMINANCE, (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_LUX,
				WB_MSW_INPUT_REG_LUMEN_VALUE_SIZE, WB_MSW_INPUT_REG_LUMEN_VALUE_PRECISION)));
				zunoSetZWChannel(channel, channel + 1);
				zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
				break ;
			case WB_MSW_CHANNEL_TYPE_C02:
				zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER, ZUNO_SENSOR_MULTILEVEL_TYPE_CO2_LEVEL, (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_PARTS_PER_MILLION,
				WB_MSW_INPUT_REG_C02_VALUE_SIZE, WB_MSW_INPUT_REG_C02_VALUE_PRECISION)));
				zunoSetZWChannel(channel, channel + 1);
				zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
				break ;
			case WB_MSW_CHANNEL_TYPE_VOC:
				zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER, ZUNO_SENSOR_MULTILEVEL_TYPE_VOLATILE_ORGANIC_COMPOUND, (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(1,
				WB_MSW_INPUT_REG_VOC_VALUE_SIZE, WB_MSW_INPUT_REG_VOC_VALUE_PRECISION)));
				zunoSetZWChannel(channel, channel + 1);
				zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
				break ;
			case WB_MSW_CHANNEL_TYPE_NOISE_LEVEL:
				zunoAddChannel(ZUNO_SENSOR_MULTILEVEL_CHANNEL_NUMBER, ZUNO_SENSOR_MULTILEVEL_TYPE_LOUDNESS, (SENSOR_MULTILEVEL_PROPERTIES_COMBINER(SENSOR_MULTILEVEL_SCALE_DECIBELS,
				WB_MSW_INPUT_REG_NOISE_LEVEL_VALUE_SIZE, WB_MSW_INPUT_REG_NOISE_LEVEL_PRECISION)));
				zunoSetZWChannel(channel, channel + 1);
				zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
				break ;
			case WB_MSW_CHANNEL_TYPE_MOTION:
				zunoAddChannel(ZUNO_SENSOR_BINARY_CHANNEL_NUMBER, ZUNO_SENSOR_BINARY_TYPE_MOTION, 0);
				zunoSetZWChannel(channel, channel + 1);
				zunoAddAssociation(ZUNO_ASSOC_BASIC_SET_NUMBER, 0);
				break ;
		}
	if (ZUNO_CFG_CHANNEL_COUNT > 1)
		zunoSetZWChannel(0, 1 | ZWAVE_CHANNEL_MAPPED_BIT);
}
// Setting up handlers for all sensor cannels. Handler is used when requesting channel data from controller
void _channelSetHandler(WBMSWSensor* wb_msw, uint8_t channel_max) {
	for (size_t channel = 0; channel < channel_max; channel++)
		switch (_channel[channel].type) {
			case WB_MSW_CHANNEL_TYPE_TEMPERATURE:
				zunoAppendChannelHandler(channel, WB_MSW_INPUT_REG_TEMPERATURE_VALUE_SIZE, CHANNEL_HANDLER_SINGLE_VALUEMAPPER, (void*)&_channel[channel].temperature);
				break ;
			case WB_MSW_CHANNEL_TYPE_HUMIDITY:
				zunoAppendChannelHandler(channel, WB_MSW_INPUT_REG_HUMIDITY_VALUE_SIZE, CHANNEL_HANDLER_SINGLE_VALUEMAPPER, (void*)&_channel[channel].humidity);
				break ;
			case WB_MSW_CHANNEL_TYPE_LUMEN:
				zunoAppendChannelHandler(channel, WB_MSW_INPUT_REG_LUMEN_VALUE_SIZE, CHANNEL_HANDLER_SINGLE_VALUEMAPPER, (void*)&_channel[channel].lumen);
				break ;
			case WB_MSW_CHANNEL_TYPE_C02:
				_c02_auto = WB_MSW_CONFIG_PARAMETER_GET(WB_MSW_CONFIG_PARAMETER_C02_AUTO);
				wb_msw->setC02Autocalibration(_c02_auto);
				zunoAppendChannelHandler(channel, WB_MSW_INPUT_REG_C02_VALUE_SIZE, CHANNEL_HANDLER_SINGLE_VALUEMAPPER, (void*)&_channel[channel].c02);
				break ;
			case WB_MSW_CHANNEL_TYPE_VOC:
				zunoAppendChannelHandler(channel, WB_MSW_INPUT_REG_C02_VALUE_SIZE, CHANNEL_HANDLER_SINGLE_VALUEMAPPER, (void*)&_channel[channel].voc);
				break ;
			case WB_MSW_CHANNEL_TYPE_NOISE_LEVEL:
				zunoAppendChannelHandler(channel, WB_MSW_INPUT_REG_NOISE_LEVEL_VALUE_SIZE, CHANNEL_HANDLER_SINGLE_VALUEMAPPER, (void*)&_channel[channel].noise_level);
				break ;
			case WB_MSW_CHANNEL_TYPE_MOTION:
				zunoAppendChannelHandler(channel, 1, CHANNEL_HANDLER_SINGLE_VALUEMAPPER, (void*)&_channel[channel].bMotion);
				break ;
		}
}

// Static function where system events arrive
static void _systemEvent(ZUNOSysEvent_t *ev) {
	switch(ev->event){
		// A new firmware image for the second chip from the Z-Wave controller has arrived
		case ZUNO_SYS_EVENT_OTA_IMAGE_READY:
			if (ev->params[0] == 0) {
				_fw.size =  ev->params[1];
				_fw.bUpdate = true;
			}
			break;
	}
}

void processAnalogSensorValue(int32_t current_value, uint8_t chi){
	uint8_t hyst_param  	= WB_TYPE2PARAM_MAPPER[_channel[chi].type];

	// The parameters should ALWAYS go in the order "hysteresis", "inverted logic", "threshold"
	int32_t hyst 	= WB_MSW_CONFIG_PARAMETER_GET(hyst_param);
	bool 	inv 	= WB_MSW_CONFIG_PARAMETER_GET(hyst_param+1);
	int32_t thres 	= WB_MSW_CONFIG_PARAMETER_GET(hyst_param+2);
	if((hyst != 0) && (abs(current_value - _channel[chi].reported_value) > hyst)){
		_channel[chi].reported_value = current_value; // Remember last sent value
		zunoSendReport(chi + 1); // Device channels are counted from 1
	}
	// Is the threshold exceeded?
	if(_channel[chi].triggered) {
		if((current_value + hyst) < thres){
			_channel[chi].triggered = false; // No exceed
			// Sent to the Basic.Off group (On with inverted logic)
			zunoSendToGroupSetValueCommand(_channel[chi].groupIndex, (!inv) ? WB_MSW_OFF: WB_MSW_ON);
		}
	} else {
		if((current_value - hyst) > thres){
			_channel[chi].triggered = true; // Exceed
			// Sent to the Basic.On group (Off with inverted logic)
			zunoSendToGroupSetValueCommand(_channel[chi].groupIndex, (!inv) ? WB_MSW_ON: WB_MSW_OFF);
		}
	}
}
// Processing of various types of sensors
void processTemperature(WBMSWSensor* wb_msw, size_t channel) {
	int16_t	 currentTemperature;
	if (!wb_msw->getTemperature(currentTemperature))
		return;
	if(currentTemperature == WB_MSW_INPUT_REG_TEMPERATURE_VALUE_ERROR)
		return;
	LOG_FIXEDPOINT_VALUE("Temperature:        ", currentTemperature, 2);
	_channel[channel].temperature = currentTemperature;
	processAnalogSensorValue(currentTemperature, channel);
}
void processHumidity(WBMSWSensor* wb_msw, size_t channel) {
	uint16_t	currentHumidity;
	if (!wb_msw->getHumidity(currentHumidity))
		return;
	if (currentHumidity == WB_MSW_INPUT_REG_HUMIDITY_VALUE_ERROR)
		return;
	LOG_FIXEDPOINT_VALUE("Humidity:           ", currentHumidity, 2);
	_channel[channel].humidity = currentHumidity;
	processAnalogSensorValue(currentHumidity, channel);
}
void processLumen(WBMSWSensor* wb_msw, size_t channel) {
	uint32_t	currentLumen;
	if (!wb_msw->getLumminance(currentLumen))
		return;
	if(currentLumen == WB_MSW_INPUT_REG_LUMEN_VALUE_ERROR)
		return;
	LOG_FIXEDPOINT_VALUE("Lumen:              ", currentLumen, 2);
	_channel[channel].lumen = currentLumen;
	processAnalogSensorValue(currentLumen, channel);
}
void processC02(WBMSWSensor* wb_msw, size_t channel) {
	uint16_t	currentC02;
	uint8_t		c02_auto;
	// Check if automatic calibration is needed
	c02_auto = WB_MSW_CONFIG_PARAMETER_GET(WB_MSW_CONFIG_PARAMETER_C02_AUTO);
	if (_c02_auto != c02_auto) {
		_c02_auto = c02_auto;
		wb_msw->setC02Autocalibration(c02_auto);
	}
	if (!wb_msw->getC02(currentC02))
		return;
	if (currentC02 == WB_MSW_INPUT_REG_C02_VALUE_ERROR)
		return;
	LOG_INT_VALUE("C02:                ", currentC02);
	_channel[channel].c02 = currentC02;
	processAnalogSensorValue(currentC02, channel);
}
void processVOC(WBMSWSensor* wb_msw, size_t channel) {
	uint16_t currentVoc;

	if (!wb_msw->getVoc(currentVoc))
		return;
	if (currentVoc == WB_MSW_INPUT_REG_VOC_VALUE_ERROR)
		return;
	LOG_INT_VALUE("VOC:                ", currentVoc);
	_channel[channel].voc = currentVoc;
	processAnalogSensorValue(currentVoc, channel);
}
void processNoiseLevel(WBMSWSensor* wb_msw, size_t channel) {
	uint16_t currentNoiseLevel;
	if (!wb_msw->getNoiseLevel(currentNoiseLevel))
		return;
	LOG_FIXEDPOINT_VALUE("Noise Level:        ", currentNoiseLevel, 2);
	_channel[channel].noise_level = currentNoiseLevel;
	processAnalogSensorValue(currentNoiseLevel, channel);
}
void processMotion(WBMSWSensor* wb_msw, size_t channel) {
	static uint32_t					ms = 0;
	uint16_t						currentMotion;
	size_t							bMotion;
	bool 							inv = WB_MSW_CONFIG_PARAMETER_GET(WB_MSW_CONFIG_PARAMETER_MOTION_INVERT);

	if ((bMotion = _channel[channel].bMotion))
		if (millis() >= ms && abs(millis() - ms) < 16777215)
			_channel[channel].bMotion = false;
	if (!_channel[channel].bMotion) {
		if (!wb_msw->getMotion(currentMotion))
			return;
		if (currentMotion == WB_MSW_INPUT_REG_MOTION_VALUE_ERROR)
			return;
		LOG_INT_VALUE("Motion:             ", currentMotion);
		if (currentMotion >= (size_t)WB_MSW_CONFIG_PARAMETER_GET(WB_MSW_CONFIG_PARAMETER_MOTION_THRESHOLD)) {
			ms = millis() + (WB_MSW_CONFIG_PARAMETER_GET(WB_MSW_CONFIG_PARAMETER_MOTION_TIME) * 1000);
			_channel[channel].bMotion = true;
			zunoSendReport(channel + 1);
			zunoSendToGroupSetValueCommand(_channel[channel].groupIndex, (!inv) ? WB_MSW_ON: WB_MSW_OFF);
		}else if (bMotion) {
			zunoSendReport(channel + 1);
			zunoSendToGroupSetValueCommand(_channel[channel].groupIndex, (!inv) ? WB_MSW_OFF: WB_MSW_ON);
		}
	}
}

// For update firmware version
static bool updateOtaDesriptor(WBMSWSensor* wb_msw) {
	uint32_t version;

	if (!wb_msw->getFwVersion(&version)){
		#ifdef LOGGING_DBG
		LOGGING_UART.print("*** (!!!) Can't connect to WB chip. It doesn't answer to version request!\n");
		#endif
		return false;
	}
	g_OtaDesriptor.version = version;
	#ifdef LOGGING_DBG
	LOGGING_UART.print("FW:                 ");
	LOGGING_UART.println(version, 0x10);
	#endif
	return true;
}

// Device channel management and firmware data transfer
static void processChannels(WBMSWSensor* wb_msw) {
	#ifdef LOGGING_DBG
	//LOGGING_UART.println("--------------------Measurements-----------------------");
	#endif
	// Check all channels of available sensors
	for (size_t channel=0; channel < ZUNO_CFG_CHANNEL_COUNT; channel++)
		switch (_channel[channel].type) {
			case WB_MSW_CHANNEL_TYPE_TEMPERATURE:
				processTemperature(wb_msw, channel);
				break ;
			case WB_MSW_CHANNEL_TYPE_HUMIDITY:
				processHumidity(wb_msw, channel);
				break ;
			case WB_MSW_CHANNEL_TYPE_LUMEN:
				processLumen(wb_msw, channel);
				break ;
			case WB_MSW_CHANNEL_TYPE_C02:
				processC02(wb_msw, channel);
				break ;
			case WB_MSW_CHANNEL_TYPE_VOC:
				processVOC(wb_msw, channel);
				break ;
			case WB_MSW_CHANNEL_TYPE_NOISE_LEVEL:
				processNoiseLevel(wb_msw, channel);
				break ;
			case WB_MSW_CHANNEL_TYPE_MOTION:
				processMotion(wb_msw, channel);
				break ;
			default:
				break ;
		}
	// If a new firmware came on the radio, send it to the bootloder of the WB chip
	// IMPORTANT: We do it here, not in the system event handler!
	if (_fw.bUpdate) {
		wb_msw->fwUpdate((void *)WB_MSW_UPDATE_ADDRESS, _fw.size);
		_fw.bUpdate = false;
		updateOtaDesriptor(wb_msw);
	}
}

// The function is called at the start of the sketch
void setup() {
	ZUnoState = TZUnoState::ZUNO_OK;

	wb_msw = new WBMSWSensor(&Serial1, WB_MSW_TIMEOUT);
	
	// Connecting to the WB sensor
	if (!wb_msw->OpenPort(WB_MSW_UART_BAUD, WB_MSW_UART_MODE, WB_MSW_UART_RX, WB_MSW_UART_TX)) {
		ZUnoState = TZUnoState::ZUNO_OPEN_PORT_ERROR;
		return;
	}

	uint16_t Address = 0;
	bool Success = false;

	while ((Address <= 247) && !Success){
		wb_msw->SetModbusAddress(Address);
		if (updateOtaDesriptor(wb_msw)){
			Success = true;
		}
	}

	if (!Success){
		ZUnoState = TZUnoState::ZUNO_WB_SENSOR_NOT_FOUND;
		return;
	}
	
	// Initializing channels
	size_t channel_count = _channelInit(wb_msw);
	if (!channel_count){
		ZUnoState = TZUnoState::ZUNO_WB_SENSOR_NO_CHANNELS;
		return;
	}

	// Initialize the configuration parameters (depending on the number of channels)
	_configParameterInit();
	if(zunoStartDeviceConfiguration()) {
		// If the device is offline, set the channel configuration
		_channelSet(channel_count);
		zunoSetS2Keys((SKETCH_FLAG_S2_AUTHENTICATED_BIT | SKETCH_FLAG_S2_UNAUTHENTICATED_BIT | SKETCH_FLAG_S0_BIT));
		zunoCommitCfg(); // Transfer the received configuration to the system
	}
	// Install Z Wave handlers for channels
	_channelSetHandler(wb_msw, channel_count);
	// Install a system event handler (needed for firmware updates)
	zunoAttachSysHandler(ZUNO_HANDLER_SYSEVENT, 0, (void*) &_systemEvent);
}
// Main loop
void loop() {
	switch (ZUnoState) {
  		case ZUNO_OK:
    		processChannels(wb_msw);
			delay(50);
    		break;
  		case ZUNO_OPEN_PORT_ERROR:
    		#ifdef LOGGING_DBG
			LOGGING_UART.print("*** Fatal ERROR! Can't open port!\n");
			#endif
			delay(1000);
    		break;
		case ZUNO_WB_SENSOR_NOT_FOUND:
			#ifdef LOGGING_DBG
			LOGGING_UART.print("*** Fatal ERROR! WB sensor not found!\n");
			#endif
			delay(1000);
    		break;
		case ZUNO_WB_SENSOR_NO_CHANNELS:
			#ifdef LOGGING_DBG
			LOGGING_UART.print("*** (!!!) WB chip doesn't support any kind of sensors!\n");
			#endif
			delay(1000);
    		break;
  		default:
    		break;
}
	
}