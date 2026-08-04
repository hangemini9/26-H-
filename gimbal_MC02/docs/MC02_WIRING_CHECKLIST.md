# DM-MC-Board02 wiring and commissioning checklist

Applies to MC02 build `2026073106`, DM-J4310-2EC V1.2, Jetson Orin Nano,
TI LP-MSPM0G3507 and J-Link.

Source checked visually against DM-MC-Board02 V1.1 manual pages 3, 4, 6, 7,
13 and 15. Connector numbering below follows the keyed connector front views
in that manual. Always confirm the board silkscreen before inserting a cable.

## 1. Non-negotiable rules

- [ ] Disconnect the 6S battery before changing power, CAN or UART wiring.
- [ ] Keep an accessible master switch or battery connector for emergency stop.
- [ ] Add a fuse in the 6S positive lead near the battery.
- [ ] Never power Jetson from an unverified MC02 5 V/24 V output.
- [ ] Never use J-Link VTref as a board power source.
- [ ] Never join two 5 V or 3.3 V regulator outputs together.
- [ ] All signal-linked controllers share GND.
- [ ] Do not hot-plug CAN or UART while motor power is present.
- [ ] Secure the motor and rail; remove the ball for first powered motion.
- [ ] Do not halt Ozone on a breakpoint while the motor is enabled.
- [ ] Never place, move or remove handheld meter probes on a live XT30 or
      exposed high-current output. If a voltage check is essential, disconnect
      the battery, attach insulated clip leads securely, cover exposed metal,
      check that the clips cannot bridge adjacent contacts, and only then
      energize. Disconnect power again before touching the clips.
- [ ] After the initial polarity/on/off validation has passed, use serial,
      Watch and CAN feedback for routine checks instead of repeating live
      voltage measurements.

## 2. MC02 power

The manual specifies MC02 input `12-24 V` and explicitly supports 6S.

Recommended battery tree:

```text
6S battery
  -> fuse
  -> master switch
  -> branch A: MC02 VCC_INPUT/GND
  -> branch B: fused motor power for initial CAN commissioning
  -> separate suitable regulator: Jetson
  -> separate suitable supply/regulator: TI chassis system
```

MC02 controlled output is documented as 5 A continuous per channel.
DM-J4310 nominal input is compatible with limited bench operation, but its
peak demand can exceed the documented MC02 output capability. Do not assume
the MC02 output can supply stall/impact current.

Checks before power:

- [ ] Battery is within the allowed MC02 range.
- [ ] XT30 polarity matches board `VCC_INPUT` and `GND`.
- [ ] Fuse and wire gauge are appropriate.
- [ ] No loose strands or exposed conductor can touch the PCB.
- [ ] Jetson and TI supplies are not back-feeding MC02.

## 3. J-Link SWD connection

MC02 manual SWD connector:

| MC02 SWD pin | Signal | Connect to J-Link |
|---:|---|---|
| 1 | VCC 3.3 V | VTref |
| 2 | GND | GND |
| 3 | SWCLK / PA14 | SWCLK |
| 4 | SWDIO / PA13 | SWDIO |

The four-pin MC02 SWD connector does not expose NRST. Use connect-under-reset
only if an additional verified reset connection is provided; otherwise use
normal SWD connection and the board reset button.

- [ ] Power MC02 separately from USB or its main input.
- [ ] Measure SWD pin 1 near 3.3 V before attaching J-Link VTref.
- [ ] Set Ozone device `STM32H723VG`, target interface SWD, initial speed 4 MHz.
- [ ] Load `build/Debug/gimbal_MC02.elf`.

## 4. DM4310 CAN and power

The current firmware uses FDCAN1:

```text
MC02 MCU PD0 = FDCAN1_RX
MC02 MCU PD1 = FDCAN1_TX
DM4310 CAN ID = 0x01
DM4310 Master/feedback ID = 0xF1
Classic standard CAN = 1 Mbit/s
```

### 4.1 Initial recommended wiring: motor power separate

Use this for the first CAN/feedback and small-motion tests.

- Motor power comes from a separately fused 6S branch.
- MC02 CAN1 two-pin connector:

| CAN1 pin | Signal |
|---:|---|
| 1 | CAN_L |
| 2 | CAN_H |

