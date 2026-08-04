# Five-question competition control

Implementation baseline:

- MC02 `2026080107` (user-requested Q2 rollback)
- TI `2026080104`
- Jetson link V3
- MC02/TI link V2

## Question mapping

| Key | Internal ID | Printed requirement | Implemented behavior |
|---:|---:|---:|---|
| K1 | 1 | 2 | One lap, stop at A |
| K2 | 2 | 3 | Chassis disarmed; ball O -> +5 cm -> -5 cm |
| K3 | 3 | 4 | A -> B; B is the encoder-derived 1500 mm crossing |
| K4 | 4 | 5 | One lap; ball target 0 |
| K5 | 5 | 6 | One lap; capture the first calibrated ball position as the target |
| K6 | - | - | Toggle chassis software-power permission (`PWR 0/1`) |
| K7 | - | - | Normal stop/reset |
| K8 | - | - | Latched emergency stop |

After TI reports route completion for K3/K4/K5, MC02 keeps the DM4310 closed
loop active for at least 5000 ms. Completion also still requires the ball to
remain within 5 mm and 30 mm/s for 3000 ms continuously; if that takes longer,
the hold continues up to the 15000 ms route-settle deadline. Emergency stop,
link failure and invalid-vision safety paths bypass the hold and stop at once.

K3 remains at 100 rpm cruise with a 30 rpm/s common-speed acceleration/
deceleration profile. K4/K5 use 70 rpm cruise and 14 rpm/s, a 30 percent
reduction from their preceding 100/20 profile. Their 8 s / 30 s scored timing
limits are unchanged; the Q3-Q5 local gentle-stop guard is 8000 ms.

Printed requirement 1 is the common video/display/recording requirement and
is not a selectable sixth question.

## Runtime flow

1. TI sends the debounced K1-K5 `BUTTON_EVENT`.
2. MC02 allocates `run_id`, sends Jetson event `question selected`, and
   publishes the selected `question_id` in `0x81 GIMBAL_STATUS`.
3. MC02 waits for Jetson READY/calibrated vision. Q2-Q5 also power and
   initialize DM4310, capture the current mechanically-level motor position,
   and enable a zero-tilt hold.
4. For a moving question, K6 must first show `PWR 1`. MC02 sends TI PREPARE,
   waits for READY, performs a three-second countdown, then sends START.
5. TI executes the selected stationary or moving route. MC02 executes the
   ball controller and sends status at 10 Hz.
6. Q3-Q5 route completion leaves TI disarmed in `ROUTE_COMPLETE` while MC02
   keeps balancing. Whole-system completion requires `|error| <= 5 mm` and
   `|velocity| <= 30 mm/s` continuously for at least 3000 ms; then MC02
   disables the gimbal, SAFE_STOPs TI, and returns to selection standby.

## Ball controller

Frozen physical signs:

- `x > 0`: toward motor/physical left end;
- `alpha_pipe > 0`: motor end above level;
- installed positive motor offset produces negative pipe angle.

The controller uses the measured full-loop delay to project the latest
calibrated sample before applying the original PD law:

```text
x_pred = clamp(x + velocity * horizon, +/-130 mm)
horizon = clamp(total_delay_ms + age_since_MC02_receive, 0..250 ms)
a_des = Kp * (target - x_pred) - Kd * velocity
alpha_pipe = clamp(-(1.4*a_des + a_chassis_x) / g,
                   +/- max_pipe_angle)
motor_offset = -alpha_pipe / pipe_per_motor_ratio
```

`a_chassis_x` comes from the existing V2 `CHASSIS_STATUS` measured left and
right wheel RPM fields. MC02 averages the wheel speeds, differentiates the
20 Hz samples over a bounded 50 ms window and applies a 0.5 low-pass update.
TI asserts V2 flag bit 10 or 11 only during an intentional common-wheel
acceleration or deceleration ramp. MC02 accepts the estimate only while
exactly one of those gates is active, rejects magnitudes below 0.05 m/s^2 and
signs inconsistent with the advertised phase, and clamps the remainder to
+/-0.40 m/s^2. During steady cruise the estimator and feed-forward are reset
to zero. They are also invalidated whenever chassis motor output clears,
chassis state leaves RUNNING, or TI status becomes stale.
`control_chassis_forward_to_x_sign` maps vehicle-forward acceleration onto
the installed pipe x-axis; it is initialized to +1 and may be changed to -1
in Ozone if the physical mounting is reversed, or 0 for a feed-forward A/B
test. This mapping must be confirmed by one low-speed start before a full
course run.

`total_delay_ms` defaults to the measured median `125 ms` (49 FPS, 9 accepted
steps). The measured dispersion was 32 ms, so this is the first commissioning
value, not a final precision constant. The V3 `processing_latency_us` field is
decoded and exposed for diagnostics but is not added separately because it is
already contained in the measured full-loop delay.

