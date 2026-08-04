#ifndef GIMBAL_CONFIG_H
#define GIMBAL_CONFIG_H

/*
 * Desktop-safe build for DAMIAO DM-MC-Board02 + DM-J4310-2EC V1.2.
 *
 * Verified project settings from the motor assistant:
 *   supply: 24 V nominal (6S battery)
 *   motor CAN ID: 0x01
 *   motor Master ID / feedback CAN ID: 0xF1
 *   control mode: position-speed mode
 *   CAN: classic standard frame, 1 Mbit/s
 *
 * Build 2026072903 adds an explicitly armed, time-limited OUT1 bench-power
 * path. Build 2026072904 makes torque-disabled OBSERVE repeat the selected
 * mode's disable frame so the DM4310 returns continuous feedback. Build
 * Build 2026072905 extends guarded OUT1 power from 30 s to 90 s for parameter
 * setup. Build 2026072906 extends HOLD auto-stop from 5 s to 15 s for
 * operator observation. Build 2026072907 raises the bounded desktop relative
 * angle from +/-3 degrees to +/-10 degrees after stable HOLD verification;
 * every immediate cutoff path and the 0.20 rad/s slew limit remain unchanged.
 * Build 2026073001 adds installed-mechanism ZERO and zero-referenced JOG
 * commands without increasing any motion, speed, time, or power limit.
 * Build 2026073002 keeps OUT1 on but explicitly disables torque and returns
 * to OBSERVE after JOG, preserving one motor coordinate frame for
 * +offset/zero/-offset calibration. STOP/fault/reset and the 90 s power
 * timeout still remove OUT1 power.
 * Build 2026073003 allows 200 ms for enabled feedback to clear after the
 * explicit post-JOG disable and extends the guarded calibration power window
 * to 180 s. Motion bounds and immediate STOP/fault cutoffs are unchanged.
 * Build 2026073004 adds one explicitly named manual-only CALRUN command for
 * the complete +angle/zero/-angle/zero installed test. Disable
 * GIMBAL_ENABLE_MANUAL_CALRUN before any vehicle build.
 * Build 2026073005 keeps sending torque-disable/feedback-request frames
 * while CALRUN waits for the motor to boot, extends that preflight to 15 s,
 * and returns a no-feedback preflight timeout to safe idle for a clean retry.
 * Build 2026073006 expands only the manual CALRUN path to +/-30 degrees with
 * five-second legs for the next ball-free linkage test. Normal STEP/JOG/
 * SWEEP remain limited to +/-10 degrees and Vehicle still removes CALRUN.
 * Build 2026073007 expands that same Debug-only ball-free CALRUN envelope to
 * +/-45 degrees after the installed +/-30-degree contact test passed. Speed,
 * acceleration parameters, leg time and all other command limits are unchanged.
 * Build 2026073008 fixes run_motion retaining the normal +/-10-degree clamp
 * during CALRUN. Both command admission and execution now select the same
 * CALRUN-only limit; ordinary commands and Vehicle behavior stay unchanged.
 * Build 2026073101 adds a separate DelayTest-only fixed -1.5-degree pipe
 * pulse, empty-mechanism validation/ack gates and 100 Hz V3 status. Normal
 * Debug/Release/Vehicle builds compile the complete entry out.
 * Build 2026073102 renews the existing 180-second DelayTest OUT1 guard on
 * each accepted empty-validation, acknowledgement or measurement request,
 * allowing repeated measurements without removing the unattended timeout.
 * Build 2026073103 extends the global guarded OUT1 timeout from 180 seconds
 * to 300 seconds for two-operator total-loop-delay measurement. Immediate
 * STOP, fault, reset and link/watchdog cutoff behavior is unchanged.
 * Build 2026073104 replaces DelayTest's noisy instantaneous visual-velocity
 * admission gate with a bounded 300 ms / 2 mm position-stability window.
 * Jetson V3 and all non-DelayTest behavior remain unchanged.
 * Build 2026073105 applies the measured 125 ms full-loop delay in the Vehicle
 * ball controller by projecting the latest calibrated position with bounded
 * constant velocity. The existing V3 processing-latency field is decoded for
 * diagnostics; no wire format or actuator safety limit changes.
 * Build 2026073106 preserves an already-latched gimbal fault when the TI
 * supervisor propagates SAFE_STOP. A real TI fault still maps to TI_LINK,
 * and the competition-required five-second Q2 limit is unchanged.
 * Build 2026073107 adds one bounded +1.2-degree / 350 ms pipe breakaway
 * directional pulse when Q2 is stationary away from either target. The
 * positive-to-negative transition also starts the second-leg pulse directly
 * because the target settle gate already proves that the ball is stationary.
 * Each leg can pulse at most once, then returns to the existing
 * delay-compensated PD controller; stale vision, STOP, faults and the
 * five-second Q2 limit still cancel actuator motion.
 * Build 2026073109 adds an aggressive Q2-only static-friction floor after
 * hardware trials showed the ball stopping around +32 mm while the motor
 * continued to dither.  Away from the target and below 30 mm/s, Q2 now
 * holds at least 1.2 degrees in the required direction.  The one-shot
 * breakaway pulse uses the existing 2-degree pipe envelope for 650 ms,
 * while faster Kp/Kd, settle, stall-detection and motor-slew defaults reduce
 * dead time.  All fixed angle/motor envelopes, stale-vision handling,
 * STOP/fault paths and the five-second deadline remain unchanged.
 * Build 2026073110 keeps the Q2 static-friction floor and 3 rad/s actuator
 * response, but separates true stall recovery from normal acceleration.
 * Hardware build-3109 crossed +5 cm at about 129.5 mm/s, did not start
 * braking until +6.15 cm, and peaked near +13.36 cm.  Kp/Kd are therefore
 * rebalanced for earlier velocity braking, the floor is limited to nearly
 * stationary motion, and the shorter 1.5-degree pulse is delayed long enough
 * for normal actuator/linkage response before declaring a stall.
 * Build 2026073111 treats Q2's +50 mm point as an intermediate waypoint
 * rather than spending most of the five-second budget settling there. The
 * waypoint uses the unchanged +/-7 mm position band, a 100 mm/s velocity
 * limit and a 60 ms dwell. The final -50 mm target retains the stricter
 * 60 mm/s and 150 ms completion gate. All controller, friction, actuator and
 * safety bounds from build 3110 remain unchanged.
 * Build 2026073112 adds final-target capture scheduling after build-3111
 * reached both legs but oscillated around -50 mm.
 * Build 2026073113 replaces the symmetric capture clamp after hardware
 * showed that it also removed the braking authority needed at roughly
 * 275 mm/s. Once Q2 first enters 30 mm of the final target, capture mode is
 * latched for the rest of the run. Commands that oppose the measured ball
 * velocity may still use the full fixed 2-degree envelope, while commands
 * that do not remove kinetic energy are limited to 0.6 degree. The 1.2-
 * degree static-friction floor remains disabled after capture. This
 * asymmetric energy-removal schedule also accounts for the newly polished,
 * lower-friction pipe without weakening emergency braking.
 * Build 2026073114 separates the printed five-second performance checkpoint
 * from the powered-control safety deadline. Q2 may not report completion
 * before five seconds, so an early 150 ms settle cannot make TI disable the
 * motor just before the scoring checkpoint. If the ball is not settled at
 * five seconds, closed-loop capture continues, with the hard Q2 timeout
 * extended to 15 seconds. The final -50 mm point must remain continuously
 * inside its position/velocity gate for 1000 ms so stability is visibly
 * verifiable; the +50 mm immediate-turn waypoint remains 60 ms. Runs that
 * settle after five seconds remain tuning failures even though the extra
 * window preserves control for diagnosis.
 * Build 2026073115 addresses a low-probability Q2 positive-leg stall observed
 * around +37 mm. The original single breakaway pulse could be consumed by
 * an earlier temporary pause around +27 mm, leaving only the 1.2-degree
 * static-friction floor for the later true stall. The positive leg may now
 * use at most two separately detected 1.5-degree / 400 ms pulses. Every
 * pulse still requires a fresh 150 ms / 2 mm stationary window; the negative
 * leg remains limited to one pulse. All angle and timing safety bounds remain.
 * Build 2026073117 follows two build-3116 runs that removed the positive-leg
 * stalls but exposed excess energy on both legs. Positive braking is restored
 * after +30 mm rather than +35 mm; the low-speed -2.0-degree anti-stall
 * request is unchanged. The +50 mm transition uses one bounded +1.5-degree /
 * 250 ms launch pulse, then all further negative-leg drive-sign pipe request
 * is capped at +0.8 degree regardless of measured velocity sign. Full
 * -2.0-degree braking remains available. Targets, completion gates, actuator
 * envelopes and every immediate safety cutoff are unchanged.
 * Build 2026073118 removes the remaining positive-waypoint settle gate after
 * build-3117 logs showed a normal-run +72.7 mm median positive peak and one
 * pass-through to +125 mm. On the first valid sample at or beyond +50 mm,
 * Q2 now switches directly to the -50 mm target and starts the existing
 * bounded return pulse; no positive-waypoint velocity or dwell condition is
 * required. Both leg-specific drive schedules and all final-target and safety
 * gates are unchanged.
 * Build 2026073119 compensates the waypoint decision itself for the measured
 * full-loop delay. Five build-3118 runs switch in logged data at +56.0..
 * +59.1 mm and then peak at +59.1..+79.5 mm. Q2 now starts the existing
 * return when either raw position has reached +50 mm or the same bounded,
 * 125 ms delay-projected position already reaches +50 mm. This anticipates
 * actuator/vision latency without changing drive, braking or final capture.
 * Build 2026073120 fixes only negative-leg rebound braking authority. The
 * +0.8-degree positive-pipe cap still limits drive while the ball moves
 * toward -50 mm or is nearly stationary, but is released once measured
 * positive rebound speed reaches 30 mm/s. Positive pipe angle then removes
 * rebound energy with the existing fixed 2-degree envelope. No actuator
 * bound, gain, target, breakaway pulse, friction floor or timeout is changed.
 * Build 2026073121 replaces the Jetson velocity field with a 70 ms fixed-
 * window chord velocity estimate. Two Q2 low-speed drive floors now require
 * a 2 mm / 120 ms stationary window. Angle envelopes, gains, targets,
 * timeouts and all immediate safety paths are unchanged.
 * Build 2026073124 gates the TI V2 wheel-RPM acceleration estimate with new
 * common-acceleration/common-deceleration status flags. Q3-Q5 therefore use
 * chassis feed-forward only while TI is intentionally slewing common speed;
 * steady cruise forces it to zero. Q1's protocol timeout is widened to 40 s
 * for safe finish-marker searching. Jetson V3 and the TI V2 wire layout are
 * unchanged.
 * Build 2026073125 gives every intentional TI common-speed ramp an immediate
 * signed 0.17 m/s^2 feed-forward floor, while retaining the measured wheel-
 * RPM estimate above that floor and forcing steady cruise back to zero.
 * Q3-Q5 route completion now keeps the DM4310 closed loop enabled for at
 * least five seconds after the chassis reports stopped; the existing three-
 * second continuous ball-settle test can extend that hold up to the unchanged
 * fifteen-second safety deadline. Emergency stop, link loss and invalid
 * vision still cut power immediately. Jetson V3 and TI V2 layouts are
 * unchanged.
 * Build 2026073126 follows the measured timing margin for Q3-Q5 by reducing
 * only their TI common-wheel acceleration/deceleration slew from 50 to
 * 40 rpm/s. MC02's immediate feed-forward floor is matched to 0.14 m/s^2.
 * Cruise targets, steering slew, route distances, scored timeouts, post-stop
 * hold and every immediate safety path are unchanged.
 * Build 2026080101 uses the available timing margin to make the moving-ball
 * tasks substantially gentler: Q3 common acceleration/deceleration is
 * 30 rpm/s, while Q4/Q5 use 20 rpm/s. The corresponding immediate chassis
 * feed-forward floors are 0.10 and 0.07 m/s2. When TI's commanded-ramp flag
 * clears, MC02 linearly decays the last bounded feed-forward over 500 ms so
 * motor/chassis inertia is not left uncompensated. The Q3-Q5 local gentle-
 * stop guard is eight seconds; scored 8/30-second total bounds and all real
 * fault cutoffs remain unchanged. Jetson V3 and TI V2 layouts are unchanged.
 * Build 2026080104 keeps Q3 fixed and reduces only Q4/Q5 cruise and common
 * acceleration/deceleration commands by 30 percent: 70 rpm and 14 rpm/s.
 * Their immediate feed-forward floor is matched to 0.048 m/s2. The 30-second
 * scored timeout, eight-second gentle-stop guard, steering slew, Q2 changes,
 * all safety paths and both wire protocols remain unchanged.
 * Build 2026080107 applies only a small asymmetric Q2 trim on top of the
 * selective build-0101 rollback: slightly less positive launch energy and a
 * later/stronger negative approach. Q1 and the accepted Q3/Q4/Q5 behavior
 * remain unchanged. Jetson V3/TI V2 are unchanged.
 */

