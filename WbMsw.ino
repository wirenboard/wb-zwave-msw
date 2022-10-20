
#include "WbMsw.h"
#include "WBMSWSensor.h"
#include "Debug.h"

// Здесь определяются глобальные макросы для всего проекта.
//  С помощью системных макросов тонко настраивается работа протокола
ZUNO_ENABLE(
		/* Поддержка классов команд. Указывается здесь так как каналы устройства создаются динамически */
		WITH_CC_MULTICHANNEL 
		WITH_CC_CONFIGURATION
		WITH_CC_SENSOR_MULTILEVEL
		WITH_CC_NOTIFICATION
		// SKETCH_FLAGS=(HEADER_FLAGS_NOSKETCH_OTA)
		ZUNO_CUSTOM_OTA_OFFSET=0x10000 // 64 кБ
		/* Количество дополнительных прошивок OTA */
		ZUNO_EXT_FIRMWARES_COUNT=1
		/* Указатель на дескриптор прошивки */
		ZUNO_EXT_FIRMWARES_DESCR_PTR=&g_OtaDesriptor
		LOGGING_DBG // Закоментировать если не нужна отладочная информация. 
					// По умолчанию отладочная информация печатается вместе с выводом системной консоли RTOS в UART0 (TX0)
		//LOGGING_UART=Serial
		
	);
	// 
// Отладочная печать значений
// DBG
#ifdef LOGGING_DBG
#define LOG_INT_VALUE(TEXT, VALUE) LOGGING_UART.print(TEXT); LOGGING_UART.print(VALUE); LOGGING_UART.print("\n");
#define LOG_FIXEDPOINT_VALUE(TEXT, VALUE, PREC) LOGGING_UART.print(TEXT); LOGGING_UART.fixPrint(VALUE, PREC); LOGGING_UART.print("\n");
#else
#define LOG_INT_VALUE(TEXT, VALUE)
#define LOG_FIXEDPOINT_VALUE(TEXT, VALUE, PREC) 
#endif
/* ZUNO_DECLARE - определяет глобальные для всего проекта EXTERN, дескриптор должен быть виден во всех файлах проекта, чтобы правильно все собралось.*/
ZUNO_DECLARE(ZUNOOTAFWDescr_t g_OtaDesriptor);