Current Vehicle build `2026080107` retains the earlier Q2 behavior while
applying a small asymmetric trim and retaining the current Q1/Q3-Q5 code.
The +50 mm point has no
velocity or dwell gate: either raw position or the bounded delay projection
reaching +50 mm immediately switches the target to -50 mm. At the final
target, error <=7 mm and fixed-window speed <=60 mm/s must hold for 1000 ms.
Completion is held until five seconds have elapsed, and the hard MC02 safety
timeout remains 15000 ms. STOP, link, vision and motor-fault cutoffs remain
immediate.

For Q2 only, a freshly detected stationary window can use a bounded
directional breakaway command:

```text
first leg: after a 650 ms response grace, if the ball remains more than
           7 mm from +50 mm and spans no more than 2 mm for 150 ms,
           alpha_pipe = -2.0 deg for 350 ms; at most two separately
           detected pulses are permitted
second leg: the +50 mm waypoint transition immediately applies
            alpha_pipe = +1.5 deg for 250 ms; no additional negative-leg
            breakaway pulse is permitted
then resume the normal delay-compensated PD command
```

Pulses occur only while vision remains valid and are cancelled by STOP,
faults, TI safe stop or the active Q2 safety deadline. Every command stays
inside the fixed 2-degree pipe and 45-degree motor envelopes.

The two Q2 legs also use separate motion schedules. Before raw position
reaches +30 mm, the positive leg cannot command a braking-sign pipe angle:
it keeps at least -0.6 degree of forward tilt. Anywhere before the +50 mm
waypoint, a detected stationary condition while outside the 7 mm band raises
that forward request to -1.8 degrees. After +30 mm, the normal
delay-compensated PD braking is restored; the projected waypoint crossing may
start the return immediately.

On the negative leg, the 250 ms transition pulse launches the return. After
the pulse, positive-pipe drive is capped at +1.0 degree while velocity is
below +30 mm/s. A positive rebound of at least +30 mm/s makes that same sign
a braking command and restores the full fixed +2-degree authority. Final
capture latches permanently on first entry within 20 mm of -50 mm; it limits
only non-braking drive to +/-0.6 degree while retaining full braking authority.

Q2 explicitly forces chassis acceleration feed-forward to zero. The gated
TI wheel-acceleration feed-forward remains unchanged for Q3-Q5.

Initial values:

| Parameter | Value |
|---|---:|
| `Kp` | 4.0 1/s^2 |
| `Kd` | 2.5 1/s |
| total loop delay | 125 ms |
| pipe/motor ratio | 0.044 |
| maximum pipe command | 2.0 deg |
| maximum motor offset | 45 deg |
| maximum motor slew | 3.0 rad/s |

These values are writable in Ozone under `g_gimbal_debug`:

```text
control_kp_s2
control_kd_s
control_total_delay_ms
pipe_per_motor_ratio
max_pipe_angle_deg
max_motor_slew_rad_s
control_chassis_forward_to_x_sign
```

The first values are simulation-starting values, not claimed as physical
closed-loop tuning. MC02 requires calibrated vision and never accepts raw
Jetson motor commands.

Delay telemetry is also visible in Ozone:

```text
vision_processing_latency_us
control_prediction_horizon_ms
control_predicted_position_um
control_velocity_um_s
q2_breakaway_active
q2_breakaway_remaining_ms
q2_breakaway_count
q2_breakaway_pipe_angle_mdeg
q2_breakaway_used_this_leg
q2_stall_elapsed_ms
q2_stall_span_um
q2_final_capture_active
q2_stop_drift_um_s
q2_elapsed_ms
q2_score_deadline_missed
q2_friction_cooldown_remaining_ms
control_chassis_accel_m_s2
control_chassis_feedforward_mdeg
ti_left_rpm_x10
ti_right_rpm_x10
ti_chassis_accel_mm_s2
ti_route_complete_waiting
ti_ball_settle_elapsed_ms
```

Writing `control_total_delay_ms=0` disables state projection for a controlled
A/B comparison. Values above 250 ms are rejected internally and fall back to
125 ms.

## Failure behavior retained

- Invalid/stale vision over 100 ms returns requested pipe tilt to level.
- No accepted vision or no Jetson heartbeat for 500 ms stops the run.
- No expected TI status for 500 ms stops both subsystems.
- K7 and K8 stop TI locally without waiting for communication.
- K8 release cannot restart; K7 is required to clear to standby.
- Wrong run ID, version, length, CRC, duplicate/out-of-order sample or
  invalid metric range is not used for control.
- Q2 never grants chassis PWM authority.
- `PWR 0/1` is software permission, not physical 12 V feedback. Boot,
  SAFE_STOP, K7, K8, fault and link loss all restore `PWR 0`.

## First full-system test order

1. Logic power only: validate both protocols, all buttons and IDs.
2. DM4310 power with chassis motor rail disconnected: validate K2 setup and
   STOP/K7/K8.
3. Stationary Q2 with ball, starting with `max_pipe_angle_deg=0.5`.
4. Raise chassis wheels: validate Q3 1500 mm completion and Q1/Q4/Q5 routes.
5. Floor-test Q1, then Q3, then moving balance Q4/Q5.
