
#include "DebugOutput.h"
#include "SysService.h"
#include "TFWUpdater.h"
#include "TFastModbus.h"
#include "TWBMSWSensor.h"
#include "TZWAVESensor.h"
#include "WbMsw.h"
#include "ZWCCIndicator.h"
#include "ZWCCSoundSwitch.h"
#include "em_chip.h"

//  Project global macros definitions for accurate protocol configuration
// DO NOT MOVE ZUNO_ENABLE FROM THIS PLACE
// clang-format off
ZUNO_ENABLE(
		/* Commands class support. Defined here since device channels being created dynamically */
		WITH_CC_MULTICHANNEL
		WITH_CC_CONFIGURATION
		WITH_CC_SENSOR_MULTILEVEL
		WITH_CC_NOTIFICATION
		WITH_CC_SOUND_SWITCH
		WITH_CC_BASIC
		// SKETCH_FLAGS=(HEADER_FLAGS_NOSKETCH_OTA)
		ZUNO_CUSTOM_OTA_OFFSET=0x10000 // 64 kB
		/* Additional OTA firmwares count*/
		ZUNO_EXT_FIRMWARES_COUNT=1
		SKETCH_VERSION=0x010D
		/* Firmware descriptor pointer */
		ZUNO_EXT_FIRMWARES_DESCR_PTR=&g_OtaDesriptor
		CONFIGPARAMETERS_MAX_COUNT=46//expands the number of parameters available
		CERT_BUILD//Disables some config options
		WB_MSW_CERT_BUILD_INDICATOR=100
		MAX_PROCESSED_QUEUE_PKGS=1
		SKETCH_FLAGS_LOOP_DELAY=0x2
		// DBG_CONSOLE_BAUDRATE=921600//speed uart dbg
		// LOGGING_DBG // Comment out if debugging information is not needed
					// Debugging information being printed with RTOS system console output to UART0 (TX0) by default
        DBG_CONSOLE_PIN=0xFF
        //DBG_CONSOLE_BAUDRATE=115200
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
    size_t i;
    ssize_t value;
    ssize_t defaultValue;
    size_t paramNumber;
    const ZunoCFGParameter_t* parameters;

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
        case ZUNO_SYS_EVENT_LEARNSTATUS:
            if ((ev->params[0] == INCLUSION_STATUS_SUCESS) && (ev->params[1] == 0)) {
                i = 0x0;
                while (i < WB_MSW_MAX_CONFIG_PARAM) {
                    value = zunoLoadCFGParam(i + WB_MSW_CONFIG_PARAMETER_FIRST);
                    paramNumber = i + WB_MSW_CONFIG_PARAMETER_FIRST;
                    parameters = ZwaveSensor.GetParameterByNumber(paramNumber);
                    defaultValue = parameters->defaultValue;
                    if (defaultValue != value) {
                        zunoSaveCFGParam(paramNumber, defaultValue);
                    }
                    i++;
                }
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

static void SoundSwitchLoop(void);
static void SendTest(uint16_t version);
static void ServiceLedLoop(void);
static void ServiceLedInit(void);

// Main loop
void loop()
{

    switch (ZUnoState) {
        case TZUnoState::ZUNO_SCAN_ADDRESS_INITIALIZE: {
            if (FastModbus.OpenPort(WB_MSW_UART_BAUD, WB_MSW_UART_MODE, WB_MSW_UART_RX, WB_MSW_UART_TX)) {
                ZUnoState = TZUnoState::ZUNO_SCAN_ADDRESS;
            } else {
                SendTest(0xFFFF);
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
                SendTest(0xFFFF);
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
                SendTest(0xFFFF);
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
                SendTest(version);
                WbMsw.BuzzerStop();
                ServiceLedInit();
                return;
            } else {
                SendTest(0xFFFF);
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
            SoundSwitchLoop();
            ServiceLedLoop();
            // delay(50);
            break;
        }
    }
}

#define MS_LED_OPERATION_TIMEOUT_PERIOD_ON 1000
#define MS_LED_OPERATION_TIMEOUT_PERIOD_OFF_NETWORK_NOT 5000
#define MS_LED_OPERATION_TIMEOUT_PERIOD_OFF_NETWORK 10000

static WbMswLedMode_t LedModeCurrent = WB_MSW_LED_MODE_IDLE;
static WbMswLedMode_t LedModeNew = WB_MSW_LED_MODE_IDLE;
static WbMswLedMode_t LedModeSysLed = WB_MSW_LED_MODE_IDLE;
volatile bool gMotionLed = false;
static bool ServiceLedInitPower = false;
static uint32_t ServiceLedInitPowerMilis = 0;

ZUNO_SETUP_INDICATOR(ZUNO_SETUP_INDICATOR_INFO(INDICATOR_ID_NODE_IDENTIFY, 1),
                     ZUNO_SETUP_INDICATOR_INFO(INDICATOR_ID_ARMED, 2),
                     ZUNO_SETUP_INDICATOR_INFO(INDICATOR_ID_NOT_ARMED, 3));

static void ServiceLedInit(void)
{
    static bool ServiceLedInitPowerRepeat = false;

    WbMsw.SetLedRedOff();
    WbMsw.SetLedGreenOff();
    WbMsw.SetLedFlashDuration(50);
    WbMsw.SetLedFlashTimout(1);
    if (ServiceLedInitPowerRepeat == true)
        return;
    if (zunoGetWakeReason() != ZUNO_WAKEUP_REASON_POR)
        return;
    ServiceLedInitPowerRepeat = true;
    ServiceLedInitPower = true;
    ServiceLedInitPowerMilis = millis() + 1000;
    WbMsw.SetLedGreenOn();
}

static void ServiceLedLoopPower(WbMswLedMode_t ledMode, uint32_t msLedCurrent)
{
    if (ServiceLedInitPower == false)
        return;
    if (ledMode != WB_MSW_LED_MODE_IDLE) {
        ServiceLedInitPower = false;
        return;
    }
    if (msLedCurrent < ServiceLedInitPowerMilis)
        return;
    ServiceLedInitPower = false;
    WbMsw.SetLedGreenOff();
}

static void ServiceLedLoop(void)
{
    WbMswLedMode_t ledMode;
    static uint32_t msLedLastOperationNetwork = 0;
    static uint32_t msLedLastMotion = 0;
    static uint32_t msLedLastOperationNetworkPeriodOFF = 0;
    uint32_t msLedCurrent;

    msLedCurrent = millis();
    ledMode = LedModeNew;
    ServiceLedLoopPower(ledMode, msLedCurrent);
    switch ((int)LedModeCurrent) {
        case WB_MSW_LED_MODE_NETWORK:
        case WB_MSW_LED_MODE_NETWORK_NOT:
            if (ledMode != WB_MSW_LED_MODE_IDLE) {
                msLedLastOperationNetwork = msLedCurrent + msLedLastOperationNetworkPeriodOFF;
            } else {
                if (msLedCurrent >= msLedLastOperationNetwork) {
                    msLedLastOperationNetwork = msLedCurrent + msLedLastOperationNetworkPeriodOFF;
                    ledMode = WB_MSW_LED_MODE_IDLE;
                } else
                    return;
            }
            break;
        case WB_MSW_LED_MODE_MOTION:
            if (ledMode == WB_MSW_LED_MODE_IDLE) {
                if (msLedCurrent >= msLedLastMotion) {
                    ledMode = WB_MSW_LED_MODE_IDLE;
                } else
                    return;
                break;
            }
    }
    if (LedModeCurrent == ledMode) {
        if (ledMode != WB_MSW_LED_MODE_IDLE)
            return;
        if (gMotionLed == true) {
            gMotionLed = false;
            if (ZwaveSensor.GetParameterValue(WB_MSW_CONFIG_PARAMETER_MOTION_LED) != 0) {
                msLedLastMotion = msLedCurrent + MS_LED_OPERATION_TIMEOUT_PERIOD_ON;
                ledMode = WB_MSW_LED_MODE_MOTION;
            }
        } else {
            if (zunoInNetwork() == false) {
                msLedLastOperationNetworkPeriodOFF = MS_LED_OPERATION_TIMEOUT_PERIOD_OFF_NETWORK_NOT;
                if (msLedCurrent >= msLedLastOperationNetwork) {
                    msLedLastOperationNetwork = msLedCurrent + MS_LED_OPERATION_TIMEOUT_PERIOD_ON;
                    ledMode = WB_MSW_LED_MODE_NETWORK_NOT;
                }
            } else {
                if (ZwaveSensor.GetParameterValue(WB_MSW_CONFIG_PARAMETER_OPERATION_LED) != 0) {
                    msLedLastOperationNetworkPeriodOFF = MS_LED_OPERATION_TIMEOUT_PERIOD_OFF_NETWORK;
                    if (msLedCurrent >= msLedLastOperationNetwork) {
                        msLedLastOperationNetwork = msLedCurrent + MS_LED_OPERATION_TIMEOUT_PERIOD_ON;
                        ledMode = WB_MSW_LED_MODE_NETWORK;
                    }
                }
            }
        }
    }
    switch (ledMode) {
        case WB_MSW_LED_MODE_LERN:
            WbMsw.SetLedGreenOff();
            WbMsw.SetLedRedOn();
            break;
        case WB_MSW_LED_MODE_RED_GREEN:
        case WB_MSW_LED_MODE_DUO:
            WbMsw.SetLedGreenOn();
            WbMsw.SetLedRedOn();
            break;
        case WB_MSW_LED_MODE_IDLE:
            WbMsw.SetLedGreenOff();
            WbMsw.SetLedRedOff();
            break;
        case WB_MSW_LED_MODE_NETWORK_NOT:
        case WB_MSW_LED_MODE_MOTION:
        case WB_MSW_LED_MODE_RED:
            WbMsw.SetLedGreenOff();
            WbMsw.SetLedRedOn();
            break;
        case WB_MSW_LED_MODE_NETWORK:
        case WB_MSW_LED_MODE_GREEN:
            WbMsw.SetLedRedOff();
            WbMsw.SetLedGreenOn();
            break;
        default:
            return;
            break;
    }
    LedModeCurrent = ledMode;
}

void zunoIndicatorLoopOpen(uint8_t pin, uint8_t indicatorId)
{
    switch (indicatorId) {
        case INDICATOR_ID_ARMED:
        case INDICATOR_ID_NODE_IDENTIFY:
            if (LedModeNew != WB_MSW_LED_MODE_GREEN)
                LedModeNew = WB_MSW_LED_MODE_RED;
            else
                LedModeNew = WB_MSW_LED_MODE_RED_GREEN;
            break;
        case INDICATOR_ID_NOT_ARMED:
            if (LedModeNew != WB_MSW_LED_MODE_RED)
                LedModeNew = WB_MSW_LED_MODE_GREEN;
            else
                LedModeNew = WB_MSW_LED_MODE_RED_GREEN;
            break;
        default:
            break;
    }
    (void)pin;
}

void zunoIndicatorLoopClose(uint8_t pin, uint8_t indicatorId)
{
    switch (indicatorId) {
        case INDICATOR_ID_ARMED:
        case INDICATOR_ID_NODE_IDENTIFY:
            if ((LedModeNew & WB_MSW_LED_MODE_GREEN) == 0x0)
                LedModeNew = WB_MSW_LED_MODE_IDLE;
            else
                LedModeNew = WB_MSW_LED_MODE_GREEN;
            break;
        case INDICATOR_ID_NOT_ARMED:
            if ((LedModeNew & WB_MSW_LED_MODE_RED) == 0x0)
                LedModeNew = WB_MSW_LED_MODE_IDLE;
            else
                LedModeNew = WB_MSW_LED_MODE_RED;
            break;
        default:
            break;
    }
    (void)pin;
}

void zunoIndicatorBinary(uint8_t pin, uint8_t value, uint8_t indicatorId)
{
    if (value == LOW)
        zunoIndicatorLoopClose(pin, indicatorId);
    else
        zunoIndicatorLoopOpen(pin, indicatorId);
}

void zunoIndicatorDigitalWrite(uint8_t pin, uint8_t value, uint8_t indicatorId)
{
    (void)indicatorId;
    (void)pin;
    (void)value;
}

void zunoIndicatorPinMode(uint8_t pin, uint8_t value, uint8_t indicatorId)
{
    (void)pin;
    (void)indicatorId;
    (void)value;
}

void zunoSysServiceLedInit(void)
{}

void zunoSysServiceLedOff(uint8_t pin)
{
    switch (pin) {
        case SYSLED_LEARN:
            LedModeNew = WB_MSW_LED_MODE_IDLE;
            break;
        default:
            break;
    }
}

void zunoSysServiceLedOn(uint8_t pin)
{
    if (LedModeSysLed == WB_MSW_LED_MODE_IDLE)
        return;
    switch (pin) {
        case SYSLED_LEARN:
            LedModeNew = LedModeSysLed;
            break;
        default:
            break;
    }
}

void zunoSysServiceLedSetMode(uint8_t pin, uint8_t mode)
{
    if (pin != SYSLED_LEARN)
        return;
    switch (mode) {
        case SYSLED_LEARN_MODE_SUBMENU_READY:
            LedModeSysLed = WB_MSW_LED_MODE_DUO;
            break;
        case SYSLED_LEARN_MODE_INCLUSION:
        default:
            LedModeSysLed = WB_MSW_LED_MODE_LERN;
            break;
    }
    zunoSysServiceLedOn(pin);
}

// For the test, the release is not needed.
// On the radio when sending a message.
static void SendTest(uint16_t version)
{
    static bool fSend = false;
    uint64_t uuid;
    uint8_t array[] = {0xAA,
                       0xAE,
                       0xDA, // SIGN
                       0x00,
                       0x00,
                       0x00,
                       0x00,
                       0x00,
                       0x00,
                       0x00,
                       0x00, // UUID
                       0x00,
                       0x00}; // MSW FIRMWARE

    if (zunoRSTRetention(0) != 0xA8) {
        return;
    }
    if (fSend == true)
        return;
    fSend = true;
    uuid = SYSTEM_GetUnique();
    _zme_memcpy(array + 3, (uint8_t*)&uuid, sizeof(uuid));
    memcpy(array + 3 + sizeof(uuid), &version, sizeof(version));
    zunoSendTestPackage(&array[0x0], sizeof(array), 240);
}

// // Fail signal
// // Fail signal
// ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION(FAIL_SIGNAL,
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(100, 500),
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(100, 300),
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(100, 300),
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(400, 1200));

// // Alarm signal
// ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION(ALARM_SIGNAL,
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(100, 200),
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(100, 200),
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(100, 200),
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(100, 200),
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(100, 200),
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(100, 200),
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(100, 200),
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(100, 800));

// ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION(ACCEPT_SIGNAL,
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(100, 200),
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(400, 300));

// ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION(HORSE_SIGNAL,
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(100, 200),
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(100, 200),
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(100, 500),
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(100, 200),
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(100, 200),
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(100, 500),
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(100, 200),
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(100, 200),
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(100, 500),
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(100, 500),
//                                       ZUNO_SETUP_SOUND_SWITCH_TONE_DURATION_SET(100, 700));

typedef enum SoundSwitchMelody_e
{
    SoundSwitchMelodyFailSignal = 1,
    SoundSwitchMelodyAlarmSignal,
    SoundSwitchMelodyAcceptSignal,
    SoundSwitchMelodyHorseSignal,
    SoundSwitchMelodyOff,
    SoundSwitchMelodyIdle,
} SoundSwitchMelody_t;

typedef enum SoundSwitchState_e
{
    SoundSwitchStateInit,
    SoundSwitchStatePlay,
    SoundSwitchStatePlayOff,
} SoundSwitchState_t;

// Play melody 300 seconds
ZUNO_SETUP_SOUND_SWITCH(melody_set1,
                        ZUNO_SETUP_SOUND_SWITCH_TONE("Fail Signal", 3),
                        ZUNO_SETUP_SOUND_SWITCH_TONE("Alarm Signal", 3),
                        ZUNO_SETUP_SOUND_SWITCH_TONE("Accept Signal", 1),
                        ZUNO_SETUP_SOUND_SWITCH_TONE("Horse Signal", 5));

