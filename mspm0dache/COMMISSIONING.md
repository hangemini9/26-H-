# Hardware commissioning and calibration

## Non-negotiable electrical limits

### Encoder signals

The installed encoders require a stable 5 V VCC and have push-pull A/B
outputs. Treat every raw A/B output as potentially 5 V. Route all four A/B
signals through proper unidirectional 5 V-to-3.3 V level translation before
the MSPM0 pins unless measurement proves that the actual output-high voltage
does not exceed 3.3 V.

Before connecting A/B to the LaunchPad:

1. Power the encoder.
2. Rotate the shaft slowly.
3. Measure the high level of A and B with an oscilloscope or multimeter.
4. Connect them only after both have been verified at no more than 3.3 V.

A 5 V push-pull output must not be connected directly to MSPM0 GPIO.

### 6S battery and 12 V motors

A 6S LiPo is about 22.2 V nominal and 25.2 V fully charged. The MG513XP28
motors are rated for 12 V. Use:

```text
6S battery
  -> main fuse and physical master switch
  -> high-current regulated 12 V converter
  -> DRV8701E motor power input
  -> two 12 V motors
```

The DRV8701E board accepting a higher voltage does not make a 12 V motor safe
at 25.2 V. The LaunchPad and encoder rails also require their own correct
regulated supplies. All control grounds must share a reference, while motor
current must not return through thin MCU or encoder ground wiring.

There is no physical GPIO emergency-stop input in this firmware. A reachable
master switch or hard motor-power disconnect is therefore required.

## Onboard XDS110 connection

The current workflow uses the LaunchPad onboard XDS110. Restore every J101
jumper to its factory-installed position, leave J102/J103 unused, and do not
inject external 3.3 V into J11 during USB-powered development.

Connect the LaunchPad Micro-USB debug connector to the PC. Build and download
from VS Code with `Build and Flash MSPM0 (onboard XDS110)`, or run:

```powershell
C:\ti\ccs\utils\bin\gmake.exe -C . flash
```

Ozone does not support XDS110. Use CCS Expressions for live `g_test` writes;
source editing and Makefile builds remain in VS Code.

## Phase A: MCU only

1. Disconnect the DRV motor-power input.
2. Build and download `ticlang/mspm0dache.out`.
3. Reset the MCU.
4. Confirm in CCS Expressions:

```text
g_test.build_id == 2026080104
g_test.mode == 0
g_test.arm == 0
g_test.status == 0
```

5. Check PA12, PA13, PB4, and PB1 with a scope. All four must remain at 0 V
   after reset and while mode 0 is selected.

## Phase B: encoder scale and direction

Keep motor power disconnected.

1. Supply the encoders with a verified 5 V rail and verify the translated
   A/B levels presented to the LaunchPad are no more than 3.3 V.
2. Set `g_test.mode = 2` to clear both totals.
3. Set `g_test.mode = 1`.
4. Mark the left tire and rotate it exactly one full output-wheel revolution
   in the vehicle-forward direction.
5. Record `g_test.left.encoder_count`.
6. Reset again and repeat for the right wheel.

The 2026-07-25 ten-revolution measurements produced left 1467.475 and right
1467.95 counts per revolution; firmware build 2026072504 uses left 1467 and
right 1468. If the encoder or gearbox is replaced, measure the absolute values
again rather than relying on the nominal calculation. Update:

```c
ENCODER_LEFT_COUNTS_PER_WHEEL_REV
ENCODER_RIGHT_COUNTS_PER_WHEEL_REV
```

in `chassis_config.h`.

Forward wheel rotation should produce positive corrected totals. If it is
negative, change only the corresponding `ENCODER_*_REVERSED` flag and repeat.
Do not hide a channel swap by changing several sign flags at once.

`g_test.left_invalid_transitions` should remain close to zero during slow hand
rotation. A rapidly increasing value indicates noise, incorrect pull
configuration, or missed software-decoder transitions.

## Phase C: PWM and direction without motor power

1. Disconnect all power. Use continuity mode to prove PA12/BP32 reaches only
   PWM1, PB4/BP40 reaches only DIR1, PA13/BP31 reaches only PWM2, and
   PB1/BP39 reaches only DIR2. Do not infer the mapping from wire colors or
   connector orientation.
