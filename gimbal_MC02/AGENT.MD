# gimbal_MC02 canonical handoff

Last updated: 2026-08-01

## 2026-08-01 user-requested rollback from build 0108 to 0107

- At the user's explicit request, the untested build `2026080108` Q2 energy
  schedule was removed and MC02 Vehicle was restored exactly to Q2 build
  `2026080107`: 350 ms positive breakaway, 1.8-degree stationary positive
  request, no forced raw-30-mm brake, no centre-crossing drive floor, and a
  20 mm final-capture entry with the +1.0-degree negative drive cap.
- Q1 and accepted Q3-Q5 behavior remain unchanged. TI stays at build
  `2026080104`; Jetson V3 and TI V2 packet formats and meanings are unchanged.
  Full clean Debug/Release/Vehicle/DelayTest verification passed at
  111340/716/13964, 69876/712/13952, 69588/712/13952 and
  113932/716/14028 text/data/BSS bytes. Powered behavior remains to be tested.

## 2026-08-01 Q2 data-driven asymmetric energy schedule

- MC02 Vehicle build `2026080108` changes only Q2. TI remains build
  `2026080104`; Q1 and the accepted Q3-Q5 controller/route behavior are
  unchanged, and neither Jetson V3 nor TI V2 packets changed.
- The supplied multi-run extraction contains eight active Q2 attempts. Two
  never moved despite a -2.0-degree request. Of the other six, the positive
  peak median was +94.7 mm and five exceeded +60 mm. Negative-leg braking
  first appeared at a median raw position of +28.4 mm; the negative trough
  median was only -34.6 mm, and only one run reached -50 mm. Clean near-100%
  valid-vision runs show the same failure, so it is not only vision dropout.
- Reliable positive breakaway is restored to -2.0 degrees for 400 ms. Once
  raw position reaches +30 mm at >=10 mm/s, +2.0-degree braking is forced,
  without shortening the existing raw-or-projected +50 mm waypoint switch.
  A stationary ball inside the last 7 mm receives only a -0.8-degree crossing
  nudge; outside it retains the full -2.0-degree anti-stall request.
- On return, after the existing +1.5-degree/250 ms pulse, at least +0.6 degree
  of drive is retained while the ball is moving negative but still has raw
  x>0. Normal Kd authority resumes after the centre crossing. Final capture
  is delayed from 20 mm to 10 mm final error; the +1.0-degree drive cap,
  +30 mm/s rebound rule, 1 s settle, 5 s earliest completion and 15 s safety
  timeout remain unchanged.
- Full clean Debug/Release/Vehicle/DelayTest verification passed at
  111484/716/13964, 70028/712/13952, 69748/712/13952 and
  114084/716/14028 text/data/BSS bytes. Powered behavior is unverified until
  the user flashes and tests the Vehicle ELF.

## 2026-08-01 Q2 small asymmetric trim after selective rollback

- MC02 Vehicle build `2026080107` changes only Q2 on top of the selective
  build-0101 rollback. Q1 and accepted Q3-Q5 behavior remain unchanged; TI
  stays at build `2026080104`, and Jetson V3/TI V2 are unchanged.
- To reduce the reported small positive overshoot, the stationary positive
  request is 1.8 instead of 2.0 degrees and the existing 2.0-degree positive
  breakaway pulse is shortened from 400 to 350 ms. Waypoint switching remains
  the unchanged raw-or-delay-projected +50 mm rule.
- To prevent the negative leg slowing too early, the permanent final-capture
  latch moves from 30 to 20 mm final error and the post-launch positive drive
  cap rises from 0.8 to 1.0 degree. The +30 mm/s rebound threshold, full
  bounded braking authority, 1 s final settle, 5 s earliest completion and
  15 s safety timeout remain unchanged.
- Full clean Debug/Release/Vehicle/DelayTest verification passed at
  111340/716/13964, 69876/712/13952, 69588/712/13952 and
  113932/716/14028 text/data/BSS bytes. Powered behavior is unverified until
  the user flashes and tests the Vehicle ELF.

## 2026-08-01 selective Q2 rollback to build 0101 behavior

- At the user's explicit request, only the MC02 stationary Q2 controller was
  restored to the behavior identified as build `2026080101`. Q1 and the
  accepted current Q3-Q5 route/controller behavior are intentionally retained;
  this is not a whole-repository rollback. The Vehicle image reports build
  `2026080101`, and the current user-restored TI image remains `2026080104`.
- Q2 again switches from +50 mm to -50 mm when either raw position or the
  bounded delay projection reaches +50 mm. The later raw-30-mm forced brake,
  crossing nudge, staged -25/-40 mm damping and repeated final-friction pulses
  are removed.
- The negative leg again uses one +1.5-degree/250 ms launch pulse, a +0.8
  degree positive-drive cap below +30 mm/s rebound speed, full bounded rebound
  braking above that threshold, and a permanent 30 mm final-capture latch that
  limits only non-braking drive to 0.6 degree. Final settle is the earlier
  1000 ms gate held through the five-second checkpoint; the 15000 ms safety
  timeout remains.
- Jetson V3 and TI V2 versions, packets, fields and meanings are unchanged;
  neither Jetson nor TI needs rebuilding. Full clean Debug/Release/Vehicle/
  DelayTest verification passed at 111340/716/13964, 69884/712/13952,
  69596/712/13952 and 113932/716/14028 text/data/BSS bytes. Powered behavior
  remains unverified until the user flashes and tests the Vehicle ELF.

## 2026-08-01 Q2 staged return damping and persistent capture

- MC02 Vehicle build `2026080106` pairs with current TI TEST_MODE build
  `2026080106`. The change is confined to Q2; Q1 and the accepted Q3-Q5
  controller/route behavior are unchanged.
- The supplied three-file extraction contains eight complete build-0105 Q2
  attempts. Positive peak median/maximum were +62.8/+85.2 mm and only one
  attempt stayed at or below +60 mm. The first negative trough median/worst
  were -75.0/-107.0 mm, the median first rebound span was 80.9 mm, and the
  ball crossed -50 mm a median eight times. At the alternating extrema the
  recorded pipe request repeatedly reached -2/+2 degrees, identifying a
  controller-created saturated limit cycle rather than a link failure.
- Positive full braking now starts from raw +30 mm whenever velocity is at
  least +10 mm/s; the noisy projected +42 mm gate is removed. Raw +50 mm is
  still the only waypoint switch, and the existing stationary nudge remains
  available if braking stops the ball short.
- Negative braking remains blocked only above projected -25 mm. From -25 to
  -40 mm it is admitted with a 0.8-degree cap; at and beyond -40 mm the whole
  negative-leg command is bounded to +/-1.2 degrees. The existing +1.5-degree
  250 ms transition pulse is unchanged and executes outside that cap.
- Final capture latches permanently on the first entry within 10 mm of
  -50 mm. It retains the +/-1.2-degree negative-leg envelope above 60 mm/s,
  then clamps both signs to +/-0.8 degree below that speed. It cannot exit and
  restore the measured +/-2-degree relay behavior; the existing bounded
  stationary friction releases and 2 s visible settle gate remain.
- Jetson V3 and TI V2 fields, versions, lengths and meanings are unchanged;
  neither Jetson nor TI source needs modification for this MC02 update.
  Full clean Debug/Release/Vehicle/DelayTest verification passed at
  112228/716/13988, 70468/712/13976, 70172/712/13976 and
  114836/716/14052 text/data/BSS bytes. Powered behavior remains unverified
  until the user flashes and tests the Vehicle ELF.

## 2026-08-01 Q2 asymmetric approach scheduling

- MC02 Vehicle build `2026080105` pairs with unchanged TI TEST_MODE build
  `2026080105`. The change is confined to Q2: Q1 and the already accepted
  Q3-Q5 controller/route behavior are unchanged.
- The positive leg reduces its stationary anti-stall request from 2.0 to
  1.6 degrees. Once raw position is at least +30 mm, a delay-projected
  position of +42 mm requests the existing full +2-degree brake. Only the
  raw +50 mm crossing still changes the target, so the printed waypoint is
  not shortened. If that earlier brake leaves the ball stationary within the
  final 7 mm before +50 mm, a smaller -0.8-degree crossing nudge prevents a
  deadlock without restoring the full launch command.
- On the negative leg, when the ball is moving toward -50 mm but its delay
  projection is still above -40 mm, a negative-pipe Kd request is held at
  level instead of braking the ball around +30..+40 mm. The post-launch
  positive-pipe drive cap is raised from 0.8 to 1.2 degrees so the existing
  static-friction floor is not immediately clipped. At/after projected
  -40 mm, final braking, capture hysteresis and rebound authority are the
  existing build-0104 behavior.
- Q2 explicitly forces the TI chassis-acceleration term to zero. Q3-Q5 retain
  the existing gated feed-forward unchanged. Fixed +/-2-degree pipe,
  +/-45-degree motor, 3 rad/s slew, 5-second score diagnostic, 15-second
  safety timeout and all immediate fault cutoffs remain authoritative.
- Jetson V3 and TI V2 packet versions, fields, lengths and meanings are
  unchanged; neither Jetson nor TI source needs an update. Offline build
  verification passed for Debug, Release, Vehicle and DelayTest at
  112236/716/13988, 70436/712/13976, 70148/712/13976 and
  114836/716/14052 text/data/BSS bytes. Powered Q2 behavior is not claimed
  until the user flashes and tests the Vehicle ELF.

## 2026-08-01 Q4/Q5 thirty-percent speed-profile reduction

- MC02 Vehicle build `2026080104` pairs with TI TEST_MODE build
  `2026080103`. TI changes only Q4/Q5: cruise 100 -> 70 RPM and common-wheel
  acceleration/deceleration 20 -> 14 RPM/s. Q3 stays at 100 RPM/30 RPM/s;
  Q1, Q2 and all Q2 build-2026080103 control behavior are unchanged.
- MC02's question-specific Q4/Q5 immediate chassis feed-forward floor is
  matched from 0.070 to 0.048 m/s2. The 500 ms feed-forward tail, 30000 ms
  scored total timeout, TI 8000 ms gentle-stop guard, five-second normal
  post-route gimbal hold and every immediate safety cutoff are unchanged.
- Jetson V3 and TI V2 packet versions, lengths, offsets and meanings are
  unchanged; no Jetson upload is required. Both MC02 and TI images require
  flashing. Clean Debug, Release, Vehicle and DelayTest verification passed
  at text/data/BSS sizes 112076/716/13988, 70300/712/13976,
  70004/712/13976 and 114676/716/14052 bytes respectively. Powered Q4/Q5
  timing remains unverified.

## 2026-08-01 Q2 measured-waypoint braking and visible final stop

- MC02 Vehicle build `2026080103` pairs with the unchanged TI TEST_MODE build
  `2026080102`. The three-second TI/MC02 start countdown remains unchanged and
  Q2 elapsed time still begins only when TI acknowledges RUNNING. Jetson V3
  and TI V2 packet versions, lengths, offsets and meanings are unchanged.
- Q2 prediction no longer switches to the return leg before the raw ball
  position reaches +50 mm. It instead requests the existing bounded +2-degree
  braking angle when the 125 ms projection reaches +50 mm; the raw crossing
  alone starts the existing +1.5-degree/250 ms negative launch. This addresses
  the five-run +57.3/+80.6/+81.2/+68.7/+72.6 mm positive peaks without
  permitting a turn short of the printed waypoint.
- Negative final capture enters at 10 mm error and exits at 15 mm instead of
  latching permanently inside 30 mm. The launch pulse is separate from two
  final stationary friction releases: +1.0 then +1.2 degrees for 150 ms, with
  a 200 ms observation cooldown. Active friction release bypasses only the
  0.6-degree capture clamp; the fixed +/-2-degree pipe, +/-45-degree motor and
  3 rad/s slew envelopes remain authoritative. No motor zero is retained
  across OUT1 power cycles.
- Q2 completion now requires raw final error <=10 mm and a position span <=2
  mm continuously for 2000 ms; worst-case drift over that window must be <=3
  mm/s. Five seconds is a latched diagnostic only and never cuts torque; the
  unchanged 15000 ms Q2 safety timeout remains authoritative. New Ozone fields
  expose final capture, stop drift, elapsed time, score-deadline miss and
  friction cooldown.
- Q3-Q5 controller parameters and TI source are unchanged. Clean compilation
  passed for Debug, Release and DelayTest; powered behavior is not yet claimed.

## 2026-08-01 substantially slower Q3-Q5 ramps and feed-forward tail

- MC02 Vehicle build `2026080101` pairs with TI TEST_MODE build
  `2026080102`. Q3 common acceleration/deceleration is 30 rpm/s; Q4/Q5 use
  20 rpm/s. Cruise speed, steering slew and scored total timeouts are unchanged.
- MC02 uses matched immediate feed-forward floors of 0.10 m/s2 for Q3 and
  0.07 m/s2 for Q4/Q5. After TI's commanded-ramp flag clears, the last bounded
  feed-forward decays linearly over 500 ms instead of disappearing at once.
  `ti_chassis_accel_tail_remaining_ms` exposes that interval in Ozone.
- TI's local gentle-stop guard is 8000 ms so the deliberately slow Q4/Q5
  deceleration can complete; the 30-second total deadline remains unchanged.
  Normal route completion still enters the five-second DM4310 hold. Actual
  fault, stop, link-loss and vision-loss paths remain immediate.
- Jetson V3 and TI V2 packet layouts and meanings remain unchanged. Full clean
  offline verification passed: Debug 111292/716/13956, Release
  69860/712/13952, Vehicle 69548/712/13952 and DelayTest 113892/716/14020
  text/data/BSS bytes. Powered behavior is not yet claimed.

## 2026-08-01 TI-only delayed OLED initialization

- TI TEST_MODE build `2026080101` delays its first external-OLED transaction
  by 1000 ms and retains one-second retry/redraw. MC02 Vehicle remains build
  `2026073126`; TI V2 and Jetson V3 are unchanged, so MC02 and Jetson do not
  require source changes for this display hardening.

## 2026-08-01 gentler Q3-Q5 common-speed ramps

- MC02 Vehicle build `2026073126` pairs with TI TEST_MODE build `2026073119`.
  Q3-Q5 common-wheel acceleration and deceleration are reduced from 50 to
  40 rpm/s; MC02's immediate phase-gated feed-forward floor is matched from
  0.17 to 0.14 m/s2.
- Cruise RPM, steering slew, route distances, scored total timeouts, the
  five-second post-stop DM4310 hold and all immediate safety cutoffs are
  unchanged. Jetson V3 and TI V2 formats are unchanged.
- Clean Debug/Release/Vehicle/DelayTest verification passed at
  111020/716/13940, 69684/712/13936, 69372/712/13936 and
  113612/716/14004 text/data/BSS bytes. Powered behavior remains unverified.

## 2026-07-31 ramp feed-forward floor and five-second post-stop hold

- MC02 Vehicle build `2026073125` keeps Jetson V3 and TI V2 packet versions,
  lengths, offsets and meanings unchanged. No Jetson file needs updating.
- While exactly one existing TI common-speed ramp flag is active, MC02 now
  applies the signed 0.17 m/s2 acceleration implied by TI's 50 rpm/s slew
  immediately. The wheel-RPM derivative may raise that estimate up to the
  existing 0.40 m/s2 clamp; it can no longer erase the known ramp because of
  its first-window delay or low-speed deadband. Steady cruise still resets
  feed-forward exactly to zero.
- For Q3-Q5, route completion keeps the DM4310 powered in competition closed
  loop for at least 5000 ms after chassis stop. The existing 5 mm / 30 mm/s / 
  3000 ms continuous settle gate can extend the hold, with the unchanged
  15000 ms deadline. STOP, K7/K8, link loss, invalid vision and motor faults
  remain immediate cutoffs.
- `ti_route_hold_remaining_ms` exposes the mandatory hold countdown in Ozone.
  Clean Debug/Release/Vehicle/DelayTest verification passed at
  111020/716/13940, 69684/712/13936, 69372/712/13936 and
  113612/716/14004 text/data/BSS bytes. Powered hardware behavior is not yet
  claimed.

## 2026-07-31 TI OLED-only build increment

- TI TEST_MODE build `2026073117` changes only its local OLED refresh and
  telemetry. MC02 remains Vehicle build `2026073124`; TI V2 and Jetson V3
  packet versions, layouts and meanings are unchanged, so no MC02 or Jetson
  rebuild is required for the display fix.
- The MC02 board repository contains legacy BMI088 driver and Mahony sources,
  and the original IOC assigns SPI2 plus accelerometer/gyro chip selects, but
  the current competition CMake target deliberately does not compile or
  initialize that path. Current chassis feed-forward therefore remains the
  gated TI wheel-encoder speed derivative, not an IMU measurement.

## 2026-07-31 gated chassis feed-forward and Q1 40 s supervisor

- MC02 Vehicle build `2026073124` keeps the Jetson V3 and TI V2 packet
  versions, lengths and offsets unchanged. TI V2 reserved flag bits 10/11
  now mean intentional common-wheel acceleration/deceleration. MC02 estimates
  and applies chassis acceleration only while exactly one gate is asserted,
  the TI state is RUNNING and motor output is enabled.
- During steady cruise the estimator is reset and chassis feed-forward is
  exactly zero. Samples below 0.05 m/s2 or whose sign contradicts the asserted
  gate are rejected. Existing acceleration, angle and slew limits still clamp
  every command.
- Q1's MC02-side expected supervisor window is now 40000 ms, matching TI
  build `2026073117`; the competition scoring target remains <=20 s. Q2-Q5
  timing and the existing three-second final stability requirement are
  unchanged.
- Clean Debug/Release/Vehicle/DelayTest verification passed. Text/data/BSS
  are 110844/716/13940, 69556/712/13928, 69244/712/13928 and
  113444/716/14004 bytes. No flash or powered hardware result is claimed.

## 2026-07-31 moving-question chassis feed-forward and 3 s final hold

- MC02 Vehicle build `2026073123` keeps the Jetson V3 packet version,
  messages, lengths and field meanings completely unchanged. It consumes the
  already-frozen TI V2 `CHASSIS_STATUS.left_rpm_x10/right_rpm_x10` fields,
  whose corrected sign is vehicle-forward positive. The 20 Hz average wheel
  speed is differentiated over a 50 ms-or-longer interval, low-pass updated
  with alpha 0.5 and clamped to +/-0.40 m/s2. The estimate is invalidated
  whenever TI motor output clears, chassis state leaves RUNNING or V2 status
  is stale.
- Q3-Q5 now use
  `alpha_pipe = -(1.4*a_des + a_chassis_x)/g`. The installation mapping is a
  bounded Ozone tuning field `control_chassis_forward_to_x_sign`, initialized
  to +1; -1 selects the opposite installed pipe direction and 0 disables the
  feed-forward for A/B comparison. New diagnostics expose both TI RPM values,
  derived acceleration and feed-forward pipe millidegrees. The existing
  +/-2-degree pipe, +/-45-degree motor and 3 rad/s motor-slew envelopes still
  clamp the combined request. Q2 sees zero chassis feed-forward because TI
  keeps its motor-output flag clear.
- On Q3-Q5 `ROUTE_COMPLETE`, MC02 sends the existing Jetson route-complete
  event but retains gimbal closed loop while TI stays disarmed in state 5.
  Whole-system completion now requires raw target error <=5000 um and the
  fixed-window control velocity <=30000 um/s continuously for 3000 ms. Any
  invalid/stale sample or threshold violation resets the timer. A 15000 ms
  post-route bound stops without reporting successful completion. Q1 and Q2
  completion behavior is unchanged.
