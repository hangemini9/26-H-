# mspm0dache

MSPM0G3507 LaunchPad rear-drive differential-chassis firmware.

The chassis has two MG513XP28_12V drive motors at the rear and two passive
casters at the front. The first firmware is intentionally a safe commissioning
and calibration build. The current download path uses the LaunchPad onboard
XDS110 through its Micro-USB connector. Build 2026080104 implements all
eight button inputs and all five MC02 V2 question paths while keeping
standalone motion disabled. K1/K3/K4/K5 start their assigned chassis route
only after PREPARE/READY/START; K2 reports logical RUNNING while both PWM
outputs remain disabled for the stationary ball task. Q1 alone uses the
front line-sensor row as the final A datum through a bounded forward-align,
settle and reverse-marker sequence. Q3 stops after the measured 1.5 m A-B
straight; Q4/Q5 finish one lap without reversing. Q3/Q4/Q5 use a gentler
common-speed ramp while retaining fast steering correction. Q1 uses a
40-second safety timeout with a 143 RPM straight target and 110 RPM curve
floor, exactly 10% above the previous Q1 profile, to preserve the scored goal
of finishing within 20 seconds. No Q2-Q5 speed or ramp is changed.

中文上手说明见 [调试与标定指南.md](调试与标定指南.md)。
逐线硬件连接见 [接线指导.md](接线指导.md)。

Mechanical reference: [docs/R3X_chassis_320x240mm.pdf](docs/R3X_chassis_320x240mm.pdf).
The source PDF title says "four-wheel differential", but this firmware follows
the confirmed physical configuration of two driven rear wheels and two passive
front casters.

## Confirmed hardware

| Item | Configuration |
|---|---|
| MCU | LP-MSPM0G3507 |
| Motor driver | SeekFree dual-channel DRV8701E, PH/EN interface |
| Motors | 2 x MG513XP28_12V, nominal 1:28 gearbox |
| Encoders | 13 PPR quadrature Hall, measured left 1467/right 1468 counts/wheel revolution |
| Wheel diameter | 65.0 mm |
| Drive-wheel track | 214.2 mm, center to center |
| Front/rear axle spacing | 204.7 mm, metadata only |
| Tire contact width | 25.5 mm |
| Battery | 6S LiPo through regulated power conversion |
| Debug/download | LaunchPad onboard XDS110 through Micro-USB |
| Bluetooth | JDY-31 unplugged; UART0 code/config temporarily disabled |
| External button | SW1 active low on PA10/BP33 with MCU internal pull-up |
| Line sensor | Hiwonder LineFollower_8CH V1.0, 5 V supply, I2C address 0x5D |
| OLED | GM009605-class 128x64 I2C module, 3.3 V supply, address probe 0x3C then 0x3D |
| IMU | ICM42688 API placeholder only; disabled and no pins assigned |

## Build

Installed tool paths used by this project:

```text
C:\ti\mspm0_sdk_2_10_00_04
C:\ti\sysconfig_1.28.0
C:\ti\ccs\tools\compiler\ti-cgt-armllvm_5.1.1.LTS
C:\ti\ccs\utils\bin\gmake.exe
```

In VS Code, press `Ctrl+Shift+B` and select `Build MSPM0 (TEST_MODE)`, or run:

```powershell
C:\ti\ccs\utils\bin\gmake.exe -C . -j4
```

Run the complete offline verification (clean TEST_MODE build, release build,
configuration checks, ELF symbol checks, and memory report) with:

```powershell
C:\ti\ccs\utils\bin\gmake.exe -C . verify
```

The debugger-loadable ELF output is:

```text
ticlang\mspm0dache.out
```

To build and download the TEST_MODE firmware through the onboard XDS110,
select `Build and Flash MSPM0 (onboard XDS110)` in VS Code, or run:

```powershell
C:\ti\ccs\utils\bin\gmake.exe -C . flash
```

Keep all J101 jumpers in their factory-installed positions, connect only the
LaunchPad Micro-USB debug connector, and leave J102/J103 unused. CCS `DSLite`
performs the download. SEGGER Ozone does not control XDS110; use the CCS
debugger Expressions view when live writes to `g_test` are required. Source
editing and Makefile builds remain in VS Code.

### Live parameter tuning

The VS Code flash task exits after programming, so it cannot display or edit
live target variables. Run the VS Code task `Open CCS Live Tuning`; it opens
this same folder in the installed CCS 21 Theia application. In CCS:

1. Open Run and Debug.
2. Select `MSPM0G3507 XDS110 Live Tune` and start debugging.
3. If execution stops at `main`, press Continue.
4. Open the Expressions view and add only `g_test`.
5. Keep the target running and edit values in Expressions. Do not use
   breakpoints while motor power is connected.

