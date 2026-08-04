# Jetson Orin Nano <-> DM-MC-Board02 communication protocol V3

Status: interface frozen. The Jetson owner reports its V3 sender/parser
implemented and CRC self-tested in `../下位机协议.md`; this repository has not
independently audited that code. MC02 build `2026073106` implements the V3
byte-stream parser, heartbeat/vision watchdogs, 10 Hz `0x81` status,
state-change `0x82` events and run-ID validation.

Audience: Jetson vision owner and MC02 firmware owner.

## V3 change from V2

V3 freezes the signs that were still explicitly provisional in V2:

- ball position is positive from O toward the motor-side physical left end
  and negative toward the hinge-side physical right end;
- physical pipe angle is positive when the motor end is above level and
  negative when it is below level;
- `command_angle_mdeg` reports that physical pipe angle, not DM4310 motor
  angle.

Message types, payload layouts, field widths, units and transport are
unchanged. Because a sign mismatch can reverse the closed loop, the on-wire
version byte is deliberately changed from `0x02` to `0x03`; V2 and V3 frames
must not be mixed. The canonical CRC vector is regenerated accordingly.

## 1. Responsibility boundary

Jetson owns:

- camera acquisition and timestamps;
- pipe-axis/end-point detection;
- ball detection;
- pixel-to-pipe-coordinate calibration;
- ball position and velocity estimation;
- confidence/validity flags;
- optional YOLO processing and video recording.

MC02 owns:

- accepting or rejecting vision samples;
- target position for the selected question;
- ball controller and rail-angle command;
- motor enable, angle/speed clamps and all safety actions;
- stale-data handling;
- run ID, question selection and system state.

There are five field-test questions. Internal `question_id` values map to the
printed H-problem requirements as follows:

| `question_id` | Printed requirement | Test |
|---:|---:|---|
| 1 | 2 | chassis lap and stop at A |
| 2 | 3 | stationary chassis, ball O -> +5 cm -> -5 cm |
| 3 | 4 | chassis starts at A and passes B, ball held near O |
| 4 | 5 | one lap through A, ball held near O |
| 5 | 6 | one lap through A, ball held near a specified position |

Printed requirement 1 is the common video/record/playback requirement, not a
sixth selectable test.

Jetson must never send a motor angle, motor enable command, PID output or raw
CAN frame. A bad or stale vision sample therefore cannot directly command the
DM4310.

## 2. Physical transport

- Transport: USB CDC ACM, MC02 USB device to Jetson USB host.
- USB VID:PID: `0483:5740`.
- Current product string: `STM32 Virtual ComPort`.
- Linux device selection: prefer `/dev/serial/by-id/...`; do not hardcode
  `/dev/ttyACM0`.
- Serial settings: 8 data bits, no parity, 1 stop bit, no flow control.
- USB CDC ignores the configured baud rate, but use `921600` in host code for
  a consistent configuration.
- Binary packet byte order: little-endian.
- Maximum V3 payload: 64 bytes.

The existing ASCII commissioning console remains available for manual tests.
Production Jetson communication uses only the binary frames below.

## 3. Common frame format

Every frame is:

| Offset | Size | Field | Value/meaning |
|---:|---:|---|---|
| 0 | 1 | SOF0 | `0xA5` |
| 1 | 1 | SOF1 | `0x5A` |
| 2 | 1 | version | `0x03` |
| 3 | 1 | message_type | message table below |
| 4 | 2 | payload_length | `uint16`, 0-64 |
| 6 | 2 | sequence | `uint16`, increment for every transmitted frame |
| 8 | N | payload | message-specific |
| 8+N | 2 | CRC16 | little-endian CRC |

Total frame length is `10 + payload_length`.

CRC is CRC-16/CCITT-FALSE:

- polynomial `0x1021`;
- initial value `0xFFFF`;
- no input/output reflection;
- final XOR `0x0000`;
- calculated over bytes from `version` through the final payload byte;
- SOF and the CRC bytes themselves are excluded.

Receiver behavior:

1. Search for `A5 5A`.
2. Reject a version other than 3 or payload length above 64.
3. Wait for the complete frame.
4. Reject a bad CRC without changing control state.
5. On failure, advance one byte and search for SOF again.
6. Count CRC, length, sequence-drop and timeout errors for diagnostics.

Canonical heartbeat test vector:

```text
a5 5a 03 01 08 00 01 00 78 56 34 12 02 07 00 00 2f f1
```