#define GIMBAL_BUILD_ID                    2026080107UL
#define GIMBAL_DESKTOP_BUILD               1U
#ifndef GIMBAL_ENABLE_MANUAL_CALRUN
#define GIMBAL_ENABLE_MANUAL_CALRUN         1U
#endif
#ifndef GIMBAL_ENABLE_DELAY_TEST
#define GIMBAL_ENABLE_DELAY_TEST            0U
#endif

#define DM4310_MOTOR_CAN_ID                0x01U
#define DM4310_MASTER_CAN_ID               0xF1U
#define DM4310_POSITION_SPEED_TX_ID         (0x100U + DM4310_MOTOR_CAN_ID)
#define DM4310_QUERY_TX_ID                  DM4310_MOTOR_CAN_ID

#define DM4310_PMAX_RAD                     12.5f
#define DM4310_VMAX_RAD_S                   30.0f
#define DM4310_TMAX_NM                      10.0f

#define GIMBAL_DESKTOP_MAX_ANGLE_DEG        10.0f
#define GIMBAL_CALRUN_MAX_ANGLE_DEG          45.0f
#define GIMBAL_DESKTOP_MAX_SLEW_RAD_S       0.20f
#define GIMBAL_MOTOR_COMMAND_PERIOD_MS      10U
#define GIMBAL_OBSERVE_QUERY_PERIOD_MS      20U
#define GIMBAL_FEEDBACK_TIMEOUT_MS          80U
#define GIMBAL_ARM_WINDOW_MS                3000U
#define GIMBAL_HOLD_TIMEOUT_MS              15000U
#define GIMBAL_STEP_TIMEOUT_MS              3000U
#define GIMBAL_SWEEP_TIMEOUT_MS             6000U
#define GIMBAL_DISABLE_BURST_COUNT          5U
#define GIMBAL_POWER_ARM_WINDOW_MS          3000U
#define GIMBAL_POWER_ON_TIMEOUT_MS          300000U
#define GIMBAL_POWER_STARTUP_SETTLE_MS      500U
#define GIMBAL_POST_JOG_DISABLE_SETTLE_MS   200U
#define GIMBAL_CALRUN_PREP_TIMEOUT_MS       15000U
#define GIMBAL_CALRUN_COUNTDOWN_MS          3000U
#define GIMBAL_CALRUN_LEG_TIMEOUT_MS         5000U

