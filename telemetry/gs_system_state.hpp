#pragma once

#include <stdint.h>


struct GSSystemState{
    uint16_t isValveStartButtonPressed: 1;
    uint16_t isAllowFillSwitchOn : 1;
    uint16_t isArmServoSwitchOn : 1;
    uint16_t isArmIgniterSwitchOn : 1;
    uint16_t isAllowDumpSwitchOn : 1;
    uint16_t isEmergencyStopButtonPressed : 1;
    uint16_t isFireIgniterButtonPressed : 1;
    uint16_t isUnsafeKeySwitchPressed : 1;
    uint16_t reserved: 8;
    uint8_t fcuState;
    uint8_t ecuState;
};

static_assert(sizeof(GSSystemState) == sizeof(uint32_t), "GSSystemState added padding at compile time.. add reserve fields for good alignment");