- Full clean Debug/Release/Vehicle/DelayTest verification passed. Text/data/
  BSS are 110684/716/13940, 69476/712/13928, 69164/712/13928 and
  113284/716/14004 bytes. This is offline evidence only. Before a floor run,
  verify build `2026073123`, V2 RPM signs, feed-forward sign at a slow start,
  and the 0-to-3000 ms continuous settle timer. Jetson does not need a code
  upload for this change.

## 2026-07-31 Q2 Step B + Step B2 velocity observation

- After the user hardware-confirmed the Step A baseline, Vehicle build
  `2026073121` implements only the requested Step B and Step B2 observation
  changes. MC02 now ignores the Jetson velocity field for control and derives
  velocity from position samples over a fixed 70 ms target window, accepting
  actual spans from 50 to 120 ms. Samples are keyed by the Jetson-frame MC02
  receive timestamp, so a faster control loop cannot insert duplicates.
- The two Q2 anti-stall drive-floor mechanisms no longer use a single-frame
  absolute-velocity threshold. Their three directional admission branches
  now require raw position to remain inside the existing 2 mm band for
  120 ms. The motion window is independent of the breakaway-pulse stall
  detector. The capture clamp's existing 20 mm/s condition and the final
  settle gate's 60 mm/s condition remain unchanged.
- Ozone debug adds `control_velocity_window_ms` and `q2_stationary_ms`
  immediately after `control_velocity_um_s`. Valid sample receipt updates
  these observation-only fields even before Q2 starts, allowing the required
  no-motion commissioning check; receive-time guards prevent the subsequent
  control call from inserting the same sample twice. On invalid/stale vision, the
  velocity history, filtered velocity and stationary window are cleared;
  an invalid span while vision remains valid retains the previous filtered
  velocity so Kd does not disappear for one control cycle.
- No acceleration predictor, curve speed schedule, integral term or higher
  motor slew was added. The fixed +/-2-degree pipe and +/-45-degree motor
  envelopes, 3.0 rad/s slew, Kp/Kd 4.0/2.5, targets, breakaway parameters,
  Step A rebound rule, timeouts and every immediate safety cutoff are
  unchanged.
- Full clean Debug/Release/Vehicle/DelayTest verification passed. Text/data/
  BSS are respectively 109276/716/13876, 68484/712/13872,
  68212/712/13872 and 111876/716/13940 bytes. This is offline compile/static
  evidence only. Before K2, hardware must confirm build ID `2026073121`, a
  70..110 ms reported velocity window, stationary velocity within about
  +/-10000 um/s, and `q2_stationary_ms` increasing at rest then immediately
  resetting when the ball is nudged.

## 2026-07-31 Q2 Step B/C rollback

- At the user's request, the unflashed build-3121 Step B/C estimator and
  acceleration-prediction changes were removed. The current source and
  Vehicle artifact are restored to the hardware-confirmed Step A baseline,
  build `2026073120`.
- MC02 again uses the Jetson velocity when its valid flag is set, retains the
  original control-task-time fallback, projects position with bounded
  constant velocity `x+v*T`, and uses measured velocity in Kd and the existing
  energy-sign test. The three build-3121-only Ozone fields were removed.
- The Step A `GIMBAL_Q2_REBOUND_BRAKE_UM_S=30000` behavior remains: positive
  rebound at or above 30 mm/s regains the existing +2-degree braking
  authority; below it, the +0.8-degree positive drive cap remains.
- Full clean Debug/Release/Vehicle/DelayTest verification passed. Text/data/
  BSS are exactly the recorded Step A values: 108724/716/13780,
  68212/712/13776, 67884/712/13776 and 111316/716/13844 bytes. This is
  compile/static evidence and not a new powered-hardware claim.

## 2026-07-31 Q2 Step A rebound-braking authority fix

- The external improvement directive identified a sign-dependent authority
  bug in the build-3119 negative-leg drive cap. After the transition pulse,
  all positive-pipe requests were capped at +0.8 degree. That is correct while
  the ball moves toward -50 mm, but once the ball rebounds toward positive
  position the same positive pipe angle is braking and the unconditional cap
  prevents full energy removal. In `run_1848.csv`, representative trough-to-
  rebound pairs are -85.8 to -3.4 mm, -82.1 to -8.9 mm and -67.9 to -35.8 mm.
- Vehicle build `2026073120` adds
  `GIMBAL_Q2_REBOUND_BRAKE_UM_S=30000`. With no active launch pulse, the
  +0.8-degree positive-pipe cap remains while filtered measured velocity is
  below +30 mm/s. At +30 mm/s or faster positive rebound, positive pipe angle
  is braking and may use the existing full +2.0-degree envelope. The threshold
  avoids switching authority on the measured high-frequency velocity noise.
- This Step A build deliberately does not implement the requested Step B/C
  velocity-window and acceleration-prediction changes. It must first receive
  3..5 hardware CSV runs so the rebound-only effect stays identifiable.
  Build-3119 predicted positive turning remains included.
- The fixed +/-2-degree pipe and +/-45-degree motor envelopes, 3.0 rad/s
  motor slew, Kp/Kd 4.0/2.5, +/-50 mm targets, 5000/15000/1000 ms timing,
  every breakaway parameter, the 1.2-degree friction floor and all immediate
  STOP/link/vision/motor/TI safety cutoffs are unchanged.
- Full clean Debug/Release/Vehicle/DelayTest verification passed. Text/data/
  BSS are respectively 108724/716/13780, 68212/712/13776,
  67884/712/13776 and 111316/716/13844 bytes. The user subsequently
  hardware-confirmed Step A as correct and authorized Step B+C; no
  quantitative build-3120 CSV was supplied in this handoff.

## 2026-07-31 Q2 delay-projected positive turn

- `run_1848.csv` contains five valid build-3118 Q2 attempts. Their positive
  peaks are +79.5, +75.5, +63.2, +59.1 and +68.1 mm: median +68.1 mm and
  maximum +79.5 mm. The first logged -50 mm target samples occur only once
  raw position is already +56.0..+59.1 mm, and the ball continues positive
  for another 102..357 ms before each peak. Removing the settle gate worked,
  but a raw-position-only decision is still one response horizon too late.
- Vehicle build `2026073119` passes the existing bounded delay-projected
  position into the waypoint decision. Q2 begins the existing return when
  either predicted position reaches +50 mm or raw position has actually
  crossed +50 mm. The latter remains an unconditional fallback if velocity
  is low, negative or noisy.
- This uses the already measured/default 125 ms full-loop delay and its
  existing 250 ms upper validity bound; it does not add a second delay term.
  Positive drive/braking schedules, the +1.5-degree/250 ms return pulse,
  subsequent +0.8-degree drive cap, full -2.0-degree braking, final settle,
  all actuator envelopes and every immediate safety cutoff are unchanged.
- Full clean Debug/Release/Vehicle/DelayTest verification passed. Text/data/
  BSS are respectively 108716/716/13780, 68140/712/13776,
  67844/712/13776 and 111308/716/13844 bytes. This is compile/static evidence
  only; build `2026073119` has not yet been flashed or hardware-tested.

## 2026-07-31 Q2 immediate positive-waypoint return

- `run_936.csv` contains eight build-3117 Q2 attempts. Excluding the first
  severe pass-through, the remaining seven reverse at a median raw position
  of +59.2 mm and then reach a median positive peak of +72.7 mm
  (range +59.0..+84.7 mm), about 239 ms after the logged target switch.
  Their five-second median is -49.8 mm and six of seven are inside the
  printed final +/-10 mm band, so the return schedule is broadly usable.
- The first attempt is diagnostically important rather than discardable:
  the ball reaches +125 mm while the target remains +50 mm, and the target
  does not switch to -50 mm until about 12.1 s. The existing +/-7 mm,
  <=100 mm/s and 60 ms intermediate settle gate can therefore be crossed too
  quickly without triggering, despite the printed task requiring immediate
  return rather than stability at +50 mm.
- Vehicle build `2026073118` switches from +50 mm to -50 mm on the first
  valid raw-position sample at or beyond +50 mm. The intermediate velocity
  and dwell conditions have been removed. Positive-leg braking release at
  +30 mm, low-speed -2.0-degree anti-stall behavior, the +1.5-degree/250 ms
  return pulse, subsequent +0.8-degree negative drive cap, full -2.0-degree
  braking, final settle gate, timing windows and all safety cutoffs are
  unchanged.
- Full clean Debug/Release/Vehicle/DelayTest verification passed. Text/data/
  BSS are respectively 108700/716/13780, 68116/712/13776,
  67820/712/13776 and 111292/716/13844 bytes. Build `2026073118` was hardware
  tested in the five Q2 attempts in `run_1848.csv`; its remaining
  response-horizon positive overshoot motivated build 3119.

## 2026-07-31 Q2 earlier positive brake / bounded negative launch

- Build-3116 hardware logs confirm that the stronger positive-leg schedule
  removed the previously repeatable +20..+30 mm stalls, but also identify
  excess energy. `run_277.csv` reaches a +77.8 mm positive peak, reverses
  around 3.70 s and reaches -80.3 mm; `run_364.csv` reverses around 1.20 s
  but still reaches -78.6 mm. Their largest measured negative speeds are
  approximately 289 and 273 mm/s. In both logs the +1.5-degree transition
  pulse persists briefly after measured velocity has already become negative,
  so the velocity-sign-only +1.0-degree cap starts too late.
- Vehicle build `2026073117` restores normal positive-leg braking after raw
  position reaches +30 mm instead of +35 mm. The low-speed fixed -2.0-degree
  anti-stall request and separately detected -2.0-degree/400 ms breakaway
  pulses remain, so a real early stall still receives full fixed authority.
- The +50 mm transition now receives one +1.5-degree launch pulse for 250 ms
  instead of 400 ms. When that pulse ends, every further drive-sign positive
  pipe request is capped at +0.8 degree without waiting for the delayed
  velocity estimate to become negative. Negative-pipe braking retains the
  full fixed -2.0-degree authority. Targets, gains, 125 ms prediction,
  waypoint/final gates, 5000 ms checkpoint, 15000 ms timeout, actuator
  envelopes and all immediate safety cutoffs are unchanged.
- Full clean Debug/Release/Vehicle/DelayTest verification passed. Text/data/
  BSS are respectively 108676/716/13780, 68100/712/13776,
  67796/712/13776 and 111268/716/13844 bytes. Build `2026073117` was hardware
  tested in the eight Q2 attempts in `run_936.csv`; its excessive positive
  peaks and one missed fast waypoint crossing motivated build 3118.

## 2026-07-31 Q2 asymmetric positive push / negative drive cap

- The user reports seven build-3115 trials with three positive-leg
  hesitations. Five supplied logs independently show the same split:
  `run_1472.csv` never reverses and remains near +25.1..+27.7 mm through the
  15-second timeout after two -1.5-degree pulses; `run_1583.csv` likewise
  remains near +17.6..+21.5 mm after two pulses. `run_1440.csv`,
  `run_1513.csv` and `run_1547.csv` do reverse near +45.6..+47.9 mm, but
  their negative troughs repeat at -72.0, -76.4 and -71.4 mm. At five
  seconds the three successful runs are inside the printed final +/-10 mm
  band, so the problem is excess return energy rather than wrong sign or
  invalid vision.
- Vehicle build `2026073116` separates the two legs. On the positive leg,
  raw position below +35 mm keeps at least -0.6 degree forward tilt so the
  delay-compensated velocity term cannot brake the ball to rest at +20..+30
  mm. Whenever positive-leg speed is at most 30 mm/s and the raw target is
  still outside the existing 7 mm waypoint band, the request becomes the
  fixed -2.0-degree envelope. Separately detected positive stall pulses also
  use -2.0 degrees/400 ms, still at most twice.
- On the negative leg, once measured velocity is actually toward -50 mm,
  positive-pipe drive is capped at +1.0 degree. Negative-pipe braking keeps
  the full fixed -2.0-degree authority, including after final capture.
  Global Kp/Kd, 125 ms prediction, +35 mm braking release, +/-7 mm waypoint
  band, 60 ms turn gate, 1000 ms final settle, 5000 ms earliest completion,
  15000 ms hard timeout, 2-degree pipe/45-degree motor envelopes and every
  STOP/link/vision/motor/TI cutoff are unchanged.
- Full clean Debug/Release/Vehicle/DelayTest verification passed. Text/data/
  BSS are respectively 108644/716/13780, 68108/712/13776,
  67804/712/13776 and 111236/716/13844 bytes. Build `2026073116` was hardware
  tested in `run_277.csv` and `run_364.csv`; its positive overshoot and late
  negative-leg drive cap motivated build 3117.

## 2026-07-31 Q2 positive-leg second detected breakaway

- Four build-3114 logs isolate the reported low-probability hesitation.
  `run_368.csv` never reaches +40 mm: after one -1.5-degree/400 ms pulse near
  +27 mm, it later remains around +35.7..+37.6 mm for over nine seconds while
  the remaining -1.2-degree floor cannot overcome the mechanism. By contrast,
  `run_161.csv` and `run_292.csv` show similar short pauses rescued by their
  one available -1.5-degree pulse; `run_61.csv` reaches the waypoint normally.
- Vehicle build `2026073115` changes only the positive-leg breakaway quota
  from one to at most two. A second pulse is not automatic: after the first
  pulse ends, a new 150 ms window with no more than 2 mm position span, while
  still outside the target band, must be detected. Each pulse remains
  -1.5 degrees for 400 ms. The negative leg remains limited to one pulse.
- All build-3114 timing behavior remains: +50 mm immediate waypoint,
  1000 ms final settle, no completion before 5000 ms and 15000 ms hard
  timeout. Protocol, global gains, static floor, capture schedule and fixed
  actuator/safety bounds are unchanged. Build `2026073115` was hardware
  tested in `run_1440.csv`, `run_1472.csv`, `run_1513.csv`,
  `run_1547.csv`, `run_1583.csv` and the user's two additional reported
  trials; the repeated stalls and return overshoot motivated build 3116.

## 2026-07-31 Q2 five-second checkpoint / 15-second safety window

- `run_11976.csv` contains one clean run that reaches the final band around
  3.03 s and then reports normal completion at 4.911 s near -48.6 mm and
  -24.1 mm/s. `state=1`, not fault-state 7, proves this cutoff was the
  existing final-settle completion path rather than the five-second fault.
  A second block in that file never switches from +50 mm and has only 80.8%
  valid samples, so it is not a valid tuning run.
- `run_12060.csv` is truncated while the controller is still active at
  2.667 s, but is already near -53.4 mm at 11.2 mm/s. It does not contain a
  five-second endpoint.
- Vehicle build `2026073114` implements the user's requested diagnostic
  timing behavior. Q2 cannot report completion before 5000 ms, so TI cannot
  remove motor torque before the printed five-second scoring checkpoint.
  If the final settle gate is not satisfied at five seconds, the same closed
  loop remains active and may complete later; the hard MC02 Q2 timeout is
  extended from 5000 to 15000 ms. The final -50 mm settle dwell is increased
  from 150 to 1000 ms for visible confirmation; the +50 mm intermediate
  waypoint remains a 60 ms immediate-turn gate.
- The printed performance requirement remains: the ball must already be
  near -50 mm by five seconds. A completion timestamp later than five
  seconds is a tuning failure, not a relaxed interpretation of the problem.
  STOP, link loss, invalid/stale vision, TI safe stop, motor faults and all
  angle/actuator bounds remain immediate. No TI or Jetson change is needed.
  Build `2026073114` was hardware-tested in `run_61.csv`, `run_161.csv`,
  `run_292.csv` and `run_368.csv`.

## 2026-07-31 Q2 latched asymmetric final capture

- `run_10475.csv` hardware-validates build `2026073112` far enough to expose
  a flaw in its symmetric final clamp. The actual Q2 interval is about
  4.99 s: +50 mm switches to -50 mm after about 1.39 s, but the ball first
  crosses -50 mm near -275 mm/s. The +/-0.8-degree clamp removes needed
  braking authority, after which position reaches roughly -91.7 mm, rebounds
  to +34.0 mm and times out near -28.4 mm while still moving.
- Vehicle build `2026073113` latches final capture the first time raw final
  error reaches 30 mm. After latching, the 1.2-degree static-friction floor
  remains disabled. A pipe request that opposes measured ball velocity keeps
  the full fixed 2-degree braking envelope; only non-braking drive is capped
  to +/-0.6 degree. Capture remains latched even if overshoot leaves the
  30 mm entry band, preventing a return to aggressive drive.
- The pipe was polished before this run and may have lower rolling/static
  friction. Build `2026073113` deliberately reduces energy input near the
  final target without reducing braking or changing far-field motion,
  protocol, K2 sequencing, delay compensation, actuator envelopes, safety
  cutoffs or the then-current five-second deadline. Build `2026073113` was
  hardware-tested in `run_11976.csv` and `run_12060.csv`.

## 2026-07-31 Q2 final capture zone

- Vehicle build `2026073112` implements the capture-zone change proposed
  after `run_9900.csv`. During the final Q2 -50 mm leg only, raw position
  error within 20 mm disables the 1.2-degree static-friction floor and clamps
  the final physical-pipe request to +/-0.8 degree. The clamp is applied
  after the normal PD and breakaway selection, so the final-zone bound is
  authoritative.
- Outside that final capture zone, build-3111 behavior is unchanged: Kp/Kd
  4.0/2.5, 2-degree pipe envelope, 3.0 rad/s motor slew, 1.2-degree
  near-stall floor, 1.5-degree/400 ms per-leg pulse, 60 ms intermediate
  waypoint, 125 ms compensation and the fixed five-second deadline.
- The user reports physically polishing the pipe after the previous run, so
  rolling/static friction may now be lower. This increases the relevance of
  capture-zone-only authority reduction; no global drive increase was made.
  Build `2026073112` was hardware-tested in `run_10475.csv`; the symmetric
  clamp was rejected because it also weakened high-speed braking.

## 2026-07-31 build-3111 first hardware run

- `run_9900.csv` confirms the build-3111 intermediate-waypoint change works:
  the +50 mm target switches to -50 mm after about 1.20 s instead of the
  previous 3.895 s. The positive leg reaches about +50.9 mm immediately
  before the switch, with all samples valid.
- The run still does not complete. The negative leg has about 3.79 s but
  oscillates broadly: it first passes near -47.3 mm at about -178 mm/s,
  reaches roughly -68.2 mm, returns as far as +10.0 mm, and reaches a second
  negative trough around -68.2 mm. At the five-second fault it is near
  -63.7 mm and the target is cleared. This is a final-target settling failure,
  not a waypoint timing or link failure.
- A major local mechanism is the 1.2-degree minimum static-friction drive:
  it is allowed whenever the final error is just outside the 7 mm band and
  speed is below 10 mm/s. Near the first -57.8 mm stop, only about 0.8 mm
  separates the ball from the tolerance edge, yet the full friction floor/
  saturated controller can launch the next large return. The next candidate
  change should be Q2-negative capture-zone gain/angle scheduling and should
  disable the static-friction floor close to the final target while retaining
  the current far-field and intermediate-waypoint behavior. No build beyond
  `2026073111` has been made from this observation yet.

## 2026-07-31 Q2 intermediate-waypoint timing

- `run_3803.csv` shows build-3110 does reach both Q2 targets in the correct
  order, but the +50 mm leg consumes about 3.895 s. The -50 mm leg therefore
  has only about 1.1 s before the fixed deadline and reaches about -40.5 mm
  before faulting, just 2.5 mm outside the existing -43 mm tolerance edge.
- After the timeout, requested pipe angle is zero but the now-faulted,
  torque-disabled mechanism reverses the ball from about -43.1 mm to roughly
  +120 mm. This post-fault movement is not a continued controller request;
  it is consistent with the loaded linkage/pipe falling away from level after
  the immediate safety cutoff. Do not weaken the fault cutoff to mask it.
