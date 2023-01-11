#include "TFWUpdater.h"
#include "DebugOutput.h"
#include "WbMsw.h"

TFWUpdater::TFWUpdater(TWBMSWSensor* wbMsw): WbMsw(wbMsw)
{
    NewFirmware = false;
    FirmwareSize = 0;
}

void TFWUpdater::NewFirmwareNotification(uint32_t newFirmwareSize)
{
    FirmwareSize = newFirmwareSize;
    NewFirmware = true;
}

bool TFWUpdater::GetFirmvareVersion(uint16_t& version)
{
    return WbMsw->GetFwVersion(version);
}

bool TFWUpdater::CheckNewFirmwareAvailable()
{
    return NewFirmware;
}

bool TFWUpdater::UpdateFirmware()
{
    if (WbMsw->FwUpdate((uint16_t*)WB_MSW_UPDATE_ADDRESS, FirmwareSize / 2)) {
        NewFirmware = false;
        return true;
    }
    return false;
}
