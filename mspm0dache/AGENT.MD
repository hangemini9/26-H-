# mspm0dache agent handoff

Last updated: 2026-08-01

## 2026-08-01 MC02 rollback to Q2 build 0107

- At the user's explicit request, MC02 discarded the untested build-0108 Q2
  schedule and restored build `2026080107`. TI remains build `2026080104` and
  does not need rebuilding or flashing. Q1 and accepted Q3-Q5 behavior remain
  unchanged; TI V2 and Jetson V3 formats and meanings are unchanged.

## 2026-08-01 MC02-only Q2 data-driven build 0108

- MC02 Vehicle build `2026080108` changes only its stationary Q2 energy
  schedule after analysis of eight active attempts. Positive breakaway is
  restored, braking begins from measured +30 mm during a clear positive
  motion, return drive is retained until the ball crosses centre, and final
  capture is delayed to within 10 mm of -50 mm.
- TI remains build `2026080104`; no TI source, route, OLED or safety behavior
  changed and TI does not need reflashing. Q1 and accepted Q3-Q5 behavior are
  unchanged. TI V2 and Jetson V3 formats and meanings are unchanged. MC02's
  full offline verifier passed; powered behavior remains to be tested.

## 2026-08-01 MC02-only Q2 trim build 0107

- MC02 Vehicle build `2026080107` makes only a small Q2 positive-energy and
  negative-capture adjustment. TI remains build `2026080104`; no TI source,
  route, OLED or safety behavior changed and TI does not need reflashing.
- Q1 and accepted Q3-Q5 behavior remain current. TI V2 and Jetson V3 packet
  versions, layouts and meanings are unchanged. MC02's full offline verifier
  passed; powered behavior remains to be tested.

## 2026-08-01 MC02-only selective Q2 rollback

- MC02 Vehicle now reports build `2026080101` after a selective rollback of
  only its stationary Q2 controller. The current TI source/image remains the
  user-restored TEST_MODE build `2026080104`; no TI source, route, OLED or
  safety behavior was changed and TI does not need reflashing.
- Q1 and the accepted Q3-Q5 behavior remain current. TI V2 and Jetson V3
  packet versions, layouts and meanings are unchanged. MC02's full offline
  verifier passed; powered Q2 behavior is not claimed until the new Vehicle
  ELF is flashed and tested.

## 2026-08-01 user-requested rollback to saved TI 0104

- The user explicitly requested that Q1 return to the snapshot created before
  any extra post-curve alignment was added. The verified archive is
  `../_backups/mspm0dache_TI_2026080104_before_q1_align_plus75mm.zip`, SHA-256
  `D1AD618F482F07406318261B9B248947CA1B25BD2BDF0E5425D5F0CF41A9279D`.
- `chassis_config.h`, `test_mode.c` and `tools/verify_project.ps1` were restored
  to byte-for-byte hash equality with that snapshot. Current TI build is
  intentionally `2026080104`: Q1 straight/curve/private-wheel values are
  143/110/176 RPM, forward alignment is 150-250 mm, and reverse search is
  25 RPM with 400 mm/6000 ms bounds. The later build-0105/0106 Q1 alignment
  and speed changes are no longer active.
- Q2-Q5 behavior in the saved TI image is the same as immediately before the
  rollback. Later MC02-only handoff entries were retained; MC02 Vehicle build
  `2026080106` and all frozen protocol layouts are unchanged. The pre-rollback
  TI 0106 project was separately archived as
  `../_backups/mspm0dache_TI_2026080106_before_rollback_to_0104.zip`, SHA-256
  `F84B041A45D7015BBB063C39F40ED3B43EB4CA71827F72032FC8AC03377497B5`.
- Fresh full SysConfig, TEST_MODE, safe Release, structural and ELF
  verification passed after restoration. TEST_MODE text/BSS is 33880/2599
  bytes and Release is 21296/2317 bytes. The six restored code/check/document
  files all match the saved snapshot byte-for-byte. No flash or powered
  behavior is claimed by the rollback operation itself.

## 2026-08-01 MC02-only Q2 staged damping update

- MC02 Vehicle build `2026080106` now pairs with the user-restored TI
  TEST_MODE build `2026080104`. It changes only MC02's stationary Q2 positive braking,
  staged negative approach and persistent final capture; TI source and
  artifact are unchanged and do not need rebuilding.
- Q1 speed/alignment behavior, Q2's disarmed chassis state, Q3-Q5 routes,
  OLED behavior and all TI safety paths remain unchanged. TI V2 and Jetson
  V3 versions, fields, lengths and meanings are unchanged. MC02's full
  offline verification passed; powered Q2 behavior remains to be tested.

## 2026-08-01 Q1 second speed and alignment increase

- The user reports TI build 0105 still does not travel far enough into the
  straight before reversing. TI TEST_MODE build `2026080106` adds another
  100 mm only to Q1's alignment window, from 225-325 to 325-425 mm. Relative
  to the original 150-250 mm window this is a cumulative +175 mm, while the
  same physical A marker remains the sole normal final-stop authority.
- At the user's request, only Q1 line-follow speed rises another exact 10%
  from build 0105: straight cruise 143 -> 157.3 RPM, curve-floor base
  110 -> 121 RPM, and the private Q1 wheel ceiling 176 -> 193.6 RPM to retain
  steering headroom. The 30 RPM start, 55 RPM launch cap, 80 RPM/s slew,
  steering/PID gains and 60 RPM final alignment speed remain unchanged.
- To permit the added 100 mm to return to the same line at the unchanged
  25 RPM reverse speed, only the no-marker safety envelope expands from
  475 mm/6000 ms to 575 mm/7500 ms. This does not command a fixed reverse
  distance and does not shift a successfully detected stop. Q2-Q5, OLED,
  TI V2 and Jetson V3 are unchanged; MC02 Vehicle remains build `2026080105`
  and needs no rebuild for this TI-only update.
- Full clean SysConfig, TEST_MODE, safe Release, structural and ELF
  verification passed. TEST_MODE text/BSS is 33880/2599 bytes and Release is
  21296/2317 bytes. TI build 0106 was not flashed or floor-tested. The saved
  build-0104 rollback archive remains valid and unchanged.

## 2026-08-01 Q1 extra straight alignment before reverse

- Before this change, the complete TI `2026080104` project was archived at
  `../_backups/mspm0dache_TI_2026080104_before_q1_align_plus75mm.zip` as the
  user-requested rollback point. The archive contains 142 entries and has
  SHA-256 `D1AD618F482F07406318261B9B248947CA1B25BD2BDF0E5425D5F0CF41A9279D`.
  If the user asks to return to this backup, restore that snapshot rather
  than reconstructing build 0104 from memory.
- The user reports build 0104's final longitudinal stop position is already
  excellent, but the body stops at an angle because reverse begins before
  the chassis has fully straightened after the second curve. TI TEST_MODE
  build `2026080105` therefore changes only Q1's forward alignment window
  from 150-250 mm to 225-325 mm: an exact additional 75 mm before zero-speed
  settle and reverse.
- Final position authority remains the same physical three-channel A-marker
  rule, so this change does not add or subtract a fixed final stop offset.
  The reverse-search distance guard rises by the same 75 mm, from 400 to
  475 mm, so the longer forward alignment can still return to the same A
  line. Reverse remains 25 RPM with the same 6000 ms bound. Q1 speeds,
  steering/PID, marker confirmation, Q2-Q5, OLED and protocol are unchanged.
- Full clean SysConfig, TEST_MODE, safe Release, structural and ELF
  verification passed. TEST_MODE text/BSS is 33880/2599 bytes and Release is
  21296/2317 bytes. TI build 0105 was not flashed or floor-tested. MC02
  Vehicle build `2026080105` is unchanged and needs no rebuild for this
  TI-only change. First verify the Q1 body is straighter while its sensor
  still stops on the same A line; if not, request the recorded rollback.

## 2026-08-01 MC02-only Q2 asymmetric approach update

- MC02 Vehicle build `2026080105` pairs with unchanged TI TEST_MODE build
  `2026080105`. It changes only Q2 positive braking and negative approach/
  static-friction scheduling and explicitly ignores chassis acceleration in
  that stationary task.
- TI source, Q1 speed profile, Q2 stationary chassis behavior, Q3-Q5 routes,
  OLED behavior and every local chassis safety path are unchanged. TI V2 and
  Jetson V3 wire formats and meanings are unchanged, so this update requires
  only the MC02 Vehicle image to be rebuilt/flashed. MC02's full offline
  verification passed; powered Q2 behavior remains to be tested.

## 2026-08-01 Q1 ten-percent speed increase

- At the user's request, TI TEST_MODE build `2026080104` changes only Q1's
  normal line-follow speed envelope by exactly 10%: straight cruise rises
  from 130 to 143 RPM, curve-floor base from 100 to 110 RPM, and the private
  Q1 wheel ceiling from 160 to 176 RPM so steering authority is not reduced
  at the higher base speed.
- Q1's 30 RPM/one-second initial launch, 55 RPM/300 mm launch cap, 80 RPM/s
  acceleration slew, PID/steering gains, 40-second safety timeout, forward
  alignment, zero-speed settle, 25 RPM reverse, final-marker rule and all
  fault limits are unchanged. Q2 remains stationary; Q3 stays at 100 RPM and
  30 RPM/s; Q4/Q5 stay at 70 RPM and 14 RPM/s.
- MC02 Vehicle build `2026080105`, Jetson V3 and frozen TI V2 protocol are
  unchanged; no MC02 or Jetson rebuild is required for this TI-only update.
  Full clean SysConfig, TEST_MODE, safe Release, structural and ELF
  verification passed. TEST_MODE text/BSS is 33880/2599 bytes and Release is
  21296/2317 bytes. This build was not flashed or floor-tested; first confirm
  `g_test.build_id=2026080104`, then run one guarded Q1 while checking curve
  retention, <=20 s completion and the existing reverse stop.

## 2026-08-01 Q4/Q5 thirty-percent speed-profile reduction

- TI TEST_MODE build `2026080103` pairs with MC02 Vehicle build
  `2026080104`. Only Q4/Q5 are changed: cruise is reduced from 100 to 70 RPM
  and common-wheel acceleration/deceleration from 20 to 14 RPM/s. Q3 remains
  at 100 RPM and 30 RPM/s; Q1/Q2 are unchanged.
- Q4/Q5 retain their 300 RPM/s differential steering slew, existing route
  gates, 8000 ms local gentle-stop guard and 30000 ms scored total timeout.
  At 70 RPM the nominal 6141.6 mm course needs about 25.8 seconds before
  ramp/curve overhead, so powered Q4/Q5 timing must be checked immediately.
- MC02's matched Q4/Q5 feed-forward floor is reduced from 0.070 to
  0.048 m/s2. TI V2 and Jetson V3 layouts, fields and meanings are unchanged;
  no Jetson upload is required. Both controller images require flashing.
- Full clean SysConfig, TEST_MODE, safe Release, structural and ELF
  verification passed. TEST_MODE text/BSS is 33816/2599 bytes and Release is
  21296/2317 bytes. No powered Q4/Q5 result is claimed for this pair.

## 2026-08-01 MC02-only Q2 final-stop update

- MC02 Vehicle build `2026080103` continues to pair with unchanged TI
  TEST_MODE build `2026080102`. TI's three-second countdown, Q2 stationary
  chassis behavior, OLED behavior, Q3-Q5 ramps and all TI source are unchanged.
- MC02 Q2 now uses prediction only for positive-waypoint braking, switches to
  return only after a raw +50 mm crossing, uses hysteretic final capture plus
  two bounded static-friction releases, and requires a two-second/3 mm/s final
  stop inside the printed +/-10 mm band. Its five-second mark is diagnostic;
  the 15-second safety timeout remains.
- TI V2 and Jetson V3 wire formats and meanings are unchanged. No TI or Jetson
  rebuild/upload is required for this MC02-only update.

## 2026-08-01 substantially slower Q3-Q5 ramps

- TI TEST_MODE build `2026080102` pairs with MC02 Vehicle build
  `2026080101`. Q3 common acceleration/deceleration is 30 rpm/s; Q4/Q5 use
  20 rpm/s. Cruise speed, steering slew and scored 8/30-second total timeouts
  are unchanged.
- The local gentle-stop guard is widened to 8000 ms so the 20 rpm/s Q4/Q5
  stop can finish without the benign timeout that previously bypassed MC02's
  normal five-second hold. Real safety faults remain immediate.
- The non-blocking 1000 ms OLED boot delay/retry from build 2026080101 is
  retained. TI V2 and Jetson V3 layouts are unchanged. Full clean offline
  verification passed at 33816/2599 and 21296/2317 text/BSS bytes for
  TEST_MODE and Release respectively. Powered behavior is not yet claimed.

## 2026-08-01 delayed external-OLED initialization

- TI TEST_MODE build `2026080101` keeps all chassis control, Q3-Q5 ramp
  values and TI V2 messages unchanged. MC02 remains build `2026073126` and
  Jetson remains unchanged.
- `OledDisplay_Init()` no longer accesses the externally regulated OLED
  during the supply ramp. The cooperative display tick waits 1000 ms before
  the first controller initialization; failure then uses the existing 1000 ms
  reconnect cycle and complete static/dynamic redraw.
- The delay is non-blocking: motor safety, buttons, line sensor and both UART
  links initialize immediately. Clean TEST_MODE and safe Release verification
  passed at 33760/2599 and 21296/2317 text/BSS bytes. Hardware behavior is not
  yet claimed.

## 2026-08-01 gentler Q3-Q5 common-speed ramps

- TI TEST_MODE build `2026073119` changes only the MC02-started Q3-Q5 common
  acceleration/deceleration slew from 50 to 40 rpm/s. Cruise RPM, steering
  slew and question time limits are unchanged.
- The gentle-stop internal guard is widened from 3000 to 4000 ms so the lower
  deceleration rate cannot trip that local guard; the existing Q3 8-second
  and Q4/Q5 30-second total bounds remain authoritative.
- MC02 build `2026073126` uses the matching 0.14 m/s2 feed-forward floor.
  TI V2 and Jetson V3 formats are unchanged.
- Clean TEST_MODE and safe Release verification passed at 33736/2598 and
  21272/2316 text/BSS bytes. Powered behavior remains unverified.

## 2026-07-31 contiguous OLED value update

- TI TEST_MODE build `2026073118` keeps chassis control, route timing and the
  frozen TI V2 wire layout unchanged. No Jetson code or protocol changes.
- The four time glyphs are assembled into one 24-column buffer, the OLED
  cursor is positioned once, and the complete value is streamed in order.
  This reduces visible partial-number jumps and I2C overhead.
- Timer start and power-state setters no longer perform OLED I2C work inline;
  they only update state. The existing low-priority `QuestionTimer_Tick()` at
  the end of the 10 ms cooperative task performs the queued refresh, after
  line, chassis, MC02-link and IMU work.
- Clean TEST_MODE and safe Release verification passed at 33736/2598 and
  21272/2316 text/BSS bytes. Actual OLED appearance remains a hardware check.

## 2026-07-31 OLED same-line atomic time refresh

- TI TEST_MODE build `2026073117` keeps every chassis, Q1 timing and TI V2
  behavior from build `2026073116`. It moves the four-character seconds value
  beside `TIME` on the same OLED page and writes the complete value in one
  scheduled refresh instead of one glyph per 10 ms tick.
- Decimal digit extraction uses seconds modulo 1000 only for the three
  displayed digits; the underlying elapsed millisecond/second counters do not
  wrap at 999. `g_test.timer` now exposes requested seconds, successfully
  displayed seconds and successful render count, in addition to I2C status.
- MC02 and Jetson protocols/code are unchanged. Clean TEST_MODE and safe
  Release verification passed at 33688/2598 and 21208/2316 text/BSS bytes.
  The actual OLED refresh remains a powered hardware check.

## 2026-07-31 Q1 40 s safety window, faster cruise and gated feed-forward

- TI TEST_MODE build `2026073116` raises Q1 line-follow cruise from 110 to
  130 RPM and widens only its command/supervisor safety timeout from 20 s to
  40 s. The scoring target remains <=20 s. Reverse-search still retains its
  independent 400 mm / 6000 ms bounds and the fresh three-channel marker
  confirmation rule.
- A fresh, connected line sensor sample with zero active channels is now
  tolerated during the bounded reverse search instead of being promoted to
  an immediate line fault. Disconnect/stale data and all existing distance,
  time, motor and encoder safety limits remain active.
- Frozen 28-byte TI V2 status layout/version is unchanged. Reserved status
  flag bits 10 and 11 now identify intentional common-wheel acceleration and
  deceleration. MC02 build `2026073124` uses those gates so chassis
  feed-forward is zero during steady cruise and rejects acceleration with an
  inconsistent sign. Jetson V3 is unchanged.
- OLED I2C failures no longer permanently disable the display: after three
  errors the firmware retries both supported addresses once per second,
  redraws `TIME`/`PWR`, and resumes the requested live values after recovery.
- Clean TEST_MODE and safe Release verification passed. TEST_MODE text/BSS
  is 33536/2575 bytes and Release is 21064/2293 bytes. No hardware flash or
  powered test is claimed.

## 2026-07-31 Q1 simultaneous-marker rule and paired moving balance

- TI TEST_MODE build `2026073115` changes only Q1's final reverse stop
  authority: after the existing 40 mm arm distance, the fresh Hiwonder sample
  must contain at least three simultaneously active channels for three
  consecutive new samples. The 120 ms last-seen union remains in
  `marker_recent_state/count` for diagnosis but no longer contributes to the
  stop decision. Reverse speed, forward alignment, settle behavior, 400 mm/
  6000 ms bounds and all line/motor fault paths are unchanged.
- The frozen V2 `CHASSIS_STATUS` layout is unchanged. Its existing measured
  `left_rpm_x10/right_rpm_x10` fields are now explicitly consumed by MC02
  build `2026073123` as vehicle-forward-positive speed, from which MC02
  derives bounded chassis acceleration. No target-RPM or new acceleration
  field was added. TI remains disarmed in ROUTE_COMPLETE while MC02 holds the
  ball for a continuous 3000 ms final stability check, then accepts the
  existing SAFE_STOP. Jetson V3 is unchanged.
- Full clean SysConfig, TEST_MODE, safe Release, structural and ELF
  verification passed. TEST_MODE text/BSS is 33088/2570 bytes and Release is
  20824/2289 bytes. No flash or hardware result is claimed. First verify
  build `2026073115`, then use a guarded reverse pass to confirm three fresh
  simultaneous-channel samples stop at A; separately verify both transmitted
  RPM signs during a raised-wheel moving-question start.

## 2026-07-31 MC02 Q2 Step B + Step B2 velocity observation

- MC02 Vehicle build `2026073121` replaces the Jetson velocity field in its
  controller with a 70 ms target-window position chord estimate and changes
  its Q2 anti-stall drive-floor admission from instantaneous low speed to an
  independent 2 mm / 120 ms stationary-position window.
- All MC02 gains, actuator envelopes, targets, breakaway parameters, timing,
  Step A rebound behavior and immediate safety paths remain unchanged. This
  build has passed four-preset offline verification but has not yet passed
  the required stationary Ozone checks or powered K2 testing.
- TI V2 protocol, K2 mapping, 30000 ms supervisor window, chassis authority
  and TI firmware are unchanged; no TI build or flash is required.

## 2026-07-31 MC02 Q2 Step B/C rollback

- At the user's request, MC02 was restored to the hardware-confirmed
  Step A-only Vehicle build `2026073120`; the unflashed Step B/C filtered
  state and acceleration predictor were removed.
- TI V2 protocol, K2 mapping, 30000 ms supervisor window, chassis authority
  and firmware remain unchanged; no TI build or flash is required.

## 2026-07-31 MC02 Q2 rebound-braking authority

- MC02 Vehicle build `2026073120` fixes one gimbal-side authority condition:
  after a negative-leg overshoot rebounds toward positive position at
  >=30 mm/s, positive pipe angle is braking and may regain the existing full
  +2-degree envelope. Below that threshold, the existing +0.8-degree
  positive drive cap remains.
- Targets, gains, actuator bounds, breakaway pulses, friction floor, timing
  and every safety cutoff are unchanged. The larger velocity-estimator and
  acceleration-predictor Step B/C work is intentionally deferred until
  build-3120 hardware CSVs isolate this Step A effect.
- TI V2 protocol, K2 mapping, 30000 ms supervisor window, chassis authority
  and firmware are unchanged; no TI build or flash is required.

## 2026-07-31 MC02 Q2 delay-projected positive turn

- Five build-3118 attempts in `run_1848.csv` peak at a median +68.1 mm and
  maximum +79.5 mm. Although MC02 no longer waits for intermediate stability,
  the logged target changes only after raw position is already +56.0..
  +59.1 mm because the raw waypoint decision does not anticipate the measured
  full-loop response delay.
- MC02 Vehicle build `2026073119` starts the existing return when its bounded
  125 ms delay-projected position reaches +50 mm, with actual raw +50 mm as
  an unconditional fallback. Return control, final settle, time windows and
  safety behavior are unchanged.
- This is entirely MC02-side Q2 control. TI V2 protocol, K2 mapping,
  30000 ms supervisor window, chassis authority and firmware are unchanged;
  no TI build or flash is required.

## 2026-07-31 MC02 Q2 immediate +50 mm return

- MC02 build-3117 `run_936.csv` shows seven normal-run positive peaks with a
  +72.7 mm median and one severe +125 mm pass-through. The old intermediate
  velocity/dwell gate can miss a fast crossing and leave the +50 mm target
  active even though the task requires immediate return.
- MC02 Vehicle build `2026073118` now switches to the -50 mm target on the
  first valid position sample at or beyond +50 mm. Its return drive schedule,
  final settle behavior, 5000 ms checkpoint, 15000 ms MC02 hard timeout and
  all immediate safety cutoffs are unchanged.
- This is entirely MC02-side Q2 control. TI V2 protocol, K2 mapping,
  30000 ms supervisor window, chassis authority and firmware are unchanged;
  no TI build or flash is required.

## 2026-07-31 MC02 Q2 bounded negative launch

- MC02 build-3116 hardware logs `run_277.csv` and `run_364.csv` no longer
  show the earlier +20..+30 mm stalls, but their negative troughs reach about
  -80.3 and -78.6 mm. The +1.5-degree return launch also persists after the
  delayed velocity estimate has changed sign.
- MC02 Vehicle build `2026073117` restores positive-leg braking after +30 mm,
  shortens the one return launch pulse to 250 ms, and then caps every further
  drive-sign pipe request at +0.8 degree while preserving full -2.0-degree
  braking. This is entirely gimbal-side Q2 control.
- TI V2 protocol, K2 mapping, 30000 ms supervisor window, chassis authority
  and firmware are unchanged; no TI build or flash is required.

## 2026-07-31 MC02 Q2 leg-specific drive scheduling

- Seven reported MC02 build-3115 trials contained three positive-leg
  hesitations; five supplied CSVs show two complete stalls near +21..+28 mm
  and three successful returns whose troughs repeat near -71..-76 mm.
- MC02 Vehicle build `2026073116` now prevents braking-sign requests before
  +35 mm, uses up to the fixed 2-degree positive-leg envelope when the ball
  slows, and caps only negative-leg drive at 1 degree once velocity is toward
  -50 mm. Full opposite-sign braking and all MC02 safety bounds remain.
- This is entirely MC02-side ball control. TI V2 protocol, K2 mapping,
  30000 ms supervisor window, chassis authority and firmware are unchanged;
  no TI build or flash is required.

## 2026-07-31 MC02 Q2 positive-leg repeat breakaway

- `run_368.csv` shows MC02 build `2026073114` can consume its only positive
  breakaway near +27 mm and later remain stalled around +37 mm. MC02 Vehicle
  build `2026073115` therefore permits at most two positive-leg pulses, each
  still requiring a separate 150 ms / 2 mm stationary detection. The
  negative leg remains limited to one.
- TI protocol, K2 behavior, 30000 ms supervisor window and firmware are
  unchanged; no TI flash is required.

## 2026-07-31 MC02 Q2 extended powered-control window

- MC02 Vehicle build `2026073114` prevents Q2 completion before 5000 ms and
  extends its own hard Q2 safety timeout from 5000 to 15000 ms. This keeps
  final-target torque active through the printed five-second checkpoint and
  permits continued stabilization afterward for diagnostics. MC02 also
  requires 1000 ms continuous final stability before completion; the +50 mm
  immediate-turn waypoint remains unchanged.
- The competition acceptance limit is not relaxed: position at five seconds
  must still be judged separately, and completion after five seconds is a
  tuning failure. TI already gives internal Q2 a 30000 ms supervisor window,
  so TI source, protocol, K2 behavior and firmware remain unchanged; no TI
  flash is required.

## 2026-07-31 MC02 Q2 asymmetric final capture

- `run_10475.csv` shows MC02 build `2026073112` crosses the final -50 mm
  target at high speed, then oscillates from about -91.7 mm to +34.0 mm.
  Its symmetric +/-0.8-degree final clamp also limited braking.
- MC02 Vehicle build `2026073113` now latches final capture on first entering
  +/-30 mm of -50 mm, keeps the full fixed braking envelope and limits only
  non-braking drive to +/-0.6 degree. The polished pipe may have lower
  friction, so this is an energy-removal change rather than a global gain
  increase. TI protocol, K2 behavior, timing and firmware are unchanged; no
  TI flash is required.

## 2026-07-31 MC02 Q2 final capture zone