- Connect MC02 CAN_L to motor CAN_L.
- Connect MC02 CAN_H to motor CAN_H.
- Connect MC02 GND to motor power GND/common system GND.

This separates CAN testing from the still-uncommissioned controlled XT30
power-output path.

### 4.2 Later integrated OUT1 connector

The MC02 `VCC_OUT1 / PC14 + CAN1` XT30 2+2 port is the matching integrated
port for firmware FDCAN1:

| OUT1 pin | Signal |
|---:|---|
| 1 | VCC |
| 2 | GND |
| 3 | CAN_H |
| 4 | CAN_L |

This four-pin order follows the keyed connector front view and table on
manual page 4. It is intentionally different from the separate two-pin CAN1
connector, where pin 1 is CAN_L and pin 2 is CAN_H. Never infer either order
from wire color. The currently used keyed 2+2 cable has already passed CAN
communication testing and should not be repinned merely because an older
copy of this checklist had pins 3/4 reversed.

Do not accidentally use OUT2/CAN2; the present firmware does not initialize
FDCAN2.

Build `2026073008` keeps PC14/PC13 low at boot so both controlled 24 V
outputs default OFF. OUT2 has no enable path. The normal OUT1 path is
`PWRARM 4310` followed by `PWRON` within 3 seconds. A manual-only build with
`GIMBAL_ENABLE_MANUAL_CALRUN=1` also permits the explicit bounded
`CALRUN 4310 deg` sequence; vehicle builds must set this flag to 0. OUT1 turns
off after 300 seconds. `PWROFF`, `STOP`, any fault, non-JOG motion timeout and reset
also turn it off.
CALRUN keeps transmitting torque-disable frames while waiting up to 15 seconds
for cold-start feedback; a no-feedback timeout powers off and returns SAFE_IDLE.
For the next ball-free linkage check only, CALRUN accepts up to +/-45 degrees
with five-second legs; ordinary JOG/STEP/SWEEP remain capped at +/-10 degrees.
Use the CMake `Vehicle` preset for the disabled build; it sets
`GIMBAL_MANUAL_CALRUN=OFF` and removes the active command path.

Before ever connecting a motor to OUT1:

- [ ] Power MC02 with OUT1 disconnected.
- [ ] Run build `2026072907`.
- [ ] If measurement is still required, attach insulated meter clips from
      OUT1 pin 1 to pin 2 while all power is disconnected and secure them
      against movement or contact bridging.
- [ ] Apply power; expected boot result is approximately 0 V.
- [ ] If battery voltage appears, disconnect power and report it immediately.
- [ ] With the motor unplugged, send `PWRARM 4310`, then `PWRON`; verify OUT1
      becomes approximately the main-input voltage.
- [ ] Verify `PWROFF` immediately returns OUT1 to approximately 0 V.
- [ ] Repeat and verify automatic shutoff returns OUT1 to approximately 0 V
      after 300 seconds.
- [ ] Repeat and verify `STOP` immediately returns OUT1 to approximately 0 V.
- [ ] Disconnect battery power before removing the fixed meter clips.
- [ ] Do not repeat this full voltage sequence after it has passed unless a
      hardware fault or wiring change creates a specific need.
- [ ] Measure load current and connector/PMOS temperature during later tests.

### 4.3 CAN termination checks

With all power disconnected:

- [ ] Identify whether the motor contains/enables a 120-ohm terminator.
- [ ] Identify the MC02 CAN1 `120R` jumper/resistor state.
- [ ] Across CAN_H and CAN_L, a properly terminated two-end bus normally
      measures about 60 ohms.
- [ ] Do not install more than two 120-ohm terminators.
- [ ] Keep the first bench CAN cable short and twisted.

## 5. Jetson connection

```text
Jetson USB host -> data-capable USB cable -> MC02 USB Type-C device port
```

- [ ] Do not use a charge-only USB cable.
- [ ] Jetson sees VID:PID `0483:5740`.
- [ ] Use `/dev/serial/by-id`; do not assume `ttyACM0`.
- [ ] Camera remains connected to Jetson, not MC02.
- [ ] Jetson has its own adequate regulator and cooling.
- [ ] Jetson/MC02 binary protocol follows `JETSON_MC02_PROTOCOL_V3.md`.

