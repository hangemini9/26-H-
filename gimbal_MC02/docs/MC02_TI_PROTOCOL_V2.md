# DM-MC-Board02 <-> TI LaunchPad chassis protocol V2

Status: V2 envelope is frozen and implemented in the selectively rolled-back
MC02 Vehicle build `2026080107` and current TI build `2026080104`. All five
question IDs implement the
PREPARE/READY/START/SAFE_STOP path. Q2 remains physically disarmed while the
gimbal executes its stationary sequence; Q3 ends at the encoder-derived
1500 mm B crossing; Q1/Q4/Q5 use the existing one-lap route. Compilation is
verified, while electrical UART, watchdog and end-to-end behavior still
require hardware tests after each new build.

Build 2026073125 does not change this wire format. While exactly one of the
existing common-speed ramp flags is active, MC02 combines the wheel-RPM
derivative with the known 50 rpm/s commanded ramp and applies a signed
0.17 m/s2 minimum acceleration estimate. Steady cruise still forces chassis
feed-forward to zero. For Q3-Q5, route completion starts a minimum five-second
gimbal hold; the existing three-second continuous ball-settle condition may
extend that hold up to the unchanged fifteen-second deadline.

Build 2026073126/2026073119 also leaves the wire format unchanged. Q3-Q5
common acceleration and deceleration are reduced from 50 to 40 rpm/s, and
MC02's phase-gated model floor is correspondingly reduced from 0.17 to
0.14 m/s2. Cruise and steering commands are unchanged.

TI build 2026080101 changes only local OLED startup timing. Its first OLED
transaction is delayed by 1000 ms and failures retain the one-second retry;
the V2 link and all control behavior are unchanged.

Builds MC02 2026080107 / TI 2026080104 retain the same wire format. MC02's
small follow-up trim is confined to its local stationary Q2 approach/capture
schedule and adds no field, flag, message or semantic change. Q3 uses
100 rpm cruise and 30 rpm/s common acceleration/deceleration. Q4/Q5 use
70 rpm cruise and 14 rpm/s. Their matched MC02 model floors are 0.10/0.048
m/s2, followed by a bounded 500 ms
linear tail when the existing ramp flag clears. Cruise and steering commands
are unchanged.

Audience: TI LP-MSPM0G3507 firmware owner and MC02 firmware owner.

## 1. Ownership

MC02 is the competition supervisor:

- accepts physical question-button events and owns the active question,
  run ID and start/stop sequencing;
- owns Jetson coordination, ball control and final run completion;
- may request the chassis to prepare, execute a route or stop.

TI is the exclusive chassis controller:

- scans and debounces the eight-button panel;
- owns line sensor, encoders, wheel PID, steering and finish-line detection;
- owns all PWM/direction outputs and chassis safety;
- decides whether it is ready to run;
- reports button events, state, distance, line position and faults;
- must stop itself if the supervisor link is lost while moving.

MC02 must never send left/right PWM or steering values. TI must never command
the DM4310 or ball target.

## 2. Proposed electrical link

Transport: 3.3 V TTL UART, `115200 8-N-1`, no flow control.

Preferred allocation, selected to avoid the TI PA10 external SW1 and existing
motor/encoder/I2C pins:

| Direction | MC02 USART1 connector | TI LP-MSPM0G3507 |
|---|---|---|
| MC02 -> TI | pin 1, USART1_TX, PA9 | UART1_RX, PA9 |
| TI -> MC02 | pin 2, USART1_RX, PA10 | UART1_TX, PA8 |
| reference | pin 3, GND | GND |

Do not connect either board's 5 V or 3.3 V rail to the other. Connect only
TX, RX and GND. TX/RX are crossed as shown.

The MC02 pin numbering follows the keyed/front view in the V1.1 board manual:
`pin 1 = PA9/TX`, middle `pin 2 = PA10/RX`, and `pin 3 = GND`.

TI UART1 PA8/PA9 is allocated in `mspm0dache.syscfg`. TI SDK documentation
maps PA8 TX to LaunchPad J1_4 and PA9 RX through the LaunchPad UART routing.
The XDS110 back-channel UART is also connected through J101. Before external
wiring, the TI owner must:

1. allocate UART1 PA8/PA9 in SysConfig and confirm no conflict;
2. identify the actual accessible header/jumper route on the physical board;
3. isolate XDS110 back-channel RX/TX as required by J101 positions 7:8 and
   9:10 so two transmitters cannot drive one line;
4. keep J101 SWD sections 13:14 and 15:16 available for XDS110 debug;
5. verify idle TX levels near 3.3 V before joining the boards.