/*
 * Dedicated total-loop-delay test.  This is compiled only by the DelayTest
 * preset.  It deliberately exposes one fixed pulse rather than a general
 * motion command:
 *   pipe 0 -> -1.5 deg for 500 ms -> 0
 *
 * The exact pulse must pass a ball-free validation and receive an Ozone
 * acknowledgement before a ball-present measurement can be triggered.
 */
#define GIMBAL_DELAY_TEST_ARM_KEY           0xD31A7E57UL
#define GIMBAL_DELAY_TEST_ARM_WINDOW_MS     3000U
#define GIMBAL_DELAY_TEST_PREP_TIMEOUT_MS  15000U
#define GIMBAL_DELAY_TEST_COUNTDOWN_MS      3000U
#define GIMBAL_DELAY_TEST_HOLD_MS            500U
#define GIMBAL_DELAY_TEST_RETURN_TIMEOUT_MS 2000U
#define GIMBAL_DELAY_TEST_RETURN_SETTLE_MS   100U
#define GIMBAL_DELAY_TEST_PIPE_ANGLE_MDEG  -1500L
#define GIMBAL_DELAY_TEST_MOTOR_SPEED_RAD_S 12.0f
#define GIMBAL_DELAY_TEST_RETURN_POS_RAD     0.0175f
#define GIMBAL_DELAY_TEST_RETURN_VEL_RAD_S   0.50f
#define GIMBAL_DELAY_TEST_START_POS_UM      10000L
#define GIMBAL_DELAY_TEST_STABLE_WINDOW_MS   300U
#define GIMBAL_DELAY_TEST_STABLE_SPAN_UM    2000L
#define GIMBAL_DELAY_TEST_STABLE_SAMPLES       8U
#define GIMBAL_NORMAL_STATUS_PERIOD_MS       100U
#define GIMBAL_DELAY_TEST_STATUS_PERIOD_MS    10U

