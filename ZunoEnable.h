//  Project global macros definitions for accurate protocol configuration
// clang-format off
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
// clang-format on