This is message `0x01`, sequence 1, uptime `0x12345678`, pipeline state 2,
flags `0x07`.

## 4. Jetson -> MC02 messages

### 4.1 `0x01` JETSON_HEARTBEAT

Send at 10 Hz even when no ball is detected.

Payload length: 8 bytes.

| Payload offset | Type | Field |
|---:|---|---|
| 0 | `uint32` | `jetson_uptime_ms`, monotonic clock |
| 4 | `uint8` | `pipeline_state` |
| 5 | `uint8` | `flags` |
| 6 | `uint16` | reserved, write 0 |

`pipeline_state`:

| Value | Meaning |
|---:|---|
| 0 | booting |
| 1 | camera starting |
| 2 | ready |
| 3 | degraded |
| 4 | fatal error |

Heartbeat flags:

| Bit | Name | Meaning when 1 |
|---:|---|---|
| 0 | CAMERA_READY | frames are arriving |
| 1 | DETECTOR_READY | ball/pipe detector is running |
| 2 | CALIBRATION_READY | metric pipe coordinate is available |
| 3 | YOLO_ACTIVE | current pipeline includes YOLO |
| 4 | RECORDING | video recording is active |
| 5-7 | reserved | write 0 |

### 4.2 `0x10` VISION_SAMPLE

Send once for every processed image, preferably 40-50 Hz. Do not resend an
old sample merely to maintain the link; heartbeat is separate.

Payload length: 32 bytes.

| Offset | Type | Field | Unit/range |
|---:|---|---|---|
| 0 | `uint32` | `run_id` | echo MC02 status; 0 before a run |
| 4 | `uint32` | `capture_time_ms` | Jetson monotonic clock at exposure/frame capture |
| 8 | `uint32` | `processing_latency_us` | transmit time minus capture time |
| 12 | `int32` | `position_um` | ball position relative to pipe center |
| 16 | `int32` | `velocity_um_s` | filtered ball velocity |
| 20 | `uint16` | `confidence_permille` | 0-1000 |
| 22 | `uint16` | `flags` | validity flags |
| 24 | `int16` | `ball_axis_px` | raw diagnostic pipe-axis coordinate |
| 26 | `int16` | `negative_end_px` | raw coordinate of negative travel end |
| 28 | `int16` | `positive_end_px` | raw coordinate of positive travel end |
| 30 | `int16` | `pipe_center_px` | raw coordinate of detected pipe center |

The usable physical coordinate must cover the entire pipe:

```text
motor / left end       center O       hinge / right end
    +125000 um             0               -125000 um
```

The task's +/-5 cm requirement is not the sensor range. Jetson must retain
the full +/-12.5 cm range for internal question 5 / printed requirement 6.

Coordinate convention:

- position 0 is the detected/calibrated pipe motion center;
- positive ball position is fixed from O toward the motor-side physical left
  end; negative is toward the hinge-side physical right end;
- this sign is geometric and never changes when the downhill direction
  changes;
- raw pixel direction is camera-dependent. Jetson calibration must populate
  `positive_end_px` with the motor/left endpoint and `negative_end_px` with
  the hinge/right endpoint even if their numeric pixel order is reversed;
- physical pipe angle is
  `alpha_pipe = atan2(h_motor - h_hinge, L)`: motor end above level is
  positive and motor end below level is negative;
- the installed mechanism is verified to map positive DM4310 motor offset to
  negative pipe angle and therefore positive ball acceleration toward the
  motor/left end;
- the spatial sign and installed actuation sign were physically confirmed on
  2026-07-30;
- do not set `CALIBRATED` until center, endpoints and sign are verified.

Vision flags:

| Bit | Name | Meaning when 1 |
|---:|---|---|
| 0 | BALL_VALID | ball location valid in this frame |
| 1 | PIPE_VALID | pipe axis and endpoints valid |
| 2 | CALIBRATED | `position_um` is valid metric data |
| 3 | VELOCITY_VALID | `velocity_um_s` is valid |
| 4 | OCCLUDED | partial occlusion detected |
| 5 | FRAME_DROPPED | pipeline detected skipped input/output frames |
| 6 | OUT_OF_RANGE | ball outside calibrated usable segment |
| 7-15 | reserved | write 0 |

For closed loop, MC02 requires bits 0, 1 and 2. If they are not all set,
MC02 may log the raw diagnostic fields but must not use position for control.

When detection is invalid:

- clear the corresponding validity bits;
- set `confidence_permille` to 0;
- set invalid metric values to 0;
- still transmit the frame so loss of detection is explicit.