- MC02 Vehicle build `2026073112` changes only the final -50 mm Q2 capture
  behavior. Within 20 mm of that target, the gimbal disables its
  static-friction floor and caps pipe request to +/-0.8 degree; far-field
  motion and the +50 mm waypoint are unchanged.
- The pipe has been polished and may have lower friction. TI protocol,
  timing, K2 behavior and firmware are unchanged; no TI flash is required.

## 2026-07-31 MC02 build-3111 first hardware run

- `run_9900.csv` shows MC02 build `2026073111` now switches the +50 mm
  waypoint after about 1.20 s, so the TI/MC02/Jetson sequence and the new
  waypoint timing operate correctly.
- Q2 still reaches the fixed five-second MC02 fault because the final -50 mm
  leg oscillates between approximately -68 mm and +10 mm instead of settling.
  This is gimbal-side final-target tuning; TI protocol, timing and firmware
  need no change.

## 2026-07-31 MC02 Q2 intermediate-waypoint timing

- MC02 build `2026073111` changes only Q2's intermediate +50 mm waypoint:
  +/-7 mm position, <=100 mm/s and 60 ms dwell now switch to the -50 mm leg.
  The final -50 mm completion stays at <=60 mm/s and 150 ms. This follows a
  build-3110 run where the first leg consumed about 3.895 s and the second
  reached roughly -40.5 mm just as the five-second deadline expired.
- TI protocol, K2 mapping, the five-second timer and every immediate
  safe-stop path are unchanged. No TI rebuild or flash is required.

## 2026-07-31 MC02 Q2 overshoot rebalance

- MC02 Vehicle build `2026073110` follows a build-3109 trial that crossed the
  +50 mm target at about +129.5 mm/s, peaked near +133.6 mm and never settled
  to switch to the -50 mm leg. TI behavior, K2 mapping, V2 protocol and the
  fixed five-second deadline are unchanged.
- MC02 keeps the Q2 1.2-degree static-friction floor and 3.0 rad/s motor
  response, but changes Kp/Kd to 4.0/2.5, applies the floor only below
  10 mm/s, and delays/reduces the one-shot stall pulse to
  1.5 degrees/400 ms after a 650+150 ms admission. No TI flash is needed.

## 2026-07-31 MC02 aggressive Q2 static-friction drive

- MC02 Vehicle build `2026073109` changes only the gimbal-side Q2 controller
  after the assembled mechanism stopped near +32 mm while the DM4310
  continued to dither. TI protocol, button mapping and chassis behavior are
  unchanged; K2 still starts question 2 through the existing V2 link.
- Q2 now holds at least 1.2 degrees in the target direction when slow and
  outside the 7 mm target tolerance, uses a one-shot 2.0-degree/650 ms
  breakaway pulse per leg, Kp/Kd 5.0/1.2, 3.0 rad/s motor slew and 150 ms
  settle. MC02's fixed five-second Q2 deadline and every existing STOP,
  stale-link, motor and TI safe-stop path remain unchanged.
- The supplied `run_2279.csv` did not record the K2 interval (question,
  target and command are zero throughout), so the next run must start Jetson
  CSV recording before K2. No TI firmware rebuild or flash is required for
  this MC02-only change.

## Handoff maintenance rule

After every code change or project-knowledge change, update this file in the
same work session. Record what changed, why it changed, what was verified,
what remains unverified, and the next safe action. Hardware observations must
not be recorded as verified facts until the user reports or a test captures
the actual measurement.

## Update log

- User floor-tested TI build `2026073113`: Q1 did reverse, but it stopped
  before the front Hiwonder array reached the transverse A bar. The reported
  ordinary longitudinal line is normally one/two active channels and the A
  bar is expected to produce three/four active channels. This is hardware
  evidence that the 120 ms union alone could merge a wandering ordinary-line
  footprint into a false four-channel marker.
- TI build `2026073114` keeps every Q1 route speed, PID/steering gain, forward
  alignment distance, settle rule, reverse speed and distance/time safety
  bound unchanged, but tightens the final Q1 reverse predicate: in addition
  to the existing >=4-channel recent union across both sensor halves and two
  fresh confirmations, the current fresh sample must contain at least three
  active channels. This directly rejects the observed one/two-channel
  ordinary line while still accepting the reported three/four-channel A bar.
- Q3/Q4/Q5 no longer use Q1's forward-align/settle/reverse-marker sequence.
  Q3 starts a gentle stop when encoder route distance reaches the specified
  1500 mm A-B separation. Q4/Q5 use the existing two-turn plus 5970 mm gate,
  with the independent 6070 mm bound, to enter a gentle stop and complete one
  lap without reversing. Their existing nominal speeds remain 100 RPM, below
  Q1's 120 RPM straight request.
- The current `lap_phase` values are 0 IDLE, 1 LEAVE_START, 2 SEARCH_FINISH,
  3 FORWARD_ALIGN, 4 SETTLE, 5 REVERSE_SEARCH, 6 MARKER_DETECTED,
  7 GENTLE_STOP and 8 COMPLETE. For Q1, A_MARKER_DETECTED still means the
  physical reverse marker predicate passed. For Q4/Q5 it is emitted when the
  established one-lap route gate enters GENTLE_STOP; those questions no
  longer claim a physical reverse-marker observation.
- To reduce ball disturbance only for MC02 Q3/Q4/Q5, the average/common wheel
  request now accelerates and decelerates at 50 RPM/s. The steering-difference
  request has an independent 300 RPM/s slew so curve correction remains
  responsive instead of inheriting the slow payload ramp. Their completion
  path commands a controlled ramp to zero, waits for both command targets to
  reach zero and both measured wheel speeds to fall to <=2 RPM, then disarms;
  a 3000 ms bound converts failure to settle into a safe route timeout. Q1
  motion and braking profiles are deliberately untouched.
- OLED stopwatch semantics now match actual execution. K1-K5 BUTTON_EVENT no
  longer starts the timer. Q2 starts it when MC02 START is accepted; moving
  Q1/Q3/Q4/Q5 start it only after the post-START fresh-sensor gate succeeds
  and Motor_Arm succeeds. `QuestionTimer_Tick()` continues refreshing the
  integer seconds during motion, and every finish/stop path freezes the final
  value. The frozen MC02-TI V2 frame format, IDs, lengths, CRC, question
  mapping, options and timeouts were not changed.
- Full clean offline verification passed for TI `2026073114`: SysConfig,
  TEST_MODE and safe Release builds, structural checks and ELF checks passed.
  TEST_MODE text/BSS is 33,128/2,570 bytes; Release is 20,824/2,289 bytes.
  No flash was performed in this work session. Hardware still must verify:
  Q1 reaches the physical A bar before reverse stop, Q3 covers 1.5 m and
  settles within 8 s, Q4/Q5 finish within 30 s without reverse, ball motion
  is acceptably gentle, and OLED seconds advance during each active run.

- User reports the ordinary centered longitudinal line continuously activates
  only one or two Hiwonder channels. Together with the supplied static A-line
  photo showing at least four illuminated channels, this is the current
  hardware basis for the new stop detector. Exact moving bit sequences remain
  uncaptured, so the first powered run is still calibration rather than a
  scored acceptance result.
- TI build `2026073113` replaces the Q1/Q4/Q5 fixed forward-distance A stop
  with the requested sensor-datum sequence without changing Q3, PID/steering,
  normal route speeds, button mapping, or any frozen MC02-TI V2 byte, message,
  field, length, CRC, action, option or question ID. The private two-turn gate
  at 5970 mm, or independent 6070 mm bound, now starts a 60 RPM forward-align
  phase. TI follows 150-250 mm into the next straight and requires eight fresh
  samples with one/two active channels and `abs(position_x1000)<=1000`.
- After alignment, TI commands both targets to true zero, waits at least
  200 ms and requires both measured wheel speeds at or below 2 RPM. It then
  reverses with equal slew-limited -25 RPM targets. Forward directional
  line-loss recovery is explicitly disabled during reverse; a transient loss
  removes the reverse request and the existing 300 ms sensor-fault bound
  remains authoritative.
- Reverse A detection is armed only after 40 mm. For each fresh 10 ms sensor
  sample, TI retains channel last-seen times and forms a 120 ms recent-state
  union. A requires at least four distinct channels, at least one channel in
  each half of the array, and two consecutive fresh candidate samples.
  `g_test.line.marker_recent_state/count` expose this bounded history;
  `finish_distance_mm` reports forward-align distance in phase 3 and reverse
  distance in phase 5. Missing A at 400 mm reverse, six seconds reverse, a
  1200 ms failure to settle, or a 250 mm failure to center stops/disarms as a
  route timeout.
- `lap_phase` is now 0 IDLE, 1 LEAVE_START, 2 SEARCH_FINISH, 3 FORWARD_ALIGN,
  4 SETTLE, 5 REVERSE_SEARCH, 6 MARKER_DETECTED, 7 COMPLETE. The V2
  A_MARKER_DETECTED event is no longer emitted when an encoder-only final
  approach begins; it is emitted only when phase reaches actual reverse
  marker detection, immediately before ROUTE_COMPLETE in the existing MC02
  tick order.
- Full clean offline verification passed for `2026073113`: SysConfig,
  TEST_MODE competition build, TEST_MODE=0 safe Release, structural checks
  and ELF checks all passed. TEST_MODE text/BSS is 32,368/2,562 bytes and
  Release is 20,904/2,289 bytes. No firmware was flashed in this work session.
  Forward alignment distance, reverse direction/speed, A recognition,
  physical coast/stop offset and total Q1 <=20 s timing are all hardware-
  unverified. First verify build ID and raised-wheel forward-zero-reverse
  sequencing, then perform one guarded floor run while filming the sensor and
  stop point; do not alter normal line-follow gains from this untested change.

- User supplied a static overhead photo of the fully assembled chassis with the
  front Hiwonder sensor positioned at the A finish-line intersection. The
  sensor is physically suitable as the requested front centerline datum, and
  at least four blue channel indicators appear active across a wider region
  than expected on the ordinary longitudinal guide line. Because glare and a
  single still image cannot establish exact bit numbers or dynamic reliability,
  this is evidence for, but not yet hardware verification of, the final marker
  predicate. Obtain one matching photo with the sensor on ordinary centered
  straight line only.
- Preferred initial reverse marker predicate is now temporal widening rather
  than a same-frame raw count: arm only after the completed lap has crossed A
  and the chassis has followed/centered on the next straight, stop and settle,
  reverse slowly with equal-wheel feedback, then maintain a short fresh-sample
  state-bit union. A union wider than the ordinary center-line baseline can
  confirm A even if motion exposes only two or three channels per individual
  sample. Exact window, bit-span/count and reverse speed remain unimplemented
  pending the ordinary-line comparison.

### 2026-07-31

- Cross-project update only: sibling MC02 Vehicle build `2026073108` changes
  Q2 breakaway handling from a second-leg-only trigger to at most one
  directional pulse per leg. The first leg detects a 250 ms / 2 mm stationary
  window after an 800 ms grace while still outside target tolerance and
  applies `-1.2 deg` pipe angle for 350 ms. The settled +50-to-50 mm target
  transition immediately applies the second leg's `+1.2 deg / 350 ms` pulse.
  The fixed five-second Q2 limit, STOP/fault/stale-vision/TI-safe-stop
  cancellation and fixed actuator envelopes remain in force. Four-preset
  offline verification passed; no assembled-vehicle result is claimed. No TI
  source, protocol, timeout, chassis authority, route, PWM or button behavior
  changed.
- New unverified hardware observation from the user: with the intended TI
  supply absent and the TI-MC02 UART wires unplugged, voltage is still present
  on the LaunchPad 3V3 rail. Do not treat this as a valid alternate TI supply.
  The leading back-power candidate is the externally powered OLED I2C pull-up
  path through PB2/PB3; the independently powered Hiwonder I2C module,
  encoder A/B signals, motor-driver control wiring, XDS110/J101 jumpers or USB
  are additional candidates. Exact 3V3 voltage and the guilty branch are not
  yet measured/isolated.
- Safe isolation order with all power removed before rewiring: remove OLED
  VDD or OLED SCK/SDA, then Hiwonder 5V/SCL/SDA, then encoder 5V/A/B, then
  driver control signals, re-powering the remaining system and measuring TI
  3V3 after each step. Never short 3V3 to GND as a test. Permanent wiring must
  prevent any signal or I2C pull-up from remaining high while TI VDD is off;
  preferably switch TI and its I2C pull-ups together, or add appropriately
  enabled isolation/level shifting.
- User then clarified the live configuration: OLED and the external regulator
  are off; MC02 is powered and the only stated connection to the unpowered TI
  is the MC02-TI UART. The leading diagnosis therefore changes to UART
  back-power from the idle-high MC02 USART1_TX line into TI PA9/UART1_RX.
  Confirmation test: with all power removed before changing wiring, disconnect
  only `MC02 USART1_TX -> TI PA9`, leave common GND if desired, restore MC02
  power and remeasure TI 3V3. Exact voltage and result remain unreported.
- User observed MC02/Ozone `g_gimbal_debug.ti_tx_frames` increasing in this
  condition. Source inspection confirms this counter increments when MC02
  queues its own outbound frame; the periodic heartbeat is queued every
  100 ms regardless of TI response. It is therefore not evidence that TI is
  powered or online. Valid TI communication is instead indicated by increasing
  `ti_rx_frames` and `ti_online=1`, where online is asserted only after a valid
  TI STATUS frame and cleared after a 500 ms status timeout. Even if valid
  replies appear while the intended TI supply is absent, the electrical state
  remains unsafe UART back-power and must not be treated as normal operation.
- User subsequently reports that MC02/Ozone `ti_rx_frames` also increases.
  This proves MC02 is parsing valid CRC-protected V2 frames emitted by TI, so
  the TI firmware is executing despite its intended supply being off. This is
  strong evidence of substantial UART back-power, not permission to operate
  the chassis. `ti_online` and `ti_chassis_state` are still needed to identify
  the reported logical state, but the electrical state is already unsafe.
  Power must be removed before disconnecting MC02 TX-to-TI PA9; with MC02 then
  powered alone, TI 3V3 should be near zero and `ti_rx_frames` should stop.
- Hardware isolation test passed and confirms the source: after removing power,
  disconnecting `MC02 USART1_TX -> TI PA9/UART1_RX`, and powering MC02 alone,
  the TI 3V3 rail dropped to the expected off state, `ti_rx_frames` stopped and
  `ti_online` became 0. The displayed `ti_chassis_state=1` is only MC02's last
  cached valid `IDLE_DISARMED` status; `ti_link.c` clears online after the
  500 ms timeout but does not erase the cached state byte. Do not interpret
  state 1 as live while online is zero. Permanent prevention is still required:
  switch/power TI before or together with MC02 and shut MC02 down first, or use
  a partial-power-down-safe UART isolation/buffer solution.

- Cross-project knowledge update only: sibling MC02 Vehicle build
  `2026073107` now gives Q2 one bounded `+1.2-degree / 350 ms` physical-pipe
  breakaway pulse exactly when its target changes from +50 mm to -50 mm,
  then resumes the existing delay-compensated PD output and 0.8-degree live
  commissioning clamp. STOP, invalid vision, fault, TI safe stop and the
  unchanged five-second Q2 timeout cancel the pulse. Four-preset offline
  verification passed, but no assembled-vehicle result is claimed. No TI
  source, protocol, chassis authority, route, timeout or PWM behavior changed.
- Cross-project hardware-data update only: recording-off Q2 repeat
  `run_5167.csv` restored the Jetson loop to 53.85 Hz with 100% valid active
  rows. MC02 completed its +50 mm leg in about 1.80 s and switched correctly
  to -50 mm. During all 3.173 s remaining, MC02 requested +0.8-degree pipe
  tilt on every sample, but the ball stayed near +46.7 mm; the user's phone
  video confirms the physical stop. This isolates the present failure to
  ball/pipe/motor breakaway after reversal rather than TI, visual frame rate,
  protocol validity or Q2 target switching. No TI code changed. The proposed
  sibling-MC02-only next experiment is a short bounded reverse breakaway pulse;
  it is not yet implemented or verified.
- Cross-project hardware-data update only: sibling MC02 Q2 capture
  `run_4343.csv` remained on the +50 mm leg for the full 4.936 s active
  interval and then timed out; it never issued the -50 mm leg. The file
  records only 19.65 Hz active visual rows (51.64 ms median spacing), all
  valid, and the requested pipe angle is near its +/-0.8-degree clamp for
  60.8% of rows. The visual estimate makes large apparent excursions from
  about -4.8 to +79.7 mm, while the user reports that the physical ball
  barely moved. No TI code or behavior changed. Do not change chassis logic
  or increase MC02 authority until the Jetson frame-rate discrepancy,
  visual/physical position discrepancy, and actual MC02 motor-following
  feedback have been checked.
- Cross-project knowledge update: MC02 build `2026073106` corrects only its
  retained-fault reporting after TI SAFE_STOP propagation. It preserves a
  prior gimbal/Q2 fault instead of replacing it with TI-link fault 6; genuine
  TI status timeout/fault still maps to 6. The competition-required Q2
  five-second total limit remains unchanged. No TI source, V2 payload,
  timeout, route, button or motor behavior changed for this correction.
- Cross-project knowledge update only: sibling MC02 build `2026073105`
  integrates the measured 125 ms full-loop delay into its Vehicle ball
  controller using bounded constant-velocity position projection. The user
  measured nine accepted steps at about 49 FPS (median 125 ms, mean 128 ms,
  dispersion 32 ms). No TI source, V2 packet, action, question ID, route,
  speed, stopping or motor constant changed.
- The next integrated test is stationary K2 with TI still forcibly disarmed.
  Flash MC02 through its new `gimbal_MC02_Vehicle.jdebug`, confirm build
  2026073105 and begin with the existing 0.5-degree pipe commissioning limit.
  This note does not claim that compensation or K2 has passed on hardware.

### 2026-07-30

- Recorded a proposed, not-yet-implemented A-point stopping method. Use the
  front Hiwonder 8-channel sensor's actual optical sensing row as the physical
  measurement datum. After the second turn/lap, follow into the next straight
  far enough to align the chassis, stop and settle, then reverse slowly until
  the A marker is confirmed and immediately command true zero PWM/disarm.
  This would apply to question_id 1/4/5; question_id 3 remains unchanged.
  TI build `2026073012` still contains the previously floor-tested stopping
  logic. Do not implement the new method until ordinary-straight and
  reverse-over-marker sequences of `g_test.line.state`, `active_count`, and
  `sample_sequence` have been captured.
- `active_count >= 2` is not a safe standalone A-marker condition because the
  normal longitudinal guide line can also cover two sensors. The future
  detector must be armed only after stable straight/centered travel and use
  consecutive fresh samples plus bitmap widening/change or a multi-frame
  union. Reverse should use low-speed equal-wheel feedback rather than blindly
  reuse forward line-correction polarity, and must have maximum reverse
  distance/time fail-safe limits. Motor coast after disarm must be calibrated
  on the completed vehicle.
- If WATCH cannot be used during the floor test, first obtain the required
  marker sequence with a phone slow-motion video focused on all eight onboard
  sensor LEDs while the centered chassis is moved slowly backward from the
  ordinary longitudinal line across the A marker. If the LEDs cannot be read
  reliably, the next firmware change may add a bounded RAM trace containing
  sample sequence, timestamp/distance, state bitmap, active count and position,
  then freeze it at stop and present the captured samples on the OLED after
  route completion. Do not change the frozen TI-MC02 V2 frames for this debug
  trace.

- Added the root-level Chinese handoff `../视觉联调交接.md` for the next
  visual-integration conversation. It records the live TI/MC02 build pair,
  responsibility boundaries, five-question and K6 behavior, wiring,
  frozen V2/V3 protocol entry points, current Jetson calibration blockers,
  hardware-unverified items and the safe commissioning order. It explicitly
  warns that parts of `../competition/competition` documentation still
  describe an obsolete architecture and that the frozen MC02 V3 document is
  the protocol authority. This is documentation/knowledge synchronization
  only; no firmware changed after the already verified builds.

- TI build `2026073012` and MC02 build `2026073013` add the user-requested
  K6 chassis software-power permission without changing any frozen V2 frame,
  message/action number, payload field/length, CRC, option bit, button ID or
  question ID. This is deliberately not described as a physical 12 V power
  detector or disconnect: `PWR 0` means TI software will not arm the drive,
  and `PWR 1` means a later valid MC02 START may arm it.
- Power permission defaults to zero at every TI/MC02 boot. A debounced K6
  press can set it to one only while both endpoints are online and fully
  idle. Another K6 press immediately disarms/stops and clears it. SAFE_STOP,
  K7, K8, fault and supervisor loss also clear it. K1/Q1, K3/Q3, K4/Q4 and
  K5/Q5 are ignored/rejected while zero; K2/Q2 remains available and forces
  permission zero because its chassis participation must stay stationary.
  K6 continues to travel as the existing V2 `0xA2 BUTTON_EVENT` with
  `button_id=6`; MC02 mirrors the latch locally, and the frozen V2 protocol
  document was not edited.
- OLED now renders `PWR 0` or `PWR 1` in addition to `TIME` and integer
  seconds. `g_test.timer.power_enabled` exposes the same software latch for
  non-halting Watch inspection. The existing `MOTOR_OUTPUT_ENABLED` status
  flag remains truthful to actual `Motor_IsArmed()` state and was not
  repurposed as a permission flag.
- Full offline verification passed after the change. TI Debug/TEST_MODE
  text/BSS is 30,984/2,496 bytes and safe Release is 20,888/2,281 bytes.
  MC02 clean Debug FLASH/DTCMRAM is 107,188/14,320 bytes, Release is
  67,352/14,312 and Vehicle is 67,072/14,312. No image was flashed in this
  work session. OLED orientation/content, K6 toggle synchronization,
  motion-question rejection at `PWR 0`, K6-off during motion and every
  automatic clear path remain hardware-unverified. First test with both
  motor power rails disconnected, then with raised chassis wheels.
- Material files are TI `test_mode.c/.h`, `question_timer.c/.h`,
  `oled_display.c/.h`, `mc02_link.c`, `tools/verify_project.ps1`,
  `README.md` and `接线指导.md`; MC02 changes are limited to
  `Core/Src/ti_link.c`, `Core/Inc/gimbal_config.h`, its verifier, README and
  wiring checklist. No route speed, PID, line-following, stop distance or
  ball-control constant changed.

- The user reports that the OLED power is now wired directly to the external
  regulated 3.3 V module rather than the LaunchPad 3V3 pin. `OLED VDD` is
  therefore documented as external regulated 3.3 V and `OLED GND` as system
  common ground shared with TI and MC02. SCK/SDA remain on the shared I2C1
  PB2/PB3 bus. This is a reported wiring-state update; regulator voltage
  ripple and powered OLED operation were not independently measured in this
  work session.

- Communication-document correction only: the MC02 V1.1 keyed/front-view
  USART1 connector is pin 1 PA9/TX, pin 2 PA10/RX and pin 3 GND. The two
  wiring tables were corrected accordingly. TI UART direction remains PA8
  TX and PA9 RX; no TI control, route, protocol payload or firmware source
  changed.

- Final paired-build verification was rerun after synchronizing all
  user-facing version references to MC02 `2026073012` / TI
  `2026073011`. `tools/verify_project.ps1` passed SysConfig regeneration,
  the full `TEST_MODE=1` competition build, the `TEST_MODE=0` safe build,
  all five-question structural checks and ELF checks. Current sizes are
  text/BSS 30,272/2,480 bytes for
  `ticlang/mspm0dache.out` and 20,552/2,266 for
  `ticlang/mspm0dache_release.out`.
- The competition image is the `TEST_MODE=1` output: it contains K1-K5,
  Q2 stationary participation, Q3 A-to-B, Q4/Q5 lap paths, the MC02 V2
  endpoint and local K7/K8 handling. The `TEST_MODE=0` output is retained
  as a deliberately noncompetition recovery image, not the image to use
  for a scored run. Hardware timing, distance and powered safety behavior
  are still commissioning results to be measured after flashing.

- TI build `2026073011` and MC02 build `2026073012` complete the source-level
  chassis behavior for all five internal questions without changing the
  frozen V2 frame, message IDs, field lengths, CRC, actions, option-bit
  definitions or button/question numbering. Real-time inspection found that
  the working tree already contained an unlogged intermediate TI
  `2026073010`/MC02 `2026073011` implementation of Q2-Q5 plus the gimbal/
  Jetson competition layer; those existing user-owned changes were
  preserved and audited rather than replaced.
- Q1 is unchanged: the already floor-tuned one-lap route uses the existing
  120 RPM requested base profile, turn/distance A inference, final 50 mm
  approach and a 20-second supervisor ceiling. No Q1 PID, steering, launch,
  recovery, turn or finish constant changed.
- Q2 now enters READY/RUNNING through normal PREPARE/START while TI
  reasserts Motor_Disarm, zero PWM and `mode=IDLE` every 10 ms. It does not
  invent a new completion message. MC02's existing question controller
  reports its own completion/fault internally; `ti_link.c` then sends the
  frozen `0x21 SAFE_STOP`, which ends TI's stationary participation. The TI
  fail-safe wait ceiling is 30 seconds so it does not race MC02's shorter
  task-level timeout. Q2 motor-disable and end-to-end SAFE_STOP remain
  hardware-unverified.