The currently implemented external SW1 on TI PA10 may remain in place because
this proposal uses UART1 PA8/PA9, not UART0 PA10/PA11. The remaining Button
X8 inputs are not yet allocated; the TI owner must assign and electrically
verify them before implementing the mapping in section 5.3.

## 3. Frame format

The TI link reuses the same V2 envelope and CRC as the Jetson link:

| Offset | Size | Field |
|---:|---:|---|
| 0 | 1 | `0xA5` |
| 1 | 1 | `0x5A` |
| 2 | 1 | version `0x02` |
| 3 | 1 | message type |
| 4 | 2 | payload length, little-endian, maximum 64 |
| 6 | 2 | transmit sequence, little-endian |
| 8 | N | payload |
| 8+N | 2 | CRC16 little-endian |

CRC-16/CCITT-FALSE is calculated over `version` through the payload:
polynomial `0x1021`, initial `0xFFFF`, no reflection, final XOR 0.

Both parsers must operate on a byte stream and recover after noise, partial
frames and concatenated frames. Do not cast the receive buffer to a C struct.

## 4. MC02 -> TI messages

### 4.1 `0x20` SUPERVISOR_HEARTBEAT

Send at 10 Hz in all MC02 states.

Payload length: 8 bytes.

| Offset | Type | Field |
|---:|---|---|
| 0 | `uint32` | `mc02_uptime_ms` |
| 4 | `uint32` | `run_id`, 0 while idle |

### 4.2 `0x21` CHASSIS_COMMAND

Payload length: 16 bytes.

| Offset | Type | Field |
|---:|---|---|
| 0 | `uint32` | `run_id` |
| 4 | `uint16` | `command_id` |
| 6 | `uint8` | `question_id`, 1-5 |
| 7 | `uint8` | `action` |
| 8 | `uint32` | `timeout_ms` |
| 12 | `uint32` | `options` |

Actions:

| Value | Name | TI behavior |
|---:|---|---|
| 0 | SAFE_STOP | stop/disarm immediately; always accepted |
| 1 | PREPARE | validate sensors/controller, remain stopped, report READY |
| 2 | START | begin the requested route once |
| 3 | CANCEL | cancel prepare/countdown/run and remain stopped |
| 4 | CLEAR_FAULT | clear only recoverable TI faults while stopped |

V2 option bits:

| Bit | Name |
|---:|---|
| 0 | RUN_ONE_LAP |
| 1 | LOW_SPEED_PROFILE |
| 2 | STOP_AT_A_MARKER |
| 3 | COMPLETE_AFTER_B_PASSED |
| 4-31 | reserved, write 0 |

For the current H problem:

- internal question 1 = printed requirement 2:
  `RUN_ONE_LAP | STOP_AT_A_MARKER`; command safety timeout is 40 s while the
  scored target remains total time <=20 s;
- internal question 2 = printed requirement 3: chassis remains stopped while
  MC02 performs ball O -> +5 cm -> -5 cm;
- internal question 3 = printed requirement 4:
  `COMPLETE_AFTER_B_PASSED`, with A-to-B time <=8 s;
- internal question 4 = printed requirement 5:
  `RUN_ONE_LAP | STOP_AT_A_MARKER`;
- internal question 5 = printed requirement 6:
  `RUN_ONE_LAP | STOP_AT_A_MARKER`;
- printed requirement 1 is a common video/record/playback requirement, not a
  selectable sixth question;
- `question_id` is context, not permission to bypass TI safety.

Commands are idempotent:

- the unique key is `(run_id, command_id)`;
- a duplicate must return the current status and must not restart motion;
- a new `run_id` resets per-run command history only while stopped;
- SAFE_STOP is honored even if run ID or command ID is old/unknown;
- MC02 repeats SAFE_STOP for at least 500 ms or until TI status confirms idle.

Canonical SAFE_STOP test vector:

```text
a5 5a 02 21 10 00 01 00 2a 00 00 00 07 00 02 00
00 00 00 00 00 00 00 00 12 c6
```

This is run 42, command 7, question 2, SAFE_STOP, sequence 1.

## 5. TI -> MC02 messages

### 5.1 `0xA0` CHASSIS_STATUS

Send at 20 Hz, including while idle.

Payload length: 28 bytes.

| Offset | Type | Field | Unit |
|---:|---|---|---|
| 0 | `uint32` | `run_id` | - |
| 4 | `uint16` | `last_command_id` | acknowledges processed command |
| 6 | `uint8` | `chassis_state` | table below |
| 7 | `uint8` | `fault_code` | 0 no fault |
| 8 | `uint32` | `ti_uptime_ms` | ms |
| 12 | `uint32` | `run_elapsed_ms` | ms |
| 16 | `int32` | `distance_mm` | signed diagnostic distance |
| 20 | `int16` | `left_rpm_x10` | 0.1 RPM; corrected vehicle-forward positive |
| 22 | `int16` | `right_rpm_x10` | 0.1 RPM; corrected vehicle-forward positive |
| 24 | `int16` | `line_position_x1000` | normalized line error x1000 |
| 26 | `uint16` | `flags` | status bits |

