#ifndef WB_MSW_SENSOR_H
#define WB_MSW_SENSOR_H

#include "ModBusRtu.h"

class TWBMSWSensor : private ModBusRtuClass
{
public:
	TWBMSWSensor(HardwareSerial *HardwareSerial, uint16_t TimeoutMs);
	bool OpenPort(size_t Speed, uint32_t Config, uint8_t Rx, uint8_t Tx);
	void SetModbusAddress(uint8_t Address);
	bool GetFwVersion(uint32_t *Version);
	bool GetTemperature(int16_t &Temperature);
	bool GetHumidity(uint16_t &Humidity);
	bool GetLumminance(uint32_t &Iumminance);
	bool GetC02(uint16_t &C02);
	bool GetC02Status(bool &Status);
	bool SetC02Status(bool Status);
	bool SetC02Autocalibration(bool Status);
	bool GetVoc(uint16_t &Voc);
	bool GetNoiseLevel(uint16_t &NoiseLevel);
	bool GetMotion(uint16_t &Motion);
	bool FwMode(void);
	bool FwWriteInfo(uint8_t *Info);
	bool FwWriteData(uint8_t *Info);
	bool FwUpdate(const void *Buffer, size_t Len, uint16_t TimeoutMs = 2000);

private:
	uint16_t Address;
};
#endif // WB_MSW_SENSOR_H