- Q3 uses the formal A-to-B centerline distance of 1500 mm as its route-state
  B crossing. It retains the five-fresh-sample gate and smooth 30/55 RPM
  launch, uses a 100 RPM straight-line-follow cruise, begins the existing
  slew-limited deceleration at 1250 mm, requests 45 RPM for the next 200 mm
  and 20 RPM for the final 50 mm, then disarms at 1500 mm. TI sets
  B_MARKER_SEEN, sends
  B_MARKER_PASSED before ROUTE_COMPLETE, and MC02 SAFE_STOPs the run. The
  nominal profile is designed for the <=8 s requirement but timing,
  wheel-slip and the exact physical reference point at B require floor
  calibration before acceptance.
- Q4 and Q5 reuse the exact Q1 lap/turn/A-stop state machine with a separate
  100 RPM requested base profile and 30-second ceiling. Q5's non-chassis
  target remains MC02-owned. This change does not modify or judge any ball
  controller. Q4/Q5 route timing and stop position are not yet hardware-
  verified.
- MC02 now selects K1-K5 through the same PREPARE/READY/START flow and uses a
  restored three-second countdown after READY; an intermediate unlogged
  version had reduced this to 200 ms. K2 completion uses the existing
  gimbal-app completion indication followed by V2 SAFE_STOP. K1/K3/K4/K5
  completion remains TI ROUTE_COMPLETE followed by MC02 SAFE_STOP.
- Clean verification passed. TI TEST_MODE/Debug uses 30,272 bytes text and
  2,480 bytes BSS; safe Release uses 20,552/2,266. MC02 clean Debug uses
  107,044 bytes FLASH and 14,320 bytes DTCMRAM; clean Release uses
  67,224/14,304. These are compile/static results only. Builds
  `2026073011`/`2026073012` have not been flashed in this session, and no
  Q2-Q5 movement, timing, B distance, UART completion or safety path is newly
  hardware-verified.
- Material chassis changes are in `chassis_config.h`, `test_mode.c`,
  `mc02_link.c` and `tools/verify_project.ps1`; MC02 sequencing changes are
  limited to `../gimbal_MC02/Core/Src/ti_link.c` and its build ID. Wiring/
  commissioning text is synchronized in `接线指导.md` and
  `../gimbal_MC02/docs/MC02_WIRING_CHECKLIST.md`. First test logic-only,
  then raised wheels in K1 -> K2 -> K3 -> K4 -> K5 order; calibrate Q3's
  nominal 1500 mm stop before a scored run.

- The root agent independently reran `tools/verify_project.ps1` immediately
  before authorizing initial hardware bring-up. SysConfig regeneration,
  TEST_MODE/Debug, safe Release, structural checks and ELF checks all passed.
  Fresh outputs retain `FW_BUILD_ID=2026073009` and the same reported sizes:
  Debug text 29,544/BSS 2,479 bytes; Release text 20,304/BSS 2,266 bytes.
  This authorizes logic-only testing with both motor rails disconnected, not
  powered chassis movement.

- TI build `2026073009` and MC02 build `2026073010` are the new
  compile-tested V2 integration baseline. The frozen
  `../gimbal_MC02/docs/MC02_TI_PROTOCOL_V2.md` envelope, message numbers,
  payload lengths, CRC, action numbers and button/question numbering were
  not changed.
- TI SysConfig now allocates all eight active-low Button X8 inputs with
  internal pull-ups: K1 PA10/BP34, K2 PA11/BP33, K3 PB13/BP35,
  K4 PB20/BP36, K5 PA31/BP37, K6 PA28/BP38, K7 PB12/BP19 and
  K8 PB15/BP17. All press/release events use independent 40 ms debounce and
  enter the `0xA2 BUTTON_EVENT` queue. K1-K5 remain selection events only;
  they cannot grant PWM authority without MC02 PREPARE and START. K6 is
  reserved.
- K7 and K8 remove both motor outputs locally on the first raw-low sample,
  within the 10 ms TI control task, before their debounced UART event. K8
  latches emergency stop. Releasing K8 cannot restart or clear it; after K8
  is released, a debounced K7 returns TI to stop/standby. Held K1-K7 at reset
  do not trigger until released; held K8 boots latched. These are source and
  compile results only; the physical eight-button wiring and true-zero PWM
  timing have not been measured.
- TI `0xA0 CHASSIS_STATUS` remains scheduled unconditionally every 50 ms in
  standby and motion. It now exposes all protocol state numbers including a
  minimum 60 ms `STOPPING=4` interval. `BUTTON_PANEL_OK` means the TEST_MODE
  GPIO scanner completed initialization, not an electrical continuity
  self-test. The B-marker flag is present but remains clear because no
  verified B detector exists. A is inferred only from the already
  floor-tuned Q1 route state (two-turn assistance plus encoder distance)
  entering the final 50 mm approach; it is not claimed as a raw transverse-
  line sensor detection.
- TI `0xA1` currently emits READY, STARTED, A_MARKER_DETECTED,
  ROUTE_COMPLETE, STOPPED, FAULT and EMERGENCY_STOPPED. It does not yet emit
  B_MARKER_PASSED. The A event is the route-state inference described above.
  Therefore seven of the eight event types are source-implemented; B remains
  unimplemented and hardware validation remains outstanding.
- Route status is deliberately not overstated. Internal Q1/printed
  requirement 2 is implemented using the existing tuned one-lap route and
  <=20 s supervisor timeout. Q2/printed requirement 3 is only kept safely
  stopped; the MC02 ball sequence is not implemented. Q3/printed requirement
  4 is not implemented because B detection/reporting is not complete.
  Q4 and Q5/printed requirements 5 and 6 are only partial in the sense that
  the proven Q1 lap controller can later be reused; their command paths and
  complete MC02 controllers are not connected and currently SAFE_STOP.
- MC02 recognizes and records all K1-K8 `0xA2` press/release packets. Only K1
  starts the Q1 PREPARE/READY/three-second-countdown/START sequence. K2-K5
  record `ti_unsupported_question_id` and command SAFE_STOP; K7 commands a
  normal stop/reset and K8 latches a TI-link/emergency stop. Protocol
  recognition is not five-question execution.
- Safety source review confirms unconditional SAFE_STOP before duplicate
  filtering, duplicate `(run_id, command_id)` START suppression, TI reset to
  `IDLE_DISARMED` with Motor_Disarm/true-zero outputs, and a 500 ms missing
  MC02 heartbeat stop/fault while RUNNING. None of K7, K8, K8-release/K7-
  clear, or the 500 ms unplug stop has yet been exercised on the assembled
  hardware. MC02's reciprocal 500 ms missing-TI-status stop is likewise
  compile-tested only.
- Full TI offline verification passed after a clean SysConfig regeneration:
  TEST_MODE/Debug text 29,544 bytes and BSS 2,479 bytes; safe Release text
  20,304 bytes and BSS 2,266 bytes. MC02 clean CMake Debug and Release builds
  also passed: Debug FLASH 99,164 bytes/DTCMRAM 13,464 bytes; Release FLASH
  62,436 bytes/DTCMRAM 13,448 bytes. No image from this change has been
  flashed and no UART, button, OLED, marker, route or motor behavior is
  newly hardware-verified.
- Material files for this integration are TI `mspm0dache.syscfg`, generated
  `ti_msp_dl_config.c/.h`, `test_mode.c/.h`, `mc02_link.c/.h`,
  `tools/verify_project.ps1` and `接线指导.md`; MC02
  `Core/Src/ti_link.c`, `Core/Inc/gimbal_app.h`,
  `Core/Inc/gimbal_config.h` and `docs/MC02_WIRING_CHECKLIST.md`.
  Next safe test is logic-only with both motor rails disconnected: verify
  idle 20 Hz status, every K1-K8 press/release event, then raised-wheel K7,
  K8 latch/K7 clear and deliberate heartbeat loss. Do not start Q2-Q5.

- Vision-owner follow-up reports the actual Jetson estimator now executes
  `x_ddot = -7.0 * alpha_pipe` and that V3 remains wire-compatible with no
  new fields. No TI V2 protocol or firmware change follows from this.
- Vision sign is still physically unverified. Before enabling any combined
  run, the ball must be placed at the motor end, O point and hinge end and
  the displayed x values confirmed positive, near zero and negative. The
  vision owner's reported `fitted=true` is described as a pseudo-calibration,
  so it must not authorize a gimbal closed loop until metric scale is checked
  using those known physical positions.
- MC02 build `2026073009` remains the integration baseline because it already
  contains the TI V2 endpoint. Jetson V3 work must preserve that endpoint;
  this follow-up makes no TI code change.

- Build `2026073006` adds the requested OLED question stopwatch without
  changing the floor-tested build `2026073005` steering, speed, turn or stop
  constants. The photographed four-pin `GM009605 v4.3`-class module is
  treated as a 128x64 SSD1306-compatible I2C display. It is powered from
  LaunchPad 3V3 and shares I2C1 PB2/PB3 with the Hiwonder sensor; firmware
  probes 7-bit address `0x3C` then `0x3D`. Initial clearing occurs only while
  motors are disarmed. Runtime digits are refreshed one six-column glyph per
  10 ms task instead of sending a framebuffer. Three consecutive OLED errors
  disable only the display; every line/motor safety path remains authoritative.
- The stopwatch starts at a debounced K1 press only when MC02 is online,
  includes the three-second supervisor countdown, displays integer seconds,
  and freezes on completion, cancel, SAFE_STOP, fault or link-loss stop.
  `g_test.timer` exposes timing/OLED telemetry while `g_test` remains the only
  TI Watch object. Only K1 and Q1 are physically/behaviorally implemented;
  K2-K8 remain unassigned.
- `STANDALONE_KEY1_START` is now 0. K1 no longer grants local PWM authority.
  TI requires the V2 heartbeat and PREPARE -> READY -> START sequence. MC02
  build `2026073009` implements the other endpoint at 115200 8N1 with queued
  nonblocking UART, CRC framing, 10 Hz heartbeat, Q1 retries, a three-second
  countdown, route-complete SAFE_STOP and 500 ms missing-status protection.
  Q2-Q5 IDs are recognized but deliberately SAFE_STOP because their buttons
  and complete question controllers do not exist.
- Wiring documents now include OLED VDD->TI 3V3, GND->GND and shared
  SCK/SDA->PB2/PB3, plus MC02 TX/PA9->TI RX/PA9, TI TX/PA8->MC02 RX/PA10
  and common GND. Controller power rails must never be joined. Isolate the
  TI J101 XDS110 UART routes before joining the boards.
- Full TI offline verification passed for `2026073006`: TEST_MODE uses
  28,008 bytes text and 2,394 bytes BSS; safe release uses 19,720/2,243.
  MC02 clean Vehicle and Debug builds passed for `2026073009`; Vehicle uses
  61,804 bytes FLASH and 13,424 bytes DTCMRAM, Debug uses 98,748/13,432.
  These are compile/static results only. OLED controller/address/orientation,
  shared-bus timing, UART electrical behavior, end-to-end Q1 and both 500 ms
  watchdog paths remain hardware-unverified. First test with both motor rails
  disconnected, then with raised chassis wheels.

- The vision owner's new `../gimbal_MC02/下位机协议.md` reports Jetson V3
  framing/CRC/heartbeat/40-50 Hz vision streaming implemented and a measured
  field of view sufficient for the full +/-125 mm pipe range. This is an
  owner report, not an MC02-side execution test. V3 layout remains unchanged.
- Two suggested MC02->Jetson fields (pipe angle and chassis acceleration)
  are not adopted into V3. Adding them changes the 24-byte `0x81` payload
  and would require a separately specified V4. MC02 can initially provide
  only estimated pipe angle from motor/linkage mapping and estimated chassis
  acceleration from TI wheel-speed differentiation, each with validity.
  Neither is needed for the first stationary loop. TI V2 is unchanged.
- The Jetson handoff's later `x_ddot=+7*theta` and positive inverse-dynamics
  expression conflict with the frozen signs when theta is physical pipe
  angle; the running `main.cpp` is reported to use the correct negative
  process sign. Do not copy the reference controller expressions into MC02
  until the text/signs are corrected.

- The user reports the MC02/gimbal assembly is now installed on the vehicle.
  Normal USB ownership is Jetson host -> MC02 Type-C; PC USB is service-only
  and cannot remain attached simultaneously. Jetson supplies V3 vision and
  supervisor data, while MC02 must retain motor control, limits, watchdogs
  and fail-safe authority. Raw Jetson motor-angle/enable/CAN control remains
  prohibited.
- Installation is physical only: MC02 build `2026073008` does not yet parse
  Jetson V3 or run the ball loop, and MC02/TI is not end-to-end operational.
  No TI code, wiring or readiness claim changes from this installation.

- Visual recheck of the MC02 V1.1 manual corrected the integrated
  OUT1/CAN1 2+2 table: its keyed front-view order is 1 VCC, 2 GND, 3 CAN_H,
  4 CAN_L. The separate two-pin CAN1 remains 1 CAN_L, 2 CAN_H. The user's
  already-working keyed cable must not be repinned. This corrects
  documentation only and changes no TI wiring or firmware.
- MC02's single USB Type-C device port is shared in time between the PC USB
  commissioning console and the production Jetson V3 link; it cannot serve
  two USB hosts simultaneously. TI remains on the separate MC02 USART1
  3.3 V TTL pins PA9/PA10 plus GND. No power rail is connected across that
  UART link.

- Phone-level measurement at MC02 motor +/-45 degrees found actual pipe-tilt
  magnitude between 1 and 2 degrees, closer to 2 degrees in both directions.
  Use the provisional mapping `motor +/-45 -> pipe -/+ approximately 2 deg`
  (signed average gain about -0.044) until vision closed-loop refinement.
- A +/-2-degree pipe can cancel only about 0.34 m/s^2 chassis acceleration.
  With 65 mm wheels, 100 RPM/s already consumes essentially that full
  authority, so the older 100-120 RPM/s proposal is superseded. For the first
  integrated balance test, provisionally target symmetric 50-60 RPM/s
  acceleration/deceleration slew to retain correction margin. No TI source
  value has been changed yet.
- MC02 open-loop mechanism commissioning is complete at the +/-45 motor limit.
  Next work is stationary Jetson/MC02 vision closed loop; do not integrate
  the chassis until that passes.

- Physical pipe angle is now defined unambiguously as
  `alpha_pipe = atan2(h_motor - h_hinge, L)`: motor end above level is
  positive and motor end below level is negative. The installed mapping is
  therefore positive MC02 motor offset -> negative pipe angle. This is a
  gimbal-coordinate clarification and changes no TI code.
- The user inadvertently ran CALRUN with the ball installed. When the motor
  end first lowered, the ball rolled to the physical left end and stayed
  there. CALRUN is open-loop and the current MC02 build has no vision
  controller, so this is not a balancing test. A follow-up manual/unpowered
  check confirmed the ball can freely leave the left end when the pipe is
  tilted; no persistent end or channel obstruction was found. Do not repeat
  CALRUN with the ball present.
- Ball position must use a fixed spatial sign, not a sign that changes with
  the downhill direction. The user confirms the motor end is the physical
  left end, so the frozen convention is `x_ball>0` from O toward the
  motor/left end and `x_ball<0` toward the hinge/right end. Because a wrong
  sign reverses closed-loop action, Jetson/MC02 is promoted to protocol V3
  with on-wire version `0x03` and a regenerated CRC vector; packet layouts
  are otherwise unchanged. MC02/TI remains V2 with no field, byte, CRC,
  wiring or behavior change.

- MC02 Debug build `2026073008` has now passed the corrected motor-angle
  telemetry test. `CALRUN 4310 30` reached approximately +/-523 mrad and
  returned to zero; `CALRUN 4310 45` reached +784/-785 mrad and returned to
  zero. Endpoint error was about 0-1 mrad, with no logged fault or aborted
  leg. This validates MC02 motor-angle execution, not yet the pipe contact:
  the user has not explicitly reported whether the non-captive follower
  stayed in contact throughout the real +/-45 sweep.
- No TI code or slew changed. Do not integrate the chassis or install the
  ball for open-loop motion yet. First confirm continuous contact and measure
  signed pipe-end vertical displacement at level and both +/-45 motor
  endpoints to freeze the actual motor-to-pipe mapping.

- Complete MC02 serial logs correct the earlier range-test interpretation.
  Build `2026073006` printed +/-30-degree targets and build `2026073007`
  printed +/-45-degree targets, but both actually ended at only
  `OFF=+/-174-175 mrad`, exactly +/-10 degrees. A second execution-time clamp
  remained at the normal desktop limit. Thus neither expanded range has
  passed hardware testing; the observed normal contact applies only to
  +/-10 degrees.
- MC02 build `2026073008` is compile-tested with one shared active-limit
  selector used at both command admission and motion execution. Debug-only
  CALRUN may use +/-45, while ordinary commands remain +/-10 and Vehicle
  still disables CALRUN. Speed, motor acceleration parameters, watchdogs and
  power cutoffs did not change. A valid next test must report final offsets
  near +/-785 mrad. No TI source, protocol, wiring or chassis slew changed,
  and integration remains blocked pending that real +/-45 contact test.

- The installed pipe merely rests on the MC02/DM4310 eccentric follower and
  is not captive. The user requires follower downward acceleration to remain
  below `g`; continuous contact is now a mandatory integration constraint.
  Based on the recorded motor acceleration parameter `2`, 10:1 reduction and
  15.45 mm crank, the present MC02 setting is estimated at about
  3.09 m/s^2 (`0.32 g`) maximum tangential follower acceleration, before
  physical validation. Do not raise acceleration during later chassis
  integration without rechecking this constraint.
- MC02 build `2026073006` is compile-tested for the next standalone,
  ball-free mechanism test. Only Debug CALRUN expands from +/-10 to
  +/-30 motor degrees and from three to five seconds per leg; its velocity
  remains 0.20 rad/s. Ordinary MC02 JOG/STEP/SWEEP remain +/-10, and Vehicle
  still disables CALRUN. No TI source, TI slew, protocol or wiring changed.
  The next command is `CALRUN 4310 30`; MC02-TI integration remains blocked
  until continuous contact and safe motion are physically confirmed.

- The uninterrupted MC02 CALRUN now confirms installed gimbal sign:
  positive motor offset lowers the motor-side pipe end and negative offset
  raises it. The current +/-10 motor-degree test corresponds theoretically
  to only +/-2.68 mm lift and +/-0.62-0.65 degrees pipe tilt for the frozen
  15.45 mm crank and installed `L=236-248 mm` range. That is adequate as a
  stationary-ball starting authority but cancels only about
  0.106-0.111 m/s^2 chassis acceleration, roughly 31-33 wheel RPM/s.
- The small first motion does not by itself require a mechanical redesign:
  ideal motor ranges +/-30, +/-45 and +/-60 degrees produce about
  +/-1.78-1.88, +/-2.52-2.65 and +/-3.09-3.25 degrees pipe tilt. A future
  ball-free expansion toward +/-45 motor degrees plus TI deceleration reduced
  toward 100-120 RPM/s is the current integration target. The existing
  250 RPM/s braking slew exceeds even this mechanism's absolute
  +/-3.57-3.75-degree pipe authority and cannot remain for balanced braking.
  No MC02 clamp or TI slew value has been changed yet.
- A repeated MC02 build `2026073005` CALRUN test showed green only during the
  first leg and visually red during the three later legs, while movement
  remained normal. Fresh STATUS captures during red CAL phases 4 and 6 both
  showed `STATE=STEP`, `FAULT=0`, `ONLINE=1`, `ERR=1`, age 0, zero TX error
  and actual position equal to command. DM-J4310 manual pages 10/13 define
  ERR 1 as enabled/green and ERR 0 as disabled/red, so the observed color
  conflicts with the manual but CAN proves the drive was enabled. The user
  also confirms high static friction, withdrawing the prior gravity/inertia
  explanation for sustained movement. No MC02 or TI code change follows;
  future logic must trust fresh CAN state/watchdogs rather than LED color.
- MC02 build `2026073005` passed the complete installed, ball-free
  `CALRUN 4310 10` hardware sequence. Feedback became ready about 1.069 s
  after power-on; the motor reached +0.174 rad, returned to zero, reached
  -0.174 rad, and returned to zero with the reported final offset 0 mrad.
  All four legs completed their bounded three-second intervals, physical
  behavior was reported normal, and MC02 disabled torque and OUT1 at
  completion. The user also noticed apparent disable while rotation
  continued; MC02 deliberately disables at each JOG-derived leg boundary,
  which is not mechanical braking, and the next leg starts immediately.
  Whether any apparent disable occurred mid-leg remains unverified. This
  successful standalone gimbal test changes no TI firmware and does not
  authorize MC02-TI physical integration yet.
- MC02 build `2026073004` was hardware-tested with the new single
  `CALRUN 4310 10` command. OUT1 energized and the motor briefly showed
  green, but the command aborted at its exact 5-second preflight timeout
  without motor feedback and powered OUT1 off; a second command was denied
  because that timeout had latched the MC02 feedback fault. No commanded
  motion occurred. Source review found that only the five initial
  torque-disable frames were sent while a cold motor could still be
  starting, after which CALRUN waited without the repeated 20 ms disable/
  feedback requests already used by OBSERVE.
- MC02 build `2026073005` fixes only that standalone calibration preflight:
  it repeats torque-disable frames at 20 ms, waits up to 15 seconds, and
  returns a plain no-feedback timeout to SAFE_IDLE after power-off so the
  same command can be retried. Valid motor faults and CAN transmit failures
  still latch fault and all STOP/reset/watchdog cutoffs remain. The
  pre-fix build is backed up at
  `../gimbal_MC02/backups/gimbal_MC02_BUILD_2026073004_PRE_READYFIX.zip`
  (1,704,872 bytes, SHA-256
  `40D4E107E08E49CC8ECE62913998AE5F37DEF627B5BF3ED7BB5F8484B49E6F99`).
  Clean Debug and Vehicle builds passed without warnings at
  FLASH/DTCMRAM 77,972/12,240 and 52,380/12,232 bytes respectively.
  Build `2026073005` has not yet been flashed or hardware-tested and changes
  no TI code, button behavior, chassis behavior or V2 interface.
- MC02 build `2026073004` is compile-tested for a temporary, ball-free
  installed-gimbal calibration. It adds one manual USB command,
  `CALRUN 4310 deg`, that performs guarded power-on, a 3-second countdown,
  level-zero capture, `+deg -> 0 -> -deg -> 0`, and final OUT1 power-off
  while retaining the existing +/-10 degree, 0.20 rad/s, 3-second-per-leg,
  feedback-watchdog and unconditional STOP protections. The pre-change MC02
  build `2026073003` is backed up at
  `../gimbal_MC02/backups/gimbal_MC02_BUILD_2026073003_PRE_CALRUN.zip`
  (978,160 bytes, SHA-256
  `45BC85E880757BA28AE4479685C3FF2C7DA6CB85007BBBFA0505A841F4AEDCBE`).
  The manual path is controlled by `GIMBAL_ENABLE_MANUAL_CALRUN`; it must be
  0 before vehicle use. The MC02 project now provides a dedicated CMake
  `Vehicle` preset that sets `GIMBAL_MANUAL_CALRUN=OFF`; use it rather than
  relying on an environment CFLAGS override. Clean Debug and Vehicle builds
  both passed without warnings. Debug uses 77,772 bytes FLASH and 12,240
  bytes DTCMRAM; Vehicle uses 52,208/12,232 bytes. ELF inspection confirmed
  the active `CALRUN START` string exists only in Debug and the Vehicle image
  contains the disabled reply instead. MC02 build `2026073004` has not yet
  been flashed or hardware-tested. This change does not alter TI firmware,
  chassis behavior, button mapping or the frozen V2 MC02-TI protocol.
- The user floor-tested build `2026073004` and reported that its start,
  tracking, turn assistance and stop behavior were all satisfactory. The
  only requested change is to move the stop point 30 mm earlier. Build
  `2026073005` therefore changes only the two distance constants:
  turn-assisted final approach begins at 5,970 mm instead of 6,000 mm, so
  the unchanged 50 mm approach targets about 6,020 mm; the independent
  fallback moves from 6,100 mm to 6,070 mm. All start, steering, speed, turn,
  timeout, button, MC02-retention and safety logic remains unchanged. Build
  2026073005 has not yet been flashed or floor-tested. Full offline
  verification passed: TEST_MODE uses 25,312 bytes of text and 2,329 bytes
  of BSS; safe release uses 17,024 bytes of text and 2,178 bytes of BSS.
- Build `2026073003` was floor-tested. The user reported two new observations:
  the finish bar may show only two active Hiwonder channels, and a start from
  the same transverse bar again accelerated, steered right and stopped after
  losing the line. This supersedes any assumption that a two- or
  three-channel finish signature is repeatable enough to own stopping.
- Build `2026073004` removes grayscale-channel count from finish authority.
  Starting from the transverse bar, the first 100 mm uses equal left/right
  targets under the existing slow-start and slew limits while refreshing,
  but not acting on, the line-position filter; centroid steering therefore
  resumes without carrying the bar's initial bias. For stopping, signed
  encoder wheel-distance difference
  estimates lap yaw privately; thresholds of 100 degrees after 1,300 mm and
  250 degrees after 4,500 mm expose an advisory `line.turn_count` of 1/2.
  When turn 2 is present, 6,000 mm begins the existing 50 mm slow approach,
  targeting about 6,050 mm. Turn count is not a hard requirement: if it is
  unavailable, the independent 6,100 mm fallback stops and disarms. The
  debugger-writable telemetry cannot forge this gate because runtime logic
  uses private `s_turn_count`.
