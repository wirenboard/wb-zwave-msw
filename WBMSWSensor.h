#ifndef WB_MSW_SENSOR_H
#define WB_MSW_SENSOR_H

#include "ModBusRtu.h"


class WBMSWSensor: private ModBusRtuClass {
	public:
		WBMSWSensor(HardwareSerial *hardwareSerial, uint16_t timout_ms, uint8_t addr);
		bool	begin(size_t speed, uint32_t config, uint8_t rx, uint8_t tx);
		bool	getFwVersion(uint32_t *version);
		bool	getTemperature(int16_t & temperature);
		bool	getHumidity(uint16_t & humidity);
		bool	getLumminance(uint32_t & lumminance);
		bool	getC02(uint16_t & c02);
		bool	getC02Status(bool & status);
		bool	setC02Status(bool status);
		bool	setC02Autocalibration(bool status);
		bool	getVoc(uint16_t & voc);
		bool	getNoiseLevel(uint16_t & noise_level);
		bool	getMotion(uint16_t & motion);
		bool	fwMode(void);
		bool	fwWriteInfo(uint8_t * info);
		bool	fwWriteData(uint8_t * info);
		bool	fwUpdate(const void *buffer, size_t len, uint16_t timout_ms=2000);

	private:
		uint8_t		_addr;

};
#endif//WB_MSW_SENSOR_H