## 5. MC02 -> Jetson messages

### 5.1 `0x81` GIMBAL_STATUS

Send at 10 Hz.

Payload length: 24 bytes.

| Offset | Type | Field | Meaning |
|---:|---|---|---|
| 0 | `uint32` | `mc02_uptime_ms` | MC02 monotonic clock |
| 4 | `uint32` | `run_id` | current run; changes on every new start |
| 8 | `uint8` | `question_id` | 0 idle, 1-5 selected question |
| 9 | `uint8` | `system_state` | table below |
| 10 | `uint16` | `fault_code` | 0 means no system fault |
| 12 | `int32` | `target_position_um` | ball target selected by MC02 |
| 16 | `int32` | `command_angle_mdeg` | diagnostic physical pipe angle; positive means motor end up |
| 20 | `uint16` | `vision_age_ms` | age of latest accepted sample |
| 22 | `uint8` | `motor_error` | raw DM4310 status/error nibble |
| 23 | `uint8` | `flags` | status flags |

Protocol `system_state` values:

| Value | Meaning |
|---:|---|
| 0 | boot |
| 1 | idle/safe |
| 2 | waiting for valid vision |
| 3 | stationary balancing |
| 4 | waiting for chassis ready |
| 5 | chassis running |
| 6 | run complete/final balancing |
| 7 | fault |
| 8 | manually stopped |

Status flags:

| Bit | Name |
|---:|---|
| 0 | JETSON_ONLINE |
| 1 | VISION_ACCEPTED |
| 2 | MOTOR_ONLINE |
| 3 | CHASSIS_ONLINE |
| 4 | RECORDING_REQUESTED |
| 5 | MOTOR_ENABLED |
| 6 | STOP_LATCHED |
| 7 | reserved |

### 5.2 `0x82` SYSTEM_EVENT

Send on state-changing events. Jetson should record these alongside video.

Payload length: 12 bytes.

| Offset | Type | Field |
|---:|---|---|
| 0 | `uint32` | `run_id` |
| 4 | `uint16` | `event_code` |
| 6 | `uint16` | `detail` |
| 8 | `uint32` | `mc02_event_time_ms` |

Initial event codes:

| Value | Event |
|---:|---|
| 1 | question selected |
| 2 | run armed |
| 3 | run started |
| 4 | chassis lap complete |
| 5 | run complete |
| 6 | manual stop |
| 7 | fault |

Unknown event codes must be logged and otherwise ignored.

## 6. Timing and failure behavior

- Jetson heartbeat: 10 Hz.
- Vision samples: every processed frame, target 40-50 Hz.
- MC02 status: 10 Hz.
- No accepted vision sample for 100 ms: MC02 leaves active balance control,
  commands the safe level/hold policy and marks vision stale.
- No valid Jetson heartbeat for 500 ms: MC02 latches Jetson-lost fault.
- Wrong `run_id` during an active run: discard sample from control.
- Excessive processing latency or a non-monotonic/out-of-order sequence:
  count and discard as configured; never extrapolate indefinitely.
- On reconnect, Jetson reads the current MC02 status/run ID and resumes with
  a new sequence. It must not request automatic motor re-enable.

The exact safe rail policy on stale vision will be finalized during controller
implementation; it must never preserve an old nonzero tilt indefinitely.

## 7. Jetson implementation checklist

- [ ] Enumerate by VID:PID or `/dev/serial/by-id`.
- [ ] Implement a byte-stream parser that tolerates arbitrary USB packet splits.
- [ ] Implement CRC-16/CCITT-FALSE and verify the canonical test vector.
- [ ] Use explicit little-endian `struct.pack`, never native packed structs.
- [ ] Use monotonic timestamps, not wall-clock/Unix time.
- [ ] Stream heartbeat independently from vision inference.
- [ ] Preserve full +/-125 mm metric range.
- [ ] Send raw pipe-axis pixel diagnostics in addition to metric output.
- [ ] Clear validity flags on each bad frame.
- [ ] Parse MC02 status and echo its `run_id`.
- [ ] Log sequence gaps, CRC errors, reconnects and processing latency.
- [ ] Do not send angle, current, CAN or motor-enable commands.

Reference CRC implementation:

```python
def crc16_ccitt_false(data: bytes) -> int:
    crc = 0xFFFF
    for value in data:
        crc ^= value << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 \
                  else (crc << 1) & 0xFFFF
    return crc
```