USB ground provides a signal reference, but the main high-current system
ground must still be intentionally designed rather than relying on the USB
shield/cable.

### 5.1 PC commissioning console versus Jetson

The current ASCII `PING/CALRUN/STATUS/...` commissioning console is the same
native USB CDC device on the MC02 Type-C port. During bench debugging:

```text
PC USB host -> MC02 Type-C -> virtual COM port
```

During final operation:

```text
Jetson USB host -> MC02 Type-C -> V3 binary protocol
```

The MC02 has only one USB device connection and cannot be attached to the PC
and Jetson as two simultaneous USB hosts. Do not use a passive Y cable. When
Jetson owns the USB port, use J-Link/Ozone diagnostics or Jetson-side logs;
do not move the TI link onto this USB port. USB VBUS may keep MC02 logic
partially powered after the 6S battery is removed, so disconnect both battery
and USB before treating the board as fully unpowered.

## 6. TI chassis link

Preferred MC02 USART1 connector:

| MC02 pin | MC02 signal | Connect to TI |
|---:|---|---|
| 1 | USART1_TX / PA9 | TI UART1_RX / PA9 |
| 2 | USART1_RX / PA10 | TI UART1_TX / PA8 |
| 3 | GND | TI GND |

Pin numbering above follows the keyed/front view in the DM-MC-Board02 V1.1
manual: `pin 1 = PA9/TX`, middle `pin 2 = PA10/RX`, `pin 3 = GND`.

- [ ] Do not connect VCC between MC02 and TI.
- [ ] Confirm both idle TX signals are near 3.3 V.
- [ ] TI owner has added UART1 PA8/PA9 in SysConfig.
- [ ] TI owner has isolated XDS110 back-channel UART routing so there is no
      transmitter contention.
- [ ] First test at 115200 8-N-1 with both motor systems unpowered.
- [ ] Protocol follows `MC02_TI_PROTOCOL_V2.md`.
- [ ] Keep TI build `2026080104`; flash MC02 Vehicle build `2026080107`.
- [ ] With both motor rails disconnected, require MC02
      `g_gimbal_debug.ti_online=1` and increasing `ti_rx_frames`.
- [ ] Verify K1-K5 each produce the matching PREPARE, READY, three-second
      countdown, START, and eventually SAFE_STOP.
- [ ] For K2, verify TI remains disarmed at true-zero PWM until MC02's
      completed/faulted question path sends SAFE_STOP; TI's independent
      stationary-wait ceiling is 30 seconds.
- [ ] For K3, verify B_MARKER_PASSED at 1500 mm and calibrate the physical
      stop point before accepting the nominal encoder distance.
- [ ] Verify disconnecting one UART signal during an active raised-wheel test
      stops the chassis within 500 ms.

### 6.1 TI Button X8 panel

Confirmed logical mapping:

| Button | Function |
|---:|---|
| K1 | internal question 1 / printed requirement 2 |
| K2 | internal question 2 / printed requirement 3 |
| K3 | internal question 3 / printed requirement 4 |
| K4 | internal question 4 / printed requirement 5 |
| K5 | internal question 5 / printed requirement 6 |
| K6 | toggle TI chassis software-power permission; OLED `PWR 0/1` |
| K7 | normal stop/reset back to question-selection standby |
| K8 | emergency stop |

- [x] TI source assigns K1 PA10/BP34, K2 PA11/BP33, K3 PB13/BP35,
      K4 PB20/BP36, K5 PA31/BP37, K6 PA28/BP38, K7 PB12/BP19 and
      K8 PB15/BP17. Every input is active-low with an internal pull-up.
- [x] TI source debounces every press/release for 40 ms and emits `0xA2`.
- [x] K6 can enable only while both endpoints are online/idle. K6-off,
      SAFE_STOP, K7, K8, fault and link loss all restore `PWR 0`; this does
      not physically disconnect the chassis 12 V rail.
- [x] K7 source removes chassis motor authority on the first raw-low sample
      before its debounced event.
- [x] K8 source does the same and latches emergency stop.
- [x] Source logic prevents K8 release from restarting and requires K7 after
      K8 release to return to standby.