- Power-sequence guidance was clarified. With a common ground already
  connected and no active motion request, powering the TI/control board
  first and the motor driver rail second does not require a code change and
  is preferred because `Motor_Init()`/`Motor_Disarm()` force PWM/DIR low.
  Power the driver before pressing KEY1; applying driver power during the
  countdown or an active run can cause an abrupt late enable and encoder
  supervision fault. Power-down order should be motor-driver rail first,
  then the TI board. This guidance assumes correct rail voltages, polarity,
  grounding and no unresolved LM2596 arc or loose-strand fault.
- Full offline verification passed for build `2026073004`: clean TEST_MODE
  and safe-release builds linked; the 100 mm symmetric start, private
  encoder-yaw turn estimator, advisory 6,000+50 mm finish, independent
  6,100 mm fallback, standalone KEY1 path, retained MC02 source and disabled
  Bluetooth checks all passed. TEST_MODE uses 25,312 bytes of text and
  2,329 bytes of BSS; safe release uses 17,024 bytes of text and 2,178 bytes
  of BSS. This is compile/static evidence only; build 2026073004 has not yet
  been flashed or floor-tested.
- Formal-course replay supplied a new hardware observation: at the physical
  finish line only three Hiwonder LEDs were simultaneously active for a brief
  instant. Build `2026073003` supersedes the four-channel finish criterion.
  A candidate now requires at least three adjacent active channels with
  `abs(position_x1000) <= 1500`, and it is accepted only after three
  consecutive clear samples inside a narrowed 6,000-6,300 mm encoder window;
  an early candidate resets that clear count. The missed-marker
  fallback moves from 6,600 mm to 6,300 mm; the configured 50 mm slow advance
  after a detected marker is unchanged. This avoids treating a three-channel
  indication carried into the window from the last curve as the finish.
- The user's reported late stop was about 300-400 mm beyond the desired
  position. The old 6,600 mm fallback was 458.4 mm beyond the 6,141.6 mm
  geometric lap, so a missed four-channel marker is consistent with the
  observation. This is a reasoned correlation, not a direct capture of the
  encoder value at the line. Build 2026073003 has not yet been flashed or
  floor-tested; record `lap_distance_mm`, `active_count`, `state`, final
  `status` and physical stop offset on its next run.
- Full offline verification passed for build `2026073003`: clean TEST_MODE
  and safe-release builds linked; exact three-channel/adjacency/centroid,
  three-consecutive-clear-sample, 6,000-6,300 mm window and 6,300 mm fallback checks
  passed along with the existing formal-course, standalone KEY1, fresh-start,
  MC02-source-retention and disabled-Bluetooth checks. TEST_MODE uses
  25,296 bytes of text and 2,327 bytes of BSS; safe release uses 17,016 bytes
  of text and 2,170 bytes of BSS. This is compile/static evidence only; build
  2026073003 has not been flashed or floor-tested.
- The user explicitly suspended MC02-supervised starting for the current
  chassis tests. Build `2026073002` sets `STANDALONE_KEY1_START=1`: a new
  debounced PA10/KEY1 press starts the existing 3-second countdown, requires
  fresh line-sensor samples before arming, then runs the formal-course line
  follower for at most 20,000 ms. A second press during the countdown cancels
  it; a press while running stops and disarms. The MC02 UART/protocol source
  remains compiled for later integration but is not required for this local
  start path.
- This is not the complete competition firmware. The TI-to-MC02 communication
  block has not been physically connected or end-to-end hardware-tested, and
  K2-K8 inputs are still unimplemented. Do not record the retained,
  compile-tested MC02 parser as an operational communications link. When the
  user asks to resume MC02 integration, set `STANDALONE_KEY1_START=0` and
  revalidate PREPARE/READY/START, heartbeat, event, status, CRC, timeout and
  SAFE_STOP behavior on the actual two-controller connection before granting
  it competition authority.
- Full offline verification passed for build `2026073002`: clean TEST_MODE
  and safe-release builds linked; the standalone KEY1 start selection,
  20,000 ms local lap timeout, fresh-sensor launch gate, formal-course
  configuration, retained MC02 symbols/CRC, disabled Bluetooth path and ELF
  symbols all passed. TEST_MODE uses 25,200 bytes of text and 2,326 bytes of
  BSS; safe release uses 17,016 bytes of text and 2,170 bytes of BSS. This is
  compile/static evidence only: it has not been flashed or hardware-tested,
  and it does not establish an MC02 physical communications link.
- After flashing/testing the current supervisor-controlled behavior, the user
  reported that pressing KEY1 produced no chassis response. This is expected
  for the superseded build 2026073001 rather than evidence of a failed button:
  PA10/K1 was intentionally event-only, and chassis motion required an MC02
  heartbeat followed by PREPARE/READY/START. Build 2026073002 changes this
  only because the user explicitly requested the temporary standalone test
  path above.
- The user moved chassis testing from the MINI oval to the formal competition
  field. The supplied field geometry was rechecked as two 1,500 mm straights
  and two radius-500 mm semicircles. Nominal centerline length is
  `2*1500 + 2*pi*500 = 6141.6 mm`; the outer envelope is approximately
  2,500 x 1,000 mm. With the configured 65 mm wheels, a constant-speed
  20-second lap requires about 90.23 RPM before launch, curve and finish
  allowances.
- Build `2026073001` replaces the MINI-course settings. Formal cruise is
  120 RPM, formal curve floor 100 RPM, while the verified-safe staged launch
  remains 30 RPM for the first second, at most 55 RPM for the first 300 mm,
  and 80 RPM/s acceleration slew. Ideal constant-speed lap time at 120 RPM is
  about 15.35 seconds. A conservative target-speed simulation that assigns
  the full semicircle distance to the 100 RPM floor, includes the staged
  launch and includes the final 50 mm gives about 18.5 seconds. This is
  calculated margin, not yet measured on the formal floor.
- Finish recognition now uses the user's video evidence: ordinary turns
  showed at most 2-3 black channels, while the finish bar briefly showed
  four. It therefore requires four active channels and latches a single
  10 ms sample without centroid rejection. Encoder travel remains an
  independent plausibility/fallback layer, initially widened to
  5,200-6,600 mm around the 6,141.6 mm nominal centerline; after a marker it
  still advances 50 mm at 20 RPM. The window is intentionally wider because
  the front-mounted sensor and rear drive axle trace different paths through
  the semicircles. Record `g_test.line.lap_distance_mm` at the first actual
  finish crossing before narrowing it. No hardware result is claimed yet.
- Full offline verification passed for build `2026073001`: clean TEST_MODE
  and safe-release builds linked; the exact 1500 mm/500 mm/6141.6 mm formal
  geometry, 120/100 RPM profile, four-channel zero-debounce finish latch,
  5200-6600 mm distance layer, existing fresh-sensor launch gate, MC02 UART
  and canonical CRC checks all passed. TEST_MODE text/BSS remains
  25,128/2,326 bytes and safe release remains 17,016/2,170 bytes. The build
  has not been flashed or floor-tested.
- The user froze `PITCH_NEW_RPLUS7_R3_CONTAINED.STL` as the final printed
  gimbal mechanism, superseding the 25/50 mm recommendations. A hash-verified
  backup is at
  `../gimbal_MC02/mechanical/final/PITCH_FINAL_R15P45_R3_20260730.STL`;
  size is 768,984 bytes and SHA-256 is
  `010919C1A376811A5C7BEDD237305554FC9CF96A3D4154B04B5BBF57FD02E3F5`.
  Future MC02 mapping must use the measured `r=15.45 mm`; final `L`, neutral
  motor angle, installed sign and safe endpoints remain installation
  measurements. No TI or MC02 firmware changed.
- Mechanical range was re-evaluated after the user correctly noted that a
  smaller crank improves transmission resolution and reduces required motor
  torque. The earlier 50 mm target existed only to guarantee an unnecessarily
  conservative +/-10-degree pipe range. The revised preferred geometry is
  `r=25 mm`, `L=220-230 mm`, and about +/-60 degrees motor travel, producing
  about +/-5.4 to +/-5.65 degrees usable pipe motion. Move the latest model's
  follower only 9.55 mm farther outward from its measured 15.45 mm offset.
  A future integrated profile should reduce the current 250 RPM/s chassis
  deceleration slew to roughly 100-120 RPM/s; neither firmware change has
  been implemented or hardware-tested.
- `PITCH_NEW_RPLUS7_R3_CONTAINED.STL` was inspected as the latest proposed
  gimbal crank. It is a valid connected/watertight mesh with a diameter-49
  base, diameter-12 integral follower, retained diameter-3.2/PCD-27 motor
  holes, and an actual follower-axis eccentricity of only 15.45 mm. This
  yields only about 4.92/4.03/3.85 degrees maximum pipe tilt at
  `L=180/220/230 mm`, so it remains an intermediate part rather than the
  final competition geometry. Move the follower center another 9.55 mm
  outward to reach the revised 25 mm center-to-center eccentricity; no TI or
  MC02 firmware changed.

### 2026-07-29

- Competition mechanical rule changed: midpoint seesaw support for the pipe
  is prohibited. The gimbal/DM4310 must actuate one end while the opposite
  end uses a fixed pillar and hinge; the earlier midpoint mounting baseline
  is superseded.
- The preferred gimbal-end mechanism is an eccentric crank with a transverse
  roller or captive short link, rather than a collinear loose-contact plate.
  This creates a clear one-degree-of-freedom mechanism and a calibratable
  reduction ratio. Final calibration awaits hinge-to-drive distance `L`,
  eccentric radius `r`, and the required pipe tilt range.
- This mechanical change does not alter the TI chassis controller's current
  line-following responsibility or UART protocol, but installation testing
  must redetermine pipe zero, motion sign, usable tilt limits, and the mapping
  from DM4310 output angle to pipe angle.
- Preliminary mechanical values supplied by the user are `L=180-230 mm`,
  20 mm pipe outside diameter, and less than 20 degrees maximum allowable
  pipe tilt. Current recommendation is to place the drive point near
  `L=220-230 mm`, use an adjustable 20/25/30/35 mm crank, commission at
  `r=25 mm`, and limit the initial actual pipe command to about +/-5 degrees.
  The current MC02 +/-10-degree motor test clamp is not the final pipe-angle
  range and must be remapped only after the mechanism and hard stops exist.
- Subsequent moving-chassis analysis supersedes `r=25 mm` as the final
  competition baseline. Existing 65 mm wheels and 80/250 RPM/s acceleration
  and deceleration slew limits correspond to about 0.27/0.85 m/s^2 and
  1.6/5.0 degrees equivalent longitudinal tilt. A +/-5-degree mechanism has
  essentially no braking correction reserve. The revised recommendation is
  `r=45 mm`, adjustable holes at 25/35/45/50 mm, approximately +/-10 degrees
  usable pipe travel, and about +/-12 degrees mechanical clearance at
  `L=220-230 mm`.
- A new DM4310 output attachment CAD render shows a circular plate with a
  long cylindrical pipe-support rod intended as the eccentric follower.
  Its actual center offset, final mounting heights, zero phase, and captive
  contact method remain unverified. A centered rod cannot generate lift.
  Low-level motor/CAN and both V2 communication packet formats are unchanged;
  MC02 must add the calibrated desired-pipe-angle to motor-angle mapping after
  final measurements are available.
- Direct inspection of the supplied `零件2.STL` (assuming millimetres) found
  a closed single mesh comprising an approximately diameter-35 x 5 mm plate
  and an integral diameter-7 x 70 mm exposed rod. The rod-axis eccentricity
  is 13.995 mm. At `L=180-230 mm`, this permits only about 4.46-3.49 degrees
  maximum pipe tilt, below the moving-chassis correction reserve. Final code
  mapping is deferred pending a revised mechanism, preferably about 45 mm
  eccentricity and a stronger metal follower shaft.
- User confirmed the current 14 mm-eccentric STL is already printing and will
  not be revised. The pipe will rest directly on the integral rod about 12 mm
  from the plate face; motor and hinge axes will be nearly parallel and were
  planned approximately level. Since pipe radius plus rod radius is about
  13.5 mm, equal axis height may put horizontal neutral near the crank's lower
  dead point unless the hinge pivot is similarly offset below the pipe
  centerline. Final assembly must resolve this geometry before MC02 mapping is
  coded. With the retained crank, prefer `L` near 180 mm, about +/-4 degrees
  practical pipe range, and a lower TI deceleration slew than 250 RPM/s.
- The user then allowed a redesign/reprint, superseding the retained-14 mm
  fallback. The original STL mounting interface was measured as four roughly
  3.2 mm holes on a 27 mm pitch circle (`+/-6.75`, `+/-11.69` mm relative
  coordinates). Preserve that interface but use a reinforced adjustable
  crank with 35/45/50 mm holes, `r=50 mm` initially, a steel follower pin,
  and `L=220-230 mm`. This provides roughly +/-10 degrees usable pipe travel
  without requiring the crank to operate at its dead point.
- Direct inspection of the user's thicker-rod `零件2(1).STL` found a
  diameter-12 x 70 mm integral printed rod but only 11.495 mm eccentricity,
  so it is also unsuitable as the final competition crank. With user
  authorization, the MC02 project now contains a generated and mesh-validated
  `mechanical/DM4310_CRANK_R50_M8_V1.stl`: 50 mm eccentricity, 10 mm
  thickness, preserved 27 mm-PCD motor holes, and an 8.2 mm hole for a steel
  follower pin. This is unverified CAD/mesh output, not evidence of printed
  fit or loaded hardware performance; no TI or MC02 firmware changed.
- The user rejected the generated R50 STL's physical form and will revise the
  original round-hub/integral-cylinder model themselves. The generated STL
  is superseded reference only and must not be treated as the selected part.
  Retain the underlying requirement of approximately 50 mm rod-axis
  eccentricity and reinforced support geometry.
- The user identified the intended balance-rail/gimbal controller as the
  DAMIAO `DM-MC-Board02` motor development board and supplied its V1.1 user
  manual dated 2025-07-23. The manual specifies an STM32H723VGT6 controller,
  12-24 V input with 6S support, three CAN-FD interfaces, three UARTs, USB,
  an onboard BMI088, two buttons, and a 1.69-inch 240x280 LCD expansion
  module. These are document-confirmed capabilities; the physical board
  revision, installed LCD/key module, wiring, firmware, and powered behavior
  have not yet been inspected or verified.
- The user subsequently confirmed the selected balance controller as the
  DAMIAO `DM-MC-Board02` (MC02) and the rail actuator as the integrated-drive
  `DM-J4310-2EC V1.2`. Its supplied V1.2 manual dated 2025-11-21 documents
  24 V and 48 V variants, a 10:1 reducer, 3.5 N m rated/12.5 N m peak output
  torque, dual 16-bit single-turn magnetic encoders, standard CAN at 1 Mbps,
  query-style position/speed/torque/temperature feedback, and MIT,
  position-speed, speed, and force-position hybrid modes. The CAN ID/Master
  ID, stored mode, firmware, bootloader, timeout setting, and powered behavior
  remain unverified and must be read from the commissioning tools rather than
  inferred.
- The user has now identified the installed `DM-J4310-2EC V1.2` as the 24 V
  variant. The intended supply is one DJI 6S model battery. The MC02 and motor
  are not yet using any other board interfaces; the MC02 official examples
  can be built/downloaded with J-Link, and the user has the DAMIAO serial
  tuning interface/application. Mechanically, the motor output shaft drives
  the gimbal plate directly and the water pipe is mounted on that plate.
  Maximum rail angle, hard stops, loaded center of gravity, measured battery
  voltage, motor configuration, and powered motion remain unverified.
- A user screenshot from DAMIAO tuning assistant V1.6.4.0 confirms a
  successful UART parameter read at 921600 8-N-1. The motor currently reports
  position-speed mode, 14 pole pairs, 10:1 ratio, CAN ID `0x01`, Master ID
  `0xF1`, CAN timeout `0` (disabled), 15 V undervoltage, 32 V overvoltage,
  100 deg C temperature protection, acceleration/deceleration `2/-2`, speed
  limit about 188.35, normalized overcurrent `0.8`, PMAX/VMAX/TMAX
  `12.5/30/10`, current bandwidth 1000, speed KP/KI about
  `0.003894209/0.01570796`, and position KP/KI `54/0`. Firmware and bootloader
  revisions are still blank because the separate version-read result was not
  captured.
- Before the first CAN-controlled motion, the recommended conservative
  persistent edits are CAN timeout `2000` (100 ms because one count is
  50 us), undervoltage `19 V` for the 6S battery, and normalized overcurrent
  `0.30` (about 6 A phase current from the documented 20 A peak scale).
  Retain CAN ID `0x01`, Master ID `0xF1`, position-speed mode, 32 V
  overvoltage, 100 deg C temperature limit, all identified motor constants,
  mapping values, acceleration/deceleration, and vendor loop gains for the
  first test. These are proposed settings only; no screenshot has yet
  verified that they were written and read back. Any later current increase
  must follow measured MC02 output-bus current and temperature; do not exceed
  normalized `0.49` (9.8 A phase) without a revised, documented safety basis.
- The user then wrote and read back the three proposed values successfully:
  CAN timeout `2000`, 6S undervoltage `19 V`, and normalized overcurrent
  `0.30`; the remaining displayed settings stayed unchanged. Immediately
  afterward the motor reportedly began flashing red. The exact ERR code has
  not yet been read. Because MC02 is not yet sending periodic CAN commands,
  communication-loss error `D` caused by the newly enabled 100 ms timeout is
  the leading hypothesis, not a verified diagnosis; undervoltage `9` and
  other faults remain possible. Safe next action: keep the motor unloaded and
  disabled, cold-power-cycle it, measure battery/motor-bus voltage, and
  observe whether the LED is steady red before any enable. If flashing
  persists, temporarily restore CAN timeout `0` via UART and cold-cycle again
  only as a diagnostic/configuration measure. If that clears the flashing,
  the timeout diagnosis is confirmed; restore `2000` only when MC02's
  periodic CAN command loop is running. If it does not clear, read the ERR
  nibble through CAN feedback before clearing faults or changing any other
  parameters. No motor motion is authorized by this diagnostic.
- The 100 ms value is the driver's internal timeout, not a blink interval
  that the operator must visually time. Diagnose by comparing the stable LED
  state after separate cold starts with CAN timeout `2000` and temporarily
  `0`, or preferably by reading the CAN feedback ERR nibble; do not rely on
  seeing the instant at which flashing begins.
- The user performed the timeout `2000` versus `0` comparison and reported
  that it basically confirms CAN communication loss as the cause of the red
  flashing. This verifies the causal timeout behavior sufficiently for the
  current commissioning step, although ERR nibble `D` has not yet been
  captured directly. Keep CAN timeout at `0` while MC02 has no periodic CAN
  command loop; restore `2000` only after MC02 transmits valid commands
  continuously, then verify both normal operation and deliberate CAN-unplug
  fail-safe behavior without the rail/water pipe installed.
- The MC02 V1.1 manual explicitly lists 12-24 V input with 6S support. Its
  two controllable XT30(2+2) power outputs are raw input-bus paths switched
  by PMOS and are each specified for 5 A continuous output. The 24 V J4310
  manual lists about 3.1 A rated supply current but 16.5 A peak supply
  current. Therefore one 6S battery may supply both devices and the matching
  MC02 output can plausibly power one J4310 for current-limited commissioning,
  but the documentation does not establish that the MC02 output can carry
  the motor's full peak/stall demand. Do not claim full-torque suitability
  until branch current and connector/MOSF temperature are measured. For a
  final system that needs more than 5 A, split the fused battery bus into
  separate MC02 and motor branches and retain MC02 only for CAN/control, or
  use a separately rated external motor power switch.
- The recommended competition actuator path is
  `Jetson -> DM-MC-Board02 -> DM-J4310-2EC`, not Jetson direct-to-motor.
  Jetson should supply vision-derived state and/or a bounded rail-angle
  command; MC02 should own deterministic angle/slew limiting, the actual
  rail-angle sensor, CAN command/feedback, enable/disable, watchdogs, limit
  handling, and safe fallback. Direct Jetson-to-motor CAN is technically
  possible only through a suitable CAN controller/transceiver or USB-CAN
  interface, and is reserved as a bench/debug option because Linux/camera
  load and process failure would otherwise share the final actuator safety
  path. No MC02 firmware, wiring, CAN parameters, or Jetson protocol has been
  changed yet.
- The Jetson/vision handoff is not yet frozen. MC02 needs the vision owner to
  confirm the physical transport and logic levels, baud rate, exact binary
  framing/endianness/CRC, stable and worst-case update rate/latency/jitter,
  monotonic timestamp semantics, pipe-axis coordinate origin/sign/units and
  calibrated range, ball-velocity definition/filter delay, confidence and
  invalid/occluded/out-of-range behavior, startup/ready/heartbeat behavior,
  and which side owns the outer ball controller. The recommended authority
  split is that Jetson publishes `frame_seq`, capture timestamp, ball
  position, ball velocity, validity/confidence, and optionally a proposed
  feed-forward rail angle; MC02 owns the final ball-to-angle controller,
  angle/slew clamps, stale-data timeout, and all motor authority. MC02 should
  send task/mode, run ID, START/STOP/RESET, and acknowledgements back. A
  versioned binary 3.3 V UART packet at 460800 or 921600 baud with length,
  sequence, timestamp, flags, and CRC16 is the current proposal, not a
  confirmed interface.
- The user now selects USB serial for Jetson-to-MC02 transport. Vision is
  currently conceived in pixels; the camera-to-rail pose is not fixed, the
  whole pipe should be observed, clockwise rail rotation is positive, and
  ball motion toward the end that becomes lower under positive rotation must
  be defined positive. Ball velocity is required because position alone
  cannot stop a moving ball. The YOLO path is currently about 50 FPS, while
  final rate/latency are unmeasured. There is no existing packet protocol,
  startup/invalid-data policy, or Jetson angle command.
- Raw pixels must not be the final MC02 control coordinate. The proposed
  vision contract is to identify two uniquely labelled rail/end markers,
  project the ball onto the current rail axis, and publish normalized
  longitudinal position `s` in `[-1,+1]`, normalized velocity `ds/dt`,
  capture sequence/timestamp, processing latency, confidence, validity and
  occlusion/out-of-range flags, plus raw pixel/end-point fields for
  diagnostics. This removes dependency on an arbitrary image center and
  supports variable frame rate. The camera should preferably be rigidly
  mounted above and near the pipe midpoint with its optical axis close to
  perpendicular to the rail-motion plane, the entire rail visible at all
  allowed angles, and margin around both ends. A front-corner camera looking
  diagonally across the chassis is workable only with per-frame rail-axis
  calibration and verified no-occlusion/FOV; it is not the preferred
  competition mounting. Exact mount geometry and usable rolling length
  remain unverified.
- The user reports a 25 cm usable ball travel (`-12.5 cm` to `+12.5 cm`)
  about the pipe center, a current pipe pivot/center height of 8.5 cm above
  the chassis reference, no fixed camera placement yet, and a mechanical rail
  angular range of approximately plus/minus 30 degrees. The H-problem
  plus/minus 5 cm in requirement 3 is the allowed initial ball-position
  range, not the full sensing range; requirement 6 still needs arbitrary
  specified positions, so vision/control must retain the full 25 cm travel.
  Use millimetres as the task/control coordinate (`x_mm = 125*s`) while
  preserving normalized `s` and raw pixels for diagnostics.
- Based on the user's prior empirical observation that a 6 mm lens at 10 cm
  working distance sees about 10 cm of pipe, the 3.5 mm lens should see about
  30 cm at 17.5 cm working distance under the same camera/crop. The proposed
  starting mount is therefore 18-20 cm optical distance from the pipe swing
  plane, giving margin around the full 25 cm usable travel. Prefer a
  straight-on side view normal to the rail's vertical swing plane, with lens
  centre initially at the 8.5 cm pivot height; move toward the vehicle centre
  rather than a yawed front corner when possible. This estimate must be
  verified using the actual 3.5 mm lens, focus, resolution/crop, pipe at both
  plus/minus 30-degree mechanical extremes, and a real ball. The 30-degree
  value is a mechanical limit only: initial powered control should use a
  software limit near plus/minus 3 degrees, then expand cautiously toward
  plus/minus 5 degrees after closed-loop verification rather than commanding
  the mechanical extreme.
- PDF visual review confirms five selectable field tests, not six. Internal
  Q1-Q5 map to printed requirements 2-6; printed requirement 1 is the common
  video/record/playback requirement. User-confirmed Button X8 mapping is
  K1-K5 for internal Q1-Q5, K6 reserved, K7 normal stop/reset back to
  question-selection standby, and K8 emergency stop. The panel is physically
  handled by TI, so TI must debounce and report button events while MC02 owns
  the accepted active question, ball/rail control, timer/display, and Jetson
  coordination. TI retains exclusive chassis motion and line-following
  authority. The V2 3.3 V UART protocol is now frozen in
  `../gimbal_MC02/docs/MC02_TI_PROTOCOL_V2.md`, but neither binary implements
  it yet. The preferred link uses TI UART1 PA8/PA9 and preserves the current
  PA10/K1 input.
- The user unplugged the JDY-31 and explicitly switched field-test control
  from Bluetooth to an external YwRobot Button X8 panel. Only SW1 is assigned;
  the user-described electrical behavior is active-low: GPIO internal pull-up,
  released high, pressed to GND. SW2-SW8 are intentionally unused.