The CCS launch configuration is `.theia/launch.json`. It loads
`ticlang/mspm0dache.out` through the onboard XDS110 and keeps the debug session
attached.

Build 2026080104 allows an intact, fresh sensor to report no active channel
during Q1's bounded straight reverse; sensor disconnect/staleness still
faults. The final marker still requires three simultaneous channels in three
fresh samples. TI also advertises commanded common acceleration/deceleration
in V2 status bits 10/11 so MC02 disables chassis feed-forward during steady
cruise. The OLED retries initialization once per second after runtime I2C
loss. Full clean verification results are recorded in `AGENT.MD`; they do not
replace powered floor commissioning.

The default build passes `TEST_MODE=1` and uses `-O0 -gdwarf-3` so global
variables remain easy to inspect. `gmake release` produces a build with the
test command executor disabled as `ticlang\mspm0dache_release.out`; that
build remains stopped because no production command source has been selected
yet.

## Pin assignment

| Function | MSPM0G3507 pin |
|---|---|
| Right motor PWM / DRV PWM1 | PA12 / TIMG0_CCP0 |
| Right motor direction / DRV DIR1 | PB4 |
| Left motor PWM / DRV PWM2 | PA13 / TIMG0_CCP1 |
| Left motor direction / DRV DIR2 | PB1 |
| Right encoder A/B | PB6 / PB7, J2 BP13/BP14, TIMG8 QEI |
| Left encoder A/B | PB16 / PB0, J2 BP11/BP12, GPIO both-edge IRQ |
| External button-panel SW1 | PA10 / BoosterPack 33, active low, internal pull-up |
| Temporarily unused | PA11 / BoosterPack 34 |
| Hiwonder line sensor SCL | PB2 / I2C1 SCL / BoosterPack 9 |
| Hiwonder line sensor SDA | PB3 / I2C1 SDA / BoosterPack 10 |
| OLED SCK/SDA | shared PB2/PB3 I2C1 bus with the line sensor |
| MC02 supervisor TX | PA8 / UART1 TX / J1_4 |
| MC02 supervisor RX | PA9 / UART1 RX / J14_3 |

The JDY-31 is unplugged and UART0 is not generated or linked in this build.
The retained `bluetooth.c/.h` and PC helper are inactive reference code.
Only button-panel SW1 is assigned; SW2-SW8, onboard S2, PA11, buzzer, and
ICM42688 control are unused. UART1 uses 115200 8N1 on PA8/PA9 for the MC02
binary supervisor link.

Expand `g_test.line` to observe `connected`, `sample_sequence`, `error_count`,
the eight-bit `state`, `active_count`, `line_lost`, `position_x1000`, and the
eight `analog[]` and `threshold[]` values. `position_x1000` is negative toward
S1/vehicle-left and positive toward S8/vehicle-right; one adjacent-sensor
spacing is 1000. Its value is invalid whenever `line_lost` is 1.
TEST_MODE build 2026080104 provides guarded competition line following as
mode 8. It uses `base_rpm` and `steering_kp`; `correction_rpm` and the two
wheel targets are telemetry. `steering_kd` damps residual heading after a
turn. `lap_phase` exposes the Q1 competition sequence, `lap_distance_mm`
reports total run travel, `turn_count`/`lap_yaw_deg` report the private
encoder-yaw turn estimate, and `finish_distance_mm` reports forward-align
distance in phase 3 and reverse distance in phase 5.
`marker_recent_state/count` expose the 120 ms reverse-search channel union as
diagnostics only. For Q1, a reverse marker candidate requires at least three
channels simultaneously active in three consecutive fresh samples,
preventing a drifting ordinary one/two-channel line from being accumulated
into a false marker. Q3/Q4/Q5
never enter the Q1 reverse phases. Their average/common wheel request uses
30 RPM/s for Q3 and 14 RPM/s for Q4/Q5 acceleration and deceleration slew;
their differential steering request retains a 300 RPM/s slew so gentler ball
handling does not make curve correction unresponsive. Q3 begins its gentle
stop at 1500 mm; Q4/Q5 use 70 RPM cruise and begin
the same stop after the existing two-turn/distance lap gate.

