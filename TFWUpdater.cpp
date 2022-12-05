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

bool TFWUpdater::GetFirmvareVersion(uint32_t& version)
{
    if (!WbMsw->GetFwVersion(&version)) {
        return false;
    }

    DEBUG("FW:                 ");
    DEBUG(version, 16);
    DEBUG("\n");
    return true;
}

bool TFWUpdater::CheckNewFirmwareAvailable()
{
    return NewFirmware;
}

bool TFWUpdater::UpdateFirmware()
{
    WbMsw->FwUpdate((void*)WB_MSW_UPDATE_ADDRESS, FirmwareSize);
    NewFirmware = false;
}