2. Connect DRV GND, PWM1, DIR1, PWM2, and DIR2.
3. Leave the DRV motor-power input disconnected.
4. In mode 0, verify PA12, PA13, PB4, and PB1 are all 0 V.
5. Set `g_test.duration_ms = 2000`.
6. Arm and run mode 3. Verify only PA12 has 20 kHz, 15% PWM.
7. Return to mode 0 and verify all four lines return to 0 V within 10 ms.
8. Arm and run mode 4. Verify only PA13 has 20 kHz, 15% PWM, then repeat the
   mode-0 all-low test.
9. Verify direction changes occur only while PWM is removed. The firmware
   waits at least 1000 ms for coast-down, requires measured speed below 2 RPM,
   and then gives DIR 10 ms of setup time before PWM resumes.

## Phase D: lifted-wheel motor test

1. Secure the chassis with both drive wheels off the floor.
2. Set the converter to a measured 12.0 V before connecting the DRV.
3. Open-loop diagnostic modes use a fixed 15% PWM.
4. Run mode 3 and mode 4 separately.
5. Positive PWM must rotate each physical wheel in the vehicle-forward
   direction, and its corrected encoder count must also increase.
6. Run mode 5 briefly.

If a motor direction is wrong, update the corresponding
`MOTOR_*_OUTPUT_REVERSED` flag or deliberately swap that motor's power leads.
Document which choice was made.

## Phase E: closed-loop tuning

Do not tune PID until CPR and direction are correct.

1. Set `target_rpm = 40`, `duration_ms = 5000`.
2. Arm and run mode 7.
3. Observe `g_test.left/right.rpm`, `error_rpm`, `pwm`, `integral`, and
   `pid_output`.
4. Change `g_test.left/right.kp/ki/kd` only while stopped. The gains are
   validated and applied when the next mode-7 run starts.

Mode 7 requires both encoders to keep producing counts in the commanded
direction. Its duration must be at least 2000 ms. Status `-4` indicates
encoder progress was lost or had the wrong sign; `encoder_fault_mask` bit 0
is left and bit 1 is right. Stop and diagnose sign configuration, wiring,
supply level, signal quality, or mechanical binding before retrying.

The 750 ms progress window starts only after actual PWM resumes following any
direction-change hold. `encoder_verified_mask` uses the same left/right bits;
mode 7 cannot report success until both bits have passed a complete window.
After a wheel's first window starts, supervision never pauses. Status `-5`
means actual output did not start before timeout or had the wrong sign;
`output_fault_mask` uses bit 0 for left and bit 1 for right.

Treat `g_test.status` as read-only telemetry. Editing it does not control the
private running state or disable the timeout.

The 40% closed-loop PWM cap is enforced inside the motor layer and is not
debugger-writable. Stop the test before changing target direction or gains.

The initial PID is provisional and expressed in seconds:

```text
Kp = 0.7
Ki = 20.0
Kd = 0.0
control period = 0.010 s
```

Record final left and right values in `AGENT.MD`.

Distance and turn command modes are intentionally deferred until encoder
counting and mode-7 PID commissioning pass.

## Phase F: MC02-supervised KEY1/OLED test

Build 2026080104 has `STANDALONE_KEY1_START=0`; MC02 heartbeat plus the
PREPARE/READY/START handshake is required. KEY1 only sends the debounced
button event and never directly grants PWM authority. The OLED stopwatch
starts at accepted Q2 execution, or for a moving question only after the
fresh-sensor gate succeeds and the motor controller is armed.

1. Keep the drive wheels raised and place a valid centered line under S4/S5.
2. Press/release KEY1 once. `status=3` and `start_delay_ms` counts down from
   3000 while both PWM outputs remain zero.
3. After the countdown, five newly acquired valid sensor samples are required
   within 500 ms. Only then may the motors arm.
4. Motion begins at 30 RPM for one second, remains at most 55 RPM through the
   first 300 mm, then ramps toward the formal 120 RPM/100 RPM profile.
5. Press KEY1 again during countdown or motion. It must cancel/stop and
   disarm immediately.
6. Without another press, the local course run must stop by 20,000 ms.

