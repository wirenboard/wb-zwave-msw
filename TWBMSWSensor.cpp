
#include "TWBMSWSensor.h"
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
TWBMSWSensor::TWBMSWSensor(HardwareSerial *HardwareSerial, uint16_t TimeoutMs): ModBusRtuClass(HardwareSerial, TimeoutMs) {
}

/* Public Methods */
bool TWBMSWSensor::OpenPort(size_t Speed, uint32_t Config, uint8_t Rx, uint8_t Tx) {
	if (!ModBusRtuClass::begin(Speed, Config, Rx, Tx))
		return (false);
	return true;
}

void TWBMSWSensor::SetModbusAddress(uint16_t Address){
	this->Address = Address;
}

bool TWBMSWSensor::GetFwVersion(uint32_t *Version) {
	uint32_t out, number;
	uint16_t fw_v[0x10];
	size_t i;
	size_t count;
	uint16_t letter;

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
	Version[0x0] = out;
	return (true);
}

bool TWBMSWSensor::GetTemperature(int16_t & Temperature) {
	return ModBusRtuClass::readInputRegisters(Address, WBMSW_REG_TEMPERATURE, 1, &Temperature);
}

bool TWBMSWSensor::GetHumidity(uint16_t & Humidity) {
	return ModBusRtuClass::readInputRegisters(Address, WBMSW_REG_HUMIDITY, 1, &Humidity);
}

bool TWBMSWSensor::GetLumminance(uint32_t & Iumminance) {
	uint16_t	lumen[2];
	if (!ModBusRtuClass::readInputRegisters(Address, WBMSW_REG_LUMINANCE, (sizeof(lumen) / sizeof(lumen[0x0])), lumen))
		return false;
	Iumminance = (lumen[0x0] << 0x10 )| lumen[1];
	return true;
}

bool TWBMSWSensor::GetC02(uint16_t & C02) {
	return ModBusRtuClass::readInputRegisters(Address, WBMSW_REG_CO2, 1, &C02);
}

bool TWBMSWSensor::GetC02Status(bool & Status) {
	uint8_t out;
	if (!ModBusRtuClass::readCoils(Address, WBMSW_COIL_CO2_STAUS, 1, &out))
		return false;
	Status = (out != 0);
	return true;
}

bool TWBMSWSensor::SetC02Status(bool Status) {
	return ModBusRtuClass::writeSingleCoils(Address, WBMSW_COIL_CO2_STAUS, Status);
}

bool TWBMSWSensor::SetC02Autocalibration(bool Status) {
	uint16_t value = Status ? 1 : 0;
	return ModBusRtuClass::writeSingleRegisters(Address, WBMSW_REG_CO2_AUTO_CALIB, value);
}

bool TWBMSWSensor::GetVoc(uint16_t & Voc) {
	return ModBusRtuClass::readInputRegisters(Address, WBMSW_REG_VOC, 1, &Voc);
}

bool TWBMSWSensor::GetNoiseLevel(uint16_t & NoiseLevel) {
	return ModBusRtuClass::readInputRegisters(Address, WBMSW_REG_NOIZE, 1, &NoiseLevel);
}


bool TWBMSWSensor::GetMotion(uint16_t & Motion) {
	return ModBusRtuClass::readInputRegisters(Address, WBMSW_REG_MOTION, 1, &Motion);//0x0118 - max
}

bool TWBMSWSensor::FwMode(void) {
	return ModBusRtuClass::writeSingleRegisters(Address, WBMSW_REG_FW_MODE, 1);
}

bool TWBMSWSensor::FwWriteInfo(uint8_t * Info) {
	return ModBusRtuClass::writeMultipleRegisters(Address, WBMSW_REG_FW_INFO, WBMSW_FIRMWARE_INFO_SIZE / 2, Info);
}

bool TWBMSWSensor::FwWriteData(uint8_t * Data) {
	return ModBusRtuClass::writeMultipleRegisters(Address, WBMSW_REG_FW_DATA, WBMSW_FIRMWARE_DATA_SIZE / 2, Data);
}

bool TWBMSWSensor::FwUpdate(const void *Buffer, size_t Len, uint16_t TimeoutMs) {
	uint8_t	*data;
	if(Len < WBMSW_FIRMWARE_INFO_SIZE)
		return false;
	#ifdef LOGGING_DBG
	LOGGING_UART.print("FW size: ");
	LOGGING_UART.println(Len);
	#endif
	if (!this->FwMode())
		return false;
	#ifdef LOGGING_DBG
	LOGGING_UART.print("Wait 2 sec\n");
	#endif
	delay(TimeoutMs);
	data = (uint8_t *)Buffer;
	if (!this->FwWriteInfo(data))
		return false;
	#ifdef LOGGING_DBG
	LOGGING_UART.print("Write info\n");
	#endif
	data += WBMSW_FIRMWARE_INFO_SIZE;
	Len -=  WBMSW_FIRMWARE_INFO_SIZE;
	#ifdef LOGGING_DBG
	LOGGING_UART.print("Write data\n");
	#endif
	while (Len) {
		if (!FwWriteData(data))
			return false;
		data += WBMSW_FIRMWARE_DATA_SIZE;
		Len -= WBMSW_FIRMWARE_DATA_SIZE;
	}
	#ifdef LOGGING_DBG
	LOGGING_UART.print("Write finish\n");
	#endif
	return true;
}