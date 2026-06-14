#pragma once

#include "telemetry/system_state_base.hpp"
#include "peripherals/ethernet/ethernet_info.hpp"

/* High-rate FCU telemetry record — the filling-station board's fast-changing live
 * state, downlinked over Ethernet (and logged to data_fast.bin) at the 2 kHz record
 * cadence. Adds the Ethernet link's info to the common base; the ECU record
 * (EcuSystemState) has no eth_info because the engine board has no Ethernet
 * peripheral. Slow/bulky state (thermocouples, event timestamps) does NOT live here
 * — it rides the low-rate FcuExtendedSystemState so it is not re-sent thousands of
 * times a second (see fcu_extended_system_state.hpp). */

struct FcuSystemState {
    SystemStateBase base;
    EthernetInfo    eth_info;  /**< Ethernet link: state + status + dropped-datagram count. */
};

// Wire layout guard: base + eth_info packed with no implicit padding between them.
static_assert(sizeof(FcuSystemState) == sizeof(SystemStateBase) + sizeof(EthernetInfo),
              "FcuSystemState has implicit padding — add explicit reserved bytes");
