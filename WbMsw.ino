
#include "DebugOutput.h"
#include "TFWUpdater.h"
#include "TFastModbus.h"
#include "TWBMSWSensor.h"
#include "TZWAVESensor.h"
#include "WbMsw.h"

//  Project global macros definitions for accurate protocol configuration
// DO NOT MOVE ZUNO_ENABLE FROM THIS PLACE
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
		SKETCH_VERSION=0x0103
		/* Firmware descriptor pointer */
		ZUNO_EXT_FIRMWARES_DESCR_PTR=&g_OtaDesriptor
		CONFIGPARAMETERS_MAX_COUNT=41//expands the number of parameters available - number + 0x1
		// DBG_CONSOLE_BAUDRATE=921600//speed uart dbg
		LOGGING_DBG // Comment out if debugging information is not needed
					// Debugging information being printed with RTOS system console output to UART0 (TX0) by default
		//LOGGING_UART=Serial

	);
// clang-format on

/* ZUNO_DECLARE defines global EXTERN for whole project, descriptor must be visible in all project files to make build
 * right*/
ZUNO_DECLARE(ZUNOOTAFWDescr_t g_OtaDesriptor);

/* WB chip firmware descriptor*/
ZUNOOTAFWDescr_t g_OtaDesriptor = {0x0101, 0x0103};
TWBMSWSensor WbMsw(&Serial1, WB_MSW_TIMEOUT);
TFastModbus FastModbus(&Serial1);
TZWAVESensor ZwaveSensor(&WbMsw);
TFWUpdater FwUpdater(&WbMsw);

enum class TZUnoState
{
    ZUNO_SCAN_ADDRESS_INITIALIZE,
    ZUNO_SCAN_ADDRESS,
    ZUNO_MODBUS_INITIALIZE,
    ZUNO_SENSOR_INITIALIZE,
    ZUNO_CHANNELS_INITIALIZE,
    ZUNO_POLL_CHANNELS
};

TZUnoState ZUnoState;

// ZUNO callback function return group names. "Dynamic" style is used also
// Only those groups for which there are corresponding channels are created in the device
const char* zunoAssociationGroupName(uint8_t groupIndex)
{
    return ZwaveSensor.GetGroupNameByIndex(groupIndex);
}

//  Function checks whether the device has a specified configuration parameter
const ZunoCFGParameter_t* zunoCFGParameter(size_t paramNumber)
{
    return ZwaveSensor.GetParameterIfChannelExists(paramNumber);
}

// Static function where system events arrive
static void SystemEvent(ZUNOSysEvent_t* ev)
{
    switch (ev->event) {
        // A new firmware image for the second chip from the Z-Wave controller has arrived
        case ZUNO_SYS_EVENT_OTA_IMAGE_READY:
            if (ev->params[0] == 0) {
                DEBUG("NEW FIRMWARE AVAILABLE, SIZE=");
                DEBUG(ev->params[1]);
                DEBUG(" BYTES\n");
                FwUpdater.NewFirmwareNotification(ev->params[1]);
            }
            break;
    }
}

static void UpdateParameterValue(size_t paramNumber, int32_t value)
{
    ZwaveSensor.SetParameterValue(paramNumber, value);
}