Chassis states:

| Value | Name |
|---:|---|
| 0 | BOOT |
| 1 | IDLE_DISARMED |
| 2 | READY |
| 3 | RUNNING |
| 4 | STOPPING |
| 5 | ROUTE_COMPLETE |
| 6 | FAULT |
| 7 | EMERGENCY_STOP_LATCHED |

Fault codes:

| Value | Name |
|---:|---|
| 0 | NONE |
| 1 | SUPERVISOR_LOST |
| 2 | LINE_SENSOR |
| 3 | ENCODER |
| 4 | OUTPUT |
| 5 | BAD_PARAMETER |
| 6 | ROUTE_TIMEOUT |

Status flags:

| Bit | Name |
|---:|---|
| 0 | LINE_SENSOR_OK |
| 1 | LEFT_ENCODER_OK |
| 2 | RIGHT_ENCODER_OK |
| 3 | MOTOR_OUTPUT_ENABLED |
| 4 | A_MARKER_SEEN |
| 5 | SUPERVISOR_ONLINE |
| 6 | STOP_LATCHED |
| 7 | EMERGENCY_STOP_LATCHED |
| 8 | B_MARKER_SEEN |
| 9 | BUTTON_PANEL_OK |
| 10 | COMMON_ACCEL_ACTIVE |
| 11 | COMMON_DECEL_ACTIVE |
| 12-15 | reserved |

`last_command_id` is the acknowledgement. MC02 considers PREPARE successful
only when the ID matches and state is READY. START is accepted only when the
ID matches and state becomes RUNNING.

The two RPM fields remain the chassis-motion feed-forward source. TI sets
exactly one of `COMMON_ACCEL_ACTIVE` or `COMMON_DECEL_ACTIVE` only while its
common wheel-speed command is intentionally slewing. MC02 averages the
corrected left/right RPM, converts it using the configured 65 mm wheel
diameter and differentiates fresh 20 Hz status samples locally. It rejects an
estimate with the wrong sign for the advertised phase, applies a 0.05 m/s2
deadband, and forces feed-forward to zero while both motion flags are clear.
The derived value is vehicle-forward acceleration, not ball-axis
acceleration. MC02 applies its installation sign locally before using
`alpha_ff = -a_chassis_x / g`; no new payload field is introduced. MC02 also
invalidates the estimate when `MOTOR_OUTPUT_ENABLED` clears, the chassis is
no longer RUNNING, or status is stale for 500 ms. TI must continue sending
actual measured RPM, not target RPM.

### 5.2 `0xA1` CHASSIS_EVENT

Send immediately on important transitions. Status remains the authoritative
current state.

Payload length: 16 bytes.

| Offset | Type | Field |
|---:|---|---|
| 0 | `uint32` | `run_id` |
| 4 | `uint16` | `event_sequence` |
| 6 | `uint8` | `event_code` |
| 7 | `uint8` | `detail` |
| 8 | `uint32` | `run_elapsed_ms` |
| 12 | `int32` | `distance_mm` |

Event codes:

| Value | Event |
|---:|---|
| 1 | READY |
| 2 | STARTED |
| 3 | A_MARKER_DETECTED |
| 4 | ROUTE_COMPLETE |
| 5 | STOPPED |
| 6 | FAULT |
| 7 | B_MARKER_PASSED |
| 8 | EMERGENCY_STOPPED |

Events may be repeated after reconnect; MC02 deduplicates using
`(run_id, event_sequence)`.

For internal Q3-Q5, `ROUTE_COMPLETE` means the chassis route has finished and
TI has already ramped both wheel requests to zero, observed both measured
wheel speeds at or below its stop threshold, and disarmed. TI then remains in
`ROUTE_COMPLETE` until MC02 sends `SAFE_STOP`. MC02 keeps the gimbal closed
loop active and requires the ball to remain within 5 mm of the active target
and within 30 mm/s for at least 3000 ms continuously before it reports whole-
system completion and sends `SAFE_STOP`. Any invalid/stale vision sample or
limit violation resets that continuous timer. MC02 bounds this post-route
wait to 15000 ms; expiry stops both sides without reporting successful whole-
system completion. Q1 retains immediate completion after the chassis route,
and Q2 remains MC02-owned. This is a completion-handshake clarification only:
V2 version, message IDs, payload layouts, lengths and CRC are unchanged.