static SoundSwitchMelody_t SoundSwitchCurrent = SoundSwitchMelodyIdle;
static SoundSwitchMelody_t SoundSwitchNew = SoundSwitchMelodyIdle;
static bool SoundSwitchUpdate = false;
static const uint16_t* SoundSwitchArray = NULL;
static size_t SoundSwitchSize = 0;
static size_t SoundSwitchIndex = 0;
static uint32_t SoundSwitchMilis = 0;

static void SoundSwitchLoop_init_add(const uint16_t* array, size_t size)
{
    SoundSwitchArray = array;
    SoundSwitchSize = size;
    SoundSwitchIndex = 0;
    SoundSwitchMilis = millis();
}

static void SoundSwitchLoop_init(void)
{
    static const uint16_t FailSignal[] = {100, 500, 100, 300, 100, 300, 400, 1200};
    static const uint16_t AlarmSignal[] =
        {100, 200, 100, 200, 100, 200, 100, 200, 100, 200, 100, 200, 100, 200, 100, 800};
    static const uint16_t AcceptSignal[] = {100, 200, 400, 300};
    static const uint16_t HorsetSignal[] = {100, 200, 100, 200, 100, 500, 100, 200, 100, 200, 100,
                                            500, 100, 200, 100, 200, 100, 500, 100, 500, 100, 700};

    zunoEnterCritical();
    if (SoundSwitchUpdate == false) {
        zunoExitCritical();
        return;
    }
    SoundSwitchCurrent = SoundSwitchNew;
    SoundSwitchUpdate = false;
    zunoExitCritical();
    switch (SoundSwitchCurrent) {
        case SoundSwitchMelodyIdle:
            break;
        case SoundSwitchMelodyFailSignal:
            SoundSwitchLoop_init_add(&FailSignal[0], (sizeof(FailSignal) / sizeof(FailSignal[0])));
            break;
        case SoundSwitchMelodyAlarmSignal:
            SoundSwitchLoop_init_add(&AlarmSignal[0], (sizeof(AlarmSignal) / sizeof(AlarmSignal[0])));
            break;
        case SoundSwitchMelodyAcceptSignal:
            SoundSwitchLoop_init_add(&AcceptSignal[0], (sizeof(AcceptSignal) / sizeof(AcceptSignal[0])));
            break;
        case SoundSwitchMelodyHorseSignal:
            SoundSwitchLoop_init_add(&HorsetSignal[0], (sizeof(HorsetSignal) / sizeof(HorsetSignal[0])));
            break;
        case SoundSwitchMelodyOff:
            WbMsw.BuzzerStop();
            SoundSwitchCurrent = SoundSwitchMelodyIdle;
            break;
    }
}