- Vehicle build `2026073111` changes only the intermediate +50 mm settle
  gate. It retains the existing +/-7 mm position band but permits up to
  100 mm/s and needs 60 ms dwell before switching to -50 mm. The final
  -50 mm completion remains limited to 60 mm/s and 150 ms dwell.
- Kp/Kd 4.0/2.5, 3.0 rad/s motor slew, 1.2-degree low-speed friction floor,
  1.5-degree/400 ms delayed per-leg breakaway, 125 ms compensation, fixed
  2-degree pipe/45-degree motor envelopes, five-second deadline and all
  safety cutoffs are unchanged. Build `2026073111` is not hardware-validated.

## 2026-07-31 Q2 overshoot rebalance

- `run_3417.csv` captures build-3109's first full aggressive-Q2 trial. The
  +50 mm target is first crossed about 1.02 s after it is asserted at
  +50.74 mm and roughly +129.5 mm/s. Positive braking is not requested until
  about 1.10 s at +61.49 mm, and the measured position peaks at +133.63 mm.
  The ball then oscillates across the target without meeting the velocity/
  settle gate; the target never changes to -50 mm before the fixed deadline.
- Vehicle build `2026073110` retains the 1.2-degree directional
  static-friction floor, 2-degree fixed pipe envelope, 3.0 rad/s motor slew,
  150 ms target settle and 125 ms delay compensation. It rebalances Kp/Kd
  from 5.0/1.2 to 4.0/2.5 so velocity braking starts earlier.
- The floor now applies only below 10 mm/s instead of 30 mm/s. The per-leg
  stall pulse is reduced from 2.0 degrees/650 ms to 1.5 degrees/400 ms, and
  first-leg stall declaration moves from 300+120 ms to 650+150 ms. This
  preserves a decisive recovery for a genuinely stuck ball without declaring
  the normal actuator/linkage delay to be a stall.
- `VehicleCommissioningAggressiveQ2` now writes the same live Kp=4.0,
  Kd=2.5, max-pipe=2.0 degrees, slew=3.0 rad/s and delay=125 ms values.
  Build `2026073110` is not hardware-validated yet.

## 2026-07-31 aggressive Q2 static-friction drive

- Vehicle build `2026073109` responds to the hardware observation that the
  ball stopped near +32 mm while the DM4310 continued to move back and forth.
  The supplied `run_2279.csv` does not contain that K2 interval: all 1,314
  rows have question/target/command equal to zero over 35.49 s, so the
  observation is supported by the user's direct report and the separate
  100 Hz Ozone summary, not by that CSV.
- Q2 now applies a bounded directional minimum drive whenever the raw target
  error remains outside the existing 7 mm tolerance and absolute estimated
  velocity is at most 30 mm/s: `-1.2 deg` toward the +50 mm target and
  `+1.2 deg` toward the -50 mm target. This is Coulomb/static-friction
  feed-forward, not an integral term, so it cannot wind up and reverses only
  with the target sign.
- The per-leg breakaway pulse is now the existing fixed pipe envelope
  (`2.0 deg`) for 650 ms. First-leg stall detection arms after 300 ms and
  requires a 120 ms / 2 mm stable window. Kp/Kd defaults are 5.0/1.2, motor
  slew/v_des is 3.0 rad/s, and target settle is 150 ms. Fixed 2-degree pipe,
  45-degree motor and five-second Q2 bounds, stale/invalid vision handling,
  STOP, TI/Jetson/motor fault cutoffs and 125 ms compensation are unchanged.
- `VehicleCommissioningAggressiveQ2` writes the same 2.0-degree, Kp=5.0,
  Kd=1.2, 3.0 rad/s and 125 ms values live after a reset. Existing older
  commissioning helpers remain for deliberate rollback.
- Clean Debug/Release/Vehicle/DelayTest verification passed. Text/data/BSS
  are respectively 108,100/716/13,780, 67,716/712/13,776,
  67,420/712/13,776 and 110,700/716/13,844 bytes. This is compile/static
  verification only; build `2026073109` has not yet been flashed or tested
  on the assembled vehicle.

## 2026-07-31 Q2 per-leg stall breakaway

- Vehicle build `2026073108` replaces the build-3107 second-leg-only
  breakaway trigger with one independent, directional opportunity per Q2
  leg. On the first `+50 mm` leg, after an 800 ms response grace, a fresh
  calibrated vision window that stays within 2 mm for 250 ms while still
  more than the existing 7 mm target tolerance away starts one `-1.2 deg`
  physical-pipe pulse for 350 ms. Continuous position travel resets that
  window. The pulse cannot repeat on the same leg.
- Completing the existing +50 mm position/velocity settle gate still changes
  the target to -50 mm. That gate already proves the ball is stationary, so
  the second leg immediately starts its one `+1.2 deg / 350 ms` pulse rather
  than waiting another 250 ms. Both signs follow the frozen convention
  `x > 0` toward the motor and `alpha_pipe > 0` with the motor end high.
- Each pulse is applied only while vision remains valid. Invalid/stale vision,
  STOP, any stopped/fault state, TI safe stop and the fixed five-second Q2
  timeout clear the active pulse and stall window. The existing 0.8-degree
  live PD clamp, Kp=3, Kd=2.0 commissioning value, 125 ms compensation,
  1.2 rad/s motor slew, fixed 2-degree pipe/45-degree motor envelopes,
  protocol and all non-Q2 behavior are unchanged.
- Ozone adds `q2_breakaway_used_this_leg`, `q2_stall_elapsed_ms` and
  `q2_stall_span_um`. `q2_breakaway_pipe_angle_mdeg` is now the signed last
  pulse (`-1200` first leg or `+1200` second leg), and the count can reach at
  most two in one Q2 run.
- The 100 Hz Data Sampling capture preceding this change showed the Q2 target
  reached `+50000` but never `-50000`, while `q2_breakaway_count` and
  `q2_breakaway_active` remained zero. Commanded and feedback motor positions
  tracked closely and the observed feedback velocity reached about
  0.945 rad/s. This is evidence that the old target-transition-only trigger
  was never reached on that attempt; it is not hardware validation of the new
  build.
- Clean Debug/Release/Vehicle/DelayTest verification passed. Text/data/BSS
  are respectively 107,948/716/13,780, 67,620/712/13,776,
  67,308/712/13,776 and 110,548/716/13,844 bytes. This is compile/static
  verification only. Build `2026073108` has not yet been flashed or tested on
  the assembled vehicle.

## 2026-07-31 Q2 second-leg breakaway pulse

- Vehicle build `2026073107` implements the previously proposed Q2-only
  breakaway experiment. When the raw position/velocity settle gate completes
  the +50 mm leg and changes the target to -50 mm, MC02 requests one
  `+1.2 deg` physical pipe pulse for 350 ms, then automatically resumes the
  existing delay-compensated PD controller and its live
  `max_pipe_angle_deg` clamp (currently commissioned at 0.8 degree).
- The pulse does not run at question start and cannot retrigger within the
  same target transition. It is applied only while the current vision sample
  passes the existing calibrated/fresh/confidence gate. Invalid vision
  requests level as before. STOP, any stopped/fault state, question restart,
  TI safe stop and the unchanged five-second Q2 timeout clear the pulse
  deadline.
- Existing Kp=3, Kd=2.0 live commissioning value, measured 125 ms delay
  compensation, 1.2 rad/s motor slew, 45-degree motor limit, targets and
  protocol payloads are unchanged. The pulse maps to about 27.3 motor degrees
  through the installed 0.044 ratio and remains below the existing
  45-degree motor envelope.
- Ozone aggregate debug now exposes `q2_breakaway_active`,
  `q2_breakaway_remaining_ms`, `q2_breakaway_count` and
  `q2_breakaway_pipe_angle_mdeg`; `command_pipe_angle_mdeg` should show
  approximately 1200 during the pulse and return to the normal controller
  output afterward.
- Clean Debug/Release/Vehicle/DelayTest verification passed. Text/data/BSS
  are respectively 107,260/716/13,740, 67,148/712/13,736,
  66,836/712/13,736 and 109,860/716/13,804 bytes. This was compile/static
  verification. A later 100 Hz Ozone capture showed the transition-only
  trigger was not reached during the observed first-leg failure; see the
  build-3108 section above.

## 2026-07-31 first compensated Q2 hardware attempt

- The first stationary K2 run with Vehicle build `2026073105`,
  `control_total_delay_ms=125` and `max_pipe_angle_deg=0.5` moved the ball in
  the intended positive direction but stopped near +35 mm rather than reaching
  +50 mm. The motor then returned to level and the retained MC02 state was
  `state=6`, `fault=6`, `competition_phase=8`. At the post-stop capture the
  Jetson and TI links were both online: vision age 21 ms, processing latency
  13.04 ms, flags 15, Jetson heartbeat age 64 ms, TI online/state/fault
  1/0/0 and zero TI CRC errors. This is hardware evidence only for that one
  bounded attempt.
- Source tracing found a diagnostic-fault overwrite. The Q2 application has a
  five-second `GIMBAL_Q2_TIMEOUT_MS`; if it first raises
  `GIMBAL_FAULT_BAD_COMMAND=5`, `ti_link_poll()` observes the gimbal fault and
  calls `force_stop(now_ms, 1)`, whose subsequent
  `gimbal_app_force_ti_safe_stop()` overwrites the retained cause with
  `GIMBAL_FAULT_TI_LINK=6`. Thus retained fault 6 does not prove an actual TI
  dropout in this capture.
- Build `2026073106` fixes only that overwrite. TI supervisor stops now carry
  one of three explicit modes: normal stop, real TI-link fault, or preserve
  the already-latched gimbal fault. A Q2 deadline failure therefore remains
  `GIMBAL_FAULT_BAD_COMMAND=5`; genuine missing/faulted TI status still becomes
  `GIMBAL_FAULT_TI_LINK=6`. The user explicitly confirmed that the printed Q2
  total time is five seconds, so `GIMBAL_Q2_TIMEOUT_MS` remains exactly
  5000 ms and the verifier locks it. No controller gain, 0.5-degree live
  commissioning setting, target, delay compensation, protocol, or TI firmware
  changed.
- Clean Debug/Release/Vehicle/DelayTest verification passed. Text/data/BSS are
  respectively 106,988/716/13,716, 66,932/712/13,712,
  66,636/712/13,712 and 109,588/716/13,780 bytes. Vehicle retains Ozone DWARF
  aggregate types. Build 2026073106 has not yet been flashed or hardware
  tested.
- The subsequent 2026073106 K2 attempt reached/crossed the +50 mm target and
  reversed correctly toward -50 mm, but the fixed five-second competition
  deadline expired near -20 mm. This establishes that target switching,
  physical sign and the bounded compensated controller operated in the
  intended order in that attempt; it does not yet establish scored completion.
  Keep Kp=3, Kd=2.5, total delay=125 ms and the five-second deadline fixed for
  the next single-variable trial. `gimbal_MC02_Vehicle.jdebug` now provides
  `VehicleCommissioning08`, which writes only max pipe angle 0.8 degrees and
  restores delay 125 ms. This Ozone helper does not change the firmware image
  or build ID.
- In the 0.8-degree follow-up, the user reports that the first leg reversed
  near +30 to +39 mm before visibly completing +50 mm; response and reversal
  were slow, and the five-second stop occurred near +30 mm. Do not increase
  angle again from this observation. With Kp=3 and a roughly 140 ms live
  prediction horizon, position projection contributes about 0.42 seconds of
  additional effective velocity feedback, making the current Kd=2.5 behave
  near 2.9. The next bounded single-variable trial is max pipe 0.8 degrees,
  Kp=3, Kd=2.0 and delay=125 ms. The Ozone-only helper
  `VehicleCommissioning08Kd20` writes exactly those live values; firmware,
  build ID and the five-second deadline remain unchanged.
- `run_4343.csv` captures the subsequent Kp=3, Kd=2.0,
  max-pipe=0.8-degree, delay=125 ms attempt. The active +50 mm target lasted
  4.936 s and never switched to -50 mm before the fixed timeout. The CSV has
  only 97 active samples, 19.65 Hz with 51.64 ms median spacing, despite the
  separate report that vision had been restored to about 50 FPS. All 97
  samples are marked valid.
- This capture does not support the interpretation that MC02 requested
  conservative motion: 59/97 active samples (60.8%) report at least
  0.75 degree absolute requested pipe angle, and the requested value reaches
  both -0.8 and +0.8 degrees. The reported vision estimate ranges from
  -4.84 mm to +79.67 mm and makes two large apparent excursions about the
  +50 mm target. It never remains within the existing position/velocity
  settle gate for 250 ms, so Q2 stays on its positive leg.
- The user simultaneously reports that the physical ball barely moved. This
  conflicts materially with the CSV estimate and must be resolved before
  increasing angle, gain or slew. The V3 `command_angle_mdeg` field is
  `s_command_pipe_angle_mdeg`, the controller's requested pipe tilt before
  motor slew and motor feedback; the CSV therefore cannot establish actual
  actuator following. The next safe test is recording-off/CSV-only at the
  claimed 50 FPS while Ozone observes `requested_position_rad`,
  `command_position_rad` and `feedback_position_rad`, plus a held-ball check
  that pipe motion does not create several centimetres of false visual
  position change. The current 1.2 rad/s motor slew means a 0.8-degree pipe
  command maps through the 0.044 ratio to about 18.2 motor degrees and takes
  about 0.265 s from neutral or 0.529 s for a full reversal; raise it only
  after the vision/physical discrepancy is excluded.
- The recording-off repeat `run_5167.csv` plus the user's 30 FPS phone video
  resolves that discrepancy. The active Q2 interval contains 269/269 valid
  rows at 53.85 Hz (19.10 ms median spacing); processing latency is 18.91 ms
  median. The phone video independently shows the same qualitative ball
  motion, so the calibrated vision position is not inventing the large first
  excursion.
- The +50 mm leg now succeeds: it lasts about 1.80 s, peaks at +51.10 mm and
  switches to the -50 mm leg. During the entire remaining 3.173 s negative
  leg, every recorded controller request is exactly +0.8 degree, yet position
  remains in +45.94 to +49.09 mm with median +46.70 mm and median velocity
  approximately -0.25 mm/s. The video likewise shows only a small return
  motion followed by a stop. Q2 therefore times out because the ball cannot
  restart toward -50 mm, not because of Jetson frame rate, invalid samples,
  target switching, Kp/Kd cancellation or insufficient requested command.
- The same +0.8-degree request was already used to decelerate the moving ball
  before the +50 mm settle/switch, so its sign and at least some physical
  braking effect are established. The leading, but not yet directly measured,
  explanation is breakaway/static friction or a local tube trap: the bounded
  +0.8-degree tilt can stop a moving ball but cannot restart it once stationary.
  Do not raise Kp/Kd. The most targeted next controller experiment is a
  one-shot positive-pipe breakaway pulse after the Q2 target switches to
  -50 mm (+1.2 degrees for 350 ms), then return to the existing 0.8-degree
  PD clamp. Build `2026073107` now implements this bounded experiment while
  preserving first-leg tuning and the fixed five-second deadline; hardware
  behavior remains unverified. An actual motor-feedback latch would
  distinguish static friction from a failed reverse motor-following command
  if needed.

## 2026-07-31 Vehicle full-loop-delay compensation

- The user completed the dedicated total-loop-delay experiment at about
  49 FPS: 24 edges were found, nine starting edges were accepted, median
  delay was 125 ms, mean 128 ms and reported dispersion 32 ms. This is
  hardware evidence for the current assembled visual/motor/linkage loop, but
  the dispersion means 125 ms is an initial compensation value rather than a
  precision constant.
- MC02 build `2026073105` consumes that result in the competition Vehicle
  controller. Before the existing PD calculation it projects position with
  bounded constant velocity:
  `x_pred = clamp(x + v * (125 ms + receive_age), +/-130 mm)`.
  The total horizon is capped at 250 ms. The original raw position remains
  authoritative for target-settle decisions, and invalid/stale vision still
  requests zero tilt. The existing +/-2 degree pipe limit, 45-degree motor
  offset limit, 1.2 rad/s slew limit and all watchdog/fault cutoffs are
  unchanged.
- The frozen V3 32-byte VISION_SAMPLE wire layout is unchanged. MC02 now
  decodes its already-existing byte-8 `processing_latency_us` field for Ozone
  diagnostics; it is not added separately because the measured full-loop
  delay already contains Jetson processing and USB return latency.
  `g_gimbal_debug` exposes `control_total_delay_ms` (default 125),
  `control_prediction_horizon_ms`, `control_predicted_position_um`,
  `control_velocity_um_s` and `vision_processing_latency_us`. Writing delay
  zero disables projection for a deliberate A/B check; values above 250 fall
  back to 125.
- Full offline verification passed for clean Debug, Release, Vehicle and
  DelayTest builds. Their text/data/BSS sizes are respectively
  106,972/716/13,716, 66,908/712/13,712,
  66,612/712/13,712 and 109,564/716/13,780 bytes.
  `gimbal_MC02_Vehicle.jdebug` is bound explicitly to
  `build/Vehicle/gimbal_MC02.elf` so Ozone cannot accidentally reflash the
  DelayTest image. Build 2026073105 has not yet been flashed; compensated Q2
  behavior remains hardware-unverified. First powered run must start with the
  existing `max_pipe_angle_deg=0.5` commissioning limit and K2/stationary
  chassis.
- Ozone initially displayed `g_gimbal_debug` only as one address and rejected
  `.build_id` as `<tofscope>`. The cause was Vehicle's inherited
  `Release=-Os -g0`, which removed the `GimbalDebug` DWARF type while retaining
  the global address. The Vehicle preset now enables `GIMBAL_OZONE_WATCH` and
  appends `-g3` after `-g0`; optimization and executable behavior remain
  `-Os`, while Ozone can expand the structure. Debug, Release and DelayTest
  preset semantics are unchanged.

## 2026-07-31 DelayTest position-stability admission

- MC02 build `2026073104` fixes a real first-measurement admission mismatch.
  With the field manual's temporary `r_meas_cm=0.001`, a physically stationary
  ball at -0.432 mm was reported by the Jetson Kalman state as -102.7 mm/s;
  the old instantaneous `|v| <= 20 mm/s` gate therefore denied request 4.
  The observed V3 position, age, required flags, Jetson online/ready state and
  heartbeat age were otherwise valid.
- DelayTest measurement admission now requires the ball to remain inside
  +/-10 mm, at least eight valid samples, and no more than 2 mm position span
  over a continuous 300 ms window. Invalid/occluded/out-of-range samples,
  confidence below 300, leaving the center band, or a sample gap above 100 ms
  reset the window. The window resets after every pulse, so all 8-10 trials
  require a fresh stationary hold. Ozone exposes stable-ready, elapsed time,
  span and sample count.
- The frozen Jetson V3 wire format and normal competition controller are
  unchanged. The five-minute global OUT1 guard and all immediate stop/fault/
  link-watchdog cutoffs remain in force. Full offline verification passed:
  Debug, Release, Vehicle and DelayTest use 107,316/14,400,
  67,440/14,384, 67,160/14,384 and 109,916/14,456 bytes of
  FLASH/DTCMRAM respectively. This does not prove the new admission behavior
  on hardware.

## 2026-07-31 five-minute global OUT1 guard

- MC02 build `2026073103` extends the shared guarded OUT1 power deadline
  from 180 seconds to 300 seconds for the two-operator total-loop-delay
  measurement workflow. The DelayTest accepted-request renewals continue to
  renew this same deadline, now to five minutes.
- This changes the global bench-power guard, so it also applies to the
  ordinary PWRON/CALRUN/JOG commissioning paths. STOP, PWROFF, reset, K7/K8,
  TI safe stop, feedback/CAN/motor/Jetson/vision faults and link watchdogs
  retain their existing immediate cutoff behavior.