- Build 2026072901 disables Bluetooth without deleting its source: UART0 was
  removed from SysConfig, `bluetooth.c` was removed from `APP_SOURCES`, and
  `Bluetooth_Init/Tick` were removed from `main.c`. PA10/BP33, formerly UART0
  TX, is now `GPIO_BUTTON_PANEL_BUTTON1`, a polled input with a generated
  internal pull-up. PA11/BP34, onboard S2/PB21, and SW2-SW8 are unassigned.
- SW1 is debounced for 40 ms. A new press while idle starts the existing
  3-second countdown, then internally arms low-speed line-follow mode 8 for
  exactly 90,000 ms using the compiled 30 RPM base and steering Kp 6.
  Pressing SW1 during countdown cancels; pressing it during motion stops and
  disarms. Holding SW1 through reset cannot start motion; it must be released
  and pressed again. All existing line-loss, I2C-stale, encoder, output, PWM,
  parameter, and timeout protections remain active.
- Wiring is SW1 to PA10/BP33, button-panel VCC to LaunchPad 3V3, and
  button-panel GND to system GND, with J21/J22 remaining on the
  BP/BoosterPack side. SW2-SW8 remain disconnected. Do not use 5 V for panel
  VCC and do not reconnect JDY-31 RXD to PA10 while this build is installed.
- Full offline verification passed for build 2026072901. It checks that PA10
  is generated with an internal pull-up, UART0/BLUETOOTH_UART is absent,
  Bluetooth code is not linked, and the button selects 90-second mode 8.
  TEST_MODE text/BSS is 17,232/1,293 bytes; safe release is
  11,856/1,208 bytes. Hardware flashing and behavior are not yet verified.
- With panel VCC left disconnected, the user measured SW1-to-GND at 2.16 V
  released and 0 V pressed. The pressed-low path is verified, but 2.16 V is
  not an acceptable released-high level for reliable 3.3 V GPIO operation.
  Do not run motors in this state. The panel's visible 10 kOhm (`103`)
  network likely requires its VCC rail; revised guidance is to power panel
  VCC from the LaunchPad 3V3 rail, never 5 V, then remeasure for approximately
  3.3 V released and 0 V pressed. This diagnosis remains to be confirmed by
  the user's follow-up measurement.
- The user then reported `gmake flash` failure. A direct `gmake -j4` completed
  successfully, proving this is not a compile error. DSLite reaches SEC_AP
  initialization but reports `CS_DAP_0: Error connecting to the target: DAP
  Connection Error` and aborts on `CORTEX_M0P`. Windows simultaneously shows
  the XDS110 composite device, debug probe, data port, and COM4/COM5 all
  present/OK, so the failure is between XDS110 and the target DAP rather than
  the PC USB enumeration. CCS debug execution was stopped but the DAP error
  persisted. A first connect-under-reset attempt also produced the same DAP
  error. Earlier guidance incorrectly called S1 the reset button. On the
  LP-MSPM0G3507, S1/PA18 is BSL Invoke and S3 is NRST. The next
  non-destructive recovery is the TI-documented BSL sequence with all
  external power/peripherals removed and J101 restored: hold S3 and S1,
  reconnect USB, start the download, release S3 when the terminal reaches
  `Connecting...`, then release S1 after the connection begins. The external
  button-panel SW1/PA10 is unrelated. The user then performed this recovery,
  but DSLite again failed at `CS_DAP_0` with the same DAP Connection Error.
  Before any destructive recovery, measure target 3V3-to-GND and released
  NRST-to-GND while powered only through USB, and visually verify that every
  required J101 isolation jumper is installed in the correct position. If
  those checks pass, the remaining TI recovery path is a DSSM factory reset.
  Do not perform mass erase or factory reset without explicit user
  authorization because those erase target flash/non-main configuration.
- The DAP failure was then resolved without a factory reset. The user found
  that the J101 isolation jumpers had not all been reinstalled. After fitting
  the complete J101 jumper set and repeating the S1-BSL/S3-NRST recovery,
  the connection/download succeeded. Treat complete, correctly positioned
  J101 jumpers as mandatory whenever using the onboard XDS110; no DSSM
  factory reset is needed for this incident.
- After the successful recovery/download, the user reported that pressing
  external SW1 produced no vehicle motion. The cause is not yet verified.
  Mode 8 intentionally refuses to start when the line sensor is disconnected
  or already reports `line_lost`; on an all-white surface (`state = 0`) it
  can therefore enter status -6 immediately after the 3-second delay and
  appear not to start. Next verify a normal post-BSL power cycle, SW1 voltage
  at PA10/BP33 (about 3.3 V released and 0 V pressed), and a centered black
  line with `connected = 1`, nonzero `state`, and `line_lost = 0` before
  changing firmware.
- The user subsequently confirmed that SW1-controlled line following works,
  which verifies the downloaded 2026072901 button path and mode-8 start in
  general. During testing the chassis sometimes accelerated abruptly; this
  behavior is unacceptable for competition and motivated an explicit target
  RPM slew limiter rather than a PID gain guess.
- The competition course supplied by the user consists of two 1.5 m straights
  and two radius-0.5 m semicircles, giving a nominal centerline length of
  about 6.14 m. A 20 s lap requires about 90 RPM with the configured 65 mm
  wheels. Build 2026072902 therefore uses a 105 RPM line-follow base (about
  17.2 s ideal lap), steering Kp 10, up to 35 RPM steering correction, and
  15-160 RPM commanded wheel bounds. Increasing targets are slew-limited to
  200 RPM/s and decreasing targets to 400 RPM/s; a 0-to-105 RPM launch takes
  about 0.53 s instead of an instantaneous step. The existing PID, 40% output
  limit, encoder/output supervision, sensor fault stop, and immediate disarm
  paths remain.
- SW1 in build 2026072902 is now the competition-lap sequence. After the
  3-second delay it starts on the transverse black bar, ignores that initial
  marker, and arms finish detection only after the marker is clear and the
  average encoder distance is at least 150 mm. A finish marker requires at
  least six of eight black channels continuously for 40 ms. At the next
  marker the target ramps down toward 20 RPM, follows the continuing line for
  an encoder-estimated 50 mm, then stops with status 2. Failure to find and
  complete the finish within 25,000 ms stops with status -2. `g_test.line`
  adds only `lap_phase` (0 idle, 1 leave start, 2 search finish, 3 advance
  50 mm, 4 complete) and `finish_distance_mm`.
- Full offline verification passed for build 2026072902: both TEST_MODE and
  safe-release builds link, the competition-marker/distance/slew constants
  and implementation are checked, and the Bluetooth/UART exclusion plus
  existing generated pin checks still pass. TEST_MODE text/BSS is
  18,472/1,348 bytes; safe release is 11,856/1,216 bytes. This build has not
  yet been flashed or tested on hardware. The 50 mm result depends on the
  configured 65.0 mm wheel diameter and measured 1467/1468 CPR and must be
  measured with a ruler on the actual surface before claiming accuracy.
- The first floor test used a MINI course with two 1.0 m straights and
  diameter-0.8 m semicircular ends (radius 0.4 m, about 4.51 m total), not the
  formal 1.5 m/radius-0.5 m course. The user reported that the chassis lost
  the line in a turn and correctly stopped. This verifies the fail-stop path
  but shows that fixed 105 RPM with the 100 ms no-line rule is too brittle for
  the tighter MINI curve. Exact turn direction, loss position, lap time, and
  final telemetry were not captured.
- Build 2026072903 retains 105 RPM on a centered straight but progressively
  reduces requested base speed toward 70 RPM as the line moves from center
  toward either edge. Steering Kp increases from 10 to 12 and maximum
  correction from 35 to 45 RPM. Target slew remains 200 RPM/s acceleration
  and 400 RPM/s deceleration.
- A line-only loss now performs bounded directional recovery: it remembers
  the last valid line side, commands a 45 RPM recovery base with +/-28 RPM
  differential toward that side, and resumes normal following if the line
  returns within 300 ms. Failure to reacquire stops/disarms with status -6.
  A disconnected sensor or stale I2C samples are not recoverable: targets are
  still removed immediately and status -6 occurs after the original 100 ms
  hard-fault grace. Encoder/output supervision, 25-second course timeout,
  SW1 stop, PWM limit, start/finish marker logic, and 50 mm finish advance
  remain unchanged.
- Full offline verification passed for build 2026072903. The new curve-speed
  and recovery constants are checked along with the existing marker,
  distance, slew, pin, UART-exclusion, and ELF-symbol checks. TEST_MODE
  text/BSS is 18,800/1,352 bytes; safe release is 11,856/1,216 bytes.
  Hardware flashing and MINI-course behavior are not yet verified.
- The user floor-tested build 2026072903 on the MINI course and confirmed
  that turns no longer lose the line. Two remaining problems were observed.
  First, a small residual heading error after a turn could carry the chassis
  gradually toward the inside during the following straight until all eight
  sensors lost the line and the vehicle stopped. Second, the first finish-bar
  crossing was missed; the chassis continued approximately another lap and
  stopped near the user-marked point on the next curve. Without terminal
  status this last stop cannot be distinguished between the 25-second
  timeout (`-2`) and another line-loss stop (`-6`), but the first finish was
  definitely not accepted.
- Build 2026072904 adds a bounded line-position derivative term to damp
  residual heading: steering remains Kp 12 and now uses Kd 6 multiplied by
  the per-10-ms position change, with derivative contribution hard-limited
  to +/-12 RPM. `g_test.line.steering_kd` is the only new tuning field for
  this controller term.
- Finish detection in build 2026072904 is made tolerant of angled crossing.
  Search phase still begins only after clearing the start marker and moving
  150 mm, but no finish can be accepted before average encoder travel reaches
  3,800 mm on the approximately 4,510 mm MINI lap. The finish condition is
  reduced from six channels/40 ms to at least four channels/20 ms, and the
  active centroid must remain within +/-1500 so a wide off-center curve
  pattern is not mistaken for the marker. `g_test.line.lap_distance_mm`
  exposes the distance gate; marker stop, 20 RPM approach, and 50 mm advance
  are otherwise unchanged.
- Full offline verification passed for build 2026072904. TEST_MODE text/BSS
  is 19,072/1,364 bytes and safe release is 11,872/1,224 bytes. The checks now
  require the Kd limit and finish distance/width/centroid gates in addition
  to all previous safety/configuration checks. Hardware flashing and behavior
  are not yet verified.
- Before flashing 2026072904, the user noted that the physical finish bar is
  too short to guarantee the previous multi-channel coverage and explicitly
  requested an encoder-distance layer based on course geometry. The MINI
  centerline estimate is `2*1.00 m + pi*0.80 m = 4.513 m`.
- Build 2026072905 keeps the PD straightening change and replaces the finish
  gate with a three-layer hybrid detector. A physical finish is accepted only
  inside 4,100-4,800 mm of average encoder travel; within that window at least
  three black channels, centroid within +/-1500, for 10 ms is sufficient for
  a short or angled marker. Normal marker recognition still triggers the
  20 RPM approach and measured 50 mm advance.
- If no marker is accepted by 4,800 mm, build 2026072905 stops/disarms rather
  than allowing a second lap and retains status
  `TEST_STATUS_FINISH_DISTANCE_FALLBACK = 4`. In that fallback case
  `finish_distance_mm` reports travel beyond the nominal 4,513 mm course
  estimate; it is not a claim that the physical marker was seen. The existing
  25-second status -2 timeout remains a final independent guard. These MINI
  distance constants must be changed when moving to the formal approximately
  6,142 mm course.
- Full offline verification passed for build 2026072905. TEST_MODE text/BSS
  is 19,144/1,364 bytes and safe release is 11,872/1,224 bytes. Verification
  now requires the expected course length, lower/upper finish window, and
  explicit distance-fallback status. Hardware flashing and behavior are not
  yet verified.
- The user floor-tested build 2026072905 and reported that it stopped about
  70 cm before the physical finish. The retained status was not observed, so
  whether this was status 2 or 4 is not directly verified. Given that the
  three-channel finish condition became legal at 4,100 mm, the leading
  diagnosis is a normal multi-channel pattern on the last curve being accepted
  at the start of the window and followed by the configured 50 mm advance.
  The same run showed visibly discontinuous speed changes or "stutter".
- Build 2026072906 moves the finish-acceptance lower bound from 4,100 to
  4,400 mm while keeping the 4,800 mm upper/fallback bound. This makes a
  final-curve false trigger stop near the expected 4,513 mm finish rather
  than roughly 0.4 m earlier, while the three-channel/10-ms short-marker
  tolerance remains.
- To remove discrete control steps, build 2026072906 low-pass filters the
  digital centroid before curve-speed and PD calculations with alpha 0.25.
  A line-only loss shorter than 50 ms now holds the prior wheel targets;
  only a longer loss switches to the 45 RPM directional recovery. The
  existing 300 ms loss stop and immediate stale/disconnected-I2C target
  removal remain unchanged.
- Full offline verification passed for build 2026072906. TEST_MODE text/BSS
  is 19,224/1,368 bytes and safe release is 11,872/1,224 bytes. Verification
  now also requires the position-filter and 50 ms hold constants. Hardware
  flashing and behavior are not yet verified.
- The user floor-tested build 2026072906 and reported an unsafe launch: after
  SW1 start the chassis surged forward, drifted slightly right (the user
  explicitly corrected an initial "left" description), lost the line around
  halfway along the first straight, and stopped; the whole motion took less
  than about one second. Do not repeat floor tests with 2026072906. No
  direction-specific wheel trim is justified from this one event because
  initial placement angle and line state were not measured.
- Build 2026072907 changes to a safe MINI validation speed profile. The first
  300 mm is capped at 55 RPM, nominal straight base is reduced from 105 to
  85 RPM, and curve floor from 70 to 65 RPM. Target acceleration is reduced
  from 200 to 80 RPM/s and deceleration from 400 to 250 RPM/s. The filtered
  PD steering, 50 ms dropout hold/300 ms recovery, 4,400-4,800 mm finish
  window, marker-plus-50-mm stop, and distance fallback remain unchanged.
  This is not yet the final formal-course speed setting; stability must be
  demonstrated before raising it.
- Full offline verification passed for build 2026072907. TEST_MODE text/BSS
  is 19,280/1,368 bytes and safe release is 11,872/1,224 bytes. Verification
  now requires both launch-speed and launch-distance limits. Hardware
  flashing and behavior are not yet verified.
- The user floor-tested build 2026072907. Launch and following were stable;
  the only reported problem was stopping about 50 cm before the physical
  finish. Frame-by-frame video supplied the missing marker evidence: ordinary
  turns illuminate at most 2-3 channels, while the physical finish produces
  four illuminated channels for only one or two video frames. This verifies
  that the prior three-channel marker rule could falsely accept a turn and
  that a sustained multi-sample debounce can miss the actual short marker.
- Build 2026072908 changes finish recognition to at least four active channels
  in any single 10 ms sensor sample, with immediate latch and no centroid
  restriction. The encoder eligibility window is widened to 4,000-5,500 mm;
  it remains the second layer that rejects the start/implausible positions,
  while the experimentally unique four-channel signature provides precise
  finish recognition. Distance fallback status 4 moves from 4,800 to
  5,500 mm so it cannot reproduce the observed early stop. All speed,
  smoothing, PD, recovery, and 50 mm post-marker settings are unchanged.
- Full offline verification passed for build 2026072908. TEST_MODE text/BSS
  remains 19,280/1,368 bytes and safe release 11,872/1,224 bytes. Hardware
  flashing and finish behavior are not yet verified.
- The user downloaded and floor-tested build 2026072908. From the start
  position, SW1 completed its countdown, the chassis initially steered right,
  then stopped almost immediately. No retained status was captured, so the
  exact stop reason is not hardware-verified. Code inspection found that the
  launch path initialized the filtered steering position directly from the
  transverse start bar and immediately treated its possibly one-sided
  four-channel centroid as an ordinary course-line error. This can command a
  right correction and then invoke the existing 300 ms line-loss stop,
  matching the reported sequence. The finish detector itself cannot accept
  the start marker because lap distance is still below 4,000 mm.
- Build 2026072909 adds a start-bar launch state without changing normal
  course steering or finish recognition. Button-started competition runs
  command equal 40 RPM wheel targets until at least 60 mm of average encoder
  travel has accumulated and the wide start marker has cleared. A short line
  loss during this interval also retains symmetric targets instead of using
  a potentially biased last-side recovery command; failure to reacquire
  within the existing 300 ms limit still stops with status -6. When normal
  following begins, the centroid filter and derivative history are reset
  from the current longitudinal course line to prevent a derivative kick.
  The later 55 RPM/300 mm launch cap, 85 RPM normal base, filtered PD,
  four-channel finish latch, 4,000-5,500 mm finish window, and 50 mm
  post-marker advance remain unchanged.
- Full offline verification passed for build 2026072909 after adding explicit
  checks for the symmetric start-bar state and its 40 RPM/60 mm constants.
  TEST_MODE text/BSS is 19,496/1,369 bytes; safe release remains
  11,872/1,224 bytes. Hardware flashing and launch behavior are not yet
  verified.
- The user floor-tested build 2026072909 and reported the same failure as
  2026072908: sudden acceleration followed by a right deviation. Therefore
  the transverse-bar centroid hypothesis and the added 40 RPM/60 mm
  suppression did not resolve the hardware behavior; do not continue tuning
  on 2026072908 or 2026072909. The user suggested returning to 2026072906,
  but this conflicts with the retained contemporaneous record that
  2026072906 itself surged right and lost the line, whereas 2026072907 was
  explicitly floor-tested stable with only an early finish-stop problem.
- Build 2026072910 is consequently an isolation rollback to the complete
  2026072907 motion and finish baseline rather than another gain adjustment.
  It removes the 2026072909 start-bar state and restores the 2026072907
  finish constants: three active channels for 10 ms, centroid within
  +/-1500, acceptance at 4,400-4,800 mm, and fallback at 4,800 mm. The
  floor-tested 55 RPM first-300-mm cap, 85 RPM normal base, 65 RPM curve
  floor, 80/250 RPM/s slew, Kp 12/Kd 6, position filter, 50 ms hold, and
  300 ms recovery are unchanged. This rollback intentionally reintroduces
  the known possibility of stopping about 50 cm before the finish; first
  prove that launch stability has returned before repairing finish detection
  again.
- Full offline verification passed for build 2026072910. Its TEST_MODE
  text/BSS returned exactly to the recorded 2026072907 size of
  19,280/1,368 bytes; safe release is 11,872/1,224 bytes. The verification
  script now asserts the exact rollback marker/window constants. Hardware
  flashing and launch behavior are not yet verified.
- The user floor-tested build 2026072910 and it still suddenly accelerated
  and deviated right. This disproves a source-only regression in the
  2026072908/09 finish and start-bar changes: even the exact 2026072907-sized
  rollback reproduces the current launch symptom. The user suspects that the
  controller starts before consuming live infrared data and reports that an
  earlier slow-then-fast run followed the line normally.
- Code inspection confirms that the cooperative main loop calls
  `LineSensor_Tick()` immediately before `TestMode_Tick()` every 10 ms, so
  the infrared module is not omitted. However, the old start path accepted
  whichever single valid sample was already cached when mode 8 was armed;
  it did not prove that any new samples arrived after the countdown. Its
  first distance-limited base was also 55 RPM, not the explicitly successful
  30 RPM low-speed profile.
- Build 2026072911 therefore adds a motor-disarmed sensor preflight after the
  three-second countdown. It records the current sample sequence and requires
  five consecutive newly acquired samples with `connected=1` and
  `line_lost=0`; no motor target is issued before this passes. Failure to
  obtain them within 500 ms cancels with status -6. Once passed, the first
  second is constrained to a 30 RPM base, after which the existing 80 RPM/s
  target slew rises through the 55 RPM first-300-mm cap and eventually the
  85 RPM normal base. The 2026072910/07 steering, recovery, and temporary
  rollback finish logic remain unchanged so this test isolates startup data
  freshness and staged speed.
- Full offline verification passed for build 2026072911. TEST_MODE text/BSS
  is 19,616/1,378 bytes; safe release remains 11,872/1,224 bytes.
  Verification now requires the five-sample preflight, 500 ms timeout, and
  one-second 30 RPM stage. Hardware flashing and launch behavior are not yet
  verified.
- Before the next floor test, the user reported a visible spark at the
  instant a pushbutton energized an LM2596 regulator module. No component
  damage is visible, but the spark location, battery voltage at that moment,
  switch type/DC rating, module input/output wiring, capacitor voltage
  ratings, output setting, connected loads, and whether the event repeats
  are not yet known. Suspend all powered chassis tests and do not repeatedly
  close the switch. The leading non-fault explanation is input-capacitor
  inrush arcing at the closing contact; a loose strand/connector, contact
  bounce or underrated signal button, reversed polarity, input/output short,
  failed module, and hot-plug voltage ringing remain possible and must be
  excluded while unpowered. The official LM2596 range is 4.5-40 V, but every
  installed module component and switch must independently withstand the
  actual source. TI requires a close low-ESR input capacitor, recommends its
  voltage rating at about 1.5 times maximum input, and warns that input
  bypassing/hot plug can create severe ringing. For a 6S battery with
  25.2 V maximum, this conservative capacitor guideline exceeds 35 V, so
  inspect the module input-electrolytic marking before reuse. Do not place a
  multimeter in current mode directly across the battery.
- The user localized two successive arc events. The first occurred between
  the exposed wire strands clamped in two adjacent screw terminals. After
  insulating those strands, the second arc occurred at the power button.
  This makes an initial strand-to-strand short or surface arc highly likely
  and separately shows that the button/contact is carrying the regulator
  hot-plug current. The button may be underrated for 6S DC/inrush or may have
  been damaged by the first event; it must not be reused merely because it
  still conducts. Keep the battery disconnected, replace/terminate the
  conductors so no bare copper protrudes (prefer correctly sized ferrules),
  identify and replace the switch with a suitably DC-rated power switch, add
  a correctly rated branch fuse close to the battery, and use precharge,
  anti-spark, eFuse, or controlled-slew switching if the measured inrush
  exceeds the switch capability. If the arcing button was the Button X8
  signal panel, that wiring is prohibited: the panel may only switch a GPIO
  to GND and may never carry battery or LM2596 input current. The module and
  downstream load remain unverified after the possible short and must first
  be tested from a current-limited bench supply.
- The user supplied and requested review of
  `../gimbal_MC02/docs/MC02_TI_PROTOCOL_V2.md` before authorizing TI-link
  firmware. The full V2 document was re-read. Its intended authority split is
  coherent: TI alone scans/debounces Button X8 and owns all chassis PWM,
  line/encoder control and local stops; MC02 owns accepted question/run
  selection and issues idempotent PREPARE/START/STOP commands. A K1-K5 press
  therefore sends a TI `BUTTON_EVENT` request but must not directly start
  chassis motion; MC02 must complete PREPARE -> TI READY -> START.
- The protocol's canonical SAFE_STOP frame was independently checked with
  CRC-16/CCITT-FALSE and produces `0xC612`, correctly serialized as frame
  bytes `12 c6`. Its proposed physical path is also consistent with TI's
  LP-MSPM0G3507 `uart_echo` example: UART1 PA9 RX at J14_3 and PA8 TX at
  J1_4, 115200 8-N-1. These pins do not conflict with current PA10/K1,
  motors, encoders or PB2/PB3 I2C. J101 UART RX/TX isolation is still
  required by the TI example to prevent XDS110/external contention, while
  SWDIO/SWCLK remain installed.
- Current implementation scope is only internal question 1, which the V2
  mapping defines as printed requirement 2: K1 requests one chassis lap and
  a stop at marker A within the overall 20-second requirement. K2-K5 map to
  printed requirements 3-6, K6 is reserved, K7 is local normal stop/reset and
  K8 is a locally latched chassis emergency stop; none of K2-K8 is currently
  wired or implemented in TI firmware. Printed requirement 1 is common
  recording/playback, not a selectable sixth question.
- Two implementation points require the user's confirmation before code:
  current PA10/SW1 directly starts mode 8 and must be changed to
  event-only/wait-for-MC02 behavior, and unsupported internal questions 2-5
  need a defined response until their functions exist. The V2 document has
  no explicit `fault_code` enumeration for `UNSUPPORTED_QUESTION`; safest
  first implementation is to accept/report only K1/question 1 and keep the
  chassis IDLE_DISARMED for K2-K5 until that status code/behavior is frozen.
  No UART/SysConfig/button firmware was changed during this review.
- The MC02 wiring checklist already contained the three-wire TI link.
  `接线指导.md` now duplicates it as a clearly marked future-only section:
  MC02 TX/PA9 -> TI RX/PA9, TI TX/PA8 -> MC02 RX/PA10, and GND -> GND,
  with no shared power rail. It explicitly prohibits physical connection
  until both firmware implementations and standalone 3.3 V UART/CRC/watchdog
  tests pass.
- The user authorized the first TI implementation. Build `2026072912` adds
  `mc02_link.c/.h`, SysConfig UART1 at 115200 8-N-1 on PA8 TX / PA9 RX,
  interrupt-driven 256-byte RX and 512-byte TX rings, the V2 byte-stream
  parser, CRC16/CCITT-FALSE framing, 20 Hz chassis status, transition events,
  K1 press/release events, duplicate-command suppression, and the 500 ms
  running heartbeat watchdog. A heartbeat loss calls the existing immediate
  test-mode stop/disarm path before reporting `SUPERVISOR_LOST`.