// The function is called at the start of the sketch
void setup()
{
    // Set system event handler (needed for firmware updates)
    zunoAttachSysHandler(ZUNO_HANDLER_SYSEVENT, 0, (void*)&SystemEvent);
    ZUnoState = TZUnoState::ZUNO_SCAN_ADDRESS_INITIALIZE;
}
// Main loop
void loop()
{
    switch (ZUnoState) {
        case TZUnoState::ZUNO_SCAN_ADDRESS_INITIALIZE: {
            if (FastModbus.OpenPort(WB_MSW_UART_BAUD, WB_MSW_UART_MODE, WB_MSW_UART_RX, WB_MSW_UART_TX)) {
                ZUnoState = TZUnoState::ZUNO_SCAN_ADDRESS;
            } else {
                DEBUG("*** ERROR Can't open port for fast modbus scan!\n");
                delay(1000);
            }
            break;
        }
        case TZUnoState::ZUNO_SCAN_ADDRESS: {
            uint8_t serialNumber[WB_MSW_SERIAL_NUMBER_SIZE];
            uint8_t modbusAddress;

            bool scanSuccess =
                FastModbus.ScanBus(serialNumber, WB_MSW_SERIAL_NUMBER_SIZE, &modbusAddress, 1, WB_MSW_TIMEOUT);
            FastModbus.ClosePort();

            if (scanSuccess) {
                DEBUG("Found device at ");
                DEBUG(modbusAddress);
                DEBUG("\n");
                WbMsw.SetModbusAddress(modbusAddress);
                ZUnoState = TZUnoState::ZUNO_MODBUS_INITIALIZE;
            } else {
                DEBUG("*** ERROR Fast modbus scan ends unsuccessfully!\n");
                ZUnoState = TZUnoState::ZUNO_SCAN_ADDRESS_INITIALIZE;
                delay(1000);
            }
            break;
        }
        case TZUnoState::ZUNO_MODBUS_INITIALIZE: {
            // Connecting to the WB sensor
            if (WbMsw.OpenPort(WB_MSW_UART_BAUD, WB_MSW_UART_MODE, WB_MSW_UART_RX, WB_MSW_UART_TX)) {
                ZUnoState = TZUnoState::ZUNO_SENSOR_INITIALIZE;
            } else {
                DEBUG("*** ERROR Can't open modbus port!\n");
                ZUnoState = TZUnoState::ZUNO_SCAN_ADDRESS_INITIALIZE;
                delay(1000);
            }
            break;
        }
        case TZUnoState::ZUNO_SENSOR_INITIALIZE: {
            uint16_t version;
            if (FwUpdater.GetFirmvareVersion(version)) {
                g_OtaDesriptor.version = version;
                ZUnoState = TZUnoState::ZUNO_CHANNELS_INITIALIZE;
                return;
            } else {
                DEBUG("*** ERROR WB sensor not responds!\n");
                ZUnoState = TZUnoState::ZUNO_SCAN_ADDRESS_INITIALIZE;
                delay(1000);
            }
            break;
        }
        case TZUnoState::ZUNO_CHANNELS_INITIALIZE: {
            // We need to initialize channels before reading parameters because
            // available parameters depends of available channels.
            // Watch zunoCFGParameter() function. This feature needs for specification procedure
            // Read channel values from WbMsw
            if (ZwaveSensor.ChannelsInitialize()) {
                // Read from FLASH configuration parameters
                ZwaveSensor.ParametersInitialize();
                // If the device is offline
                if (zunoStartDeviceConfiguration()) {
                    // Add channels to Z-Wave interface
                    ZwaveSensor.ChannelsSetup();
                    zunoSetS2Keys(
                        (SKETCH_FLAG_S2_AUTHENTICATED_BIT | SKETCH_FLAG_S2_UNAUTHENTICATED_BIT | SKETCH_FLAG_S0_BIT));
                    zunoCommitCfg(); // Transfer the received configuration to the system
                }
                // Bind handlers for channels fields
                ZwaveSensor.SetChannelHandlers();
                // Set parameter changing event handler (needed for firmware updates)
                zunoAttachSysHandler(ZUNO_HANDLER_ZW_CFG, 0, (void*)&UpdateParameterValue);

                ZUnoState = TZUnoState::ZUNO_POLL_CHANNELS;
            } else {
                DEBUG("*** ERROR WB sensor doesn't support any kind of sensors!\n");
                delay(1000);
            }
            break;
        }
        case TZUnoState::ZUNO_POLL_CHANNELS: {
            if (ZwaveSensor.ProcessChannels() != TZWAVESensor::Result::ZWAVE_PROCESS_OK) {
                WbMsw.ClosePort();
                ZUnoState = TZUnoState::ZUNO_SCAN_ADDRESS_INITIALIZE;
                break;
            }

            // If a new firmware came on the radio, send it to the bootloder of the WB chip
            if (FwUpdater.CheckNewFirmwareAvailable() && FwUpdater.UpdateFirmware()) {
                uint16_t version;
                FwUpdater.GetFirmvareVersion(version);
                g_OtaDesriptor.version = version;
            }

            delay(50);
            break;
        }
    }
}