- Full offline verification passed for Debug, Release, Vehicle and DelayTest.
  FLASH/DTCMRAM use is respectively 107,316/14,384 bytes,
  67,440/14,368 bytes, 67,152/14,368 bytes and 109,324/14,416 bytes.
  This is not evidence that the new five-minute timeout has been flashed or
  physically timed on hardware.

## 2026-07-31 DelayTest operator-attention timeout renewal

- MC02 build `2026073102` fixes a live-test workflow defect found during the
  first empty-mechanism run. Build 2026073101's single 180-second OUT1
  deadline continued counting while the operator inspected the empty pulse,
  so a correct phase-3/6/7 wait could end in phase 8 / fault 3 before the
  8-10 measurement repetitions finished.
- Each accepted DelayTest empty-validation, mechanical-acknowledgement or
  measurement request now renews the existing bounded 180-second deadline.
  An invalid/denied request does not renew it. STOP, K7/K8/TI safe stop,
  feedback/CAN/motor/Jetson/vision faults, reset and an unattended 180-second
  interval still disable torque and remove OUT1 power.
- The first physical empty pulse on 2026073101 completed and returned within
  about 0.00038 rad of the captured motor zero, with phase 6, pulse_count 1
  and fault 0 visible in Ozone. This is operator evidence for that one empty
  pulse only, not a full delay measurement or general mechanical validation.

## 2026-07-31 dedicated total-loop-delay firmware

- MC02 build `2026073101` adds a separate `DelayTest` CMake preset for the
  `total_delay_ms` experiment in the field measurement manual. The normal
  Debug/Release/Vehicle presets compile `GIMBAL_ENABLE_DELAY_TEST=0`;
  DelayTest alone compiles it as one and also removes manual CALRUN.
- The only test waveform is pipe `0 -> -1.5 deg` for 500 ms then zero,
  using the measured 0.044 pipe/motor ratio and a fixed 12 rad/s DM4310
  position-speed command. There is no arbitrary angle or repeating-wave
  entry. The identical pulse must complete once without a ball and then
  receive a separate Ozone mechanical acknowledgement before any
  ball-present pulse is accepted.
- Every non-STOP delay request needs the independent
  `0xD31A7E57` three-second key. A measurement additionally requires Jetson
  online/ready, a fresh calibrated vision sample, valid velocity,
  `|x| <= 10 mm` and `|v| <= 20 mm/s`. Feedback, motor, CAN, return,
  Jetson/vision and existing TI/STOP cutoffs remain active; stop/fault/reset
  clears the empty-pulse acknowledgement.
- Only DelayTest raises the unchanged V3 `0x81 GIMBAL_STATUS` payload to
  100 Hz. No V3 frame, message ID, payload length, field, unit, sign or CRC
  changed. See `docs/DELAY_TEST_FIRMWARE.md` for the Ozone request/phase
  sequence. This source/build work does not claim powered-motion validation
  or a measured delay.
- `gimbal_MC02_DelayTest.jdebug` opens the DelayTest ELF explicitly. The
  original `gimbal_MC02.jdebug` intentionally remains bound to the ordinary
  Debug ELF, which otherwise overwrites a separately flashed DelayTest image
  whenever Ozone downloads from that project.
- Ozone freezes symbol-window editing while the CPU runs, and project-script
  functions are invoked from Console through `Script.Exec`, not by typing
  their bare names. The DelayTest project provides atomic key/request
  helpers backed by `Target.WriteU32`: `Script.Exec("DelayTestPrepare")`,
  `Script.Exec("DelayTestValidateEmpty")`,
  `Script.Exec("DelayTestAckMechanical")`,
  `Script.Exec("DelayTestMeasure")` and `Script.Exec("DelayTestStop")`.
  They do not halt the control loop; never halt the CPU to post a powered
  test request.

## 2026-07-30 visual-integration conversation handoff

- Added `../视觉联调交接.md` as the self-contained entry point for the new
  visual-integration conversation. It captures live build IDs, TI/MC02/
  Jetson ownership, V2/V3 links, five-question/K6 behavior, connections,
  calibration blockers, unverified hardware claims and a staged test order.
- Synchronized `docs/FIVE_QUESTION_CONTROL.md` to MC02 `2026073013` / TI
  `2026073012`, the three-second start countdown and K6 software-power
  permission. Updated only the implementation-build reference in the frozen
  V3 document; no V3 frame, message, field, unit, sign or CRC changed.
- The handoff flags that parts of `../competition/competition` documentation
  retain obsolete protocol/architecture text. New work must treat
  `docs/JETSON_MC02_PROTOCOL_V3.md` and live source as authoritative. This
  handoff update made no firmware change and did not repeat hardware tests.
- The handoff was then expanded with the explicit scope boundary requested
  by the user (no TI route/control changes), the V3 event-to-recording
  mapping, Q2's event-5 completion marker, five-point and independent-point
  calibration procedure, MC02 Ozone variables, live controller tunables,
  production ELF path and the exact evidence the new conversation should
  request. This remained documentation-only.

## 2026-07-30 K6 chassis software-power permission

- MC02 build `2026073013` is paired with TI build `2026073012`. K6 is no
  longer behaviorally unused: its existing frozen V2 `0xA2 BUTTON_EVENT`
  toggles a mirrored chassis software-power permission while the supervisor
  is idle. No V2 frame, message/action number, field/length, CRC, option bit,
  button ID or question ID changed, and the frozen protocol document was
  not edited.
- Both controllers default the permission to zero. Motion questions
  K1/K3/K4/K5 are accepted only after K6 has set permission one. K2 is still
  accepted with permission zero and explicitly clears it because the
  chassis must remain stationary. K6-off during a sequence commands
  SAFE_STOP; route-completion SAFE_STOP, K7, K8, faults and link loss also
  clear the mirrored latch. TI remains the final motor-authority gate if a
  button event is ever lost.
- TI OLED displays `PWR 0/1`, which means software permission rather than
  measured physical 12 V. The V2 `MOTOR_OUTPUT_ENABLED` flag still means
  actual TI motor-arm state and was not redefined.
- Full MC02 offline verification passed: Debug FLASH/DTCMRAM is
  107,188/14,320 bytes, Release is 67,352/14,312 and Vehicle is
  67,072/14,312. Paired TI verification also passed at Debug text/BSS
  30,984/2,496 and Release 20,888/2,281 bytes. Neither image was flashed in
  this work session; K6 synchronization and safe cutoff remain
  hardware-unverified. No gimbal/ball-control constant or algorithm changed.

## 2026-07-30 Jetson five-point calibration handoff

- The Jetson owner reports that the geometric pipe center is visually
  correct while the displayed +/-5 cm and +/-1 cm marks are displaced.
  This confirms the current pixel-to-metric polynomial is only a guide and
  is not a usable calibration. Jetson now keeps `fitted=false` and
  `sign_verified=false`, so it does not set V3 `CALIBRATED`.
- The proposed physical calibration records ball-center samples at
  +10, +5, 0, -5 and -10 cm, fits a quadratic for five samples, checks
  monotonicity and residuals, and deliberately leaves `sign_verified=false`
  until a person confirms that the motor end produces positive x. No
  measured pixels, fitted coefficients or physical error figures exist yet.
- MC02 behavior is compatible and unchanged. `vision_sample_valid()` needs
  `BALL_VALID | PIPE_VALID | CALIBRATED`, a fresh sample and confidence at
  least 300; therefore leaving `PIPE_VALID` and `CALIBRATED` set on a frame
  with `BALL_VALID` clear cannot authorize closed-loop control. Before
  enabling competition control, also validate the fitted map at independent
  non-fit points (for example +/-2.5, +/-7.5 and the accessible near-end
  positions) so residuals at the five fitted points are not mistaken for
  full-range accuracy required by internal Q5.

## 2026-07-30 USART1 connector pin-order correction

- The user supplied the DM-MC-Board02 V1.1 USART1 connector figure and
  correctly identified that the middle contact, pin 2, is RX/PA10. The
  keyed/front-view order is now recorded consistently as pin 1 PA9/TX,
  pin 2 PA10/RX and pin 3 GND. Earlier tables that called pin 2 TX, pin 3
  RX and pin 1 GND were wrong and have been corrected in the MC02 wiring
  checklist, the V2 communication document and the TI-side wiring copy.
- Firmware direction was already correct and did not require a functional
  change: `gimbal_MC02.ioc` maps `PA9.Signal=USART1_TX` and
  `PA10.Signal=USART1_RX`; `ti_link.c` initializes PA9/PA10 as USART1 AF7
  and uses USART1 in TX/RX mode at 115200 8N1. The offline verifier now
  asserts both MCU signal mappings and all three connector-table entries so
  this documentation reversal cannot silently recur. No TI control or route
  source was changed.

## 2026-07-30 full five-question final verification

- Re-read both live agent logs after the concurrent chassis completion and
  preserved the newer paired baseline: MC02 `2026073012`, TI
  `2026073011`. The MC02 verification script and the README, Jetson V3,
  TI V2 and five-question control documents were synchronized to those
  exact build IDs; no frozen wire field, message number, payload length,
  CRC or sign convention changed.
- `tools/verify_project.ps1` then completed clean Debug, Release and Vehicle
  builds and all structural/CRC checks. FLASH/DTCMRAM use is
  107,044/14,320 bytes (Debug), 67,224/14,304 (Release) and
  66,944/14,304 (Vehicle). The Vehicle compile database confirms manual
  CALRUN is absent while Jetson V3, TI V2 and all five competition paths
  remain linked.
- This is the complete source/build baseline requested by the user, rather
  than the older K1-only restricted baseline. It is ready to flash for
  commissioning. Powered end-to-end behavior and controller gains remain
  physical test/calibration results, not facts established by compilation.

## 2026-07-30 five-question chassis completion

- MC02 build `2026073012` is paired with TI build `2026073011`. The frozen
  V2 wire envelope, message/action numbers, payload lengths, CRC,
  option-bit meanings and question/button IDs were not changed.
- K1-K5 now all enter the common question supervisor. MC02 waits for its
  existing question-preparation readiness, sends TI PREPARE, waits for the
  matching READY acknowledgement, performs a restored three-second
  countdown, then sends idempotent START. The unlogged intermediate
  `2026073011` source had used 200 ms; that was not retained.
- For Q2, TI remains continuously disarmed and does not decide when the
  ball task ends. The existing MC02 gimbal-app completion/fault query is the
  local completion indication; `ti_link.c` converts normal completion into
  the frozen V2 SAFE_STOP and converts a fault into the existing faulted
  SAFE_STOP path. TI has a 30-second stationary fail-safe ceiling, while the
  task controller may complete or fault earlier.
- Q1 retains the 120 RPM/20-second tuned lap. Q3 is TI's 1500 mm A-to-B
  route with a 100 RPM cruise, 45 RPM from 1250-1450 mm and 20 RPM over the
  final 50 mm, an 8-second ceiling and B event. Q4/Q5 reuse the one-lap route at 100 RPM with a
  30-second ceiling. MC02 sends SAFE_STOP on TI route completion. No
  ball-control constant or algorithm was changed in this chassis work.
- Clean CMake builds passed: Debug FLASH 107,044 bytes/DTCMRAM 14,320 bytes;
  Release FLASH 67,224 bytes/DTCMRAM 14,304 bytes. This is compile evidence
  only; build `2026073012`, the five button sequences, Q2 completion,
  Q3 B timing/distance and Q4/Q5 lap timing remain hardware-unverified.

## 2026-07-30 independent pre-flash rebuild

- The root agent independently reran TI `tools/verify_project.ps1` and clean
  MC02 Debug/Release CMake builds after the eight-button audit. All commands
  completed successfully. The freshly produced MC02 images retain
  `GIMBAL_BUILD_ID=2026073010`; Debug uses 99,164 bytes FLASH/13,464 bytes
  DTCMRAM and Release uses 62,436/13,448.
- This clears the software build gate only. It authorizes the first
  logic-only UART/button test with both motor power rails disconnected; it
  does not establish electrical UART operation, button wiring, stop timing,
  Jetson V3 operation or safe powered motion.

## 2026-07-30 eight-button/TI V2 audit

- MC02 build `2026073010` is the compile-tested supervisor baseline paired
  with TI build `2026073009`. The frozen V2 envelope, message IDs, field
  lengths, CRC, action IDs and button/question mapping were not changed.
- MC02 parses all K1-K8 press/release `0xA2` packets and exposes the current
  button mask, last button, emergency latch and unsupported question in
  `g_gimbal_debug`. K1 alone starts Q1. K2-K5 are intentionally SAFE_STOP
  because their complete controllers do not exist; K6 is reserved; K7 sends
  normal SAFE_STOP/reset; K8 latches the TI-link/emergency stop.
- TI now assigns all eight physical inputs and implements local K7/K8 motor
  removal, but this has only passed source/static checks. The physical
  button wires, event stream, true-zero PWM, K8-release/K7-clear sequence
  and 500 ms disconnect stops have not been hardware-tested.
- TI now sends a transient `STOPPING=4` state and a route-state-derived
  A_MARKER_DETECTED event. B_MARKER_PASSED and the Q3 B route remain
  unimplemented. Q2-Q5 protocol recognition must not be described as
  completed question behavior.
- Clean CMake/GCC builds passed: Debug FLASH 99,164 bytes and DTCMRAM
  13,464 bytes; Release FLASH 62,436 bytes and DTCMRAM 13,448 bytes. These
  are compile results only; build `2026073010` was not flashed in this work
  session.

## 2026-07-30 Jetson V3 sign-fix follow-up

- The vision owner reports that the actually executed estimator path now
  loads `control.plant.ball_accel_gain`, negates it once at initialization,
  and calls `predict(dt, accel_gain * theta_cmd)`. With the configured value
  7.0, the runtime model is therefore `x_ddot = -7.0 * alpha_pipe`; the old
  positive hard-coded model is reported removed. This remains an owner
  report until the Jetson repository/binary is independently inspected.
- The V3 wire layout remains frozen and unchanged: no new fields were added
  to the 24-byte `0x81` status payload. Any future angle or chassis-
  acceleration fields require a separately specified V4.
- Physical direction has not been verified. `calib.sign_verified` correctly
  remains false. The current calibration predicts that the motor/physical
  left end is on the image-right side near pixel 1138 and should report
  positive x. Before setting `sign_verified=true`, place the ball at the
  motor end, O point and hinge end and record all three displayed x values;
  they must be positive, approximately zero and negative respectively.
- The vision owner also reports `calib.fitted=true` while explicitly calling
  it a pseudo-calibration. Do not use that flag to authorize MC02 closed-loop
  control. A guessed/pseudo pixel-to-metric map is not a completed fit:
  `fitted` must remain false (or MC02 must otherwise reject CALIBRATED) until
  the three physical reference positions have been measured and the scale
  checked against the known pipe geometry. Do not set both calibration flags
  merely because the signs look plausible.
- The vision owner's updated protocol document lives in the vision project;
  the root `下位机协议.md` copy in this repository was still the older snapshot
  when checked. Treat `docs/JETSON_MC02_PROTOCOL_V3.md` as the MC02 canonical
  wire contract until an intentional, reviewed refresh is copied in.
- Current MC02 integration baseline is build `2026073009`, which already
  contains the new TI V2 endpoint. The Jetson V3 parser/autonomous ball loop
  must be added on top of `2026073009` while preserving the TI supervisor;
  do not resume from the older `2026073008` baseline.

## 2026-07-30 MC02/TI link and OLED integration

- Build `2026073009` implements the frozen V2 UART endpoint on MC02 USART1
  PA9 TX / PA10 RX at 115200 8N1. It uses interrupt-driven byte-stream RX,
  queued nonblocking TX, the frozen envelope and CRC16/CCITT-FALSE. It sends
  10 Hz heartbeats, accepts TI status/event/button frames, retries idempotent
  Q1 PREPARE/START commands, owns the three-second countdown, accepts route
  completion only for the active run, and sends SAFE_STOP until TI confirms
  idle. Missing expected TI status for 500 ms latches a TI-link fault and
  forces the existing gimbal/OUT1 safe-stop path.
- K1 selects the implemented Q1 route. K2-K5 packet IDs are recognized but
  deliberately produce SAFE_STOP because their physical TI inputs and full
  question controllers do not exist. K7/K8 packets force safe stop, but those
  TI buttons remain physically unassigned. `g_gimbal_debug` now contains TI
  online/supervisor/chassis/fault, run/question and RX/CRC/TX/drop fields; no
  second Watch global was added.
- TI sibling build `2026073006` disables standalone K1 motion and adds the
  OLED stopwatch on shared PB2/PB3 I2C. It starts timing at an online
  debounced K1 press and freezes on completion/cancel/fault/SAFE_STOP.
- Clean CMake/GCC builds passed: Vehicle uses 61,804 bytes FLASH and
  13,424 bytes DTCMRAM; Debug uses 98,748/13,432. Vehicle keeps manual
  CALRUN disabled. This is compile/static evidence only. USART1 electrical
  behavior, CRC exchange, heartbeat/status rates, OLED hardware, Q1
  handshake and both 500 ms stop paths remain unverified. First join only
  TX/RX/GND with chassis and gimbal motor rails disconnected; never join the
  controller power rails.

## 2026-07-30 Jetson owner `下位机协议.md` review

- The new root document `下位机协议.md` was read completely and compared
  field by field with `docs/JETSON_MC02_PROTOCOL_V3.md`. It reports that the
  Jetson side has implemented V3 framing, rejection of non-V3 frames,
  CRC-16/CCITT-FALSE self-test `0xF12F`, split-tolerant stream parsing,
  10 Hz independent heartbeat, 40-50 Hz VISION_SAMPLE, sequence/error
  counters, `/dev/serial/by-id` selection, monotonic timestamps, invalid
  sample flags and MC02 run-ID echo. This is the vision owner's report; the
  MC02 repo has not independently audited or executed the Jetson code.
- The vision owner reports a new installed-camera measurement of about
  34 px/cm over 1440 pixels, approximately 42 cm field of view. The complete
  25 cm pipe and supports are visible, so the full +/-125 mm metric range is
  expected to be available. This supersedes the old assumption that the
  6 mm setup saw only 10 cm, but still requires the Jetson calibration flags
  `fitted=true` and `sign_verified=true` before MC02 may close the loop.
- No existing V3 field, payload length, message type, unit, CRC rule or sign
  differs from the canonical document. V3 therefore remains the active
  protocol. In particular, Jetson still sends no angle, enable, current or
  raw CAN command.
- The proposed `measured_angle_mdeg` and `chassis_accel_mm_s2` additions are
  not accepted into V3 yet. Appending them changes the `0x81 GIMBAL_STATUS`
  payload from 24 bytes and is an on-wire layout change; if adopted it must
  be specified as V4 with a regenerated CRC vector and both parsers updated.
  MC02 also has no independent pipe-angle sensor: DM4310 feedback plus the
  approximate linkage mapping can provide only
  `estimated_pipe_angle_mdeg`, not a true measured physical angle. Chassis
  acceleration would initially be an estimate from TI wheel-speed
  differentiation or a future validated IMU path and needs its own validity
  flag. Neither estimate is required for the first stationary ball loop.
- Two sign expressions in the Jetson handoff need correction/clarification
  before their reference controller is reused. With frozen V3 coordinates,
  physical pipe angle has `x_ddot approximately -gain * alpha_pipe`; the
  later text `x_ddot = 7.0 * theta` is wrong if `theta` means physical pipe
  angle. Likewise the quoted inverse relation
  `theta = (1.4*a_des + ax)/g` needs the corresponding leading negative sign
  and an explicit chassis-acceleration sign convention. Jetson `main.cpp`
  is reported to already use the correct negative process-model sign, so
  this may be documentation/reference-code drift rather than a running-code
  bug.
