#ifndef WB_MSW_H
#define WB_MSW_H

#define WB_MSW_TIMEOUT 1000
#define WB_MSW_ON 255
#define WB_MSW_OFF 0

#ifdef ZUNO_CUSTOM_OTA_OFFSET
#define WB_MSW_UPDATE_ADDRESS (BOOTLOADER_STORAGE_AREA_START + ZUNO_CUSTOM_OTA_OFFSET)
#else
#define WB_MSW_UPDATE_ADDRESS (BOOTLOADER_STORAGE_AREA_START)
#endif

#define WB_MSW_INPUT_REG_TEMPERATURE_VALUE_ERROR 0x7FFF
#define WB_MSW_INPUT_REG_TEMPERATURE_VALUE_PRECISION SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS
#define WB_MSW_INPUT_REG_TEMPERATURE_VALUE_SIZE SENSOR_MULTILEVEL_SIZE_TWO_BYTES

#define WB_MSW_INPUT_REG_HUMIDITY_VALUE_ERROR 0xFFFF
#define WB_MSW_INPUT_REG_HUMIDITY_VALUE_PRECISION SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS
#define WB_MSW_INPUT_REG_HUMIDITY_VALUE_SIZE SENSOR_MULTILEVEL_SIZE_TWO_BYTES

#define WB_MSW_INPUT_REG_CO2_VALUE_ERROR 0xFFFF
#define WB_MSW_INPUT_REG_CO2_VALUE_PRECISION SENSOR_MULTILEVEL_PRECISION_ZERO_DECIMALS
#define WB_MSW_INPUT_REG_CO2_VALUE_SIZE SENSOR_MULTILEVEL_SIZE_TWO_BYTES

#define WB_MSW_INPUT_REG_VOC_VALUE_ERROR 0xFFFF
#define WB_MSW_INPUT_REG_VOC_VALUE_PRECISION SENSOR_MULTILEVEL_PRECISION_ZERO_DECIMALS
#define WB_MSW_INPUT_REG_VOC_VALUE_SIZE SENSOR_MULTILEVEL_SIZE_TWO_BYTES

#define WB_MSW_INPUT_REG_NOISE_LEVEL_PRECISION SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS
#define WB_MSW_INPUT_REG_NOISE_LEVEL_VALUE_SIZE SENSOR_MULTILEVEL_SIZE_TWO_BYTES

#define WB_MSW_INPUT_REG_LUMEN_VALUE_ERROR 0xFFFFFFFF
#define WB_MSW_INPUT_REG_LUMEN_VALUE_PRECISION SENSOR_MULTILEVEL_PRECISION_TWO_DECIMALS
#define WB_MSW_INPUT_REG_LUMEN_VALUE_SIZE SENSOR_MULTILEVEL_SIZE_FOUR_BYTES

#define WB_MSW_INPUT_REG_MOTION_VALUE_ERROR 0xFFFF

#define WB_MSW_CONFIG_PARAMETER_FIRST 64

#define WB_MSW_CONFIG_PARAMETER_TEMPERATURE_HYSTERESIS                                                                 \
    64 // Temperature. Hysteresis (degree tenths). Set 0 to disable report publishing.
// Otherwise, the value will be multiplied by 0.01 to calculate the hysteresis
#define WB_MSW_CONFIG_PARAMETER_TEMPERATURE_INVERT                                                                     \
    65 // Temperature. Logic inverting (In case of threshold exceeding Basic.Off will be sent)
#define WB_MSW_CONFIG_PARAMETER_TEMPERATURE_THRESHOLD 66 // Temperature. Threshold for Basic.On sending

#define WB_MSW_CONFIG_PARAMETER_HUMIDITY_HYSTERESIS 67 // Humidity---
#define WB_MSW_CONFIG_PARAMETER_HUMIDITY_INVERT 68     // Humidity---
#define WB_MSW_CONFIG_PARAMETER_HUMIDITY_THRESHOLD 69  // Humidity---

#define WB_MSW_CONFIG_PARAMETER_LUMEN_HYSTERESIS 70 // Lumen---
#define WB_MSW_CONFIG_PARAMETER_LUMEN_INVERT 71     // Lumen---
#define WB_MSW_CONFIG_PARAMETER_LUMEN_THRESHOLD 72  // Lumen---

#define WB_MSW_CONFIG_PARAMETER_CO2_HYSTERESIS 73 // CO2---
#define WB_MSW_CONFIG_PARAMETER_CO2_INVERT 74     // CO2---
#define WB_MSW_CONFIG_PARAMETER_CO2_THRESHOLD 75  // CO2---
#define WB_MSW_CONFIG_PARAMETER_CO2_AUTO 76       // CO2---

#define WB_MSW_CONFIG_PARAMETER_VOC_HYSTERESIS 77 // VOC---
#define WB_MSW_CONFIG_PARAMETER_VOC_INVERT 78     // VOC---
#define WB_MSW_CONFIG_PARAMETER_VOC_THRESHOLD 79  // VOC---

#define WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_HYSTERESIS 80 // Noise level---
#define WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_INVERT 81     // Noise level---
#define WB_MSW_CONFIG_PARAMETER_NOISE_LEVEL_THRESHOLD 82  // Noise level---

#define WB_MSW_CONFIG_PARAMETER_MOTION_TIME 83   // Motion sensor. Basic.Off sendind time interval
#define WB_MSW_CONFIG_PARAMETER_MOTION_INVERT 84 // Motion sensor. Level inverting
#define WB_MSW_CONFIG_PARAMETER_MOTION_THRESHOLD 85

#define WB_MSW_CONFIG_PARAMETER_LAST 86
#define WB_MSW_MAX_CONFIG_PARAM ((WB_MSW_CONFIG_PARAMETER_LAST - WB_MSW_CONFIG_PARAMETER_FIRST) + 1)

#define WB_MSW_UART_BAUD 9600
#define WB_MSW_UART_MODE SERIAL_8N2
#define WB_MSW_UART_RX 8 // Z-Uno receiver pin
#define WB_MSW_UART_TX 7 // Z-uno transmitter pin

#endif // WB_MSW_H