- K1/SW1 is now event-only. It never starts, cancels, or stops chassis motion
  directly. Q1 motion requires MC02 heartbeat, PREPARE/READY, then START.
  The START path skips the old TI-side three-second delay but preserves the
  motor-disarmed gate requiring five newly acquired valid line samples within
  500 ms. Q1 accepts only options `0x5` and 2,000-20,000 ms. It reuses the
  existing one-lap/marker-A controller. Question IDs 2-5 and all command
  envelopes are parsed and acknowledged, but deliberately remain
  IDLE_DISARMED and never report READY or arm PWM because their TI behaviors
  are not implemented. K2-K8 remain physically unassigned, so K7/K8 local
  stop semantics are not yet available.
- `g_test` remains the only Watch object. Its new compact nested `mc02`
  telemetry contains online/state/fault, run/command/question IDs, valid RX
  frame count, CRC error count, and transmitted-frame count. No second debug
  global was added. The shared V2 document now freezes fault codes 0-6:
  NONE, SUPERVISOR_LOST, LINE_SENSOR, ENCODER, OUTPUT, BAD_PARAMETER and
  ROUTE_TIMEOUT.
- Full offline verification passed for build `2026072912`: clean TEST_MODE
  and safe-release builds linked; UART1 PA8/PA9/115200 generation, absence
  of Bluetooth UART/code, K1 event-only behavior, MC02 integration symbols,
  heartbeat/duplicate/stop implementation markers, and the canonical
  SAFE_STOP CRC `0xC612` were checked. TEST_MODE text/BSS is
  25,128/2,326 bytes; safe release is 17,016/2,170 bytes. This is compile and
  static verification only. The UART parser, electrical levels, MC02
  interoperability, heartbeat stop latency and Q1 motion have not been
  bench-tested. Because of the unresolved LM2596/button arcing incident,
  powered testing remains prohibited.

### 2026-07-28

- At the user's request, `最近16小时聊天记录.md` was added at the project root.
  It is a concise chronological handoff summary covering power/encoder/PID,
  JDY-31 COM7 Bluetooth, Hiwonder calibration and mapping, build 2026072809,
  and the next LINE/LAP tests. It is supplementary only; `AGENT.MD` remains
  the authoritative technical handoff record.
- The user then requested a dialogue-style, text-only export rather than a
  technical summary. `最近16小时文字聊天记录.md` was added at the project root.
  It contains the visible user/assistant text in chronological order, omits
  image binaries, and explicitly labels early assistant replies that could
  only be reconstructed from the retained context rather than quoted
  verbatim.
- The user established that `AGENT.MD` must be updated after every code change
  or project-knowledge change so future agents can reliably inherit the work.
- TI's LP-MSPM0G3507 Rev. D user guide confirms that J11 is the direct
  external 3V3/GND power header; J10 is the separate 5V/GND header. For this
  chassis, a battery regulator intended to power the target directly must be
  adjusted and measured at 3.3 V before connecting its positive output to
  J11 3V3 and return to J11 GND. Do not connect battery, 12 V, or an
  unverified regulator output directly to J11.
- When external 3.3 V target power is used, isolate it from the XDS110 power
  source by removing the J101 3V3 power jumper. If USB may be connected at
  the same time, also isolate the J101 5V power path; retain the required
  common ground and debug-signal connections. The actual user's board wiring
  and regulator voltage have not yet been verified.
- A user photo shows the external leads placed on the J11 pins silk-screened
  `3V3` and the adjacent `GND`, which appears to be the intended connection.
  The round clear component visible beside U2 is the onboard photodiode/light
  sensor, not a power LED, and is not expected to illuminate.
  No target LED is driven by the current firmware, and the green USB power
  indicator belongs to the XDS110/USB supply path, so an unlit LED does not
  prove that J11 power is absent. The regulator output unloaded and the
  voltage measured directly across J11 while connected remain unverified.
- The user chose to restart commissioning from encoder verification (Phase
  B) before further motor testing. The next safe action is a motor-power-off
  hand-rotation check of channel mapping, corrected count signs, repeatable
  counts per wheel revolution, and left decoder invalid-transition telemetry.
- During the restarted Phase-B check, the user reported that hand rotation
  did not change the observed encoder data. It is not yet known whether this
  affects one or both wheels. Before changing firmware or swapping A/B,
  verify that the target is running and CCS continuous refresh is enabled,
  confirm J11/target 3.3 V under load, then measure encoder VCC and all four
  A/B signals for 0-to-3.3 V transitions at the J2 connections.
- The user corrected the encoder supply requirement: both installed encoders
  require 5 V VCC rather than 3.3 V. This may explain the no-count result at
  3.3 V, but that cause has not yet been electrically verified. The encoder
  supply documentation was changed to 5 V. Because the encoders are described
  as push-pull, all four A/B outputs must be treated as potentially 5 V and
  must pass through proper 5 V-to-3.3 V unidirectional level translation
  before reaching PB16/PB0/PB6/PB7, unless measurements prove that the actual
  output-high voltage never exceeds 3.3 V.
- The user then verified with a voltmeter that encoder VCC is taken directly
  from the LaunchPad/MSPM0 5 V rail and that the encoder supply connector
  measures 5 V. Encoder supply is therefore confirmed; the raw A/B high
  levels and whether level translation is already present remain unverified.
- With the encoder signals connected, the user reported PB0/PB16 fixed at
  about 0.96 V and PB6/PB7 fixed at about 4.14 V while turning the wheel.
  A DC multimeter can display the average of changing quadrature pulses, so
  these readings alone do not prove the signals are static. PB0/PB16/PB6/PB7
  are nevertheless Standard I/O, not the device's 5-V-tolerant ODIO pins;
  4.14 V exceeds their normal VDD+0.3 V input limit at 3.3 V VDD. The
  immediate safe action is to power down and disconnect all four A/B leads
  from the LaunchPad. Measure the encoder outputs unloaded, then add level
  translation before reconnecting. Possible GPIO damage has not been
  established.
- After disconnecting the signal from PB6 and measuring it against encoder
  ground with the encoder powered at 5 V, the user reported that the presumed
  right-encoder A lead remained at 4.44 V both stationary and after small
  wheel movements. This confirms a high raw signal and the need for level
  reduction, but does not yet prove that the lead is really encoder A. Next
  verify 5 V at the encoder's own VCC/GND connector, test the presumed B lead
  in the same stopped-position manner, and confirm the vendor wire pinout.
- The presumed right-encoder B lead formerly connected to PB7 behaves the
  same as the A lead: it remains high and shows no stopped-position change.
  Both right raw channels are therefore currently fixed high as observed by
  the user. Do not reconnect them to the MSPM0. The next diagnostic is to
  measure VCC directly at the plugged-in encoder connector and establish the
  actual VCC/GND/A/B wire order from the motor/encoder documentation or clear
  connector photos.
- The user subsequently restored working encoder counts. With the current
  physical wiring, both corrected channels count negative for vehicle-forward
  rotation and positive for reverse. Build 2026072801 flips only
  `ENCODER_LEFT_REVERSED` and `ENCODER_RIGHT_REVERSED`; motor output direction
  flags are unchanged. After flashing, both sides must be hand-verified as
  forward-positive/reverse-negative before any closed-loop test. Functional
  counting is confirmed by the user, but safe MCU-side A/B levels at or below
  3.3 V have not yet been reported.
- The encoders may be powered directly from a dedicated regulated 5 V module.
  Connect regulator 5 V only to both encoder VCC leads, and connect regulator
  ground to encoder ground and LaunchPad ground. Do not connect that 5 V rail
  to J11 3V3. The four A/B signals still require verified MCU-side levels no
  higher than 3.3 V. This topology is approved guidance only; actual rewiring
  has not yet been reported.
- The user requested proceeding directly to PID tuning. PID testing remains
  gated by two unresolved safety checks: build 2026072801 must be flashed and
  both channels hand-verified as forward-positive/reverse-negative, and the
  MCU-side voltage of all four A/B signals must be measured at no more than
  3.3 V. Once both pass, perform lifted-wheel mode 7 at 40 RPM for 5 seconds
  before any floor test.
- The complete offline verification passed after the sign change: TEST_MODE
  text/BSS remains 15,008/1,181 bytes and release remains 9,240/1,093 bytes.
  Build 2026072801 has not yet been flashed or hand-verified. No hardware
  wiring change is claimed by this entry.
- Added debugger telemetry mirror `volatile PIDTestWatch g_test_pid` for PID
  commissioning in build 2026072802. One expanded CCS Expressions entry now
  exposes the build and run state, command values, safety fault masks, and
  complete per-wheel target/raw/filtered/error RPM, requested/applied PWM,
  encoder count, closed-loop state, PID gains/state/output/limits, plus
  per-run minimum/maximum RPM, mean/maximum absolute error, and maximum
  absolute applied PWM. A run sequence and retained last-run status make a
  screenshot after automatic stop useful. The mirror refreshes every 10 ms
  and is intentionally read-only; commands remain in `g_test` and gain edits
  remain in `g_motor`.
- `tools/verify_project.ps1` now requires the `g_test_pid` ELF symbol. Full
  offline verification passed for build 2026072802: TEST_MODE text/BSS is
  15,856/1,478 bytes and release text/BSS is 10,096/1,390 bytes. The new
  build has not been flashed or tested on hardware. Documentation now tells
  the user in both the README and Chinese tuning guide to add `g_test_pid` as
  the single PID screenshot expression.
- During the first reported PID attempt, the user observed that the wheels
  began turning but stopped automatically after roughly one second. The exact
  flashed build ID, test mode, terminal status, and fault masks have not yet
  been reported. This timing is consistent with, but does not yet prove, the
  750 ms encoder/output supervision stopping the run. Do not change PID gains
  or repeat powered tests until the retained `g_test_pid` snapshot identifies
  `last_run_status`, `encoder_fault_mask`, `output_fault_mask`, direction,
  counts, and RPM for both wheels.
- The next CCS screenshots confirm build 2026072802 is flashed and
  `g_test_pid` is updating. They were captured during mode 7 at only 170 ms
  elapsed / 150 ms active motion, with status 1, motor armed, no fault masks,
  no verified encoder bits yet, target 80 RPM, and duration 20,000 ms.
  Therefore they do not capture the reported stop and cannot identify its
  cause; zero `encoder_verified_mask` is expected before the first 750 ms
  supervision window completes. The command also did not use the requested
  first-test limit of 40 RPM for 5,000 ms. The next safe diagnostic is one
  lifted-wheel run at 40 RPM / 5,000 ms, followed by a no-reset screenshot
  showing the retained terminal fields and expanded `left` and `right`.
- The retained terminal snapshot from run sequence 4 confirms the stop cause:
  mode 7 ended with status/last status -4 at exactly 750 ms active motion,
  `encoder_fault_mask = 3`, `encoder_verified_mask = 0`, and
  `output_fault_mask = 0`. Thus both motor outputs acquired the requested
  positive sign, but neither corrected encoder count made at least two
  positive counts during the first 750 ms window. The user clarified that
  `command_target_rpm = 80.0` was intentional, not an accidental write to the
  read-only mirror. The higher target does not explain a signed-progress
  failure while both wheels visibly turn. Do not tune PID yet. With motor
  power disconnected, use mode 1 and hand-rotate each wheel in the same
  physical direction seen during positive mode-7 output; record the sign and
  change of `g_test_pid.left/right.encoder_count`. This distinguishes
  both-sign mismatch from missing counts without another powered run.
- The user clarified that during positive-target mode 7 both wheels physically
  rotated in the vehicle-reverse direction and both encoder counts remained
  unchanged. The first observation verifies that both motor output direction
  corrections were wrong, so build 2026072803 changes
  `MOTOR_LEFT_OUTPUT_REVERSED` and `MOTOR_RIGHT_OUTPUT_REVERSED` from 1 to 0.
  This direction correction is not yet flashed or physically verified. The
  zero encoder counts are a separate unresolved input/wiring problem; encoder
  sign flags were deliberately not changed.
- At the user's request, build 2026072803 removes the separate
  `g_test_pid`/`g_fw_build_id` debugger globals and hides `g_motor` at file
  scope. CCS WATCH now needs only `g_test`. Modes 8-10 and their step,
  distance, turn, summary, and odometry telemetry were removed for the current
  commissioning stage. Modes 0-7 remain; open-loop modes use fixed 15% PWM.
  `g_test.left/right` now contain only encoder count, RPM, error, applied PWM,
  writable Kp/Ki/Kd, integral, and PID output. Gains are validated and copied
  into the private motor controller only when a new mode-7 run starts.
  Full offline verification passed: TEST_MODE text/BSS is 12,560/1,104 bytes
  and release text/BSS is 9,120/1,057 bytes. Build 2026072803 has not yet
  been flashed or tested on hardware. All active commissioning documents now
  use the new `g_test.left/right.encoder_count` and PID-field paths, describe
  the fixed 15% open-loop diagnostics, and no longer direct the user to
  removed mode-8/9/10 fields.
- The user subsequently reported that the encoders are working again. This
  confirms restored counting in general, but the new build's corrected
  forward sign on both wheels, the exact counts observed, and safe MCU-side
  A/B voltage levels have not yet been reported. Before PID tuning, flash
  build 2026072803, hand-check forward-positive counts in mode 1, then use
  lifted-wheel modes 3 and 4 separately to verify that positive output now
  produces physical vehicle-forward rotation and increasing encoder count.
- A CCS screenshot confirms build 2026072803 is flashed and mode 7 can run at
  a 40 RPM target. At 640 ms it showed status 1, motor armed, zero output and
  encoder fault masks, zero invalid left transitions, and
  `encoder_verified_mask = 0`; the last value is expected because the first
  750 ms supervision window had not completed. The user perceived the right
  wheel as slightly slower and both wheels as accelerating quickly. No
  left/right numeric telemetry was expanded, so RPM mismatch, overshoot, PWM,
  and PID error remain unquantified. Capture expanded `left` and `right`
  during the steady portion (about 2-3 seconds) and the terminal top-level
  status before changing gains. A softer launch, if desired, should normally
  be implemented as a target-RPM ramp rather than inferred solely from gain
  reduction.
- A completed 7,000 ms mode-7 run at 40 RPM on build 2026072803 passed:
  terminal status 2, `encoder_verified_mask = 3`, both fault masks zero, and
  motor disarmed. A steady screenshot showed left 40.4049 RPM / error
  -0.4049 RPM / PWM 12 / integral about 0.6115 / PID output about 12.576,
  and right 39.9037 RPM / error +0.0963 RPM / PWM 11, with both gains still
  Kp 0.7, Ki 20, Kd 0. The instantaneous left/right speed difference was only
  about 0.50 RPM (1.25% of target), so no gain change is justified from this
  sample. Encoder totals 8593/8344 were cumulative and were not reset at run
  start, so they must not be used to infer wheel mismatch. The left software
  decoder accumulated 8 invalid transitions; this is under 0.1% of the shown
  count but should be monitored and investigated if it grows quickly.
  The next control improvement, if the user wants a gentler launch, is a
  target-RPM slew/ramp rather than reducing otherwise acceptable steady-state
  gains.
- The user then reported that startup now appears acceptable and the earlier
  unusually fast launch did not immediately repeat. No ramp or PID change was
  made. Keep Kp 0.7 / Ki 20 / Kd 0 and verify repeatability before adding a
  slew limiter; the earlier perception may have been a transient observation,
  the low inertia of lifted wheels, or comparison with the prior 80 RPM run,
  but its exact cause is unverified.
- The user reports a lifted-wheel 100 RPM test in which both wheels traveled
  nearly the same distance over equal time. Exact RPM/PWM snapshots, duration,
  status, and fault masks were not supplied, so this is qualitative rather
  than a complete numeric record. Together with the successful 40 RPM run it
  is sufficient to freeze the provisional Kp 0.7 / Ki 20 / Kd 0 and stop
  further bench gain tuning for now. Do not remove/bypass PID; proceed to a
  conservative loaded straight-line floor validation and revisit gains only
  if load introduces persistent error, oscillation, saturation, or drift.
- Before floor testing, the user asked whether an onboard button can be used.
  TI's LP-MSPM0G3507 Rev. D user guide and schematic identify S2 as the USER
  button connected to PB21; S1 is the PA18 BSL-invoke button through J8, and
  the separate reset button must not be used as an application start input.
  PB21 is currently absent from this project's SysConfig and source pin
  assignments, so S2 is available without conflicting with the current motor
  or encoder map. Adding S2 as a start/stop request is recommended for untethered
  floor commissioning, but it remains a software input and cannot replace the
  required physical motor-power master switch. No button code or SysConfig
  change has yet been authorized or made.
- The user authorized the S2 workflow and requested a 20,000 ms motion time.
  Build 2026072804 assigns onboard S2/PB21 as an active-low, pull-up GPIO
  input in SysConfig and polls it in the existing 10 ms task with 40 ms
  debounce. An idle press starts a 3,000 ms countdown (`status = 3`), then
  internally performs the arm handshake and starts mode 7 using the current
  target RPM and PID gains while forcing `duration_ms = 20000`. A second
  press cancels the countdown or stops/disarms an active run. A button held
  during reset cannot start motion; it must be released and pressed again.
  `g_test.button_pressed` and `g_test.start_delay_ms` are the only added WATCH
  fields, preserving the single-`g_test` interface. S2 remains software-only
  and does not replace the physical motor-power switch. TEST_MODE compilation
  and SysConfig regeneration passed; the verification script now also checks
  the generated PB21 button symbols. Full offline verification passed:
  TEST_MODE text/BSS is 13,080/1,124 bytes and release text/BSS is
  9,144/1,065 bytes. Build 2026072804 has not yet been flashed or tested on
  hardware.
- The user reports having an unspecified Bluetooth module and a Bluetooth-
  capable PC and asks about wireless debugging. This can likely provide
  command/telemetry transport if the module is classic Bluetooth SPP UART, or
  with additional GATT/client work if it is BLE, but it cannot provide SWD
  firmware flashing or halt/step debugging. The module model, pin labels,
  supply voltage, UART logic levels, and classic/BLE profile are not yet
  known. No UART pins or Bluetooth protocol have been allocated in the
  project. Obtain a clear module photo/model before changing SysConfig; do not
  connect an unverified 5 V UART TX signal to an MSPM0 GPIO.
- The user wants Bluetooth to initiate test mode as a replacement/alternative
  to pressing S2. The intended safe architecture is to route validated UART
  commands into the same private 3-second countdown and fixed 20,000 ms
  mode-7 start path, not to expose raw memory writes to `g_test.mode`.
  `STOP` must always stop/disarm, connection or MCU reset must never start
  motion, and a command/heartbeat timeout should stop a Bluetooth-started run
  if the link is lost. Keep S2 available as a local alternative and retain the
  physical motor-power switch. Implementation remains blocked on identifying
  the module's exact model/profile, pins, supply, UART levels, and whether a
  connection-state output is available.
- The user explicitly deferred Bluetooth work. No Bluetooth implementation
  should be started until they resume it.
- The first reported loaded floor test behaved differently from lifted-wheel
  tests: the motors shook and made an audible noise, while unloaded operation
  had been normal. The target RPM, whether one or both motors shook, terminal
  status/masks, live RPM/PWM/integral values, and loaded 12 V rail have not
  yet been reported. Stop repeated floor tests until this is separated into
  (a) converter/DRV voltage sag or current-limit cycling, (b) low-speed
  stiction plus aggressive integral control, (c) mechanical binding, or
  (d) encoder noise under motor load. Do not increase gains or PWM. With motor
  power off first check mechanics, then obtain a brief lifted comparison and
  a retained `g_test` snapshot; measure the DRV 12 V rail during any tightly
  controlled loaded reproduction and monitor left invalid transitions.
- The user identified and removed a physical obstruction that caused the
  loaded motor shaking/noise; this was mechanical, not a PID or supply fault.
  No PID change is required from that event.
- The LaunchPad is now powered from an external regulated 3.3 V module, but
  the user reports intermittent failure to power/start at switch-on. The
  regulator model/current rating, battery input, J101 jumper state, J11
  voltage during a failed start, NRST level, USB connection state, and whether
  encoder 5 V is already present are not yet known. Likely causes include a
  slow/oscillating regulator startup, inrush voltage dip, source contention
  through the J101 3V3/5V paths, reset not releasing, poor J11 contact, or
  back-power through GPIO. Official guidance confirms J11 is the direct
  external supply header and target voltage must remain within 1.62-3.6 V;
  J101 connects XDS110 power and debug signals into the target domain. Diagnose
  with motor power disabled by measuring J11 3V3 and NRST during both good and
  failed starts before changing firmware.
- The user confirms J101 is still populated while external 3.3 V is applied
  through J11; the failed-start J11 voltage, NRST level, and regulator model
  are not yet measured. The first corrective diagnostic is a fully powered-
  down removal of the J101 `3V3` and `5V` jumpers so the external target rail
  cannot contend with or back-power the XDS110 supply domain. Preserve the
  labeled jumpers for later restoration. Retest standalone startup before
  changing anything else; if intermittence remains, measure J11 and NRST in
  the failed state.
- The user removed the J101 `3V3` and `5V` jumpers and the target still
  intermittently fails to start/appear powered. This rules out J101 supply
  contention as the sole cause. Keep those two jumpers removed while using
  external J11 power. The next required observations, made specifically while
  the board is in the failed state and with motor power disabled, are: voltage
  directly across J11 `3V3`-`GND`, voltage from `NRST` to GND, and the
  regulator model/input/output details. Do not add a capacitor or reconnect
  supply jumpers before these measurements.
- In the failed condition the user confirms that a target-side 3.3 V header
  measures voltage, but no LED is visible and pressing the S2 user button
  produces no wheel motion after the expected three-second delay. The LED
  alone is not a valid target-execution indicator, but the missing S2 action
  suggests that firmware is not executing, reset/BSL is asserted, or the
  motor-power path is absent. The next standalone diagnostic is, with every
  supply fully off and after photographing jumper positions, to remove all
  remaining J101 isolation jumpers (GND, RXD, TXD, NRST, SWDIO, SWCLK, and
  BSL) as well as leaving 3V3/5V removed. Retest from J11 external 3.3 V with
  USB disconnected and wheels lifted. If it still fails, measure the exact
  J11 voltage and target-side NRST voltage in the failed state.
- The user then removed all remaining J101 jumpers and S2 still produced no
  wheel motion. This largely excludes an unpowered XDS110 signal connection
  as the direct cause. It is not yet confirmed whether S3 reset followed by
  S2 changes the result, what the exact J11 and target-side NRST voltages are,
  or whether build 2026072804 containing the S2 standalone-start feature was
  actually programmed into nonvolatile flash and verified before standalone
  testing. These must be distinguished before changing the power circuit or
  firmware.
- The standalone-start issue is resolved. The last previously confirmed
  target was build 2026072803, which did not contain S2 support; after the
  user programmed build 2026072804, external J11 3.3 V power and S2 start
  worked. Do not treat the unlit XDS110 USB-side LED as a target-power
  failure. Restore J101 only for USB/XDS programming with external J11 power
  fully off; isolate J101 again for standalone external-power operation.
- User photos identify the proposed Bluetooth board as a JDY-31 Bluetooth
  3.0 SPP UART transparent module, not an HC-05. The photographed six-pin
  carrier is labeled `STATE, RXD, TXD, GND, VCC, EN`, uses 3.6-6 V carrier
  power (5 V recommended), and exposes 3.3 V UART logic. Default settings from
  the JDY-31 manual are 9600 8N1, device name `JDY-31-SPP`, and PIN `1234`.
  STATE is low disconnected/high connected; EN is unused on this carrier.
- Build 2026072805 adds JDY-31 control on UART0: PA10/BP33 is MSPM0 TX to
  module RXD and PA11/BP34 is MSPM0 RX from module TXD. J21/J22 must both
  select the BoosterPack UART path. Power module VCC from the stable regulated
  5 V accessory/encoder supply, never J11 3V3, and share ground; leave STATE
  and EN open. If J101 5V is isolated and USB is absent, do not assume J10
  carries 5 V.
- The Bluetooth line protocol accepts `PING`, `STATUS`, `ARM`, `START`,
  `KEEP` (or `K`), and `STOP` at 9600 8N1. ARM is valid for five seconds.
  START enters the same private 3,000 ms countdown and fixed 20,000 ms
  closed-loop mode-7 run as S2. A Bluetooth-started countdown/run requires a
  KEEP within every 2,000 ms or it cancels/stops with timeout status. STOP is
  always accepted, pairing/reconnection never starts motion, and TEST_MODE=0
  rejects remote starts. `tools/bluetooth_control.ps1` performs link-only
  PING/STATUS by default and requires explicit `-Run` for ARM/START plus
  heartbeat. Hardware pairing and UART operation are not yet tested.
- Windows has successfully paired with the module and Device Manager now
  shows two generic Bluetooth SPP ports, COM6 and COM7. A paired-but-not-
  connected indication is normal until an application opens the outgoing SPP
  COM port. The exact outgoing port and end-to-end UART PING response remain
  to be identified; use the Windows Bluetooth COM Ports tab or run the
  link-only helper against COM6 and COM7 one at a time, never with `-Run`.