- The proposed physical sign check is consistent: positive MC02 motor STEP
  lowers the motor/left end and should accelerate the ball toward positive
  x. The test must identify STEP as motor angle, not physical pipe angle.

## 2026-07-30 vehicle installation and normal host ownership

- The user reports that the MC02/gimbal assembly is now installed on the
  vehicle. In normal operation Jetson is the USB host connected to MC02's
  single Type-C device port; the PC commissioning console is disconnected.
  PC USB is used only as a bench/service replacement for Jetson, never as a
  simultaneous second host.
- Keep the frozen responsibility boundary: Jetson provides calibrated ball
  position/velocity, timestamps, confidence and validity through protocol V3.
  MC02 owns the ball controller, pipe/motor mapping, DM4310 enable and CAN
  commands, angle/speed clamps, watchdogs and all gimbal safety cutoffs.
  “Jetson controls MC02” means it supplies supervisory/vision input, not that
  it sends raw motor angle, current, enable or CAN commands.
- Physical installation does not prove the production link. Build
  `2026073008` still has no Jetson V3 binary parser or autonomous ball loop,
  and the MC02/TI link is also not implemented end to end. Do not describe
  the installed vehicle as vision-controlled until those firmware paths and
  their stale/link-loss tests pass.

## 2026-07-30 wiring-manual recheck and OUT1 pin-order correction

- The MC02 V1.1 manual interface pages were rendered and visually rechecked
  before issuing the complete wiring handoff. The board's single native USB
  connector is USB Type-C device/CDC; the PC commissioning console and
  production Jetson link use this same port at different times. It cannot be
  connected to two USB hosts with a passive Y cable. MC02 USART1 remains the
  dedicated 3.3 V TTL link to TI.
- A material error was found in the earlier local wiring checklist. The
  manual page-4 keyed front view defines the integrated OUT1/CAN1 2+2 order
  as pin 1 VCC, pin 2 GND, pin 3 CAN_H, pin 4 CAN_L. The checklist had pins
  3/4 reversed and is now corrected. The separate two-pin CAN1 connector is
  still pin 1 CAN_L, pin 2 CAN_H; the two connector layouts must not be
  conflated.
- The user's existing keyed 2+2 cable has already passed continuous CAN and
  motor-motion testing, so do not repin it based on the superseded checklist.
  Always follow the keyed connector front view and board silkscreen rather
  than cable colors.
- The wiring checklist now explicitly records USB ownership, the fact that
  USB VBUS can leave logic powered after 6S removal, and the unused current
  ports (UART7/UART10, OUT2/CAN2, CAN3, RS485, SBUS and PWM). No firmware,
  protocol payload or installed wiring changed in this documentation-only
  correction.

## 2026-07-30 approximate installed pipe-angle calibration

- The user measured the real pipe tilt with a phone level at the +/-45 motor
  endpoints. Both magnitudes were between 1 and 2 degrees and closer to
  2 degrees. Record the first controller calibration conservatively as
  `motor +/-45 deg -> pipe -/+ approximately 2 deg`, or average signed gain
  `alpha_pipe/theta_motor approximately -0.044`. This is a phone-level
  estimate, not a precision calibration; retain tunable mapping parameters
  and refine them from vision closed-loop behavior.
- The observed usable pipe-angle envelope is therefore provisionally about
  +/-2 degrees at the current safe motor limit. This is sufficient authority
  for the stationary O -> +50 mm -> -50 mm task, but it can cancel only about
  `g*tan(2 deg) = 0.34 m/s^2` of chassis acceleration before leaving no
  authority for ball-error correction.
- The TI chassis's current 250 RPM/s deceleration is incompatible with this
  measured envelope. With 65 mm wheels, 100 RPM/s is already about
  0.34 m/s^2 and consumes essentially the full +/-2-degree tilt. For the
  first integrated balance test, a safer provisional chassis acceleration
  and deceleration slew is 50-60 RPM/s in both directions, leaving roughly
  0.8-1.0 degree for ball correction. This is a coordination recommendation,
  not an implemented TI change.
- Mechanical open-loop range commissioning is complete; do not expand beyond
  +/-45 motor degrees or repeat CALRUN with the ball. The next firmware phase
  is the Jetson V3 receive/parser, freshness handling and a tunable,
  low-authority stationary ball-position controller before any chassis
  integration.

## 2026-07-30 physical pipe-angle sign and unintended open-loop ball result

- Freeze the physical pipe-angle convention independently of the DM4310
  coordinate. Let `alpha_pipe = atan2(h_motor - h_hinge, L)`, with zero at
  level and vertical height positive upward. Therefore the motor-side end
  below the hinge plane is a negative pipe angle; above it is positive.
  The installed mechanism has the locally inverted mapping
  `positive motor offset -> negative pipe angle` and
  `negative motor offset -> positive pipe angle`. Do not label the CALRUN
  `+45` motor target as `+45` pipe angle.
- The user ran the open-loop CALRUN with the ball present. On the first
  positive-motor/downward-pipe leg, the ball rolled directly to the physical
  left end. It then remained there through the later return/reverse legs.
  The user now confirms that this physical left end is the motor end. The
  follow-up unpowered/manual check confirms the ball can leave that end and
  move freely when the pipe is tilted; no persistent end-stop wedge or
  channel obstruction was found. The earlier lack of visible recovery
  therefore does not establish a mechanical jam.
- CALRUN is a ball-free actuator calibration, not a ball controller. The
  current firmware has no Jetson parser or vision closed loop, so it cannot
  brake or recover the ball. Do not repeat CALRUN with the ball installed.
  Remove the ball before further mechanism measurement and inspect that the
  channel/end stops allow the ball to move freely.
- Ball-position sign is now frozen as a fixed spatial coordinate: zero at O,
  positive toward the motor-side physical left end, and negative toward the
  hinge-side physical right end. It never follows the changing downhill
  direction. With the pipe-angle convention above, small-angle ball
  acceleration has the opposite sign to `alpha_pipe`. Because this sign
  changes closed-loop semantics, the Jetson protocol was promoted from V2
  to `docs/JETSON_MC02_PROTOCOL_V3.md`, on-wire version `0x03`. Packet
  layouts remain unchanged, but V2/V3 must not be mixed and the canonical
  CRC vector was regenerated.

## 2026-07-30 build 2026073008 commanded-range hardware result

- The user flashed Debug build `2026073008`; PING confirmed the exact build.
  The corrected `CALRUN 4310 30` reached `OFF=+523 mrad`, returned to
  `0 mrad`, reached `OFF=-523 mrad`, and returned to `0 mrad`. Targets were
  +/-524/523 mrad, so endpoint error was approximately 1 mrad.
- Without reflashing, `CALRUN 4310 45` then reached `OFF=+784 mrad`,
  returned to `0 mrad`, reached `OFF=-785 mrad`, and finally returned to
  `0 mrad` (final absolute position 3.116 rad versus captured zero
  3.117 rad, rounded offset 0 mrad). Targets were +/-785/786 mrad.
  This verifies that the shared-limit fix works and that DM4310 follows the
  intended +/-30 and +/-45 motor-angle commands accurately.
- The serial log contains no CAN fault, feedback timeout or aborted leg.
  The user has not yet explicitly stated whether the pipe remained in contact
  throughout the now-real +/-45 movement, so mechanical contact, bounce,
  collision, slip and abnormal sound are still pending physical confirmation.
  Do not infer them solely from successful motor telemetry.
- Do not expand past +/-45. The next calibration is ball-free measurement of
  the same marked point on the motor-side pipe at level, +45 and -45 motor
  positions. Record signed vertical displacement relative to level and
  confirm continuous follower contact. This freezes the real motor-to-pipe
  mapping before any open-loop ball motion or vision controller test.

## 2026-07-30 CALRUN execution-clamp diagnosis and build 2026073008

- The user's complete serial logs disprove the earlier interpretation that
  builds `2026073006` and `2026073007` physically exercised +/-30 and
  +/-45 motor degrees. Build `2026073006` printed targets
  `Z +/- 524 mrad`, but ended at only `OFF=+174/-174 mrad`; build
  `2026073007` printed targets `Z +/- 786/785 mrad`, but likewise ended at
  only `OFF=+174/-175 mrad`. These actual offsets are exactly +/-10 degrees.
  The normal physical observations therefore validate only another
  +/-10-degree sequence; neither +/-30 nor +/-45 contact/range is hardware
  tested yet.
- Source diagnosis found two angle clamps. `start_motion()` correctly chose
  the expanded CALRUN limit, which is why the printed TARGET was correct,
  but `run_motion()` always reclamped the executing request to
  `GIMBAL_DESKTOP_MAX_ANGLE_DEG` (+/-10 degrees). Build `2026073008` replaces
  the duplicated selection with one `active_motion_maximum_rad()` helper
  called by both admission and execution. CALRUN now uses +/-45 in both
  places; ordinary JOG/STEP/SWEEP still use +/-10.
- The old archive was renamed to
  `backups/gimbal_MC02_BUILD_2026073006_RANGE_CLAMP_BUG.zip` so it cannot be
  mistaken for a passed +/-30 build. Its contents, size 25,296,119 bytes and
  SHA-256
  `D0AB98DC602DB7A3CB917BD08193233E300DF69EBFABDC6F50DB1169581B488C`
  are unchanged.
- Clean Debug and Vehicle builds passed without warnings. Debug uses
  78,068 bytes FLASH and 12,240 bytes DTCMRAM; Vehicle uses 52,428 bytes
  FLASH and 12,232 bytes DTCMRAM. Debug ELF SHA-256 is
  `9F0A42BEB8F044879EB88B6F9AAAFC8A80ADE38B416C5E0D0D42BB7319F1E503`;
  Vehicle ELF SHA-256 is
  `C1546276E2AA0E9275DD5C148B04A5329A0A756EBA6120CC42FB45300A5D8E8D`.
  ELF inspection still proves active CALRUN exists only in Debug and Vehicle
  contains only the disabled response. `g_gimbal_debug` remains at
  `0x20002154`, size `0x94`. Build `2026073008` was subsequently flashed and
  passed the commanded-angle telemetry test described in the newer section
  above.

## 2026-07-30 builds 2026073006/3007 preliminary result (superseded)

- The user initially reported the build `2026073006` `CALRUN 4310 30`
  behavior as physically normal: the motor-side pipe end moved downward
  first and then upward, with no observed contact separation, bounce,
  collision, slip or abnormal sound. The later serial log proves the actual
  motion was still only +/-10 degrees because of the execution clamp.
  Therefore this is not a +/-30 range/contact validation.
- That build was archived before further range work. The archive has now
  been correctly renamed
  `backups/gimbal_MC02_BUILD_2026073006_RANGE_CLAMP_BUG.zip`. It is
  25,296,119 bytes with SHA-256
  `D0AB98DC602DB7A3CB917BD08193233E300DF69EBFABDC6F50DB1169581B488C`.
- Build `2026073007` expands only the Debug-only CALRUN maximum from
  +/-30 to +/-45 motor degrees for the next ball-free contact/clearance test.
  Its five-second leg timeout, 0.20 rad/s speed and all motor-side
  acceleration parameters are unchanged, so the modeled downward
  acceleration bound does not increase. Ordinary JOG/STEP/SWEEP remain
  limited to +/-10 degrees and Vehicle still compiles CALRUN inactive.
- Clean Debug and Vehicle builds passed without warnings. Debug uses
  78,036 bytes FLASH and 12,240 bytes DTCMRAM; Vehicle uses 52,412 bytes
  FLASH and 12,232 bytes DTCMRAM. Debug ELF SHA-256 is
  `8A8BB1807B007A19181897DD18FBFD7F6BF407669C9E4D3B915333CD9650A016`;
  Vehicle ELF SHA-256 is
  `BD79CA50B565F753D93241B17782047C47BF0941C2B35035619F959D15D0D0BC`.
  ELF inspection proves the active 45-degree command exists only in Debug,
  while Vehicle contains only the disabled reply. `g_gimbal_debug` remains
  at `0x20002154`, size `0x94`. Build `2026073007` is not yet flashed or
  hardware-tested.
- Build `2026073007` was subsequently flashed, but its `CALRUN 4310 45`
  serial result also stopped at +/-10 degrees. The newer diagnosis and build
  `2026073008` above supersede this section's original next action.

## 2026-07-30 non-captive contact constraint and build 2026073006

- The pipe only rests on the eccentric follower and is not captive. The user
  therefore requires the follower's downward acceleration to remain below
  gravity so that it cannot accelerate away from the pipe and break contact.
  Contact retention, not merely collision clearance, is now a mandatory
  pass/fail condition for every ball-free range test and all later control.
- With the recorded DM4310 acceleration setting `2`, 10:1 reduction and the
  frozen crank radius `r=15.45 mm`, interpreting that setting as
  2 krad/s^2 at the rotor gives an estimated worst output tangential
  acceleration of about `15.45 mm * 200 rad/s^2 = 3.09 m/s^2`, approximately
  `0.32 g`. The radial contribution at the firmware's `0.20 rad/s` output
  speed cap is negligible. This gives a theoretical factor of about three
  below `g`, but it is a parameter-based estimate rather than a measured
  installed acceleration; the next physical test must still watch for
  lift-off, bounce or intermittent contact.
- The fully hardware-passed build `2026073005` was backed up before extending
  the test range at
  `backups/gimbal_MC02_BUILD_2026073005_HW_PASS.zip`. It is 1,708,891 bytes
  with SHA-256
  `E916AB6FBFFC8C9509F47B0EA5377B0FA355B2B759C440522BFE92FA382BB71E`.
- Build `2026073006` changes only the Debug-only manual CALRUN envelope:
  `CALRUN 4310 deg` now accepts 0.1-30.0 motor degrees and uses a five-second
  timeout per `+deg -> 0 -> -deg -> 0` leg. The existing `0.20 rad/s`
  velocity, feedback/fault watchdogs, unconditional STOP and power cutoffs
  are unchanged. Ordinary JOG/STEP/SWEEP commands remain clamped to
  +/-10 degrees, so this does not silently enlarge any other desktop path.
  The Vehicle preset still compiles CALRUN out.
- Clean Debug and Vehicle builds passed without warnings. Debug uses
  78,028 bytes FLASH and 12,240 bytes DTCMRAM; Vehicle uses 52,404 bytes
  FLASH and 12,232 bytes DTCMRAM. ELF inspection confirmed the Debug image
  contains the active 30-degree CALRUN path, while Vehicle contains only the
  disabled reply. `g_gimbal_debug` remains at `0x20002154`, size `0x94`.
  Build `2026073006` has not yet been flashed or hardware-tested.
- Next safe action: remove the ball, level the pipe, keep the mechanism clear,
  flash the Debug ELF, confirm `PING` reports `2026073006`, then send the
  single command `CALRUN 4310 30`. Record continuous follower contact,
  direction, any bounce/slip/collision/noise, and final return to zero.
  Send `STOP` or use the master cutoff immediately if contact separates.

## 2026-07-30 installed sign and available control authority

- The repeated ball-free CALRUN visually confirms the installed sign:
  positive motor offset first lowers the motor-side pipe end, and negative
  motor offset then raises it. This agrees with the earlier isolated +10
  observation and finally resolves the sign using one uninterrupted motor
  coordinate frame.
- The current +/-10-degree motor clamp is a commissioning limit, not the
  mechanism's final pipe range. With frozen crank radius `r=15.45 mm`, neutral
  near the 3-o'clock phase, and disputed but bounded installed lever distance
  `L=236-248 mm`, ideal follower lift is `r*sin(motor_angle)`. Theoretical
  pipe amplitudes are approximately:
  motor +/-10 -> lift +/-2.68 mm -> pipe +/-0.62-0.65 degrees;
  +/-30 -> +/-7.73 mm -> +/-1.78-1.88 degrees;
  +/-45 -> +/-10.92 mm -> +/-2.52-2.65 degrees;
  +/-60 -> +/-13.38 mm -> +/-3.09-3.25 degrees.
  The absolute geometric limit near +/-90 degrees is only about
  +/-3.57-3.75 degrees of pipe tilt.
- The present +/-10 test range is theoretically sufficient for the stationary
  O -> +50 mm -> -50 mm test: a rolling solid sphere at about 0.63 degrees
  has ideal acceleration around 0.078 m/s^2 and an ideal symmetric
  accelerate/brake transfer time around 1.6 seconds for 50 mm, before
  actuator/vision/settling overhead. This provides margin under 5 seconds,
  but requires closed-loop braking and must still be proven with the real
  ball and vision.
- +/-10 motor degrees is not a robust authority range for balancing during
  chassis acceleration. A 0.62-0.65-degree pipe tilt cancels only about
  0.106-0.111 m/s^2 longitudinal chassis acceleration, equivalent to roughly
  31-33 wheel RPM/s for the TI chassis's 65 mm wheels. The current TI
  acceleration/deceleration slew values 80/250 RPM/s exceed that.
- The existing mechanism itself need not be replaced solely because the
  first test was small. A future ball-free staged expansion toward about
  +/-45 motor degrees would provide roughly +/-2.5-2.65 degrees pipe
  authority while retaining the small crank's fine transmission resolution.
  Even the mechanism's absolute maximum cannot cancel the current 250 RPM/s
  braking slew (about 5 degrees equivalent tilt), so integrated operation
  must also reduce TI deceleration, initially toward the previously proposed
  100-120 RPM/s. These are calculated next-step targets, not implemented or
  hardware-verified limits. Do not raise the firmware clamp until a
  ball-free clearance/contact test is explicitly authorized.

## 2026-07-30 enabled-state versus LED correction

- A second build `2026073005` CALRUN pass reproduced correct +10/0/-10/0
  position tracking. The user reports green continuously only during the
  first 0-to-+10-degree leg; the remaining three legs appeared steady red
  even though the mechanism continued to move normally. The user explicitly
  corrects the mechanical assumption: DM4310 static friction is high enough
  that an unpowered mechanism will essentially not continue from gravity or
  inertia. The earlier gravity/inertia explanation for sustained motion is
  therefore withdrawn.
- Two STATUS captures taken during the visually red later legs provide direct
  evidence that the drive was actually enabled. In CAL phase 4 and phase 6,
  both reported `STATE=STEP`, `FAULT=0`, `ONLINE=1`, `ERR=0x1`, `AGE=0`,
  `TXERR=0`, and feedback position equal to command/level zero at 3.141 rad.
  RX/TX counts were increasing. This is fresh CAN state, not stale telemetry,
  and proves the driver remained in state 1 while accurately closing its
  position loop.
- DM-J4310 V1.2 manual pages 10 and 13 were re-rendered and visually checked.
  They unambiguously define state/ERR 1 as enabled with steady green, and
  state/ERR 0 as disabled with steady red. Thus this unit's observed red
  color during later enabled legs conflicts with the manual. The exact reason
  is unverified (indicator identification/visibility or motor-firmware LED
  behavior are possibilities); do not invent one. For control and safety,
  use fresh CAN ERR/state, position, watchdogs and fault codes rather than
  LED color alone.
- CALRUN still sends a deliberate disable at each three-second leg boundary,
  but that momentary boundary event cannot explain steady red throughout the
  later enabled legs. No firmware change is justified by the current data:
  state 1, fresh feedback, zero MC02 fault, zero CAN TX error and exact
  position tracking all passed.

## 2026-07-30 CALRUN installed hardware pass

- The user flashed build `2026073005`; `PING` returned the correct build.
  `CALRUN 4310 10` then passed end to end. Cold-start feedback became ready
  after about 1.069 s, proving the repeated torque-disable startup request
  fixed build `2026073004`'s preflight deadlock.