/* Значения дескриптора прошивки чипа WB */
// ZUNOOTAFWDescr_t g_OtaDesriptor = {0x010A, 0x0103};
ZUNOOTAFWDescr_t g_OtaDesriptor = {0x0101, 0x0103};
WBMSWSensor wb_msw(&Serial1, WB_MSW_TIMEOUT, WB_MSW_ADDRES);
static WbMswChannel_t _channel[WB_MSW_CHANNEL_MAX];
static int32_t _config_parameter[WB_MSW_MAX_CONFIG_PARAM];
static uint8_t _c02_auto = false;
// Сюда сохраняются значения образа новой прошивки
static WbMswFw_t_t _fw = {
	.size = 0,
	.bUpdate = false
};
// Описание всех доступных в устройстве параметров. 
// Каналы в устройстве создаются динамически, поэтому и параметры описываются в "динамическом" стиле
static const ZunoCFGParameter_t config_parameter_init[WB_MSW_MAX_CONFIG_PARAM] = {
	// Настройки канала температуры
	ZUNO_CONFIG_PARAMETER_INFO("Temperature hysteresis",  // Название параметра
							  "If 0 reports are not sent.Value in 0.01*C",   // Детальное описание параметра
							  0,                          // Минимальное допустимое значение
							  2000,                         // Максимальное допустимое значение
							  100),                         // Значение по умолчанию
	ZUNO_CONFIG_PARAMETER_INFO("Temperature invert", "If set device sends Basic.off instead of Basic.on", false, true, false),
	ZUNO_CONFIG_PARAMETER_INFO_SIGN("Temperature threshold", ".Value in 0.01*C", -4000, 8000, 4000),
	// Настройки датчика влажности
	ZUNO_CONFIG_PARAMETER_INFO("Humidity hysteresis", "If 0 reports are not sent.Value in 0.01%", 0, 2000, 100),
	ZUNO_CONFIG_PARAMETER_INFO("Humidity invert", "If set device sends Basic.off instead of Basic.on", false, true, false),
	ZUNO_CONFIG_PARAMETER_INFO("Humidity threshold", "Value in 0.01%", 0, 10000, 5000),
    // Настройки датчика освещенности
	ZUNO_CONFIG_PARAMETER_INFO("Lumen hysteresis", "Value in 0.01Lux.", 0, 1000000, 200),
	ZUNO_CONFIG_PARAMETER_INFO("Lumen invert", "If set device sends Basic.off instead of Basic.on", false, true, false),
	ZUNO_CONFIG_PARAMETER_INFO("Lumen threshold", "Value in 0.01Lux.", 0, 10000000, 20000),
	// Настройки датчика CO2
	ZUNO_CONFIG_PARAMETER_INFO("C02 hysteresis", "C02 hysteresis", 0, 200, 5),
	ZUNO_CONFIG_PARAMETER_INFO("C02 invert", "If set device sends Basic.off instead of Basic.on", false, true, false),
	ZUNO_CONFIG_PARAMETER_INFO("C02 threshold", "C02 threshold", 400, 5000, 600),
	ZUNO_CONFIG_PARAMETER_INFO("C02 auto", "C02 auto", false, true, true),
	// Настройки датчика VOC
	ZUNO_CONFIG_PARAMETER_INFO("VOC hysteresis", "VOC hysteresis", 0, 200, 1),
	ZUNO_CONFIG_PARAMETER_INFO("VOC invert", "If set device sends Basic.off instead of Basic.on", false, true, false),
	ZUNO_CONFIG_PARAMETER_INFO("VOC threshold", "VOC threshold", 0, 60000, 660),
    // Настройки датчика измерения шума
	ZUNO_CONFIG_PARAMETER_INFO("Noise level hysteresis", "Noise level hysteresis", 0, 2000, 100),
	ZUNO_CONFIG_PARAMETER_INFO("Noise level invert", "If set device sends Basic.off instead of Basic.on", false, true, false),
	ZUNO_CONFIG_PARAMETER_INFO("Noise level threshold", "Noise level threshold", 3800, 10500, 5000),
	// Настройки датчика движения 
	ZUNO_CONFIG_PARAMETER_INFO("Motion time", "Motion time", 0, 300, 20),
	ZUNO_CONFIG_PARAMETER_INFO("Motion invert", "If set device sends Basic.off instead of Basic.on", false, true, false),
	ZUNO_CONFIG_PARAMETER_INFO("Motion threshold", "Motion threshold", 0, 1000, 200),
};
// Функция возвращает названия групп. Также используется "динамический" подход. 
// В устройстве создаются ТОЛЬКО те группы для которых есть соответсвующие каналы
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
// Находим канал нужного типа.
static WbMswChannel_t *_channelFindType(size_t type) {
	for (size_t channel = 0;channel < ZUNO_CFG_CHANNEL_COUNT; channel++) 
		if (_channel[channel].type == type)
			return (&_channel[channel]);
	return 0;
}
// Функция проверяющаю есть ли у устройства заданный конфигурационный параметр
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
// Обработчик вызывается при обновлении значения кофигурационного параметра с контроллера Z-Wave
static void _configParameterChanged(size_t param, int32_t value) {
	if (param < WB_MSW_CONFIG_PARAMETER_FIRST || param > WB_MSW_CONFIG_PARAMETER_LAST)
		return;
	_config_parameter[param - WB_MSW_CONFIG_PARAMETER_FIRST] = value;
}
// Инициализация конфигурационных параметров устройства
static void _configParameterInit(void) {
	// Загружаем кофигурационные параметры из FLASH-памяти
	for (size_t i =0; i < WB_MSW_MAX_CONFIG_PARAM; i++) 
		_config_parameter[i] = zunoLoadCFGParam(i + WB_MSW_CONFIG_PARAMETER_FIRST);
	// Устанавливаем обработчик
	zunoAttachSysHandler(ZUNO_HANDLER_ZW_CFG, 0, (void*) _configParameterChanged);
}
// Функция инициализирует определяется количество доступных Z-Wave-каналов(EndPoints) устройства и заполняет структуры по типу каналов
static size_t _channelInit(void) {
	size_t							channel;
	size_t							groupIndex;
	bool							C02_enable;
	uint16_t						motion;

	channel = 0;
	groupIndex = CTRL_GROUP_1;
	_channel[channel].triggered = false;
	_channel[channel].reported_value = 0;
	// Канал температуры
	if (wb_msw.getTemperature(_channel[channel].temperature)) {
		// Такой канал существует
		if (_channel[channel].temperature != WB_MSW_INPUT_REG_TEMPERATURE_VALUE_ERROR) {
			// Значение валидно
			_channel[channel].type = WB_MSW_CHANNEL_TYPE_TEMPERATURE;
			_channel[channel].groupIndex = groupIndex;
			channel++;
			groupIndex++;
		}
	}
	// Канал влажности
	if (wb_msw.getHumidity(_channel[channel].humidity)) {
		if (_channel[channel].humidity != WB_MSW_INPUT_REG_HUMIDITY_VALUE_ERROR) {
			_channel[channel].type = WB_MSW_CHANNEL_TYPE_HUMIDITY;
			_channel[channel].groupIndex = groupIndex;
			channel++;
			groupIndex++;
		}
	}
	// Канал освещенности
	if (wb_msw.getLumminance(_channel[channel].lumen)) {
		if (_channel[channel].lumen != WB_MSW_INPUT_REG_LUMEN_VALUE_ERROR) {
			_channel[channel].type = WB_MSW_CHANNEL_TYPE_LUMEN;
			_channel[channel].groupIndex = groupIndex;
			channel++;
			groupIndex++;
		}
	}
	if (wb_msw.getC02Status(C02_enable)) {
		if (C02_enable || wb_msw.setC02Status(true)) {
			if (wb_msw.getC02(_channel[channel].c02)) {
				if (_channel[channel].c02 == WB_MSW_INPUT_REG_C02_VALUE_ERROR)
					_channel[channel].c02 = 0;
				_channel[channel].type = WB_MSW_CHANNEL_TYPE_C02;
				_channel[channel].groupIndex = groupIndex;
				channel++;
				groupIndex++;
			}
		}
	}
	if (wb_msw.getVoc(_channel[channel].voc)) {
		if (_channel[channel].voc != WB_MSW_INPUT_REG_VOC_VALUE_ERROR) {
			_channel[channel].type = WB_MSW_CHANNEL_TYPE_VOC;
			_channel[channel].groupIndex = groupIndex;
			channel++;
			groupIndex++;
		}
	}
	if (wb_msw.getNoiseLevel(_channel[channel].noise_level)) {
		_channel[channel].type = WB_MSW_CHANNEL_TYPE_NOISE_LEVEL;
		_channel[channel].groupIndex = groupIndex;
		channel++;
		groupIndex++;
	}
	if (wb_msw.getMotion(motion)) {
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
// Настраиваем каналы Z-Wave, задаем индексы Multichannel
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
// Устанавливает хэндлеры для всех каналов датчиков. Хэндлер используется при запросе данных канала с контроллера
void _channelSetHandler(uint8_t channel_max) {
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
				wb_msw.setC02Autocalibration(_c02_auto);
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

// Статическая функция, куда приходят системные события
static void _systemEvent(ZUNOSysEvent_t *ev) {
	switch(ev->event){
		// Пришел новый образ прошивки для второго чипа от контроллера Z-Wave
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

	// Параметры ВСЕГДА должны идти в порядке "гистерезис", "инвертированная логика", "порог"
	int32_t hyst 	= WB_MSW_CONFIG_PARAMETER_GET(hyst_param);
	bool 	inv 	= WB_MSW_CONFIG_PARAMETER_GET(hyst_param+1);
	int32_t thres 	= WB_MSW_CONFIG_PARAMETER_GET(hyst_param+2);
	if((hyst != 0) && (abs(current_value - _channel[chi].reported_value) > hyst)){
		_channel[chi].reported_value = current_value; // Запоминаем последнее отправленное значение
		zunoSendReport(chi + 1); // Каналы устройства отсчитываются от 1-цы
	}
	// Было уже превышение порога ?
	if(_channel[chi].triggered) {
		if((current_value + hyst) < thres){
			_channel[chi].triggered = false; // Нет превышения
			// Отправляем в группу Basic.Off (On при инвертированной логики)
			zunoSendToGroupSetValueCommand(_channel[chi].groupIndex, (!inv) ? WB_MSW_OFF: WB_MSW_ON);
		}
	} else {
		if((current_value - hyst) > thres){
			_channel[chi].triggered = true; // Сработало превышение 
			// Отправляем в группу Basic.On (Off при инвертированной логики)
			zunoSendToGroupSetValueCommand(_channel[chi].groupIndex, (!inv) ? WB_MSW_ON: WB_MSW_OFF);
		}
	}
}
// Обработка различных типов сенсоров
void processTemperature(size_t channel) {
	int16_t	 currentTemperature;
	if (!wb_msw.getTemperature(currentTemperature)) 
		return;
	if(currentTemperature == WB_MSW_INPUT_REG_TEMPERATURE_VALUE_ERROR)
		return;
	LOG_FIXEDPOINT_VALUE("Temperature:        ", currentTemperature, 2);
	_channel[channel].temperature = currentTemperature;
	processAnalogSensorValue(currentTemperature, channel);
}
void processHumidity(size_t channel) {
	uint16_t	currentHumidity;
	if (!wb_msw.getHumidity(currentHumidity)) 
		return;
	if (currentHumidity == WB_MSW_INPUT_REG_HUMIDITY_VALUE_ERROR)
		return;
	LOG_FIXEDPOINT_VALUE("Humidity:           ", currentHumidity, 2);
	_channel[channel].humidity = currentHumidity;
	processAnalogSensorValue(currentHumidity, channel);
}
void processLumen(size_t channel) {
	uint32_t	currentLumen;
	if (!wb_msw.getLumminance(currentLumen))
		return;
	if(currentLumen == WB_MSW_INPUT_REG_LUMEN_VALUE_ERROR)
		return;
	LOG_FIXEDPOINT_VALUE("Lumen:              ", currentLumen, 2);
	_channel[channel].lumen = currentLumen;
	processAnalogSensorValue(currentLumen, channel);
}
void processC02(size_t channel) {
	uint16_t	currentC02;
	uint8_t		c02_auto;
	// Проверяем нужна ли автоматичнская калибрация
	c02_auto = WB_MSW_CONFIG_PARAMETER_GET(WB_MSW_CONFIG_PARAMETER_C02_AUTO);
	if (_c02_auto != c02_auto) {
		_c02_auto = c02_auto;
		wb_msw.setC02Autocalibration(c02_auto);
	}
	if (!wb_msw.getC02(currentC02)) 
		return;
	if (currentC02 == WB_MSW_INPUT_REG_C02_VALUE_ERROR) 
		return;
	LOG_INT_VALUE("C02:                ", currentC02);
	_channel[channel].c02 = currentC02;
	processAnalogSensorValue(currentC02, channel);
}
void processVOC(size_t channel) {
	uint16_t currentVoc;

	if (!wb_msw.getVoc(currentVoc)) 
		return;
	if (currentVoc == WB_MSW_INPUT_REG_VOC_VALUE_ERROR)
		return;
	LOG_INT_VALUE("VOC:                ", currentVoc);
	_channel[channel].voc = currentVoc;
	processAnalogSensorValue(currentVoc, channel);
}
void processNoiseLevel(size_t channel) {
	uint16_t currentNoiseLevel;
	if (!wb_msw.getNoiseLevel(currentNoiseLevel))
		return;
	LOG_FIXEDPOINT_VALUE("Noise Level:        ", currentNoiseLevel, 2);
	_channel[channel].noise_level = currentNoiseLevel;
	processAnalogSensorValue(currentNoiseLevel, channel);
}
void processMotion(size_t channel) {
	static uint32_t					ms = 0;
	uint16_t						currentMotion;
	size_t							bMotion;
	bool 							inv = WB_MSW_CONFIG_PARAMETER_GET(WB_MSW_CONFIG_PARAMETER_MOTION_INVERT);

	if ((bMotion = _channel[channel].bMotion))
		if (millis() >= ms && abs(millis() - ms) < 16777215)
			_channel[channel].bMotion = false;
	if (!_channel[channel].bMotion) {
		if (!wb_msw.getMotion(currentMotion))
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

// Для обновления верии прошивки
static void updateOtaDesriptor(void) {
	uint32_t					version;

	if (!wb_msw.getFwVersion(&version)){
		#ifdef LOGGING_DBG
		LOGGING_UART.print("*** (!!!) Can't connect to WB chip. It doesn't answer to version request!\n");
		#endif
		return ;
	}
	g_OtaDesriptor.version = version;
	#ifdef LOGGING_DBG
	LOGGING_UART.print("FW:                 ");
	LOGGING_UART.println(version, 0x10);
	#endif
}

// Управление каналами устройства и передача данных прошивки
static void processChannels(void) {
	#ifdef LOGGING_DBG
	//LOGGING_UART.println("--------------------Measurements-----------------------");
	#endif
	// Проход по всем каналам доступных сенсоров
	for (size_t channel=0; channel < ZUNO_CFG_CHANNEL_COUNT; channel++)
		switch (_channel[channel].type) {
			case WB_MSW_CHANNEL_TYPE_TEMPERATURE:
				processTemperature(channel);
				break ;
			case WB_MSW_CHANNEL_TYPE_HUMIDITY:
				processHumidity(channel);
				break ;
			case WB_MSW_CHANNEL_TYPE_LUMEN:
				processLumen(channel);
				break ;
			case WB_MSW_CHANNEL_TYPE_C02:
				processC02(channel);
				break ;
			case WB_MSW_CHANNEL_TYPE_VOC:
				processVOC(channel);
				break ;
			case WB_MSW_CHANNEL_TYPE_NOISE_LEVEL:
				processNoiseLevel(channel);
				break ;
			case WB_MSW_CHANNEL_TYPE_MOTION:
				processMotion(channel);
				break ;
			default:
				break ;
		}
	// Если пришла по радио новая прошивка - отправляем ее в бутлодер чипа WB
	// ВАЖНО: Делаем это здесь, а не в хэдлере системных событий!
	if (_fw.bUpdate) {
		wb_msw.fwUpdate((void *)WB_MSW_UPDATE_ADDRESS, _fw.size);
		_fw.bUpdate = false;
		updateOtaDesriptor();
	}
}

// Функция вызывается при старте скетча
void setup() {
	size_t channel_count;
	// Подключаемся к датчику WB 
	if (!wb_msw.begin(WB_MSW_UART_BAUD, WB_MSW_UART_MODE, WB_MSW_UART_RX, WB_MSW_UART_TX)) {
		while (1) {
			#ifdef LOGGING_DBG
			LOGGING_UART.print("*** Fatal ERROR! Can't connect to WB!\n");
			#endif
			delay(500);
		}
	}
	updateOtaDesriptor();
	// Инициализируем каналы
	channel_count = _channelInit();
	if (channel_count == 0){
		#ifdef LOGGING_DBG
		LOGGING_UART.print("*** (!!!) WB chip doesn't support any kind of sensors!\n");
		#endif
	}
	// Инициализируем конфигурационные параметры (зависят от количества каналов)
	_configParameterInit();
	if(zunoStartDeviceConfiguration()) {
		// Если устройство не в сети - устанавливаем конфигурацию каналов
		_channelSet(channel_count);
		zunoSetS2Keys((SKETCH_FLAG_S2_AUTHENTICATED_BIT | SKETCH_FLAG_S2_UNAUTHENTICATED_BIT | SKETCH_FLAG_S0_BIT));
		zunoCommitCfg(); // Передаем полученную конфигурацию системе
	}
	// Устанавливаем обработчики ZWave для каналов
	_channelSetHandler(channel_count);
	// Устанавливаем обработчик системных осбытий - нужен для обновления прошивки
	zunoAttachSysHandler(ZUNO_HANDLER_SYSEVENT, 0, (void*) &_systemEvent);
}
// Основной цикл программы
void loop() {
	processChannels();
	delay(50);
}