- End-to-end Bluetooth UART hardware is now verified on Windows outgoing SPP
  port COM7. Running the link-only helper returned `PONG 2026072805`,
  `S M=0 ST=0 T=0 L=0 R=0`, and `OK STOP`. This confirms the PC-to-JDY-31
  radio link, JDY-31 UART wiring/routing, PA10/PA11 firmware configuration,
  bidirectional 9600-baud traffic, and the correct flashed build. COM6 is not
  the application port. Motion via `-Run` has not yet been tested.
- The next chassis sensor is confirmed by the user's board photo and supplied
  introduction PDF as the Hiwonder `LineFollower_8CH V1.0`. The official
  specification requires DC 5 V at about 85 mA, supports eight GPIO outputs,
  UART, and I2C, returns digital and analog data, and uses fixed 7-bit I2C
  address `0x5D`. Register 5 is the one-byte digital state; registers 6-20
  are eight little-endian 16-bit analog readings; registers 22-36 are eight
  little-endian 16-bit thresholds.
- I2C was selected because UART0 is reserved for JDY-31 and eight GPIO inputs
  are unnecessary. The TI LaunchPad pinout exposes I2C1 on
  PB2/BoosterPack 9 (SCL) and PB3/BoosterPack 10 (SDA). Power the sensor
  from a stable regulated 5 V accessory rail and share GND, not from J11 3V3.
  The Hiwonder introduction does not state the SDA/SCL pull-up rail or maximum
  logic-high voltage. Before connecting SDA/SCL to the MSPM0, power only the
  sensor and measure both idle lines to GND; direct connection is allowed
  only if neither exceeds 3.3 V, otherwise use a bidirectional I2C level
  shifter.
- Hiwonder requires calibration at the same installed height used during
  operation (specified sensing range 0.5-8 cm). Their documented sequence is:
  place the array over the background, hold KEY until the learning indication
  completes and release it, then place it over the target line and briefly
  press KEY. The user confirmed a black line on a white background and that
  S1 is the vehicle-left end when the front-mounted array faces forward.
  Installed height and track line width remain unverified. First firmware
  integration must be read-only telemetry with motors disarmed;
  do not add automatic line-following motion until all eight channels,
  polarity, orientation, loss-of-line behavior, and stop handling are tested.
- Build 2026072806 implements the first read-only Hiwonder integration.
  SysConfig assigns I2C1 at 100 kHz to PB2/BP9 SCL and PB3/BP10 SDA.
  `line_sensor.c/.h` reads register 5 on each active sample, rotates through
  the eight analog channels, and slowly refreshes the eight thresholds.
  Transfers are polling-based but have a 3 ms timeout; an absent sensor is
  probed only every 250 ms, so a disconnected module cannot permanently block
  the cooperative motor task. Three consecutive state-read failures clear
  the connection flag. The only debugger interface remains `g_test`; its new
  nested `line` member contains `connected`, `sample_sequence`, cumulative
  `error_count`, raw state byte, and eight analog/threshold values. This build
  does not compute line position, add a line-following mode, arm motors, or
  change any existing test-mode safety behavior.
- Full offline verification passed for build 2026072806 after SysConfig
  regeneration. TEST_MODE text/BSS is 18,408/1,472 bytes and safe release is
  14,136/1,408 bytes. Verification now checks the I2C1/PB2/PB3/100 kHz
  generated map and the `LineSensor_Tick` ELF symbol. Hardware I2C, line
  voltage, channel order, calibration, and sensor orientation are not yet
  verified.
- With the Hiwonder module powered from 5 V and not connected to the MSPM0
  I2C pins, the user measured stable idle voltages of 3.31 V on both SDA and
  SCL. This is consistent with a 3.3 V pull-up rail and is within the MSPM0
  common-I/O absolute input limit of VDD+0.3 V, so direct I2C connection to
  PB3/PB2 is approved. This verifies only idle high voltage; I2C traffic,
  installed-height calibration, and all eight channel responses remain to be
  tested. The next safe action is to power down, wire SCL to PB2/BP9 and SDA
  to PB3/BP10, flash build 2026072806, keep mode 0/motor power off, and
  capture `g_test.line` over white background and with the black line centered
  under S4/S5.
- The user reported an accidental edit followed by a failed compile. A
  standard project build then succeeded, and the canonical full `verify`
  workflow clean-regenerated SysConfig and rebuilt both TEST_MODE and release
  successfully with the same 18,408/1,472 and 14,136/1,408 text/BSS sizes.
  `chassis.c` was the most recently saved source file and was inspected; its
  kinematics and conversion functions are structurally intact. No firmware
  source repair or build-ID change was required. Treat the incident as stale
  generated/object state or an unsaved editor-buffer error unless the user
  supplies a reproducible compiler diagnostic.
- After wiring and flashing build 2026072806, the user captured
  `g_test.line.connected = 0`, `sample_sequence = 0`, and an increasing
  `error_count` (369 in the screenshot). This verifies that no complete
  sensor read succeeded; it does not distinguish wiring/NACK from a
  controller state left latched by an early probe failure. Build 2026072807
  therefore delays the first probe for 250 ms and adds bounded I2C1 recovery:
  disable the controller, reset the controller transfer state, flush both
  FIFOs, clear pending interrupt status, and re-enable the controller after
  an error or timeout. No motor or autonomous-following behavior was added.
  Full offline verification passed: TEST_MODE text/BSS is 18,632/1,472 bytes
  and safe release is 14,360/1,408 bytes. Hardware behavior of the recovery
  has not yet been tested. The next action is to flash build 2026072807,
  power-cycle the LaunchPad and sensor together with motors disabled, and
  check whether `connected` becomes 1 and `sample_sequence` increments.
- The user flashed build 2026072807 and verified successful Hiwonder I2C
  communication: `g_test.line.connected = 1`, `sample_sequence` was
  increasing (2740 in the screenshot), `error_count = 0`, and the captured
  state byte was 255. This confirms the PB2/PB3 bus, address, register read,
  and controller-recovery build operate on the assembled hardware. State 255
  was captured without a documented target placement, so it must not yet be
  interpreted as black or white. Next keep motors disabled, calibrate at the
  final installed height, expand the analog and threshold arrays, and record
  white background, centered black line, S1-only/left, and S8-only/right
  samples to establish polarity and physical channel order.
- The user then reported that sensor calibration was probably complete. A
  photo shows the black tape crossing approximately S5/S6 with the S5 and S6
  blue indicators illuminated. The simultaneous telemetry was analog
  `[3381, 3005, 2988, 2828, 2772, 3042, 2767, 2995]` and thresholds
  `[3515, 3218, 3127, 2875, 2759, 2789, 2800, 2803]`. The `[4]` and `[5]`
  positions are consistent with S5/S6 in the documented left-to-right
  mounting, but this single placement does not yet conclusively establish
  array order or comparator polarity; channel 7 also appears above its
  displayed threshold without a clearly visible blue S8 indicator. Capture
  `state` plus arrays on all-white, S1-only black, and S8-only black before
  encoding the mapping or line-error calculation.
- The requested three-point test conclusively established the digital
  mapping and polarity after calibration: all-white gives `state = 0`,
  black under vehicle-left S1 gives `state = 1`, and black under
  vehicle-right S8 gives `state = 128`. Therefore state bit 0 is S1, bit 7
  is S8, and a set bit means black line. Build 2026072808 adds read-only
  derived telemetry inside the existing `g_test.line` only:
  `active_count`, `line_lost`, and signed `position_x1000`. The position is
  the centroid of active digital channels using S1..S8 weights
  -3500,-2500,-1500,-500,+500,+1500,+2500,+3500; negative is vehicle-left,
  positive is vehicle-right, and one adjacent-sensor spacing is 1000. The
  position is zero/invalid while `line_lost = 1`. No motor command or
  autonomous mode was added. Full offline verification passed: TEST_MODE
  text/BSS is 18,904/1,492 bytes and safe release is 14,632/1,428 bytes.
  Next flash build 2026072808 and hand-move the line across the array to
  verify the reported sign, endpoints, center, active count, and loss flag.
- The user confirmed build 2026072808 derived telemetry is correct and
  requested low-speed line following. The user will not keep a debugger cable
  connected during motion; all field-test commands and observations must use
  the verified JDY-31/COM7 Bluetooth link. The user's black-line test course
  is rectangular and one lap takes at least one minute, so 20 seconds is
  insufficient for a full-lap test.
- Build 2026072809 adds guarded TEST_MODE command 8. It runs both wheels under
  the existing speed PID with a 30 RPM default base and proportional steering
  Kp 6. Positive/right line position raises the left-wheel target and lowers
  the right; negative/left does the reverse. Correction is limited to 20 RPM,
  commanded wheel targets remain forward between the configured 8 and 60 RPM
  bounds, and the existing 40% PWM and encoder/output supervision remain
  active. Sensor disconnect, no fresh sample, or continuous line loss removes
  both targets immediately and ends/disarms with status -6 after a 100 ms
  grace period. S2 and Bluetooth START intentionally remain straight mode 7.
- Bluetooth now accepts `LINE` for a bounded 5-second mode-8 validation and
  `LAP` for a bounded 90-second rectangular-course run. Both require the
  existing ARM window, 3-second countdown, continuous sub-2-second KEEP
  heartbeat, and honor STOP. STATUS additionally reports `P` =
  `position_x1000`, `B` = digital state, and `X` = line-loss flag.
  `tools/bluetooth_control.ps1` adds mutually exclusive `-Line` and `-Lap`
  switches, sends KEEP/STATUS automatically, and sends STOP on exit. Mode 8
  has a 120-second absolute duration ceiling; modes 3-7 retain their
  30-second ceiling. The full offline verification, including PowerShell
  parsing and LINE/LAP source checks, passed: TEST_MODE text/BSS is
  20,544/1,529 bytes and safe release is 15,072/1,444 bytes. Build
  2026072809 has not yet been flashed or hardware-tested.

## Objective and current state

This is a new, independent LP-MSPM0G3507 firmware project for a larger
rear-drive differential chassis. It was derived from the useful low-level
parts of sibling project `../mspm0g3507_chassis`, but the old project has not
been modified.

The project currently implements:

- two-channel SeekFree DRV8701E PH/EN motor output at 20 kHz;
- right hardware QEI and left both-edge GPIO quadrature decoding, with both
  encoder connectors grouped on J2 / BoosterPack pins 11-14;
- provisional per-wheel encoder scaling and filtered 10 ms RPM feedback;
- per-wheel PID;
- differential-drive velocity and odometry math;
- one bounded debugger-writable `g_test` command/telemetry interface;
- MC02-supervised Button X8 SW1/K1 on PA10/BP33; the press starts the
  stopwatch/event but cannot grant PWM without PREPARE/READY/START;
- active MC02 V2 supervisor UART on UART1 PA8 TX / PA9 RX, compile-tested on
  both controllers but not yet physically connected or end-to-end verified;
- retained but disabled JDY-31 Bluetooth source; UART0 is not generated or
  linked;
- Hiwonder 8-channel line sensing on I2C1 PB2/PB3 plus guarded low-speed
  TEST_MODE line following;
- nonfatal SSD1306-compatible OLED stopwatch sharing I2C1 PB2/PB3;
- a disabled ICM42688 API placeholder;
- Makefile/SysConfig/TI-Clang build, XDS110 flash task, and a legacy Ozone project.

The latest canonical offline verification passed on 2026-07-30 for build
2026073006: both
TEST_MODE and safe-release ELFs linked, project/configuration checks passed,
the generated PA10 pull-up button, I2C map, and UART1 PA8/PA9 map were
checked, UART0/Bluetooth absence was checked, the formal 1.5 m/radius-0.5 m
geometry, 120/100 RPM speed profile, 100 mm symmetric start-bar release,
private two-turn encoder-yaw estimator, advisory 5,970+50 mm finish,
independent 6,070 mm fallback, slew, filtered PD steering, recovery,
active MC02 integration and OLED/timer symbols plus canonical CRC were
checked, and the single debugger interface `g_test` is present. TEST_MODE
uses 28,008 bytes text and 2,394 bytes BSS; safe release uses
19,720/2,243. Build 2026073006 has not yet been downloaded or
hardware-tested. The MC02 link remains physically unconnected and
end-to-end unverified.
Build 2026072910 was downloaded and reproduced the sudden acceleration/right
deviation even with the exact 2026072907 rollback. Build 2026072909 was
downloaded and reproduced the same sudden acceleration
and right deviation as 2026072908. Build 2026072908 was downloaded but
steered right and stopped shortly after launch; it must not be floor-tested
again. Build 2026072907 was stable but stopped about 50 cm before
the finish; video then proved turns use no more than three active channels
and the finish briefly uses four. Build 2026072906 surged slightly right and lost the line
halfway along the first straight in under about one second; it must not be
floor-tested again. Build 2026072905 stopped about 70 cm before the physical
finish and showed visible speed discontinuities. Build 2026072903 was
floor-tested: turns stayed on the line, but post-turn straight heading drift
and a missed finish marker remain. Build 2026072902 was floor-tested and lost
the line on the tighter MINI curve before stopping safely. Build
2026072901 was downloaded after restoring all J101
jumpers with the BSL/NRST sequence, and its SW1 mode-8 start was subsequently
confirmed working. Build 2026072807 was downloaded and its Hiwonder I2C
telemetry was verified connected with zero reported errors; subsequent
placement tests established digital channel order and polarity, and the user
confirmed build 2026072808 position telemetry is correct. Build 2026072805
was downloaded and its bidirectional JDY-31 link was verified on COM7, but
that module is now unplugged and its firmware path disabled.

`tools/verify_project.ps1` is the canonical offline verification entry point.
It performs clean TEST_MODE/release builds, configuration checks, ELF
symbol checks, and memory-size reporting. It does not claim hardware success.

There is intentionally no Jetson UART, buzzer, or active IMU implementation.
PA10 is the external SW1 input; PA11 and onboard S2/PB21 are unused. There is
no cloud remote configured for this project.

The current hardware download path uses the LaunchPad onboard XDS110 through
Micro-USB and CCS `DSLite`; the user abandoned the external J-Link path on
2026-07-25. Editing/building remains VS Code + Makefile. CCS may still be used
as the XDS110 debugger for the Expressions view because Ozone does not support
XDS110. `.theia/launch.json` provides the project-less CCS live-tuning launch,
and the VS Code task `Open CCS Live Tuning` opens the same folder in CCS 21.

The user-supplied mechanical drawing is preserved at
`docs/R3X_chassis_320x240mm.pdf`. Its title says "four-wheel differential",
but the confirmed hardware and this firmware use two driven rear wheels plus
two passive front casters. Do not infer four motor channels from the filename.

## Authoritative hardware configuration

MCU and board:

- TI LP-MSPM0G3507, MSPM0G3507 LQFP-64.
- Onboard XDS110 through the LaunchPad Micro-USB debug connector.
- J101 remains in the factory-installed state; J102 and J103 are unused.

Motor system:

- two MG513XP28_12V motors;
- nominal 1:28 gearbox;
- nominal 13 PPR quadrature Hall encoder;
- one SeekFree dual-channel DRV8701E board.

Pin map:

| Function | MCU |
|---|---|
| right PWM / PWM1 | PA12 / TIMG0_CCP0 |
| right DIR / DIR1 | PB4 |
| left PWM / PWM2 | PA13 / TIMG0_CCP1 |
| left DIR / DIR2 | PB1 |
| right encoder A/B | PB6 / PB7 / J2 BP13-BP14 / TIMG8 QEI |
| left encoder A/B | PB16 / PB0 / J2 BP11-BP12 / GPIO both-edge IRQ |
| external Button X8 SW1 | PA10 / BP33 / active-low GPIO with internal pull-up |
| MC02 supervisor UART | PA8 TX / PA9 RX / UART1 / 115200 8-N-1 |
| currently unused | PA11/BP34 and onboard S2/PB21 |
| Hiwonder line sensor I2C | PB2 SCL / PB3 SDA / I2C1 / BP9-BP10 / 100 kHz |
| OLED stopwatch I2C | shares PB2 SCL / PB3 SDA; VDD uses external regulated 3.3 V and system common GND |

`mspm0dache.syscfg` is the authoritative peripheral allocation. Generated
`ti_msp_dl_config.c/.h`, `device.*`, and `Event.dot` must be regenerated from
SysConfig and must not be hand-edited.

The Hiwonder LineFollower_8CH V1.0 is allocated to I2C1 on PB2/BP9 SCL and
PB3/BP10 SDA. The user measured both idle lines at 3.31 V while the module was
powered from 5 V, so direct connection is approved. S1 is vehicle-left and S8
vehicle-right; the track is black on white.

## Mechanical constants

Defined in `chassis_config.h`:

| Parameter | Value |
|---|---:|
| chassis envelope | 320 x 240 mm |
| wheel diameter | 65.0 mm |
| drive-wheel track | 214.2 mm |
| caster-to-drive axle spacing | 204.7 mm |
| tire contact width | 25.5 mm |
| measured left CPR | 1467 |
| measured right CPR | 1468 |

Only wheel diameter and drive-wheel track enter the ideal differential model.
The 204.7 mm axle spacing and 25.5 mm tire width are mechanical metadata.
Caster friction and tire slip will require an effective-track calibration.

The current values come from four manual ten-revolution trials performed on
2026-07-25: left average 1467.475 counts/revolution and right average 1467.95.
They must be remeasured if an encoder, gearbox, or decode method changes.

## Safety invariants

Future agents must preserve all of these unless the user explicitly supplies
validated replacement hardware:

1. Reset and initialization produce zero PWM and a disarmed motor layer.
2. Every motion test needs the one-shot internal `g_test.arm = 1` handshake.
   KEY1 never writes it directly. Only MC02 START reaches it through the
   bounded supervisor API after MC02's three-second countdown and TI's
   fresh-sensor validation.
3. Every motion test has a timeout. Modes 3-7 accept at most 30 seconds;
   the MC02 Q1-launched mode 8 is fixed at at most 20 seconds; debugger mode
   8 accepts at most 120 seconds.
4. Open-loop test PWM is hard-limited to 20%, default 15%.
5. Closed-loop PID output is absolutely hard-limited to 40%, independently
   of the debugger-writable `pid.out_min/out_max` fields.
6. Closed-loop modes 7 and 8 stop if either required encoder fails to
   keep producing at least two correctly signed counts in each 750 ms
   supervision window.
7. Encoder-dependent modes 7 and 8 reject a duration below 2000 ms. Their
   recurring progress window remains 750 ms. The initial window begins only after actual
   PWM is applied in the expected direction, then supervision never pauses.
   A later wrong-sign output faults immediately. Both modes require both
   `encoder_verified_mask` bits before reporting success. Distance and turn
   command modes are currently deferred.
8. Mode 7 rejects a live sign change. The motor layer removes PWM before any
   direction reversal, waits
   at least 1000 ms and then requires speed below 2 RPM, then gives DIR another
   10 ms setup interval. Encoder feedback can never shorten the fixed coast
   interval.
9. Invalid modes/parameters, completion, and timeout stop and disarm.
10. No motion starts merely because a debugger reconnects or the MCU resets.
    Runtime safety uses private `s_run_active`; debugger-writable
    `g_test.status` is telemetry only and cannot suspend timeout supervision.
    SW1 held during reset also cannot emit a new press or start motion; a
    verified release and new debounced press are required. In build
    2026073002 that press enters the local 3-second start delay; with the
    standalone flag disabled it again requires MC02 PREPARE/READY/START.
11. Encoder A/B may not exceed 3.3 V at MSPM0 pins.
12. The installed encoders require 5 V VCC. Their four push-pull A/B outputs
    require proper 5 V-to-3.3 V unidirectional level translation before the
    MSPM0 pins unless measured output-high voltage is proven not to exceed
    3.3 V.
13. A 6S LiPo may not directly power the 12 V motors. Use a regulated,
    adequately rated 12 V stage, fuse, and reachable master switch.
14. Never suggest powered motor testing with CPU breakpoints. When the core is
    halted, software timeout no longer runs and PWM output state is not proven
    safe. Use CCS Expressions without halting.
15. The 5 V Hiwonder line sensor may not connect SDA/SCL directly to PB3/PB2
    until both idle levels are measured no higher than 3.3 V. Use a
    bidirectional I2C level shifter if either line exceeds 3.3 V.

The user explicitly declined a GPIO physical emergency-stop input. PB16 is
occupied by left encoder A. The user later authorized onboard S2/PB21 only as
a software start/cancel/stop request for floor commissioning. Do not describe
it as an emergency stop; a mechanical motor-power disconnect remains required.

## TEST_MODE interface

Compile-time macro `TEST_MODE` defaults to 1 through the Makefile. Runtime
control and telemetry use the single `ChassisTestControl g_test` in
`test_mode.c`; with XDS110, add only `g_test` to CCS Expressions.

Command order for motion:

1. Write parameters.
2. Write `g_test.arm = 1`.
3. Write `g_test.mode` last.

The arm value is consumed and cleared at motion start. Mode 0 stops/disarms.

Mode map:

- 0: idle/stop/disarm
- 1: encoder observe
- 2: reset encoders
- 3: right open loop
- 4: left open loop
- 5: both open forward
- 6: both open reverse
- 7: both closed-loop RPM
- 8: guarded low-speed black-line following

Status values:

- -6 line sensor disconnected/stale for 100 ms, or line not reacquired
  within the 300 ms directional-recovery window
- -5 output readiness/direction fault; `output_fault_mask` bit 0=left,
  bit 1=right
- -4 encoder progress fault; `encoder_fault_mask` bit 0=left, bit 1=right
- -3 bad parameter
- -2 goal timeout
- -1 not armed
- 0 idle
- 1 running
- 2 complete
- 3 start pending / fresh line-sensor validation
- 4 two-turn estimate unavailable; stopped by the formal-course 6,070 mm
  independent distance fallback

The top-level structure contains mode, arm, status, build ID, duration, target
RPM, elapsed time, debounced button/countdown state, motor-armed state,
fault/verified masks, and left software decoder invalid transitions.
`left` and `right` contain corrected encoder
count, filtered RPM, error, applied PWM, Kp/Ki/Kd, integral, and PID output.
The nested `line` member contains the Hiwonder connection flag, sample
sequence, cumulative communication error count, raw digital state byte,
active count, loss flag, signed position, base/steering settings, dynamic
correction/targets, competition `lap_phase`, final-approach
`finish_distance_mm`, full-run `lap_distance_mm`, advisory `turn_count`,
signed `lap_yaw_deg`, and eight analog and threshold values. The current
MC02-supervised KEY1 run uses the safe validation profile: the first 100 mm has
equal wheel targets, the first 300 mm is at most 55 RPM, then formal straight
base 120 RPM, curve floor 100 RPM,
steering Kp 12/Kd 6, and asymmetric 80/250 RPM/s
acceleration/deceleration slew limits. Its centroid input is filtered with
alpha 0.25. A line-only loss holds prior targets for 50 ms, then searches for
at most 300 ms using the last valid side; an I2C hard fault still removes
drive immediately. PID telemetry is retained after a
stop while count/RPM/PWM remain live. In build 2026073006, K1 starts only the
stopwatch/event; MC02 owns the three-second countdown and START, while TI
retains the fresh-sensor gate and 2,000-20,000 ms Q1 envelope. The physical
communications link remains unconnected and unverified.

## File ownership

- `chassis_config.h`: all user-tunable mechanical, encoder, direction, and
  safety constants.
- `motor.c/.h`: motor arming, PWM/DIR, QEI, software quadrature, RPM, PID.
- `chassis.c/.h`: kinematics and odometry conversion.
- `test_mode.c/.h`: only component allowed to initiate commissioning motion.
- `line_sensor.c/.h`: bounded read-only Hiwonder I2C acquisition; no motor
  authority.
- `oled_display.c/.h`: nonfatal SSD1306-compatible shared-I2C display.
- `question_timer.c/.h`: question elapsed-time state independent of display.
- `imu.c/.h`: stable placeholder API; keep disabled until bus/pins are known.
- `main.c`: deterministic cooperative schedule only.
- `interrupts.c`: left encoder interrupt only.
- `mspm0dache.syscfg`: pin/peripheral source of truth.
- `README.md`: user entry point.
- `接线指导.md`: authoritative staged wiring guide for this hardware.
- `COMMISSIONING.md`: mandatory staged hardware test procedure.

Do not add direct motor writes outside `motor.c`, and do not bypass
`Motor_Arm()`/`Motor_Disarm()`.

`Motor_SetPWM()` stores the requested open-loop command. The 10 ms motor task
continues the DIR coast/setup state machine even if the caller writes the
command only once. Encoder raw state uses unsigned modular arithmetic and
corrected totals are signed 64-bit.

## Local toolchain and build

Known installed versions/paths:

```text
SDK:       C:\ti\mspm0_sdk_2_10_00_04
SysConfig: C:\ti\sysconfig_1.28.0
TI-Clang:  C:\ti\ccs\tools\compiler\ti-cgt-armllvm_5.1.1.LTS
gmake:     C:\ti\ccs\utils\bin\gmake.exe
DSLite:    C:\ti\ccs\ccs_base\DebugServer\bin\DSLite.exe
J-Link:    C:\Program Files\SEGGER\JLink_V914a
Ozone:     C:\Program Files\SEGGER\Ozone
```

Build from the project root:

```powershell
C:\ti\ccs\utils\bin\gmake.exe -C . -j4
```

Full offline verification:

```powershell
C:\ti\ccs\utils\bin\gmake.exe -C . verify
```

Artifact:

```text
ticlang\mspm0dache.out
```

Active XDS110 target configuration and flash script:

```text
targetConfigs\mspm0g3507_xds110.ccxml
tools\flash_xds110.ps1
```