- The captured level zero was 3.142 rad. The four reported legs were:
  +10 degrees target/final 3.316/3.316 rad (offset +0.174 rad), return target/
  final 3.142/3.141 rad (reported offset 0 mrad after integer conversion),
  -10 degrees target/final 2.967/2.967 rad (offset -0.174 rad), and final
  return target/final 3.142/3.142 rad (offset 0 mrad). Each leg completed its
  bounded three-second interval, the user reported physical behavior normal,
  and CALRUN finally disabled torque and turned OUT1 off.
- The initial interpretation of apparent disable was that CALRUN's explicit
  inter-leg disable allowed inertia/gravity motion before immediate
  re-enable. The user's later high-static-friction correction and fresh
  in-leg `ERR=1` STATUS captures supersede that explanation; see the newer
  enabled-state-versus-LED section above.

## 2026-07-30 CALRUN cold-start result and build 2026073005

- The user flashed build `2026073004` and sent the intended single command
  `CALRUN 4310 10`. OUT1 energized and the motor briefly showed green, but
  exactly 5,000 ms after the start reply firmware printed
  `CALRUN ABORT: motor feedback not ready`, removed OUT1 power, and all motor
  LEDs went out. A second command was denied because the first preflight
  timeout had latched `GIMBAL_FAULT_FEEDBACK_TIMEOUT`, so the state was no
  longer SAFE_IDLE. No commanded motor motion occurred.
- Source review found the startup deadlock: after the 500 ms delay,
  SAFE_IDLE sent only the initial five torque-disable frames. If a cold motor
  was not ready to answer those frames, CALRUN waited for fresh feedback but
  sent no further request. This differs from the already verified OBSERVE
  behavior, which repeats the torque-disable/feedback-request frame every
  20 ms.
- Before fixing it, build `2026073004` was archived at
  `backups/gimbal_MC02_BUILD_2026073004_PRE_READYFIX.zip`. The archive is
  1,704,872 bytes with SHA-256
  `40D4E107E08E49CC8ECE62913998AE5F37DEF627B5BF3ED7BB5F8484B49E6F99`.
- Build `2026073005` keeps transmitting the same explicit torque-disable
  frame every 20 ms after the initial burst while CALRUN waits for fresh
  feedback. The cold-start preflight window is extended from 5 to 15
  seconds. A valid nonzero motor fault still cuts power and latches FAULT,
  while a plain no-feedback preflight timeout now cuts power and returns to
  SAFE_IDLE so the same one-line command can be retried without CLEAR.
  `STOP`, CAN-transmit failure, reset and all existing watchdog/motion
  cutoffs remain unconditional.
- Clean Debug and Vehicle builds passed with no warnings. Debug uses
  77,972 bytes FLASH and 12,240 bytes DTCMRAM; Vehicle uses 52,380/12,232
  bytes. Debug ELF SHA-256 is
  `7FAB13F86DE7BCB2779A540BEB2460BDA2BE8C1A71C11AFC8877713AA99F5344`;
  Vehicle ELF SHA-256 is
  `76CF305FF7A0EA30CAA9EBA1FC5576D56112103723F96CC0C4D63A9B287698E1`.
  ELF inspection again proves CALRUN active only in Debug and disabled in
  Vehicle. `g_gimbal_debug` remains at `0x20002154`, size `0x94`.
  Build `2026073005` has not yet been flashed or hardware-tested.

## 2026-07-30 one-command installed calibration build

- Before this change, the complete compile-tested build `2026073003` source,
  documents and Debug ELF/HEX/BIN were archived as
  `backups/gimbal_MC02_BUILD_2026073003_PRE_CALRUN.zip`. The archive is
  978,160 bytes with SHA-256
  `45BC85E880757BA28AE4479685C3FF2C7DA6CB85007BBBFA0505A841F4AEDCBE`.
- Build `2026073004` adds exactly one convenience command for the present
  ball-free manual bench calibration: `CALRUN 4310 deg`, where `deg` is
  restricted to 0.1-10.0 degrees. From SAFE_IDLE it performs the guarded
  OUT1 power-on, waits up to 5 seconds for the startup disable burst and
  fresh fault-free feedback, gives a 3-second visible countdown, captures
  the level zero, then automatically runs `+deg -> 0 -> -deg -> 0`. Each leg
  reuses the existing 0.20 rad/s, 3-second zero-referenced JOG path,
  80 ms feedback watchdog, motor-error checks and 200 ms post-disable
  settling guard. Completion, failure, `STOP`, `PWROFF`, reset or any fault
  disables torque and turns OUT1 off.
- While CALRUN is active, USB accepts only `PING`, `HELP`, `STATUS` and
  unconditional `STOP`; debugger motion requests are suppressed except
  STOP. `g_gimbal_debug.calrun_phase` reports 0 idle, 1 wait-ready,
  2 countdown, 3 positive, 4 first return, 5 negative and 6 final return.
- CALRUN is deliberately not a vehicle-control interface. It is compiled
  only when `GIMBAL_ENABLE_MANUAL_CALRUN=1`, which is the present Debug
  default for the requested manual test. The CMake `Vehicle` preset sets
  `GIMBAL_MANUAL_CALRUN=OFF`, which defines the firmware flag as 0; use that
  preset for every vehicle build. The command then returns a disabled
  message and cannot energize OUT1.
- Final Debug GCC/CMake/Ninja build passed without warnings in the captured
  output: FLASH is 77,772 bytes and DTCMRAM is 12,240 bytes.
  `g_gimbal_debug` remains at `0x20002154` and grows to `0x94` bytes.
  The Debug artifact contains the manual command. A clean build using the
  dedicated disabled-CALRUN `Vehicle` preset also passed without warnings:
  FLASH is 52,208 bytes and DTCMRAM is 12,232 bytes. ELF string inspection
  confirmed the Debug image contains the active `CALRUN START` path while
  the Vehicle image does not and instead contains only the disabled reply.
  Build `2026073004` was subsequently flashed and exposed the cold-start
  preflight issue recorded in the newer section above.

## 2026-07-30 mechanical update log

- The user froze the external `PITCH_NEW_RPLUS7_R3_CONTAINED.STL` as the
  final printed mechanism,
  superseding the 25 mm recommendation. An exact backup is stored at
  `mechanical/final/PITCH_FINAL_R15P45_R3_20260730.STL`; both files are
  768,984 bytes and have matching SHA-256
  `010919C1A376811A5C7BEDD237305554FC9CF96A3D4154B04B5BBF57FD02E3F5`.
- All future mechanism code must therefore use the measured crank radius
  `r=15.45 mm`, not the earlier proposed 25/50 mm values. The fixed STL's
  remaining verified geometry and pending installed calibration inputs are
  recorded in `mechanical/final/README.md`. Firmware is not changed yet:
  final `L`, level-pipe motor feedback angle, installed sign, and safe motor
  endpoints still require measurement after assembly.
- The user correctly questioned the need for a 50 mm crank radius. Reanalysis
  distinguishes resolution from travel: 50 mm was a conservative choice made
  solely to guarantee roughly +/-10 degrees pipe travel at `L=220-230 mm`;
  it is not required by DM4310 torque, and that travel target is larger than
  necessary if chassis acceleration/deceleration is deliberately smoothed.
- The revised preferred balance is `r=25 mm` at `L=220-230 mm`, with
  approximately +/-60 degrees motor travel. This gives about +/-5.65 degrees
  pipe travel at 220 mm and +/-5.40 degrees at 230 mm, with theoretical
  maxima about +/-6.52 and +/-6.24 degrees. From the latest model's measured
  `r=15.45 mm`, move the follower center only another 9.55 mm outward.
  `r=30 mm` remains a fallback if vehicle testing proves that more recovery
  authority is needed.
- To make the smaller crank viable during chassis motion, plan to reduce the
  TI chassis deceleration slew from its present 250 RPM/s to approximately
  100-120 RPM/s after the power-path issue is cleared and integrated testing
  begins. This is a proposed coordination parameter, not an implemented or
  hardware-verified change. Final pipe limits and slew must be tuned from
  installed ball-free and ball tests.
- Direct inspection of user-supplied
  `PITCH_NEW_RPLUS7_R3_CONTAINED.STL` found a valid binary STL with 15,378
  triangles, one connected watertight component, and about 17,263 mm^3
  volume. Its mounting base is a diameter-49 mm circular plate, the four
  diameter-3.2 mm holes remain on the diameter-27 mm pitch circle, and the
  integral follower is diameter 12 mm with the R3 containment/transition.
- The motor rotation center derived from the four mounting holes is
  `(17.5,17.5) mm`; the follower center is approximately
  `(2.05,17.39) mm`. Actual follower-axis eccentricity is therefore only
  15.45 mm, not 50 mm. At `L=180/220/230 mm`, its absolute geometric pipe
  tilt maxima are only about 4.92/4.03/3.85 degrees.
- This model has acceptable mesh construction but insufficient control
  stroke for the moving-chassis baseline. From its current position, move
  the follower center another 9.55 mm outward along the same radial
  direction so the motor-center-to-follower-center distance is 25 mm.
  The offset requirement is center-to-center, not plate diameter or follower
  outer-edge distance. Do not change firmware mapping or limits against this
  intermediate geometry.

## 2026-07-29 protocol and wiring update log

- Competition mechanical rule changed: a midpoint seesaw support is now
  prohibited. The prior midpoint DM4310 mounting baseline is superseded.
  The pipe will use a fixed pillar/hinge at one end and DM4310 actuation at
  the other end.
- Preferred new mechanism is an eccentric crank with a transverse roller or
  captive short link at the motor end. A pipe merely resting on a smooth bar
  is not the final recommendation. The existing collinear support plate is
  also not recommended as the final mechanism because rigid attachment would
  overconstrain the two separated axes, while loose contact would introduce
  slip, friction, and a changing contact point.
- With the new linkage, firmware position commands describe DM4310 output
  angle rather than pipe angle. After hinge-to-drive distance `L`, crank
  radius `r`, and required pipe travel are fixed, calibrate the mapping; near
  the centered small-angle operating point, use
  `pipe_angle ~= (r/L) * motor_angle` only as an initial estimate.
- Existing STEP/SWEEP bench verification remains valid for the motor, but its
  measured angles must not be interpreted as pipe angles after installation.
- User-supplied preliminary linkage dimensions: hinge-to-drive distance `L`
  will be 180-230 mm, pipe outside diameter is 20 mm, and no task requires a
  pipe angle above 20 degrees. The first mechanical baseline is to maximize
  `L` toward 220-230 mm, provide adjustable crank radii of 20/25/30/35 mm,
  and commission with `r=25 mm`.
- The initial control target is approximately +/-5 degrees of actual pipe
  tilt, not +/-20 degrees. With `r=25 mm`, this requires about +/-39 degrees
  motor travel at `L=180 mm`, +/-50 degrees at `L=220 mm`, and +/-53 degrees
  at `L=230 mm`. The existing firmware +/-10-degree motor clamp will therefore
  need a later, deliberate revision after the final mechanism and hard stops
  are verified.
- Moving-chassis review revised the final mechanical range recommendation.
  The TI chassis currently slews acceleration at 80 RPM/s and deceleration at
  250 RPM/s with 65 mm wheels, corresponding to approximately 0.27 m/s^2 and
  0.85 m/s^2, or about 1.6 and 5.0 degrees of equivalent longitudinal tilt.
  Therefore a mechanism limited to +/-5 degrees has no braking correction
  margin. Prefer approximately +/-10 degrees usable pipe travel, about
  +/-12 degrees mechanical clearance, `r=45 mm` at `L=220-230 mm`, and
  adjustable crank holes at 25/35/45/50 mm. Normal control should still use
  only the angle needed, with larger angles reserved for acceleration,
  braking, and recovery.
- The user supplied a new CAD render of the DM4310 output attachment: a
  circular output plate carrying a long cylindrical transverse support/follower.
  It is intended to act as the eccentric crank contact for the pipe. Exact
  crank offset, installed axis heights, final `L`, and contact retention are
  not yet confirmed. The rod center must be offset from the DM4310 rotation
  axis; a centered rod would spin without producing pipe lift.
- This attachment does not change DM4310 CAN handling, guarded power-up,
  observation, HOLD, or desktop STEP/SWEEP semantics. Production control does
  require a mechanism layer that converts desired pipe angle to motor output
  angle, applies a calibrated zero/phase and direction, and enforces separate
  pipe-angle and motor-angle limits. Do not enlarge the present bench clamp
  until the installed mechanism, hard stops, and zero point are measured.
- Existing Jetson and TI V2 packet formats remain suitable: Jetson continues
  to supply ball position/velocity and TI supplies chassis state plus wheel
  speeds. MC02 owns the ball controller, optional chassis-acceleration
  feed-forward, and the new pipe-angle-to-motor-angle conversion.
- The supplied `零件2.STL` was inspected directly as a binary STL (956
  triangles). Assuming millimetres, it is one closed connected mesh with
  overall bounds about 35 x 75 x 35 mm: an approximately 35 mm diameter,
  5 mm thick plate and an integral approximately 7 mm diameter, 70 mm exposed
  rod. The measured rod-axis offset from the plate center is 13.995 mm.
- With this STL's `r=14 mm` and the stated `L=180-230 mm`, the largest
  geometrically possible pipe tilt is only about 4.46 degrees at 180 mm and
  3.49 degrees at 230 mm, even at the crank's maximum vertical excursion.
  This cannot provide the previously selected moving-chassis correction
  reserve. Do not implement the final mapping against this geometry yet.
  Prefer a redesigned radial crank arm near `r=45 mm`, subject to physical
  sweep clearance and a stronger metal follower shaft.
- The current STL uses a long integral 7 mm printed cantilever. Its material,
  print orientation, pipe contact distance from the plate, dynamic strength,
  and retention method are unknown; a steel pin/shaft with a roller or
  captive connection is preferred for the final vehicle mechanism.
- User confirmed that the current STL is already printing and will not be
  changed, despite sufficient surrounding sweep space. The 20 mm OD pipe will
  simply rest on the integral rod, with the contact point approximately
  12 mm from the plate face (minor later adjustment possible). Motor and
  opposite hinge axes can be made nearly parallel and are currently planned
  at approximately the same height.
- Because pipe radius 10 mm plus follower radius 3.5 mm is approximately
  13.5 mm, nearly equal to the 14 mm crank offset, equal motor/hinge axis
  heights may place the horizontal pipe condition near the crank's lower
  dead point if the hinge axis passes through the pipe centerline. This would
  be strongly nonlinear and provide almost no downward travel. Before coding
  the mapping, confirm the hinge-axis-to-pipe-center vertical offset and set
  the motor mounting height so that the crank is approximately horizontal at
  pipe-level neutral. If the hinge axis passes through the pipe centerline,
  the motor axis should be about 13.5 mm below the hinge axis.
- If the 14 mm crank is retained, prefer final `L` near 180 mm, use about
  +/-4 degrees as the practical pipe limit, and reduce chassis deceleration
  below the present 250 RPM/s to preserve balancing authority. Exact limits
  remain pending neutral-geometry confirmation and installed sign testing.
- The user subsequently confirmed that the part can be redesigned and
  reprinted; the 14 mm-offset STL is no longer the final mechanical baseline.
  Direct bottom-face loop extraction found four approximately 3.2 mm through
  holes on an approximately 27 mm pitch circle. Relative to plate center,
  their centers are approximately `(x,z)=(+/-6.75,+/-11.69) mm`; preserve
  this already-matched DM4310 mounting interface.
- Revised printable crank baseline: retain the diameter-35 mounting hub,
  extend a filleted/ribbed arm with 35/45/50 mm follower-axis holes, and
  commission at `r=50 mm` with `L` near 220-230 mm. At `r=50 mm`,
  approximately +/-55 degrees motor motion gives about +/-10.3 to +/-10.7
  degrees pipe motion over that `L` range, while the geometric maximum is
  about +/-12.6 to +/-13.1 degrees.
- Do not print the long follower as an integral 7 mm cantilever for the final
  vehicle part. Prefer an 8 mm steel shoulder bolt/shaft, ideally with a free
  roller or 8x16 mm bearing outer race, a short projection, and an upper
  keeper with 1-2 mm clearance. Use an arm thickness of at least 8-10 mm,
  generous root/pin-boss fillets, and a pin boss around 18-20 mm diameter.
  Final motor-axis height depends on the follower's outside radius: with a
  pipe-center hinge and a horizontal crank at neutral, lower the motor axis
  by `pipe_radius + follower_outer_radius` relative to the hinge.
- A second supplied model, `零件2(1).STL`, was inspected directly. It retains
  the same approximately diameter-35 x 5 mm hub and 70 mm integral rod, but
  increases the rod to approximately 12 mm diameter. Because the rod remains
  tangent to the small hub, its center offset falls to 11.495 mm, making
  available pipe travel worse than the first model. It is not the final
  recommended part.
- At the user's authorization, a replacement mechanical artifact was created
  under `mechanical/`: `DM4310_CRANK_R50_M8_V1.stl`, its reproducible
  generator, a dimension/assembly README, and a top-view preview. The part is
  a 10 mm-thick reinforced convex crank, preserves the measured diameter-35
  hub and four diameter-3.3 holes on the 27 mm pitch circle, and provides one
  diameter-8.2 follower hole at exactly 50 mm eccentricity for an M8 steel
  shoulder bolt/shaft. Bounds are 78 x 35 x 10 mm.
- Independent mesh validation found one connected watertight solid volume,
  5,656 faces, 2,820 vertices, no degenerate faces, and approximately
  19,809 mm^3 volume. This is geometry validation only: DM4310 physical fit,
  print strength, screw length, steel follower/688 bearing assembly,
  collision clearance, neutral height, and loaded behavior remain unverified.
  No firmware or angle clamp was changed.
- The user rejected `DM4310_CRANK_R50_M8_V1.stl` as an acceptable physical
  model and will modify the original round-hub/integral-cylinder CAD instead.
  Keep the generated file only as a superseded reference; do not print it or
  treat it as the selected mechanism. The dimensional design target remains
  a 50 mm rod-axis eccentricity, a reinforced radial extension from the
  original hub, and approximately +/-10 degrees usable pipe travel.
- Added the initial Jetson/MC02 protocol, including coordinate units,
  validity flags, timing, CRC test vector, status, and event packets.
- Added the initial MC02/TI UART protocol, idempotent commands, question
  mapping, acknowledgements, and the 500 ms independent safe-stop rule.
- Added the manual-checked MC02 wiring/commissioning checklist with exact SWD,
  USART1, CAN1, and OUT1 connector pinouts.
- Corrected PC13/PC14 generated startup levels to low/off in source and `.ioc`;
  no output-enable path was added.
- Incremented MC02 build ID to `2026072902`; clean GCC build passed with no
  warnings in captured output. This is compile evidence only, not hardware
  validation.
- Rechecked the source PDF and corrected the field-test model from six
  questions to five: internal Q1-Q5 map to printed requirements 2-6.
  Printed requirement 1 is a common video/record/playback requirement.
- User-confirmed Button X8 mapping: K1-K5 select internal Q1-Q5, K6 is
  reserved, K7 performs normal stop/reset back to selection standby, and K8
  is emergency stop.
- Promoted both communication documents to V2 after the incompatible
  five-question/button revision. Filenames now end in `_V2.md`, the on-wire
  version byte is `0x02`, and both canonical CRC vectors were regenerated.
- TI build `2026072912` now implements the TI side of the frozen V2 UART
  endpoint; MC02 build `2026072903` still does not. The shared TI protocol
  additionally freezes fault codes 0-6 as NONE, SUPERVISOR_LOST,
  LINE_SENSOR, ENCODER, OUTPUT, BAD_PARAMETER and ROUTE_TIMEOUT. Q1 is the
  only TI route currently allowed to become READY; Q2-Q5 are parsed and
  acknowledged but remain IDLE_DISARMED. This is TI compile/static evidence,
  not an MC02/TI bench test.