The OLED stopwatch starts when the accepted MC02 START reaches actual Q2
execution or, for a moving question, when the fresh-sensor gate succeeds and
the motor controller is armed. It updates once per second while running and
freezes when the chassis run ends. The value is rendered beside `TIME` as
three decimal seconds plus `S`; only the displayed three digits wrap after
999 seconds, while the internal timer never wraps there. Each change is
written as one complete four-character refresh. If three consecutive runtime
OLED writes fail, the display reconnects and redraws automatically once per second. A
K1-K5 press by itself no longer starts the displayed timer.
The first test must use raised drive wheels.
Sensor disconnect or stalled sampling removes drive immediately and faults
within 100 ms. Outside Q1 reverse, a line-only loss uses the last valid side
to search at reduced speed for at most 300 ms, then stops and disarms with
status -6. During Q1's bounded straight reverse, zero active channels does
not stop the reverse; the existing 400 mm/6 s bounds remain authoritative.
The first 50 ms of other line loss holds prior targets. Line position uses a
0.25 low-pass factor before curve-speed and PD calculation.
Before connecting PB2/PB3, power the sensor from its regulated 5 V rail with
common ground and measure idle SCL/SDA to ground. Connect them directly only
if both are no higher than 3.3 V; otherwise use a bidirectional I2C level
shifter. Do not power the sensor from J11 3V3.

## Debugger test workflow

Download with the VS Code XDS110 task above. For all commissioning control
and telemetry, attach CCS through the same onboard XDS110 and add only the
single expression `g_test`, then expand it. Set `duration_ms`, `target_rpm`,
and `left/right.kp/ki/kd` only while stopped. Encoder counts, RPM, error,
applied PWM, PID integral/output, button/countdown state, and safety faults
update inside the same structure. Line-sensor telemetry is under
`g_test.line`. The legacy
[mspm0dache.jdebug](mspm0dache.jdebug) file is retained only in case J-Link is
used again later.

Reset state is always:

```text
g_test.mode = 0
g_test.arm = 0
both PWM outputs = 0
```

For every motion command:

1. Set all parameters first.
2. Set `g_test.arm = 1`.
3. Set `g_test.mode` last.
4. The firmware consumes and clears `arm`.
5. It stops and disarms at completion or timeout.

Setting `g_test.mode = 0` also stops and disarms within the 10 ms task period.

| mode | Operation | Needs arm |
|---:|---|:---:|
| 0 | Stop/disarm | No |
| 1 | Stop and observe encoder telemetry | No |
| 2 | Reset both encoder totals | No |
| 3 | Right wheel only, positive open-loop PWM | Yes |
| 4 | Left wheel only, positive open-loop PWM | Yes |
| 5 | Both wheels positive open-loop PWM | Yes |
| 6 | Both wheels negative open-loop PWM | Yes |
| 7 | Both wheels closed-loop at `target_rpm` | Yes |
| 8 | Low-speed black-line following | Yes |

Motion results:

| status | Meaning |
|---:|---|
| -6 | Line sensor disconnected/stale for 100 ms, or line not reacquired within 300 ms |
| -5 | Required motor output never started or had the wrong sign |
| -4 | A required wheel encoder did not count or stopped counting |
| -3 | Bad parameter |
| -2 | Motion timed out |
| -1 | Motion requested without `arm = 1` |
| 0 | Idle |
| 1 | Running |
| 2 | Completed |
| 3 | Start pending / fresh line-sensor validation |
| 4 | Two-turn estimate unavailable; stopped by the 6.1 m distance fallback |

Open-loop diagnostic modes use a fixed 15% PWM. `duration_ms` defaults to
20000 ms and is frozen when a run starts. Modes 3-7 are hard-limited to
30000 ms; mode 8 is separately hard-limited to 120000 ms for the long test
track. Closed-loop modes require at least 2000 ms. Output has an absolute 40%
PWM safety cap that is not debugger-writable.

Modes 7 and 8 require both encoders to show continued progress in the commanded
direction. A missing, stalled, or oppositely signed encoder stops the run.
Each wheel's first 750 ms encoder window begins only after actual PWM has
cleared any direction-change hold. Once supervision starts, it never pauses:
zero output with no continued wheel motion still faults within the window,
and a non-zero wrong-sign output faults immediately. Mode 7 must verify both
encoders for at least one complete window before reporting success; see
`encoder_verified_mask`. Output readiness/direction uses `output_fault_mask`.

`g_test.status` is output telemetry, not a control input. The timeout and
fault state machine is private, so accidentally editing `status` in a debugger
cannot suspend protection.

For the first mode-8 test, raise the drive wheels and keep the sensor at its
calibrated gap above the track. Set `duration_ms = 5000`,
`line.base_rpm = 30`, and `line.steering_kp = 6`, then set `arm = 1` and set
`mode = 8` last. A centered S4/S5 line produces equal targets. Moving the line
toward S1 makes the left target lower and right target higher; moving it
toward S8 does the opposite. Removing the line must produce status -6 and
`motor_armed = 0` after the 300 ms directional-recovery window. Disconnecting
I2C or freezing samples must remove drive immediately and fault within about
100 ms. Only after these checks pass should a short floor test be attempted.