static void SoundSwitchLoop_play(void)
{
    uint32_t ms;

    ms = millis();
    if (SoundSwitchMilis > ms)
        return;
    SoundSwitchMilis = ms + SoundSwitchArray[SoundSwitchIndex];
    Serial0.print(SoundSwitchArray[SoundSwitchIndex]);
    if ((SoundSwitchIndex & 1) == 0) {
        WbMsw.BuzzerStart();
    } else {
        WbMsw.BuzzerStop();
    }
    SoundSwitchIndex++;
    if (SoundSwitchIndex >= SoundSwitchSize)
        SoundSwitchCurrent = SoundSwitchMelodyIdle;
}

static void SoundSwitchLoop(void)
{
    SoundSwitchLoop_init();
    switch (SoundSwitchCurrent) {
        case SoundSwitchMelodyIdle:
        case SoundSwitchMelodyOff:
            break;
        case SoundSwitchMelodyAlarmSignal:
        case SoundSwitchMelodyAcceptSignal:
        case SoundSwitchMelodyHorseSignal:
        case SoundSwitchMelodyFailSignal:
            SoundSwitchLoop_play();
            break;
    }
}

void playMelody1(uint8_t melody_id, uint8_t volume)
{
    SoundSwitchNew = (SoundSwitchMelody_t)melody_id;
    SoundSwitchUpdate = true;
    (void)volume;
}
void stopMelody1(uint8_t melody_id)
{
    SoundSwitchNew = SoundSwitchMelodyOff;
    SoundSwitchUpdate = true;
    (void)melody_id;
}

zuno_handler_soundswitch_t __hdata_playMelody1 = {(void*)&melody_set1, (void*)&playMelody1, (void*)&stopMelody1};