- First MC02 logic-only hardware check passed with motor power, motor CAN,
  OUT1/OUT2 loads, Jetson and TI disconnected. Ozone downloaded and ran build
  `2026072902`; Watch showed uptime increasing, state 1/SAFE_IDLE, fault 0,
  startup `can_tx_count=5`, `can_tx_error_count=0`, and no CAN/vision receive
  data. `feedback_age_ms` and `vision_age_ms` correctly showed the
  `0xFFFFFFFF` never-received sentinel. This does not validate power outputs,
  CAN wiring or motor motion.
- The user subsequently reported the powered OUT1/OUT2 check had no problem,
  so commissioning may proceed to power-off CAN wiring/termination checks.
  Exact input, OUT1 and OUT2 voltages were not supplied and remain unrecorded;
  do not substitute assumed numeric values.
- The motor is physically connected through the keyed MC02 OUT1/CAN1 2+2
  connector, so connector orientation is not considered ambiguous. Build
  `2026072902` still holds OUT1/PC14 off and therefore cannot power the motor
  through that connector; verify the motor LED and OUT1 voltage before trying
  CAN OBSERVE. No separate motor-power branch has been confirmed.
- With the keyed 2+2 cable installed, the user reported the step-5 powered
  check was normal: there was no unexpected OUT1/motor energization. Exact
  voltage and LED observations were not supplied. The next step remains
  blocked on choosing a separate fused motor-power breakout versus adding a
  reviewed guarded OUT1-enable path.
- The user selected the integrated OUT1 option. Build `2026072903` therefore
  adds a USB-only guarded bench-power path for OUT1/PC14: `PWRARM 4310` opens
  a 3-second authorization window, `PWRON` enables OUT1 for at most 30
  seconds, and `PWROFF`, `STOP`, any fault, motion timeout or reset forces
  OUT1 low/off. OUT2/PC13 has no enable path. After PWRON, firmware waits
  500 ms and completes five disable transmissions before accepting OBSERVE
  or motion. Direct Ozone/GPIO power-on is intentionally unsupported.
- The user flashed build `2026072903` with the DM4310 2+2 cable unplugged and
  reported the default-off check was normal. Exact Watch values and OUT1
  voltage were not supplied, so only the qualitative boot-off result is
  recorded. Guarded high/on polarity and voltage remain unverified.
- Initial USB commands lacked a transmitted line terminator, so PWRARM
  produced no response and remained inactive (`power_output_enabled=0`,
  arm remaining 0, TX drops 0). After enabling CRLF and resetting, PING and
  `PWRARM 4310` both returned the expected text and the 3-second Watch
  countdown behaved correctly. No PWRON had been issued at this checkpoint.
- With the DM4310 2+2 cable still unplugged, the user then completed the
  unloaded `PWRARM 4310` -> `PWRON` -> `PWROFF` test and reported that all
  results were normal. This qualitatively verifies guarded active-high OUT1
  turn-on and manual turn-off in build `2026072903`; exact main-input,
  PWRON and PWROFF voltages were not supplied. STOP-triggered cutoff and the
  30-second automatic cutoff remain unverified.
- Still unloaded, the user completed `PWRARM 4310` -> `PWRON` -> `STOP` and
  reported that the serial reply, Watch fields and OUT1 cutoff all matched
  the expected results. This verifies the build `2026072903` STOP-triggered
  OUT1 shutdown path qualitatively. The 30-second automatic cutoff remains
  the final unloaded power-output test.
- The user then reported that the unloaded 30-second automatic cutoff also
  behaved correctly. During the recent voltage-check handling, however, an
  accidental electrical spark occurred. Its exact location and whether it
  left damage have not yet been reported. Treat this as a near miss: do not
  repeat handheld live-probe checks, and do not continue powered
  commissioning until battery power is removed and the connector, probes,
  wiring, fuse and board are visually checked for exposed conductor, loose
  strands, burn marks, deformation, heat or odor. Future essential voltage
  checks must use insulated clip leads attached and secured while power is
  off; power must be removed again before moving or removing the clips.
  Routine follow-up tests should use Watch/serial/CAN feedback instead of
  repeated voltage measurements.
- With all power removed, the user completed the requested post-spark visual
  inspection and reported no problem. No burn mark, deformation, loose
  conductor, abnormal heat/odor or other visible damage was reported. This
  clears progression to the powered CAN-observe stage, but it does not
  identify the original spark location. Continue without handheld live
  probing; first loaded operation must remain motor-disabled OBSERVE only.
- The user then connected the keyed DM4310 2+2 cable to MC02 OUT1/CAN1 while
  powered off. Powered CAN feedback remains unverified; the next operation is
  guarded PWRON followed by OBSERVE/STATUS only, with no ARM or motion
  command.
- A source audit was performed before the first loaded PWRON at the user's
  request. Power-on and motor torque enable are not simultaneous:
  `power_output_enable()` schedules the first CAN action after
  `GIMBAL_POWER_STARTUP_SETTLE_MS=500 ms`; firmware then sends five disable
  frames at 10 ms spacing. `power_output_is_ready()` remains false until all
  five complete, and `start_motion()` rejects enable unless power is ready,
  the explicit 3-second motion arm is valid, and motor feedback is fresh.
  Thus the earliest possible enable is later than approximately 540 ms and
  also requires the separate OBSERVE -> ARM -> motion-command sequence.
  PWRON itself never sends an enable frame. No firmware change was needed.
- The first powered OBSERVE attempt used build `2026072903`. PWRON occurred
  at 17:24:35, OBSERVE was accepted at 17:24:42 with the explicit
  torque-disabled reply, and the only captured STATUS was not obtained until
  17:25:43, after the 30-second OUT1 timeout. Therefore its SAFE_IDLE/PWR=0
  result is expected. However, retained counters showed `TX=1208`,
  `TXERR=0`, `RX=0`, `ONLINE=0` and never-received age, proving that many
  query frames were queued during the powered OBSERVE interval but no motor
  feedback frame was received. Do not ARM. Repeat once with separate STATUS
  commands inside the 30-second power window; if RX remains zero, diagnose
  motor power/LED, CAN path/termination and query/feedback IDs while powered
  motion remains prohibited.
- The short-window repeat confirmed the motor LED is steady red, which the
  DM-J4310 V1.2 manual defines as the normal disabled state. At 17:27:37,
  while PWR=1 with 17.9 seconds remaining, STATUS was
  `STATE=OBSERVE FAULT=0 ONLINE=0 ERR=0 RX=0 TX=1546 TXERR=0` with the
  never-received age. Thus motor power and the intended disabled state are
  present, but no CAN feedback is reaching the parser. The manual was
  rechecked: any standard data frame whose low eight ID bits equal motor
  CAN ID `0x01` should cause a feedback frame at configured Master ID
  `0xF1`, so query ID `0x001` is valid. Current FDCAN kernel clock/timing is
  120 MHz / (prescaler 24 * 5 time quanta) = 1 Mbit/s, but its 60% sample
  point is less robust than the dedicated DAMIAO MC02 CAN example's 75%
  sample point. Before changing firmware, inspect live FDCAN1 PSR/ECR in
  Ozone during OBSERVE to distinguish no-ACK/bus faults from an RX/filter
  issue. Motion remains prohibited.
- The user reports that Ozone's apparent program/download control is disabled
  without an error, while USB `PING` still returns
  `PONG BUILD=2026072903 DESKTOP=1`. No firmware has changed since that
  build, so reprogramming is not required for the current PSR/ECR diagnostic.
  The installed Ozone manual confirms that with the default
  `Download & Reset Program` connection mode, starting a new debug session
  performs connect, reset and download. The `.jdebug` file does not override
  this default. The likely explanation for a disabled start/download control
  is an already-active debug session, but this has not yet been confirmed by
  an Ozone screenshot. If a future reflash is needed, first STOP and remove
  motor power, stop the existing debug session, then start a new session in
  `Download & Reset Program` mode.
- Follow-up diagnosis established that the user currently has no Ozone target
  connection, so live Watch data is unavailable and clicking `Download &
  Reset Program` produces no visible action. Windows enumerates the SEGGER
  J-Link driver, its CDC UART on COM3, and probe serial `000069401064`
  normally. Three standalone `JLink.exe` instances left by command-line
  diagnostics were found and terminated; the Ozone application was not
  terminated. A direct 1 MHz SWD handshake had stalled while Ozone was open,
  so probe ownership and the target SWD link have not yet been separated.
  Next safe action: disconnect 6S/motor power, fully exit Ozone, then run a
  standalone J-Link Commander connection test. Only after that test exits
  should Ozone be reopened.
- After the user closed the Ozone window, four windowless Ozone processes
  remained and were explicitly terminated. With no Ozone or other SEGGER
  process left, standalone J-Link Commander still stalled at `Connecting to
  J-Link via USB...`; it never reached the SWD target handshake. Windows PnP
  continued to report `J-Link driver` as present and OK. This narrows the
  immediate fault to a stale PC/probe USB session rather than firmware,
  target device selection or SWD signalling. Re-enumerate the probe by
  unplugging only its PC USB cable for five seconds and reconnecting it, then
  repeat the standalone connection test before reopening Ozone.
- After the probe's USB cable was replugged, the standalone 1 MHz SWD test
  passed completely. J-Link serial `69401064` opened successfully, measured
  `VTref=3.296 V`, found SW-DP ID `0x6BA02477`, and identified the target as
  Cortex-M7 r1p2 for `STM32H723VG`. The test disconnected normally without
  programming flash. This verifies PC/probe USB, target reference voltage,
  SWD wiring and target selection. Ozone may now be reopened as one fresh
  instance; start a new `Download & Reset Program` session while 6S/motor
  power remains disconnected.
- A subsequent powered build `2026072903` OBSERVE captured two relevant FDCAN
  snapshots. The first was PSR=1903 (`0x76F`) and ECR=168 (`TEC=168`,
  `REC=0`), showing an error-passive transmit period. The later snapshot was
  PSR=1800 (`0x708`) and ECR=0, showing that another CAN node subsequently
  acknowledged transmissions and the controller recovered. STATUS then
  showed `RX=3`, `TX=1534`, `ONLINE=0`, `ERR=0xD` and feedback age 91,374 ms:
  only three stale motor frames had been accepted, and the motor reported
  communication loss. This is consistent with the five startup disable
  frames on control ID `0x101` eliciting limited feedback while the repeated
  zero-filled `0x001` OBSERVE frames did not elicit continuous feedback.
- During that test the user observed the motor LED turn green briefly and
  then go out. DM-J4310 V1.2 manual page 10 was rendered and visually
  checked: steady green means enabled/normal, steady red means
  disabled/normal, and flashing red means a fault; code D means
  communication loss. Treat the green event as a possible transient enable.
  Its exact timing relative to PWRON/OBSERVE and whether the shaft moved have
  not yet been reported. Do not ARM or repeat powered build `2026072903`
  testing.
- Build `2026072904` is prepared but not yet flashed or hardware-tested.
  Torque-disabled OBSERVE now repeats the selected position-speed mode's
  explicit disable frame (`0x101`, payload ending `0xFD`) every 20 ms instead
  of sending zero-filled `0x001` frames. In addition, any fresh enabled-state
  or fault feedback during OBSERVE immediately enters MC02 FAULT and removes
  OUT1 power. A clean GCC build completed without warnings in the captured
  output; FLASH is 74,156 bytes and DTCMRAM is 12,200 bytes. This is compile
  evidence only. Before flashing, remove 6S/motor power. Before the next
  powered OBSERVE, confirm motor CAN Timeout is actually written and read
  back as 0.
- The user subsequently flashed build `2026072904` with 6S/motor power
  disconnected and confirmed the four required logic-only startup values:
  `build_id=2026072904`, `state=SAFE_IDLE`, `power_output_enabled=0`, and
  `fault=0`. This verifies only the new image and default-off logic startup;
  the revised disable-frame OBSERVE and its unexpected-enable cutoff remain
  untested. Before powering the motor, use the DAMIAO assistant to write
  CAN Timeout=0 and read it back as 0.
- At the user's request, build `2026072905` extends the guarded OUT1
  automatic cutoff from 30 seconds to 90 seconds so there is enough time to
  open the DAMIAO assistant and write/read back motor parameters. The
  `PWRARM 4310` three-second authorization, default-off behavior and
  immediate `PWROFF`, `STOP`, fault, motion-timeout and reset cutoffs are
  unchanged. HELP and PWRON replies and the commissioning documents now
  report 90 seconds. A clean GCC build completed without warnings in the
  captured output; FLASH remains 74,156 bytes and DTCMRAM 12,200 bytes.
  Build `2026072905` has not yet been flashed or hardware-tested.
- The user subsequently completed the DAMIAO assistant parameter operation:
  motor CAN Timeout was changed from 2000 to 0 and read back as 0. This
  verifies the stored commissioning value and removes the configured motor
  communication watchdog from the next feedback-only test. The revised
  build `2026072905` OBSERVE path still requires a powered bench test; do not
  ARM until ONLINE=1, RX rises continuously, ERR=0 and feedback remains fresh.
- Powered build `2026072905` OBSERVE then passed. Two STATUS captures 3.399 s
  apart showed `ONLINE=1`, `ERR=0`, feedback age 8/7 ms, and RX increasing
  from 1196 to 1366 while TX increased from 1213 to 1383. The exact increase
  of 170 frames is approximately 50 Hz and matches the 20 ms explicit-disable
  OBSERVE period. Position was stable at 0.969 rad and reported velocity was
  -0.007 rad/s. This verifies continuous CAN feedback and torque-disabled
  OBSERVE; it does not verify torque enable or commanded motion.
- The motor again flashed green once immediately at PWRON, then settled to
  steady red. This places the transient before/around the 500 ms motor boot
  wait and first disable burst. The manual does not define a momentary startup
  green flash; subsequent steady red plus error/state 0 feedback proves the
  post-burst disabled state. Whether the shaft physically twitched during the
  flash is still unreported. Keep the mechanism secured and the ball removed
  for the first deliberate HOLD test.
- At the user's request, build `2026072906` extends the deliberate HOLD
  auto-stop from 5 seconds to 15 seconds so there is enough time to observe
  and capture STATUS. The 80 ms feedback watchdog, motor-fault reaction,
  3-second ARM window, bounded command, immediate STOP and power-cutoff paths
  are unchanged. The serial acknowledgement and commissioning documents now
  report 15 seconds. A clean GCC build passed without warnings in the
  captured output; FLASH remains 74,156 bytes and DTCMRAM 12,200 bytes.
  Build `2026072906` has not yet been flashed or hardware-tested.
- Build `2026072906` was then flashed and its first deliberate HOLD passed.
  PING reported the correct build, PWRON reported 90,000 ms, OBSERVE was
  established, and ARM was followed by HOLD within the 3-second window.
  Three HOLD STATUS captures showed state HOLD, fault 0, ONLINE=1,
  enabled/state `ERR=0x1`, position 0.968-0.969 rad, command 0.969 rad,
  feedback age 0-9 ms and continuously increasing RX/TX with zero HAL queue
  errors. The user manually sent STOP after about 13.55 seconds; firmware
  replied `OK STOPPED/DISABLED; OUT1 OFF`. This verifies stable current-
  position torque hold and manual cutoff. Physical noise, vibration and
  visible displacement were not explicitly reported. The next bounded test
  is STEP +0.5 degrees with the ball removed, followed by direction capture.
- The user then attempted a +1 degree step and observed no visible rotation,
  while noting that one degree may simply be too small to see. No STATUS
  capture or measured angle from that step was supplied, so do not record it
  as a control failure. At the user's request, build `2026072907` raises the
  desktop relative-angle clamp from +/-3 degrees to +/-10 degrees for a
  visible direction test. The maximum speed remains 0.20 rad/s, so a
  10-degree move takes about 0.87 seconds; the user-confirmed mechanical
  range is +/-30 degrees. Feedback watchdogs, fault reactions, STOP and all
  power cutoffs are unchanged. A clean GCC build passed without warnings in
  the captured output; FLASH remains 74,156 bytes and DTCMRAM 12,200 bytes.
  Build `2026072907` has not yet been flashed or hardware-tested.
- Build `2026072907` was flashed and the +10 degree STEP produced a clearly
  visible 10-degree rotation. In the photographed bench orientation, the
  motor body stands vertically on the table and the gimbal PITCH-axis plate
  is mounted above it; viewed from above, a positive STEP command rotated the
  plate counterclockwise. This verifies bounded nonzero motion and freezes
  the positive motor sign for this bench orientation. It does not yet freeze
  the water-pipe sign after the motor is mounted horizontally on the vehicle.
  A single bidirectional SWEEP 10 test can replace the separate -10 degree
  step and repeated small-angle tests before mounting the pipe.
- The user then completed `SWEEP 10` and reported normal bidirectional
  operation. This verifies the +/-10 degree bounded desktop motion path,
  direction reversal, continuous feedback and automatic sweep execution.
  The bare-motor bench commissioning phase is complete. Next, with all power
  removed, mount the motor with its output axis horizontal and mount the pipe
  so its longitudinal centerline is perpendicular to and centered on that
  axis. The final installed sign still requires one ball-free +10 degree
  check to record which pipe end rises.

## Scope and ownership

This directory is the DAMIAO gimbal-controller firmware. Hardware target:

- controller: DAMIAO DM-MC-Board02, STM32H723VGT6;
- motor: DAMIAO DM-J4310-2EC V1.2, 24 V, 10:1;
- intended supply: one DJI 6S battery;
- debugger: J-Link over SWD;
- development/debug tools: VSCode, CMake/Ninja, GCC, SEGGER Ozone.

The TI LaunchPad chassis firmware is in sibling directory `mspm0dache`;
build `2026073006` is the matching V2/OLED endpoint. Jetson code is owned by
another agent.

## Current firmware status

Build ID `2026073009` is the current compile-tested installed-mechanism and
TI-link build. It retains the hardware-tested `2026073008` gimbal behavior,
USB `ZERO`, zero-referenced `JOG deg`,
and the manual-only one-command `CALRUN 4310 deg` workflow described above.
Only CALRUN accepts up to +/-45 motor degrees with five-second legs; normal
JOG/STEP/SWEEP remain clamped to the verified +/-10 degrees. The 0.20 rad/s
speed, 80 ms feedback watchdog and every STOP/fault/reset/power cutoff are
unchanged. Vehicle compiles CALRUN inactive. Debug build `2026073008` has
passed commanded-angle telemetry at +/-30 and +/-45 degrees with endpoint
errors around 0-1 mrad; physical contact behavior during the real +/-45 test
still needs explicit user confirmation. Builds `2026073006/7` printed
expanded targets but actually executed only +/-10 degrees; do not count
them as range tests. The earlier build `2026072907` passed powered +10
degree STEP and +/-10 degree SWEEP.
Positive command is counterclockwise when viewed from above in the
photographed vertical bench orientation. Bare-motor bench commissioning is
complete.
Build `2026072906` passed
powered OBSERVE and a stable 13.55-second HOLD with manual STOP. Its +1
degree step produced no visible rotation but had no telemetry capture.
Bounded nonzero motion remains unverified. Build `2026072905` passed
logic-only startup and powered torque-disabled OBSERVE with continuous 50 Hz
feedback. Torque enable and motion remain untested. Build `2026072904` passed
the logic-only build-ID, SAFE_IDLE, fault-zero and OUT1-off startup check,
but not a powered CAN test. Build `2026072902` passed
J-Link/Ozone logic-only SAFE_IDLE startup. Build `2026072903` has been
flashed; boot-off, USB power arming, unloaded PWRON and
PWROFF, and STOP-triggered cutoff have passed qualitatively, although exact
voltage readings were not recorded. The unloaded 30-second cutoff also
passed qualitatively. A spark occurred during voltage-check handling; the
subsequent unpowered visual inspection was reported normal. Loaded CAN
feedback and motor motion are not yet verified.