### MC02-supervised KEY1 formal-course control

Button-panel SW1 is connected to PA10/BP33, panel VCC to LaunchPad 3V3, and
panel GND to system GND. It is active low with the MSPM0 internal pull-up,
debounced for 40 ms, and polled by the 10 ms test task. Never power panel VCC
from 5 V.

1. Press/release KEY1 once. TI sends a debounced button event to MC02. It
   neither starts the stopwatch nor arms the motors at button time.
2. MC02 sends PREPARE, waits for TI READY, uses a three-second countdown
   interval, then sends START. For moving questions TI still requires five
   newly acquired valid line
   samples within 500 ms before arming.
3. Starting on the transverse bar, the first 100 mm commands equal left/right
   targets at the slow-start speed before normal centroid steering takes
   over. The run uses 30 RPM for the first second, a 55 RPM first-300-mm cap,
   143 RPM Q1 straight cruise and 110 RPM Q1 curve floor. The course model is two
   1500 mm straights plus two radius-500 mm semicircles: 6141.6 mm nominal
   centerline length. The Q1 safety timeout is 40,000 ms while the scored
   completion target remains 20 seconds.
4. After the two-turn/distance finish gate, TI follows 150-250 mm farther
   into the next straight at 60 RPM and requires eight fresh centered
   one/two-channel samples. It commands true zero and waits for both wheels
   to settle below 2 RPM, then reverses with equal 25 RPM targets. A is
   accepted only after at least three channels are simultaneously active in
   three consecutive fresh sensor samples. The 120 ms union remains visible
   for diagnosis but does not grant stop authority. Failure
   to find A within 400 mm or 6 seconds stops as a route timeout.
5. Q1 route completion, MC02 SAFE_STOP, a fault, or a link-loss stop freezes
   the displayed integer seconds. OLED failure never grants or removes motor
   authority.

`STANDALONE_KEY1_START=0` makes PREPARE/READY/START the only motion
authority. K1-K5 select the five internal questions. K6 toggles a software
chassis-power permission only while TI/MC02 are online and idle: OLED
`PWR 0` blocks Q1/Q3/Q4/Q5, while `PWR 1` permits their later START.
Q2 remains stationary and works with `PWR 0`. SAFE_STOP, K7, K8, faults and
link loss all restore `PWR 0`; this is not a physical 12 V disconnect.
K7 is normal stop/reset and K8 is the latched emergency stop. The physical
motor-power disconnect remains mandatory during first communication tests.

Expand `g_test.mc02` for compact link telemetry: `online`, `state`, `fault`,
`run_id`, `last_command_id`, `question_id`, and RX/CRC/TX counters.

### JDY-31 Bluetooth status

The JDY-31 module has been unplugged at the user's request. UART0 and all
Bluetooth start/heartbeat/status handling are excluded from build 2026073006.
The source and helper script are retained only so Bluetooth can be restored
later. PA10 is now SW1 and must not be connected to the JDY-31 at the same
time.

The motor layer also guards direction reversal: PWM is removed, the wheel is
given at least 1000 ms to coast and must then read below 2 RPM, then the DIR
line receives a further 10 ms setup interval before PWM can resume. Encoder
feedback can never shorten the fixed 1000 ms interval.

## Mandatory first commissioning order

Read [COMMISSIONING.md](COMMISSIONING.md) before connecting motor power.
The short version is:

1. Keep the DRV8701E motor power disconnected and verify reset PWM is zero.
2. Power both encoders from verified 5 V, pass all four push-pull A/B outputs
   through proper 5 V-to-3.3 V level translation, and measure each MCU-side
   high level before connecting it to the MCU.
3. Calibrate left and right counts per wheel revolution independently.
4. Verify each PWM/DIR channel with a scope.
5. Apply regulated 12 V, lift both wheels, and start at 10-15% PWM.
6. Tune PID only after encoder scale and direction are correct.
7. Calibrate effective track using repeated in-place turns.

Do not halt the CPU at a breakpoint while motor power is enabled. A halted
core cannot execute the timeout, and a PWM timer may stop while its output is
high. Use live Expressions updates without breakpoints for powered testing.

## Source map

- `chassis_config.h`: geometry, encoder placeholders, signs, safety limits.
- `motor.c/.h`: PH/EN output, QEI/GPIO encoders, filtered RPM, PID, arming.
- `chassis.c/.h`: differential kinematics and odometry conversions.
- `test_mode.c/.h`: one-shot debugger command interface and telemetry.
- `imu.c/.h`: disabled ICM42688 placeholder.
- `mspm0dache.syscfg`: authoritative peripheral and pin allocation.
- `AGENT.MD`: handoff state for future agents and conversations.
