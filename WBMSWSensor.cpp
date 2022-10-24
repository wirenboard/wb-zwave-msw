
#include "WBMSWSensor.h"
#include "Debug.h"

#define WBMSW_REG_TEMPERATURE 		0x0004
#define WBMSW_REG_HUMIDITY    		0x0005
#define WBMSW_REG_LUMINANCE   		0x0009
#define WBMSW_REG_CO2   	  		0x0008
#define WBMSW_COIL_CO2_STAUS   		0x0003
#define WBMSW_REG_CO2_AUTO_CALIB  	0x005F
#define WBMSW_REG_VOC   			0x000B
#define WBMSW_REG_NOIZE   			0x0003
#define WBMSW_REG_MOTION            0x011B
#define WBMSW_REG_FW_MODE 			0x0081
#define WBMSW_REG_FW_INFO 			0x1000
#define WBMSW_REG_FW_DATA 			0x2000
#define WBMSW_REG_FW_VERSION		0x00FA

#define WBMSW_FIRMWARE_INFO_SIZE   32
#define WBMSW_FIRMWARE_DATA_SIZE   136


/* Public Constructors */
WBMSWSensor::WBMSWSensor(HardwareSerial *hardwareSerial, uint16_t timeoutMs): ModBusRtuClass(hardwareSerial, timeoutMs) {
}

/* Public Methods */
bool WBMSWSensor::OpenPort(size_t speed, uint32_t config, uint8_t rx, uint8_t tx) {
	if (!ModBusRtuClass::begin(speed, config, rx, tx))
		return (false);
	return true;
}

void WBMSWSensor::SetModbusAddress(uint16_t Address){
	this->Address = Address;
}

bool WBMSWSensor::getFwVersion(uint32_t *version) {
	uint32_t									out;
	uint32_t									number;
	uint16_t									fw_v[0x10];
	size_t										i;
	size_t										count;
	uint16_t									letter;

	if (ModBusRtuClass::readInputRegisters(this->Address, WBMSW_REG_FW_VERSION, (sizeof(fw_v) / sizeof(fw_v[0x0])), &fw_v[0x0]) == false)
		return (false);
	out = 0x0;
	number = 00;
	i = 0x0;
	count = 0x0;
	while ((letter = fw_v[i]) != 0x0) {
		if (letter == '.') {
			out = (out << 0x8) | number;
			number = 0x0;
			count = 0x0;
		}
		else {
			number = number * 0xA + (letter - 0x30);
			count++;
		}
		i++;
	}
	if (count != 0x0)
		out = (out << 0x8) | number;
	version[0x0] = out;
	return (true);
}

bool WBMSWSensor::getTemperature(int16_t & temperature) {
	return readInputRegisters(Address, WBMSW_REG_TEMPERATURE, 1, &temperature);
}

bool WBMSWSensor::getHumidity(uint16_t & humidity) {
	return readInputRegisters(Address, WBMSW_REG_HUMIDITY, 1, &humidity);
}

bool WBMSWSensor::getLumminance(uint32_t & lumminance) {
	uint16_t	lumen[2];
	if (!readInputRegisters(Address, WBMSW_REG_LUMINANCE, (sizeof(lumen) / sizeof(lumen[0x0])), lumen))
		return false;
	lumminance = (lumen[0x0] << 0x10 )| lumen[1];
	return true;
}

bool WBMSWSensor::getC02(uint16_t & c02) {
	return readInputRegisters(Address, WBMSW_REG_CO2, 1, &c02);
}

bool WBMSWSensor::getC02Status(bool & status) {
	uint8_t						out;
	if (!readCoils(Address, WBMSW_COIL_CO2_STAUS, 1, &out))
		return false;
	status = (out != 0);
	return true;
}

bool WBMSWSensor::setC02Status(bool status) {
	return writeSingleCoils(Address, WBMSW_COIL_CO2_STAUS, status);
}

bool WBMSWSensor::setC02Autocalibration(bool status) {
	uint16_t value = status ? 1 : 0;
	return writeSingleRegisters(Address, WBMSW_REG_CO2_AUTO_CALIB, value);
}

bool WBMSWSensor::getVoc(uint16_t & voc) {
	return readInputRegisters(Address, WBMSW_REG_VOC, 1, &voc);
}

bool WBMSWSensor::getNoiseLevel(uint16_t & noise_level) {
	return readInputRegisters(Address, WBMSW_REG_NOIZE, 1, &noise_level);
}


bool WBMSWSensor::getMotion(uint16_t & motion) {
	return readInputRegisters(Address, WBMSW_REG_MOTION, 1, &motion);//0x0118 - max
}

bool WBMSWSensor::fwMode(void) {
	return writeSingleRegisters(Address, WBMSW_REG_FW_MODE, 1);
}

bool WBMSWSensor::fwWriteInfo(uint8_t * info) {
	return writeMultipleRegisters(Address, WBMSW_REG_FW_INFO, WBMSW_FIRMWARE_INFO_SIZE / 2, info);
}

bool WBMSWSensor::fwWriteData(uint8_t * data) {
	return writeMultipleRegisters(Address, WBMSW_REG_FW_DATA, WBMSW_FIRMWARE_DATA_SIZE / 2, data);
}

bool WBMSWSensor::fwUpdate(const void *buffer, size_t len, uint16_t timout_ms) {
	uint8_t	*data;
	if(len < WBMSW_FIRMWARE_INFO_SIZE)
		return false;
	#ifdef LOGGING_DBG
	LOGGING_UART.print("FW size: ");
	LOGGING_UART.println(len);
	#endif
	if (!this->fwMode())
		return false;
	#ifdef LOGGING_DBG
	LOGGING_UART.print("Wait 2 sec\n");
	#endif
	delay(timout_ms);
	data = (uint8_t *)buffer;
	if (!this->fwWriteInfo(data))
		return false;
	#ifdef LOGGING_DBG
	LOGGING_UART.print("Write info\n");
	#endif
	data += WBMSW_FIRMWARE_INFO_SIZE;
	len -=  WBMSW_FIRMWARE_INFO_SIZE;
	#ifdef LOGGING_DBG
	LOGGING_UART.print("Write data\n");
	#endif
	while (len) {
		if (!fwWriteData(data))
			return false;
		data += WBMSW_FIRMWARE_DATA_SIZE;
		len -= WBMSW_FIRMWARE_DATA_SIZE;
	}
	#ifdef LOGGING_DBG
	LOGGING_UART.print("Write finish\n");
	#endif
	return true;
}