- [ ] The eight physical wires, all `0xA2` events, true-zero PWM behavior and
      latch/reset sequence have not yet been checked on hardware.
- [ ] K8 is currently connected only to TI, so it cannot guarantee gimbal
      shutdown if UART fails.
- [ ] Retain an accessible master power disconnect, or add an independent
      hardwired stop path to MC02, before calling K8 a whole-system hard
      emergency stop.

### 6.2 Ports intentionally unused in the current architecture

- USART1 is reserved exclusively for the TI link.
- UART7 and UART10 are not used.
- OUT2/CAN2, CAN3, both RS485 ports, SBUS and the four PWM outputs are not
  used.
- The onboard BMI088 IMU needs no external wire and is not used by the
  current firmware.
- Do not connect any unused 5 V pin to Jetson, TI, J-Link or another
  regulator output.

## 7. First power-on inspection sheet

Record actual measurements:

| Item | Expected | Measured |
|---|---:|---:|
| MC02 main input | 6S battery, within 12-24 V | |
| SWD VTref | about 3.3 V | |
| OUT1 VCC-GND at boot on build 2026073008 | about 0 V | |
| OUT1 VCC-GND after guarded PWRON | main-input voltage | |
| OUT1 after 180 s/PWROFF/STOP | about 0 V | |
| OUT2 VCC-GND on build 2026073008 | about 0 V | |
| CAN_H-CAN_L resistance, power off | about 60 ohm if two terminators | |
| MC02 USART1 TX idle (build 2026073106) | about 3.3 V | |
| TI UART1 TX idle | about 3.3 V | |

Photograph the wiring and fill this table before the first motor-enable test.

## 8. Commissioning order

### Stage A - logic only

- [ ] Motor power disconnected.
- [ ] Build and flash the Vehicle image `2026073106`.
- [ ] Ozone Watch shows `g_gimbal_debug.build_id = 2026073106`.
- [ ] `state = SAFE_IDLE`, `fault = 0`.
- [ ] Startup `can_tx_count` increases by at least five disable attempts.
- [ ] Watch shows `power_output_enabled = 0`.
- [ ] With the motor unplugged, perform one fixed-clip validation of guarded
      OUT1 on, PWROFF, 300-second timeout and STOP; do not move probes live.

### Stage B - CAN observe only

- [ ] Secure motor; rail/ball removed or restrained.
- [ ] Connect the keyed OUT1/CAN1 2+2 cable only after unloaded output tests.
- [ ] Keep a hand on the master switch.
- [ ] Send `PWRARM 4310`, then `PWRON`; wait at least 600 ms.
- [ ] Send `PING`, then `OBSERVE`, then `STATUS`.
- [ ] Require `ONLINE=1`, `RX` increasing and motor state 0/disabled.
- [ ] Confirm that any enabled-state or fault feedback during OBSERVE
      immediately switches OUT1 off and enters MC02 FAULT.
- [ ] Do not continue if motor red LED flashes or CAN feedback is stale.

### Stage C - bounded motor motion

- [ ] Send `ARM`, then `HOLD`; expect no large movement and auto-stop at 15 s.
- [ ] Send `OBSERVE`, `ARM`, `STEP 10`.
- [ ] Send `OBSERVE`, `ARM`, `STEP -10`.
- [ ] Record physical direction and feedback sign.
- [ ] Only after both directions pass, test `SWEEP 1.0`.
- [ ] `STOP` remains ready in the terminal.

### Stage D - fault tests

- [ ] Verify command timeout auto-disables.
- [ ] Verify manual STOP.
- [ ] After normal 10 ms CAN control is proven, restore motor CAN Timeout to
      2000 and deliberately unplug CAN to verify independent motor protection.
- [ ] Record motor LED/error code and MC02 fault state.

### Stage E - communications

- [ ] Jetson protocol parser/CRC loopback test before any vision control.
- [ ] Stream recorded/synthetic ball samples while motor remains disabled.
- [ ] TI protocol test through USB-UART adapters with chassis motor power off.
- [ ] Verify TI supervisor-heartbeat loss stops PWM within 500 ms.
- [ ] Join MC02 and TI only for the motor-power-off link test first.