#define GIMBAL_VISION_STALE_MS              100U
#define GIMBAL_VISION_LOST_MS               500U
#define GIMBAL_JETSON_HEARTBEAT_LOST_MS     500U

/* Competition controller and measured installed linkage. */
#define GIMBAL_PIPE_PER_MOTOR_RATIO          0.044f
#define GIMBAL_COMP_MAX_MOTOR_ANGLE_DEG     45.0f
#define GIMBAL_COMP_MAX_PIPE_ANGLE_DEG       2.0f
#define GIMBAL_COMP_MAX_SLEW_RAD_S           3.00f
#define GIMBAL_BALL_ACCEL_GAIN_M_S2_RAD      7.0f
#define GIMBAL_GRAVITY_M_S2                  9.80665f
#define GIMBAL_ROLLING_INVERSE_FACTOR        1.4f
#define GIMBAL_BALL_KP_S2                    4.0f
#define GIMBAL_BALL_KD_S                     2.5f
#define GIMBAL_CHASSIS_FORWARD_TO_X_SIGN     1.0f
#define GIMBAL_CHASSIS_ACCEL_FF_GAIN         1.0f
#define GIMBAL_TOTAL_DELAY_MS                125U
#define GIMBAL_TOTAL_DELAY_MAX_MS            250U
#define GIMBAL_PREDICTED_POSITION_LIMIT_UM 130000L
#define GIMBAL_VELOCITY_HISTORY_DEPTH          8U
#define GIMBAL_VELOCITY_WINDOW_MS             70U
#define GIMBAL_VELOCITY_WINDOW_MIN_MS         50U
#define GIMBAL_VELOCITY_WINDOW_MAX_MS        120U
#define GIMBAL_TARGET_TOLERANCE_UM         7000L
#define GIMBAL_TARGET_VELOCITY_TOL_UM_S   60000L
#define GIMBAL_TARGET_SETTLE_MS             150U
#define GIMBAL_ROUTE_SETTLE_POSITION_UM     5000L
#define GIMBAL_ROUTE_SETTLE_VELOCITY_UM_S  30000L
#define GIMBAL_ROUTE_SETTLE_MS              3000U
#define GIMBAL_ROUTE_POST_STOP_HOLD_MS      5000U
#define GIMBAL_ROUTE_SETTLE_TIMEOUT_MS     15000U
#define GIMBAL_Q2_FINAL_SETTLE_MS           1000U
#define GIMBAL_Q2_EARLIEST_COMPLETE_MS      5000U
#define GIMBAL_Q2_TIMEOUT_MS               15000U
#define GIMBAL_Q2_POSITIVE_TARGET_UM       50000L
#define GIMBAL_Q2_NEGATIVE_TARGET_UM      -50000L
#define GIMBAL_Q2_POSITIVE_BREAKAWAY_PIPE_MDEG 2000L
#define GIMBAL_Q2_NEGATIVE_BREAKAWAY_PIPE_MDEG 1500L
#define GIMBAL_Q2_POSITIVE_BREAKAWAY_MS      350U
#define GIMBAL_Q2_NEGATIVE_BREAKAWAY_MS      250U
#define GIMBAL_Q2_POSITIVE_BREAKAWAY_MAX       2U
#define GIMBAL_Q2_NEGATIVE_BREAKAWAY_MAX       1U
#define GIMBAL_Q2_STALL_ARM_DELAY_MS          650U
#define GIMBAL_Q2_STALL_WINDOW_MS             150U
#define GIMBAL_Q2_STALL_SPAN_UM              2000L
#define GIMBAL_Q2_STATIONARY_MS               120U
#define GIMBAL_Q2_MIN_DRIVE_PIPE_MDEG        1200L
#define GIMBAL_Q2_POSITIVE_NO_BRAKE_END_UM  30000L
#define GIMBAL_Q2_POSITIVE_KEEP_DRIVE_MDEG    600L
#define GIMBAL_Q2_POSITIVE_LOW_SPEED_MDEG    1800L
#define GIMBAL_Q2_NEGATIVE_DRIVE_CAP_MDEG    1000L
#define GIMBAL_Q2_REBOUND_BRAKE_UM_S        30000L
#define GIMBAL_Q2_FINAL_CAPTURE_ERROR_UM     20000L
#define GIMBAL_Q2_FINAL_CAPTURE_PIPE_MDEG      600L

#define GIMBAL_DEBUG_ARM_KEY                0xA55A5AA5UL

/*
 * Frozen printed mechanism geometry. The installed hinge-to-follower
 * distance is deliberately not defined until the conflicting 236/248 mm
 * measurements are resolved. These constants are not used for motion yet.
 */
#define GIMBAL_CRANK_RADIUS_MM               15.45f
#define GIMBAL_FOLLOWER_DIAMETER_MM          12.0f
#define GIMBAL_PIPE_OUTER_DIAMETER_MM        20.0f

/*
 * OUT1/PC14 only. OUT2/PC13 remains permanently low/off.
 * High-active behavior follows the MC02 V1.1 schematic and still requires
 * the first unloaded voltage test before a motor is connected.
 */
#define GIMBAL_ALLOW_BOARD_POWER_OUTPUT     1U

#endif