Implemented:

- bare-metal main loop; FreeRTOS is not used;
- FDCAN1 classic CAN at 1 Mbit/s using official MC02 PD0/PD1 mapping;
- DM-J4310 position-speed control, enable, disable, query, and feedback decode;
- motor CAN ID `0x01`, Master/feedback ID `0xF1`;
- exact `0xF1` receive filter and rejection of nonmatching frames;
- USB CDC ASCII commissioning console;
- one compact Ozone Watch structure: `g_gimbal_debug`;
- an installed-mechanism level-zero capture and zero-referenced JOG path;
- a compile-time-removable manual `CALRUN 4310 deg` convenience path;
- desktop safe states: SAFE_IDLE, OBSERVE, HOLD, STEP, SWEEP, FAULT;
- explicit short-lived arm, local-neutral capture, angle/speed/time clamps,
  CAN feedback watchdog, motor fault reaction, disable burst, and STOP;
- guarded, USB-only, 180-second OUT1 calibration-power control with default-off and
  unconditional shutdown on stop/fault/timeout/reset;
- frozen V2 interface documents plus implemented MC02/TI Q1 parser,
  supervisor, heartbeat and fail-safe;
- VSCode CMake presets/tasks and Ozone J-Link project.

Not implemented:

- autonomous ball-position controller;
- Jetson USB binary parser/transmitter and vision closed loop;
- Q2-Q5 question controllers and TI K2-K8 physical inputs;
- hardware verification of the TI link and TI-hosted OLED/run timer;
- vehicle profile;
- production-duration power management and measured current/thermal limits.

Interface handoff documents:

```text
docs/JETSON_MC02_PROTOCOL_V3.md
docs/MC02_TI_PROTOCOL_V2.md
docs/MC02_WIRING_CHECKLIST.md
```

Both communication documents freeze the proposed packet formats for the
other owners to implement, but their existence does not mean build
`2026072905` accepts those binary frames.

## Protocol facts used by the code

The DM-J4310 V1.2 manual says:

- CAN is standard classic CAN, default 1 Mbit/s;
- position-speed command identifier is `0x100 + CAN_ID`, therefore `0x101`;
- D0..D3 are little-endian IEEE-754 `p_des` in rad;
- D4..D7 are little-endian IEEE-754 `v_des` in rad/s;
- in this mode `v_des` is the maximum absolute speed during the motion;
- enable is seven `0xFF` bytes followed by `0xFC`;
- disable is seven `0xFF` bytes followed by `0xFD`;
- enable/disable use the selected mode's control identifier;
- feedback identifier is Master ID, currently `0xF1`;
- feedback D0 contains motor ID low nibble and state/error high nibble;
- position uses 16-bit mapping across ±PMAX; speed and torque use 12-bit
  mappings across ±VMAX and ±TMAX;
- state 0 means disabled and state 1 means enabled; these are not faults.

The commissioning assistant values currently recorded are PMAX=12.5 rad,
VMAX=30 rad/s, TMAX=10 N-m, mode=position-speed, CAN ID=0x01, Master ID=0xF1,
undervoltage=19 V, overvoltage=32 V, overtemperature=100 C, overcurrent=0.30.
CAN Timeout is temporarily 0 because a previous 2000-count setting caused
confirmed communication-loss red flashing before the MC02 command loop existed.

## Desktop safety contract

- Boot never enables the motor.
- Startup sends five disable frames.
- OBSERVE uses explicit position-speed disable frames on ID `0x101` every
  20 ms and never enables. Enabled-state or fault feedback in OBSERVE
  immediately removes OUT1 power.
- Motion requires fresh feedback and an arm valid for only 3000 ms.
- The feedback position at arm time becomes the test-local neutral.
- Relative command is limited to ±10.0 degrees.
- Position-speed `v_des` is limited to 0.20 rad/s.
- HOLD/STEP/SWEEP stop after 15/3/6 seconds.
- Missing feedback for more than 80 ms during motion enters FAULT and disables.
- Motor states other than 0 or 1 during motion enter FAULT and disable.
- STOP always works without arm.
- Direct debugger and USB interfaces share the same safety state machine.
- Do not halt on a breakpoint while the motor is enabled.

The MC02 V1.1 schematic shows PC14 and PC13 driving the OUT1 and OUT2
high-side power switches through NPN stages. The original CubeMX output
initialized both pins high, which could energize those outputs. Build
`2026072903` keeps both GPIOs and the `.ioc` defaults low/off before normal
application startup. OUT2 remains permanently off. OUT1 can only be driven
high by the guarded USB sequence described above. This schematic
interpretation has now passed the boot-off, guarded-on, PWROFF, STOP and
30-second timeout checks qualitatively. Because an accidental spark occurred
during the checking process, inspect the unpowered hardware before connecting
the motor. Do not repeat these checks with handheld probes on a live output.

## Build and artifacts

Validated on 2026-07-29 with GCC 13.3.0:

```powershell
cmake --preset Debug
cmake --build --preset Debug --clean-first
```

Result: successful build with no compiler warnings in the captured output.
Linker usage for compile-tested Debug build `2026073008`:

```text
FLASH   78,068 bytes / 1 MiB (7.45%)
DTCMRAM 12,240 bytes / 128 KiB (9.34%)
```

Artifacts:

```text
build/Debug/gimbal_MC02.elf
build/Debug/gimbal_MC02.hex
build/Debug/gimbal_MC02.bin
build/Vehicle/gimbal_MC02.elf
build/Vehicle/gimbal_MC02.hex
build/Vehicle/gimbal_MC02.bin
```

The Debug ELF contains `g_gimbal_debug` at `0x20002154`, size `0x94`.
`cmake --preset Vehicle` followed by `cmake --build --preset Vehicle` is the
required on-vehicle build path and compiles CALRUN inactive. Ozone project
is `gimbal_MC02.jdebug`, device `STM32H723VG`, SWD at 4 MHz. Installed tools
were found at:

```text
C:\Program Files\SEGGER\Ozone
C:\Program Files\SEGGER\JLink_V914a
C:\msys64\mingw64\bin\arm-none-eabi-gcc.exe
```

The copied `MDK-ARM` project is not a supported build path. Do not use or
repair it; the user explicitly requires VSCode + Ozone.

## USB commissioning interface

Commands are newline terminated and case-insensitive:

```text
PING
HELP
STATUS
PWRARM 4310
PWRON
PWROFF
CALRUN 4310 45
OBSERVE
ARM
HOLD
STEP 0.5
STEP -0.5
SWEEP 1.0
ZERO
JOG 10
STOP
CLEAR
```

`CALRUN 4310 45` is the sole convenience command for the current ball-free
installed sign test. It automatically powers OUT1, waits for fresh disabled
feedback, counts down three seconds, captures the current level reference,
runs +45/0/-45/0, prints leg/final feedback, and turns OUT1 off. The pipe
must be physically level before the command, the ball must be removed, and
hands/tools must be clear before the countdown ends. `STOP` remains an
unconditional abort. This command is available only when
`GIMBAL_ENABLE_MANUAL_CALRUN=1`; use the CMake `Vehicle` preset before
installing a vehicle build.

`ZERO` is accepted only in OBSERVE with fresh feedback and captures the
current feedback angle as the installed level-pipe reference. `JOG deg`
requires that zero plus a fresh three-second ARM and commands an absolute
offset from the saved level reference, clamped to +/-10 degrees. Its
three-second timeout explicitly disables torque and returns to OBSERVE
without cycling motor power. STOP, fault and feedback loss still remove
OUT1 power. Any motor power cycle invalidates the physical meaning of a
retained RAM zero and therefore requires a new ZERO; MCU reset clears it.

Recommended first sequence is PING, OBSERVE, STATUS. Continue only if
the motor is already safely powered. For integrated OUT1, first verify the
guarded supply with the motor unplugged. Then reconnect while powered off,
send PWRARM 4310 and PWRON, wait at least 600 ms, and send OBSERVE/STATUS.
Continue only if ONLINE=1 and RX increases. Then use ARM followed by HOLD.
Only after HOLD is verified should STEP ±0.5 and SWEEP 1.0 be attempted.

For Ozone Watch, write `arm_key=0xA55A5AA5`; for STEP/SWEEP write
`request_param_tenths_deg`; write `request` last. Request values:
STOP=1, OBSERVE=2, HOLD=3, STEP=4, SWEEP=5, CLEAR=6.

## Unverified hardware facts and next steps

1. The manual defines the four-pin SWD connector as 1=3.3 V/VTref, 2=GND,
   3=SWCLK/PA14, 4=SWDIO/PA13; it has no NRST pin. Confirm connector
   orientation from pin-1 marking before attaching J-Link.
2. Confirm which physical MC02 CAN connector is FDCAN1 and verify CANH/CANL,
   common ground, and bus termination.
3. J-Link/Ozone logic-only startup is verified for build `2026072902`:
   SAFE_IDLE, fault=0, uptime running, TX=5 and TX errors=0.
4. Build `2026072903` has passed build ID, boot-off, guarded PWRON, PWROFF,
   STOP and 30-second timeout checks qualitatively. The unpowered visual
   inspection after the reported probe-handling spark was reported normal.
   Do not repeat routine live voltage measurements.
5. Power the secured motor through the verified OUT1 path, run OBSERVE only,
   and verify fresh `0xF1` feedback without torque enable.
6. Run HOLD, then bounded STEP/SWEEP. Record physical direction,
   feedback position/velocity/torque, state, current, and temperature.
7. Record motor feedback at mechanical horizontal and at both ±45-degree
   CALRUN endpoints. Do not increase the normal JOG/STEP/SWEEP ±10-degree
   clamp until the Debug-only ±45-degree ball-free test passes.
8. Keep motor CAN Timeout=0 for the first loop test. After continuous 10 ms
   commands and fault handling are verified, restore Timeout=2000 and perform
   a deliberate unplug-CAN fail-safe test.
9. Implement and host-test the frozen Jetson protocol in
   `docs/JETSON_MC02_PROTOCOL_V3.md` before enabling vision control.
10. Implement the UART link in `docs/MC02_TI_PROTOCOL_V2.md` on both ends,
    then verify heartbeat-loss safe stop before adding the five-test state
    machine.

## 2026-07-30 M3508 crank-link alternative assessment

- A second end-hinged pipe mechanism was proposed: a DJI RoboMaster M3508
  with C620 drives a crank/link near the free end of the pipe.
- This is an alternative only. The frozen mechanical baseline remains
  `mechanical/final/PITCH_FINAL_R15P45_R3_20260730.STL` with the DM-J4310-2EC
  and DM-MC-Board02; no firmware or geometry constants were changed.
- The proposed rigid/captive crank-link geometry is mechanically preferable
  to a pipe merely resting on an eccentric follower because it can remove
  contact loss and sliding. That benefit comes from the linkage, not from
  changing to the M3508.
- For the complete actuator/control chain, retain the DM-J4310-2EC unless a
  later mechanical constraint forces a change. Its position-speed control
  path is already commissioned on MC02. An M3508+C620 conversion would
  require a new CAN driver, multi-turn tracking, an MCU-side cascaded
  position/speed loop, reversal/backlash compensation, and new fail-safe
  commissioning.
- If the M3508 alternative is pursued, add a pipe-output angle sensor at the
  fixed hinge and preload the linkage; rotor-position feedback alone cannot
  observe gearbox/linkage lost motion at the pipe. The best hybrid is a
  rigid/captive crank-link driven by the existing DM-J4310-2EC.

## 2026-07-30 installed-mechanism measurement

- The user reports that the mechanism is assembled. The DM4310 output axis
  is `1 mm` higher than the hinge axis.
- The pipe midpoint is `115 mm` from the follower contact station and
  `133 mm` from the hinge axis. If these points lie on opposite sides of the
  midpoint, their sum gives `L=248 mm`, conflicting with the earlier direct
  report `L=236 mm`. The `12 mm` difference equals the nominal follower
  diameter, suggesting that one measurement may reference the follower edge
  rather than its axis. Do not enter `L` into the firmware until the direct
  hinge-axis-to-follower-axis dimension is remeasured.
- The reported pipe-top-to-hinge-axis distance is `1.2 mm`; its vertical
  sign still needs confirmation. For the upward-facing longitudinally
  halved 20 mm tube, the rim plane passes through the original full-circle
  center, so this can define the hinge-to-pipe-center offset once the sign
  is known.
- The user confirmed that the printed mechanism matches the frozen model
  dimensions and that the pipe matches the competition statement. Therefore
  retain nominal follower diameter `12 mm`, pipe outside diameter `20 mm`,
  and crank radius `15.45 mm`.
- The physical pipe endpoints are `+/-125 mm` from O and the scored control
  positions are `+/-50 mm`.
- The pipe is a longitudinally halved tube. Define its centerline for
  calibration as the center of the original full circular cross-section.
  The pending hinge offset is the signed vertical distance between this
  imaginary pipe centerline and the hinge-pin axis.

## 2026-07-30 installed commissioning build

- Build `2026073001` implements the first installed-mechanism calibration
  layer. `ZERO` records level-pipe motor feedback while torque-disabled in
  OBSERVE. `JOG +/-deg` moves relative to that fixed zero instead of
  recapturing the current position on every command.
- STATUS and Ozone Watch now expose `level_zero_valid`,
  `level_zero_position_rad`, and `motor_offset_from_level_rad`; serial STATUS
  reports the same information as `ZVALID`, `Z`, and `OFF`.
- No hinge distance or motor-to-pipe kinematic conversion is used yet because
  installed `L` is still disputed. The frozen radius/diameter constants are
  recorded as 15.45/12/20 mm for later mapping.
- A clean GCC/CMake/Ninja build completed without warnings in the captured
  output. FLASH is 74,980 bytes and DTCMRAM is 12,216 bytes. The ELF, HEX and
  BIN artifacts were regenerated. `g_gimbal_debug` remains at `0x20002154`
  and is now `0x90` bytes. This is compile evidence only; the build has not
  been flashed or tested on the assembled mechanism.
- The first attempted installed test did not flash this build and did not
  power the motor. USB `PING` still reported build `2026072907`. With only
  MC02 logic powered, PWRON/OBSERVE produced the expected `ONLINE=0`, `RX=0`,
  never-received age and no motor feedback while OUT1 logic state was high.
  This is not a CAN or firmware failure. Before testing ZERO/JOG, issue STOP,
  remove 6S power, flash `2026073001`, verify its PING, then connect the
  previously commissioned 6S/OUT1 motor-power path while unpowered.
- The user then reported that the corrected `2026073001` flash, 6S motor
  power and powered OBSERVE check were normal. Exact STATUS values were not
  supplied, so record this as a qualitative pass only. The next test is
  ball-free level `ZERO`, followed by one armed `JOG 10` while observing
  pipe-end direction, follower contact, collision and abnormal sound.
- The ball-free installed `ZERO` -> `ARM` -> `JOG 10` test passed
  qualitatively. Positive motor offset lowers the motor-side/free end of the
  pipe. The user estimated about 8 mm downward motion, reported ample
  follower overlap with no possibility of contact loss over the motor sweep,
  and observed no collision, jump or abnormal sound. No in-motion STATUS was
  captured. The pipe flexed slightly under load. Because an ideal 15.45 mm
  crank can change follower height by at most about 2.7 mm over a 10-degree
  motor increment, treat the estimated 8 mm as including measurement error,
  structural flex, or an unresolved angle/geometry scale; do not freeze the
  motor-to-pipe mapping from this observation alone.
- A later `ARM` containing a trailing tab was rejected as an unknown command;
  a plain `ARM` was accepted. This is a console whitespace-robustness issue,
  not a state-machine or motor failure. It can be fixed in the next build
  without blocking the present test.
- Two accepted zero-referenced `JOG -10` trials also made the motor-side pipe
  end descend; contact and all other behavior remained normal. This is not
  the expected inverse of `JOG +10`. The main candidates are an installed
  neutral near an eccentric dead point and load-driven relaxation after each
  three-second power cutoff. Do not infer a control sign yet. The shortest
  discriminator is a zero-referenced `JOG 0`: returning to the recorded
  level position supports correct firmware reference and a mechanical
  dead-point/relaxation problem; further descent would indicate a lost zero,
  feedback-scale, or command-reference problem.
- The subsequent accepted `JOG 0` did not return to the recorded horizontal
  pose. It continued in the prior physical direction, passed the lowest pipe
  position, rose slightly and stopped. This proves that retaining an MC02
  RAM zero across the OUT1 power cut is not a valid calibration method for
  this motor/setup; repeated motor reboot is the leading coordinate-frame
  confounder. Do not continue installed motion on build `2026073001`.
- Build `2026073002` fixes this calibration workflow. At JOG timeout it sends
  an explicit disable, enters torque-disabled OBSERVE and keeps OUT1 on, so
  +10/0/-10 share one uninterrupted motor power/position frame. It prints
  FROM/Z/TARGET at JOG start and P/OFF at completion. Console normalization
  now trims leading/trailing whitespace and accepts tabs, fixing `ARM<TAB>`.
  Any OUT1 cutoff also invalidates ZERO in firmware, so a stale calibration
  cannot be reused across a motor reboot.
  A clean GCC/CMake/Ninja build completed without warnings; FLASH is 75,428
  bytes, DTCMRAM is 12,216 bytes, and `g_gimbal_debug` is unchanged at
  `0x20002154`, size `0x90`. This is compile evidence only and the build has
  not yet been flashed or hardware-tested.
- Build `2026073002` was then flashed. ZERO captured `-2.988 rad`; an armed
  JOG +10 reported target `-2.813 rad` and completed at `-2.813 rad`, proving
  the motor feedback followed the commanded +0.175 rad exactly. The user
  observed no physical pipe/motor motion, which is consistent with the
  installed follower being near a vertical eccentric dead point, but the
  output-shaft observation remains qualitative.
- Immediately after JOG DONE, later ARM had no fresh feedback and OBSERVE
  reported OUT1 unavailable. Part of the delay also exceeded the then
  90-second PWRON window, but source review found a separate transition
  hazard: the first post-disable feedback can still report state 1, and the
  normal OBSERVE guard can interpret it as unexpected enable and cut power.
- Build `2026073003` adds a bounded 200 ms post-JOG disable-settling window
  in which only state 1 is tolerated while explicit disable frames continue;
  any other nonzero state still faults immediately, and state 1 persisting
  beyond the window faults. It also extends the guarded calibration power
  window to 180 seconds. STOP, fault, reset, PWROFF, feedback watchdog and
  all angle/speed/motion limits are unchanged. A clean GCC/CMake/Ninja build
  completed without warnings; FLASH is 75,548 bytes, DTCMRAM is 12,224
  bytes, and `g_gimbal_debug` remains at `0x20002154`, size `0x90`.
  This is compile evidence only; build `2026073003` has not been flashed or
  hardware-tested.
- The user then directly inspected the installed phase and corrected the
  mechanical interpretation: at the intended neutral the eccentric follower
  is at approximately the 3-o'clock position, not a vertical dead point.
  The previous lack of visible pipe motion was likely an operator/test
  observation issue. The earlier recommendation to lower the motor by about
  16 mm is explicitly withdrawn; do not change the installed motor height
  from that inference. Repeat one uninterrupted, marked
  ZERO -> JOG +10 -> JOG 0 -> JOG -10 sequence on build `2026073003` to
  freeze the actual installed sign and displacement.

After every hardware measurement or material firmware change, update this
file and the project-wide handoff in `../mspm0dache/AGENT.MD`.