`mspm0dache.jdebug` is retained only as a legacy J-Link artifact.

Debug flags are `-O0 -gdwarf-3`. Build ID is exposed as
`g_test.build_id = 2026073006`.

## Unverified assumptions and next actions

On 2026-07-25, the onboard XDS110 enumerated successfully as TI USB device
`0451:bef3`, serial `MG350001`, runtime firmware `3.0.0.25`. CCS `DSLite`
loaded `ticlang/mspm0dache.out` successfully and started the target. This
proves the build/download path only; no motor power, pin level, encoder, or
mechanical behavior has been verified. Do not replace the remaining
measurements below with guessed values.

Also on 2026-07-25, a hardware test made both motors run at full speed in mode
5, and the earlier firmware did not stop them reliably. Build 2026072502 was
then measured without motor power: mode 0 correctly held PA12, PA13, PB4, and
PB1 at 0 V. In mode 3, PA12 measured about 0.48 V average while inactive PA13
was 3.28 V; mode 4 produced the mirror image. This proved the channel mapping
was correct and exposed the actual timer bug: compare equal to the 1600-count
period never occurs in the 1599-to-0 down counter, so the PWM LOAD action
leaves an inactive channel constantly high. Build 2026072503 fixes true zero
duty per channel by switching that PWM pin to GPIO-low; a pin reconnects to
the timer only for non-zero duty. It has not yet been electrically verified.
Do not reconnect 12 V motor power until the no-power pin tests pass.

The user subsequently confirmed the PWM pin behavior is normal. Four manual
10-wheel-revolution encoder trials gave right magnitudes 14673, 14665, 14674,
and 14706 counts, and left magnitudes 14680, 14683, 14671, and 14665 counts.
Vehicle-forward rotation was negative on both corrected telemetry channels.
Build 2026072504 therefore uses right 1468 counts/rev, left 1467 counts/rev,
and flips both encoder correction signs so vehicle-forward becomes positive.
At equal open-loop command the left wheel was about 1-4 RPM faster in the
pre-calibration telemetry. A requested open PWM of 30 was actually clamped to
the test hard limit of 20; no 30% open-loop test has been performed.

1. Verify encoder VCC at 5 V and verify that every level-translated A/B signal
   presented to the MSPM0 remains at or below 3.3 V.
2. Verify SysConfig pin continuity to the actual LaunchPad headers.
3. Verify reset PA12/PA13 are inactive before connecting motor power.
4. Measure left and right wheel CPR independently.
5. Verify all four direction sign macros.
6. Measure PWM/PH behavior and debug-halt behavior with an oscilloscope.
7. Confirm the unknown 6S-to-12 V converter current rating against motor
   startup/stall demand.
8. Tune left/right PID on the actual loaded chassis.
9. Calibrate wheel diameter and effective track from floor tests.
10. Do not flash/run build 2026073005 from the battery until the LM2596 spark
    is diagnosed. With all power removed, first identify the exact arc
    location, photograph the switch/module/wiring and capacitor markings,
    verify polarity and absence of loose strands or heat damage, and provide
    the battery voltage, regulator input/output settings, switch DC rating,
    and loads connected to the output. Prefer first re-energization from a
    current-limited bench supply with the chassis motor rail disconnected.
    Only after the power path passes inspection, flash build 2026073005 and
    first perform a raised-wheel KEY1 test with the physical power switch
    reachable. Expect a 3-second countdown, about 50 ms of motor-disarmed
    fresh-sensor confirmation, then visibly slow 30 RPM motion for one second
    before gradual acceleration. A second KEY1 press must stop and disarm;
    otherwise remove motor power. On the first formal-field run, record lap
    time, marker `lap_distance_mm`, state/active_count and final status before
    tuning the 5,970+50/6,070 mm stop points. If the vehicle moves immediately at
    high speed
    or right bias remains during the 30 RPM second, inspect the actually
    flashed `build_id` and retain left/right RPM/PWM/targets plus line state
    before any further speed/finish changes. MC02 connection and protocol
    validation are explicitly deferred until the user asks to resume them.
11. Choose ICM42688 bus, pins, sample rate, mounting orientation, and fusion
    requirement before enabling `ICM42688_ENABLED`.

After any verified hardware measurement, update both the source constant and
this file in the same change. Never convert an assumption to a claimed fact
without a test record.

## 2026-07-29 MC02 gimbal firmware handoff

The new DAMIAO controller project is now at sibling path `../gimbal_MC02`.
Its controller is `DM-MC-Board02` (STM32H723VGT6), motor is
`DM-J4310-2EC V1.2`, and build ID is `2026072903`. The user explicitly chose
VSCode + CMake/Ninja/GCC for builds and J-Link + SEGGER Ozone over SWD for
download/debug; do not use or repair the copied MDK project.

The first build implements FDCAN1 classic standard CAN at 1 Mbit/s,
DM-J4310 position-speed mode with CAN ID `0x01` and Master/feedback ID
`0xF1`, USB CDC commissioning commands, and a desktop-safe state machine.
Boot does not enable the motor. Motion requires fresh feedback plus a
three-second arm, captures current position as local neutral, clamps relative
angle to +/-3 degrees and maximum speed to 0.20 rad/s, automatically stops
HOLD/STEP/SWEEP after 5/3/6 seconds, and faults/disables after 80 ms feedback
loss. The MC02 V1.1 schematic shows PC14/PC13 controlling OUT1/OUT2 through
NPN high-side driver stages. Build `2026072903` keeps both outputs low/off at
boot, leaves OUT2 permanently off, and adds a guarded USB-only OUT1 bench
path after the user selected integrated 2+2 power. `PWRARM 4310` opens a
three-second authorization window; `PWRON` supplies OUT1 for no more than
30 seconds; `PWROFF`, STOP, any fault, motion timeout or reset immediately
drives OUT1 low. Firmware waits 500 ms after PWRON and completes five disable
transmissions before OBSERVE/motion can start. This still requires an
unloaded voltage test before reconnecting the motor. Vision closed loop, TI
communication, and the five-test selector are intentionally disabled.

GCC 13.3.0 clean build was successful with no warnings in the captured
output. Artifacts are `../gimbal_MC02/build/Debug/gimbal_MC02.elf`, `.hex`,
and `.bin`; FLASH use is 74,116 bytes and DTCMRAM use is 12,200 bytes.
Ozone observes the single structure `g_gimbal_debug`. Full safety procedure,
USB commands, source-of-truth protocol facts, and unverified measurements are
recorded in `../gimbal_MC02/README.md` and `../gimbal_MC02/AGENT.MD`.

The first logic-only MC02 hardware check then passed with motor power/CAN,
OUT1/OUT2 loads, Jetson and TI disconnected. Ozone successfully downloaded
and ran build `2026072902`; `g_gimbal_debug` showed increasing uptime,
state 1/SAFE_IDLE, fault 0, startup TX count 5, TX error count 0, no received
CAN/vision data, and the expected `0xFFFFFFFF` never-received age sentinels.
This is not evidence that OUT1/OUT2, CAN wiring or motor motion are safe.
The user then reported the OUT1/OUT2 check had no problem, but did not provide
numeric input/output voltages; record those values during the next full
measurement pass rather than assuming zero.
The DM4310 is physically connected through the keyed MC02 OUT1/CAN1 2+2
connector. This removes connector-orientation ambiguity, but MC02 build
`2026072903` deliberately holds OUT1/PC14 off until its guarded USB sequence.
The user selected this integrated output path rather than a separate motor
power breakout.
The user reported the follow-up check with the keyed 2+2 cable installed was
normal, with no unexpected energization, but supplied no numeric voltage or
LED observation. Do not infer that motor power is available.
The user then flashed MC02 build `2026072903` with the DM4310 2+2 cable
unplugged and reported the default-off check was normal. Exact Watch values
and OUT1 voltage were not supplied. Guarded PWRON polarity/voltage remains
unverified and must be tested unloaded next.
The first PWRARM attempt lacked a transmitted CR/LF and was not executed.
After enabling CRLF and resetting, PING and `PWRARM 4310` returned the
expected replies and the 3-second arm countdown was visible in Ozone.
With the DM4310 2+2 cable still unplugged, the user then reported that the
unloaded `PWRARM 4310` -> `PWRON` -> `PWROFF` sequence behaved normally.
This qualitatively verifies guarded active-high OUT1 turn-on and manual
turn-off in MC02 build `2026072903`, but exact input/on/off voltage readings
were not supplied. With the motor still unplugged, the user then reported
that `PWRARM 4310` -> `PWRON` -> `STOP` produced every expected serial,
Watch and OUT1 cutoff result. The STOP-triggered OUT1 shutdown path is
therefore qualitatively verified. The user then reported that the unloaded
30-second automatic cutoff also behaved correctly. During the recent
voltage-check handling, an accidental spark occurred; its exact location and
any resulting damage are not yet known. Do not continue powered MC02 testing
until battery power is removed and the relevant connector, wiring, fuse,
probes and board are visually inspected. Do not repeat handheld live-probe
checks. Any essential future measurement must use insulated clips secured
with power off and removed only after power is disconnected; routine checks
should use Watch, serial and CAN telemetry. The user subsequently completed
the requested unpowered visual inspection and reported no problem; no burn
mark, deformation, loose conductor, abnormal heat/odor or other visible
damage was reported. This clears progression to motor-disabled CAN OBSERVE,
without further handheld live probing. The user then connected the keyed
DM4310 2+2 cable to MC02 OUT1/CAN1 while powered off. Powered CAN feedback
remains unverified; proceed with guarded PWRON and OBSERVE/STATUS only, with
no ARM or motion command. A subsequent source audit confirmed that PWRON
does not automatically enable motor torque: build `2026072903` waits 500 ms,
then completes five disable frames at 10 ms spacing, and blocks motion until
that burst finishes. Actual enable additionally requires fresh feedback, an
explicit short-lived ARM and a separate HOLD/STEP/SWEEP request. Earliest
possible enable is therefore later than approximately 540 ms and cannot
occur from PWRON alone; no code change was needed. The first powered OBSERVE
was accepted about 7 seconds after PWRON, but the captured STATUS was delayed
until after the 30-second OUT1 cutoff, so SAFE_IDLE/PWR=0 was expected.
Retained counters nevertheless showed `TX=1208`, `TXERR=0`, `RX=0`,
`ONLINE=0` and never-received age: MC02 queued many query frames during the
powered interval but received no DM4310 feedback. Do not ARM. Repeat one
short-window OBSERVE/STATUS capture; if RX stays zero, diagnose power/LED,
CAN wiring/termination and CAN IDs without enabling motion.
The short-window repeat then showed the DM4310 steady red, which its V1.2
manual defines as the normal disabled state. While OUT1 was still powered,
STATUS showed OBSERVE, fault 0, `RX=0`, `TX=1546`, `TXERR=0`, ONLINE=0 and
the never-received age. The manual recheck confirms query ID `0x001` is
valid for motor CAN ID `0x01` and feedback should use Master ID `0xF1`.
Current MC02 timing calculates to 1 Mbit/s from a 120 MHz FDCAN clock but
uses a 60% sample point, versus 75% in DAMIAO's dedicated MC02 CAN example.
Before changing firmware, capture live FDCAN1 PSR/ECR during OBSERVE to
separate no-ACK/bus faults from an RX/filter problem. Do not ARM.
The user additionally reports that Ozone's apparent download/program control
is disabled without an error, while USB PING proves build `2026072903` is
running. No MC02 code has changed, so this is not currently a flashing
blocker. Ozone's local manual and the `.jdebug` file show that a new debug
session in the default `Download & Reset Program` mode performs the download;
the disabled control likely reflects an already-active session, pending a UI
screenshot. Before any future reflash, STOP and remove motor power, stop the
active debug session, then start a new Download & Reset Program session.
The follow-up clarified that Ozone is not connected to the target at all, so
Watch data cannot be read and clicking Download & Reset produces no visible
action. Windows still enumerates the SEGGER driver, CDC COM3 and J-Link probe
serial `000069401064` correctly. Three standalone JLink Commander processes
created during diagnosis were found and terminated without terminating
Ozone. Because a direct 1 MHz SWD connection attempt stalled while Ozone was
open, the next safe diagnostic is to remove 6S/motor power, fully exit Ozone,
and test the SWD target with standalone J-Link Commander before reopening
Ozone.
After the Ozone window was closed, four windowless Ozone processes remained
and were terminated. With all Ozone/SEGGER processes gone, standalone J-Link
Commander still stalled at `Connecting to J-Link via USB...` and therefore
never attempted the STM32 SWD handshake, although Windows continued to show
the J-Link driver present and OK. The next action is to unplug only the
probe's PC-side USB cable for five seconds, reconnect it, and repeat the
standalone test before reopening Ozone.
After replugging the probe USB cable, the standalone 1 MHz SWD test passed:
J-Link serial `69401064` opened, reported target reference voltage
`3.296 V`, found SW-DP ID `0x6BA02477`, and identified the STM32H723
Cortex-M7 core. The test performed no flash programming and disconnected
normally. This verifies probe USB, target power/reference, SWD wiring and
device selection. Ozone should now be opened as a single fresh instance and
started in `Download & Reset Program` mode with 6S/motor power removed.
The next powered build `2026072903` OBSERVE produced FDCAN PSR/ECR snapshots
of `0x76F/168` followed by `0x708/0`: the controller first reached transmit
error-passive with TEC=168, then recovered after another CAN node began
acknowledging. STATUS nevertheless showed only three stale received motor
frames, `ERR=0xD` and feedback age 91,374 ms. The DM4310 therefore reported
communication loss and continuous feedback was not established. The user
also saw a brief green motor LED. Manual page 10 was rendered and checked:
green means enabled, red means disabled, flashing red means fault, and code
D is communication loss. Exact event timing and any shaft movement remain
unknown; do not ARM or repeat powered build `2026072903`.
Compile-tested build `2026072904` changes torque-disabled OBSERVE to repeat
the real position-speed disable frame on ID `0x101` every 20 ms instead of a
zero-filled ID `0x001` query. Fresh enabled-state or fault feedback during
OBSERVE now immediately faults MC02 and removes OUT1 power. The clean GCC
build passed with FLASH 74,156 bytes and DTCMRAM 12,200 bytes; it has not
been flashed or hardware-tested. Before flashing, disconnect 6S/motor
power. Before its first powered OBSERVE, write and read back motor CAN
Timeout=0 in the DAMIAO assistant.
The user then flashed build `2026072904` with motor power disconnected and
confirmed `build_id=2026072904`, SAFE_IDLE, `power_output_enabled=0` and
`fault=0`. This verifies the new image and default-off logic-only startup,
not the revised powered OBSERVE behavior. The next step remains to write
motor CAN Timeout=0 in the DAMIAO assistant and read it back as 0 before
reconnecting MC02 for the next powered OBSERVE.
At the user's request, compile-tested build `2026072905` extends the guarded
OUT1 auto-cutoff from 30 seconds to 90 seconds to allow enough time for
DAMIAO assistant parameter access. The three-second PWRARM window,
default-off behavior, and immediate PWROFF/STOP/fault/motion-timeout/reset
cutoffs are unchanged. HELP/PWRON text and commissioning documents now match
90 seconds. The clean GCC build passed with FLASH 74,156 bytes and DTCMRAM
12,200 bytes; build `2026072905` has not yet been flashed or hardware-tested.
The user then used the DAMIAO assistant to change motor CAN Timeout from 2000
to 0 and read the stored value back as 0. The next action is a powered,
torque-disabled build `2026072905` OBSERVE test. Do not ARM unless feedback
is continuous and fresh with ONLINE=1 and ERR=0.
Powered build `2026072905` OBSERVE subsequently passed: two STATUS captures
3.399 seconds apart had ONLINE=1, ERR=0, feedback ages 8/7 ms, and RX rising
from 1196 to 1366, exactly about 50 Hz. Position was stable near 0.969 rad.
This verifies continuous CAN feedback while repeatedly disabled, not torque
enable or motion. The motor flashed green once immediately at PWRON and then
settled to steady red. The manual does not define this momentary boot
transient; post-burst error/state 0 feedback verifies the disabled state.
Physical shaft motion during the flash has not been reported, so keep the
mechanism secured and the ball removed for the first HOLD test.
At the user's request, compile-tested build `2026072906` extends HOLD
auto-stop from 5 seconds to 15 seconds to allow time for observation and a
STATUS capture. The feedback watchdog, fault reactions, 3-second ARM window,
bounded command and immediate STOP/power-cutoff paths are unchanged. Serial
text and commissioning documents now match 15 seconds. The clean GCC build
passed with FLASH 74,156 bytes and DTCMRAM 12,200 bytes; it is not yet
flashed or hardware-tested.
Build `2026072906` was subsequently flashed and the first deliberate HOLD
passed. During three STATUS captures the motor remained ONLINE with fault 0,
enabled state 1, position 0.968-0.969 rad versus command 0.969 rad, fresh
0-9 ms feedback and no CAN queue errors. The user manually sent STOP after
about 13.55 seconds and received confirmation that the motor was disabled
and OUT1 off. This verifies stable current-position hold and manual cutoff;
physical vibration/noise was not explicitly reported. The next bounded test
is STEP +0.5 degrees with the ball removed and physical direction recorded.
The user then reported no visible rotation from a +1 degree STEP, while
noting that one degree may have been too small to see; no motion telemetry
was captured, so this is not yet evidence of a control failure. At the
user's request, compile-tested build `2026072907` raises the desktop relative
angle clamp from +/-3 degrees to +/-10 degrees for a visible direction test.
The 0.20 rad/s speed limit, feedback watchdogs, fault handling, STOP and
power cutoffs are unchanged, and the mechanical range was previously
reported as +/-30 degrees. The clean GCC build passed with FLASH 74,156 bytes
and DTCMRAM 12,200 bytes; it has not yet been flashed or hardware-tested.
Build `2026072907` was then flashed and a +10 degree STEP produced a clearly
visible 10-degree move. With the motor standing vertically on the table and
the PITCH plate mounted above it as photographed, positive command rotated
the plate counterclockwise when viewed from above. This verifies bounded
nonzero motion and establishes the sign only for this bench orientation;
the water-pipe sign must be rechecked after final horizontal vehicle
mounting. One SWEEP 10 test may replace separate reverse and repeated
small-step tests.
The user subsequently reported that `SWEEP 10` operated normally in both
directions. This completes bare-motor bench commissioning for the bounded
desktop path. With power removed, the next step is final mechanical mounting:
motor output axis horizontal, pipe longitudinal centerline perpendicular to
and centered on that axis, followed by one ball-free +10 degree sign check
to record which pipe end rises.

Three handoff documents were frozen on 2026-07-29:

```text
../gimbal_MC02/docs/JETSON_MC02_PROTOCOL_V3.md
../gimbal_MC02/docs/MC02_TI_PROTOCOL_V2.md
../gimbal_MC02/docs/MC02_WIRING_CHECKLIST.md
```

These documents define interfaces for implementation. Current TI build
2026073005 retains the V2 UART endpoint but temporarily enables standalone
KEY1 chassis start; current MC02 build 2026072903 still does not implement
the link. The inter-controller communication is therefore not connected,
not end-to-end tested and not a completed competition feature. Do not wire
the boards together until the user resumes this work and both ends have built
and passed standalone 3.3 V UART tests.

The proposed TI physical link is 3.3 V TTL UART at 115200 8N1 with common
ground and no power-rail connection. MC02 USART1 uses PA9 TX / PA10 RX. The
preferred currently-unused TI link is UART1 on PA8 TX / PA9 RX, preserving
the existing external SW1 on TI PA10. Because the LaunchPad may route these
UART1 pins to the onboard XDS110 backchannel through J101, the TI owner must
configure UART1 in SysConfig and isolate the relevant J101 UART jumpers
before attaching MC02; keep the SWD jumpers intact.

The V2 link uses framed packets with `A5 5A`, on-wire version `0x02`, type, little-endian
length/sequence, payload, and CRC16/CCITT-FALSE. MC02 sends a 10 Hz heartbeat
and explicit chassis commands; TI returns 20 Hz status plus events. Commands
are idempotent through `command_id`. Any 500 ms supervisor/link loss requires
the TI firmware to set both PWM channels to true zero, disarm, and report a
link-loss fault. Internal Q1 runs one lap and stops at A; Q2 keeps the chassis
stationary for the O -> +5 cm -> -5 cm ball test; Q3 drives from A until B
has been passed; Q4 and Q5 run one lap and stop at A.

The confirmed Button X8 mapping is K1-K5 = internal Q1-Q5, K6 reserved,
K7 = normal stop/reset to question-selection standby, and K8 = emergency
stop. TI must set both PWM outputs to true zero locally on K7 or K8 without
waiting for UART. K8 is latched; releasing it never restarts, and K7 is
required to return to standby after K8 is released and no nonrecoverable
fault remains. Because the buttons connect only to TI, K8 is not a guaranteed
whole-system hard stop for the gimbal if UART fails. Retain the accessible
master power disconnect or add an independent hardwired MC02 stop input.
Current TI build 2026073005 retains PA10/K1 button events and the UART
supervisor endpoint, but `STANDALONE_KEY1_START=1` temporarily gives K1 local
start/cancel/stop authority without MC02. K2-K8 inputs remain unimplemented.
The MC02-controlled Q1 path is source-level only and must not be described as
connected or operational.

## 2026-07-30 gimbal mechanical alternative

An end-hinged rigid crank-link mechanism driven by a DJI RoboMaster
M3508+C620 was proposed as an alternative to the frozen DM-J4310-2EC
eccentric-follower mechanism. It has not been selected and causes no TI
firmware or interface change. Current recommendation is to retain the
already-commissioned DM-J4310-2EC/MC02 control chain and, if practical,
borrow only the captive crank-link geometry to eliminate contact loss and
sliding. A future M3508 conversion would belong entirely to the gimbal side
and should use a pipe-hinge output-angle sensor to close the loop across
gearbox and linkage backlash.

The assembled DM4310 mechanism was initially reported as `L=236 mm`, but
later center-referenced measurements gave 115 mm from pipe midpoint to the
follower station and 133 mm from midpoint to the hinge, implying `L=248 mm`.
The 12 mm discrepancy equals the nominal follower diameter, so no gimbal
mapping may use either value until a direct axis-center-to-axis-center
remeasurement. The DM4310 axis is confirmed 1 mm above the hinge axis.
Nominal frozen dimensions remain crank radius 15.45 mm, follower diameter
12 mm, and pipe outside diameter 20 mm. Physical pipe travel is +/-125 mm
from O and scored targets are +/-50 mm. These measurements do not change TI
firmware or the V2 MC02-TI interface.

MC02 build `2026073001` is now compile-tested for installed-mechanism
commissioning. It adds torque-disabled `ZERO` capture and armed, bounded
zero-referenced `JOG` commands while retaining the existing +/-10 degree,
0.20 rad/s and cutoff protections. It has not yet been flashed or
hardware-tested and does not implement the MC02-TI binary link, autonomous
ball control, or any chassis behavior.

An attempted MC02 installed test remained on old build `2026072907` and had
no 6S motor supply. Its ONLINE=0/RX=0 result is expected and is not a TI,
CAN, or gimbal fault. Build `2026073001` still requires flashing and powered
standalone gimbal commissioning before any MC02-TI integration.

The user subsequently reported that the corrected MC02 `2026073001` flash,
6S motor power and OBSERVE stage were normal. This is a qualitative gimbal
pass without retained STATUS numbers; installed ZERO/JOG motion remains
unverified and TI integration is still not authorized.

MC02's first ball-free installed `JOG +10` subsequently passed without
contact loss, collision, jump or abnormal sound. Positive motor angle lowers
the motor-side pipe end. Approximate observed motion was 8 mm and the pipe
flexed slightly, so no precise gimbal geometry mapping is frozen yet. This
does not change TI firmware.

Two accepted MC02 `JOG -10` trials unexpectedly also lowered the motor-side
pipe end. This leaves the installed gimbal sign unresolved and suggests a
dead-point or power-off relaxation problem; a `JOG 0` return-to-level test is
pending. Do not begin TI/gimbal integrated motion from these results.

The follow-up `JOG 0` continued through the pipe's lowest point instead of
returning to the saved level pose. MC02 build `2026073002` is now
compile-tested to keep the DM4310 powered in torque-disabled OBSERVE after
each bounded JOG, preventing a motor reboot between ZERO/+10/0/-10, and to
print automatic start/end angle telemetry. It also accepts trailing tab/
whitespace on console commands and invalidates ZERO on any OUT1 cutoff.
This build has not yet been flashed or
hardware-tested; TI integration remains blocked on its installed sign test.

MC02 build `2026073002` later proved one +10 command numerically:
`P=-2.988 rad`, target/final `-2.813 rad`, exactly +0.175 rad. No pipe motion
was visible, supporting a mechanical dead-point diagnosis. Post-JOG power
then became unavailable due a combination of the 90-second window and a
possible transient enabled-state feedback during disable. Compile-tested
build `2026073003` adds a 200 ms disable-settling guard and a 180-second
calibration power window; it is not yet hardware-tested and changes no TI
behavior.

The user directly confirmed that the installed neutral eccentric follower is
at approximately 3 o'clock, not a vertical dead point. The previous
recommendation to lower the DM4310 by about 16 mm is withdrawn and must not
be acted on. The prior lack of visible pipe motion was likely an operator
observation issue; one uninterrupted MC02 +10/0/-10 sign test remains
pending before TI integration.
