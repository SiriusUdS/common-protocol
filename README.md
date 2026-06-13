# common-protocol

The **single source of truth for the filling-station on-wire protocol** — the frame
headers, ids, command/response payloads, and telemetry record layouts exchanged
between the **FCU**, the **ECU**, and the **ground station (GS)**.

Header-only C++17, no dependencies. Both transports (UDP/Ethernet GS↔FCU, CAN
FCU↔ECU) share these definitions so every endpoint agrees on the bytes.

## Layout

Add the repository root to your include path; then include by area:

| Path | What |
|------|------|
| `framing/` | Frame headers + the payload class. `ethernet_header.hpp` (12-byte UDP header), `can_header.hpp` (29-bit CAN id), `payload_type.hpp` (Command / Telemetry / Response). |
| `command/` | `command_type.hpp` (the `CommandType` id SSOT + payload sizing) and the payload frames `set_state.hpp`, `set_valve_position.hpp`. |
| `response/` | `response_type.hpp` — replies to commands (`Pong`). |
| `telemetry/` | `telemetry_type.hpp` + the telemetry records: `system_state_base.hpp` and the per-board `fcu_system_state.hpp` / `ecu_system_state.hpp`. |
| `system/` | System-wide values: `board_id.hpp`, `state.hpp` (the state machine), `crc32_polynomial.hpp` (the CRC the GS recomputes), `valves/` (per-board valve identities). |
| `peripherals/` | Per-peripheral telemetry sub-structs (`adc/`, `can/`, `ethernet/`, `storage/`): `*_state`, `*_status`, `*_info`. |
| `devices/` | Per-device telemetry sub-structs (`valve/`). |

```cpp
#include "framing/ethernet_header.hpp"
#include "command/command_type.hpp"
#include "telemetry/fcu_system_state.hpp"
```

## CMake

```cmake
add_subdirectory(common-protocol)          # or FetchContent
target_link_libraries(your_target PRIVATE common_protocol)
```

`common_protocol` is an `INTERFACE` target that puts the repo root on your include path.

## Reliable commands — the `seq` field

Both `EthernetHeader` and `FrameCanHeader` carry a `seq` field. The **GS stamps it** on a
command; it is propagated through the FCU to the ECU, echoed back in the response, and
relayed to the GS so the GS can match a response to the command it sent (and retry on
timeout — UDP is lossy). **The CAN `seq` is 4-bit, so the GS must use `seq` values
0–15** for it to round-trip losslessly.

## Not here: the CAN fragmentation codec

A telemetry record is larger than a CAN frame, so the ECU splits it across CAN frames and
the FCU reassembles it. That **fragmentation codec is firmware-internal (FCU↔ECU) logic,
not wire protocol the GS needs** — the GS receives whole records over UDP — so it lives in
the firmware repo, not here. This repo is strictly the shared *definitions*.