### 5.3 `0xA2` BUTTON_EVENT

TI sends this immediately after debouncing a button transition. Payload
length: 12 bytes.

| Offset | Type | Field |
|---:|---|---|
| 0 | `uint16` | `event_sequence` |
| 2 | `uint8` | `button_id`, 1-8 |
| 3 | `uint8` | `button_action` |
| 4 | `uint32` | `ti_uptime_ms` |
| 8 | `uint32` | `run_id_snapshot`, 0 while idle |

`button_action` values:

| Value | Meaning |
|---:|---|
| 1 | debounced press |
| 2 | debounced release |

Confirmed Button X8 mapping:

| Button | Meaning |
|---:|---|
| K1 | select internal question 1 / printed requirement 2 |
| K2 | select internal question 2 / printed requirement 3 |
| K3 | select internal question 3 / printed requirement 4 |
| K4 | select internal question 4 / printed requirement 5 |
| K5 | select internal question 5 / printed requirement 6 |
| K6 | reserved; ignore except diagnostics |
| K7 | normal stop/reset; return to question-selection standby after outputs stop |
| K8 | emergency stop |

Button behavior:

- K1-K5 press events select and start only while the overall system is in
  question-selection standby. MC02 ignores them during an active run.
- K7 must make TI command true-zero PWM and disarm locally before waiting for
  MC02. MC02 stops/disables the gimbal, cancels the active run and returns to
  selection standby after both controllers confirm safe idle.
- K8 has the highest priority. TI commands true-zero PWM and disarms in the
  same local control cycle, latches emergency stop, then reports both the
  event and status flag. MC02 disables the DM4310 immediately when either is
  received.
- Releasing K8 must never restart anything. After K8 is released, K7 is
  required to clear the operator-stop latch and return to selection standby,
  and only if no nonrecoverable fault remains.
- A button event is never direct permission for motor PWM or DM4310 enable;
  the normal PREPARE/READY/START sequence remains mandatory.

Because all eight buttons are physically connected only to TI, K8 by itself
is a communication-dependent emergency stop for the gimbal. A real whole
system emergency stop must additionally cut actuator power or use an
independent hardwired stop line to MC02. Keep the accessible master power
switch until that hardware path is implemented and verified.

## 6. Failure behavior

TI requirements:

- K7 and K8 stop chassis motion locally; they must not wait for an MC02
  acknowledgement;
- no valid SUPERVISOR_HEARTBEAT for 500 ms while RUNNING: immediately command
  zero motor output, disarm and latch supervisor-lost fault;
- bad CRC/length/version: discard without executing;
- START received while not READY: reject by retaining current state/fault;
- any internal line/encoder/output safety fault: stop first, then report;
- reset always returns to IDLE_DISARMED with zero PWM.

MC02 requirements:

- no CHASSIS_STATUS for 500 ms when chassis participation is expected:
  latch chassis-link fault and put gimbal into its safe policy;
- do not mark a route complete from elapsed time alone;
- accept ROUTE_COMPLETE only for the active run ID;
- STOP/CANCEL remain available regardless of Jetson or motor state.

The UART link supplements, but does not replace, each board's independent
watchdogs.

## 7. Bring-up sequence for the TI owner

- [ ] Implement frame/CRC parser and unit tests on a PC first.
- [ ] Add UART1 PA8 TX / PA9 RX in SysConfig; do not edit generated files.
- [ ] Preserve current PWM, encoders, PA10/K1 button and PB2/PB3 I2C
  allocation; allocate and verify K2-K8 separately.
- [ ] Verify UART1 pins and J101/XDS110 isolation on the actual LaunchPad.
- [ ] Debounce all buttons and verify the K1-K8 mapping with motor power off.
- [ ] Verify K7 and K8 set both PWM outputs to true zero locally even with
  the UART disconnected.
- [ ] Verify K8 release does not restart; require K7 to return to standby.
- [ ] With no motor power, use a USB-UART adapter at 3.3 V to test heartbeat,
  PREPARE, button events, duplicate commands, wrong CRC and SAFE_STOP.
- [ ] Verify loss of heartbeat produces zero PWM within 500 ms.
- [ ] Verify duplicate START cannot restart/reset a running course.
- [ ] Only then connect MC02 TX/RX/GND.
- [ ] Run PREPARE/READY tests with chassis wheels raised.
- [ ] Run START followed immediately by SAFE_STOP.
- [ ] Perform one low-speed lap only after all above checks pass.

Reference CRC implementation is identical to
`JETSON_MC02_PROTOCOL_V3.md`.