First connect only the crossed 3.3 V UART TX/RX and common GND with both motor
systems unpowered. Verify increasing `g_test.mc02.rx_frames` and
`g_test.mc02.online=1` before a raised-wheel Q1 test.

K7/K8 are not connected in this build. Keep the physical motor-power master
switch reachable.

## Phase G: Bluetooth temporarily disabled

The JDY-31 has been unplugged. UART0 and `bluetooth.c` are excluded from build
2026073006; PA10 is reassigned to SW1. Do not reconnect the JDY-31 to PA10
while this build is installed. The Bluetooth sources and PC helper remain in
the repository only for possible later restoration.

## Phase H: Hiwonder 8-channel line-sensor input

Build 2026080104 configures the guarded formal-course line-following controller
for MC02-supervised KEY1 start and a 40-second Q1 safety timeout; the scored
completion target remains 20 seconds.

1. Keep motor power disconnected and leave PB2/PB3 disconnected.
2. Power the `LineFollower_8CH V1.0` from a stable regulated 5 V rail and
   connect its GND to system GND.
3. Measure idle `SCL` and `SDA` at the sensor against GND. Both must be no
   higher than 3.3 V before direct connection to the MSPM0. If either is
   higher, insert a bidirectional I2C level shifter.
4. With power off, connect sensor `SCL` to PB2/BP9 and sensor `SDA` to
   PB3/BP10. Leave S1-S8 and TX/RX disconnected.
5. Flash build 2026080104. The already completed debugger tests established a
   working I2C link and the channel mapping; no debugger cable is required for
   the following motion tests.
6. Mount the array at a fixed height within the documented 0.5-8 cm range.
   Calibrate the background first by holding KEY, then place it over the
   target line and briefly press KEY.
7. Move each of S1-S8 over the learned line one at a time. Record bit order,
   bit polarity, analog response, threshold values, and which end of the
   board is physically left when the vehicle faces forward.

8. For the first motion check, raise the drive wheels while keeping the sensor
   at its calibrated track gap, then press KEY1. TI requires five newly acquired valid
   samples before arming and uses 30 RPM for the first second, 55 RPM for the
   first 300 mm, then Q1 uses 143 RPM straight base and 110 RPM curve floor,
   steering Kp 12/Kd 6,
   and an 80 RPM/s acceleration limit.
9. Centered line must produce equal targets. S1 must lower the left target and
   raise the right; S8 must do the opposite. Removing the line must stop and
   disarm with status -6 after 100 ms.
10. Press KEY1 again to end the raised-wheel test early. Only after these
    checks pass may Q1 be tested on the course with the bounded 40-second
    safety run while recording whether it meets the scored 20-second target.
    PID/output/encoder, sensor-loss, and
    duration-stop protections remain active.
11. Formal geometry is two 1500 mm straights and two radius-500 mm
    semicircles, giving 6141.6 mm centerline length and a 2500 x 1000 mm
    outer envelope. The first 100 mm from the transverse start bar uses equal
    wheel targets before centroid steering begins. After the two-turn/distance
    gate near A, the vehicle follows 150-250 mm into the next straight,
    requires eight fresh centered one/two-channel samples, stops and settles
    below 2 RPM, then reverses at equal 25 RPM. A is accepted when at least
    three channels are simultaneously active in three consecutive fresh
    samples. The 120 ms union is telemetry only. Missing A at 400 mm reverse
    or 6 seconds is a timeout, never permission for a second lap. Record `turn_count`,
    `lap_yaw_deg`, `lap_distance_mm`, `lap_phase`, `finish_distance_mm`,
    `marker_recent_state/count`, final status and measured stop offset.
12. Q3 drives the measured 1500 mm A-B route and then ramps to zero. Q4/Q5
    finish after the existing one-lap turn/distance gate; they do not perform
    Q1's reverse-marker sequence. Q3 retains 100 RPM cruise with 30 RPM/s
    common acceleration/deceleration. Q4/Q5 use 70 RPM cruise with 14 RPM/s
    common acceleration/deceleration. Their steering difference retains the
    300 RPM/s slew. Verify Q3 completes and settles within 8 s and Q4/Q5
    within 30 s while filming the payload.
13. During every accepted run, verify the OLED integer seconds advance once
    per second and freeze when the chassis finishes or is stopped.
