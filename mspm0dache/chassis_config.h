#ifndef CHASSIS_CONFIG_H
#define CHASSIS_CONFIG_H

/*
 * Mechanical configuration for the 320 mm x 240 mm rear-drive chassis.
 * Distances are measured between wheel centerlines, not chassis edges.
 */
#define CHASSIS_OVERALL_LENGTH_MM          320.0f
#define CHASSIS_OVERALL_WIDTH_MM           240.0f
#define CHASSIS_DRIVE_TRACK_MM             214.2f
#define CHASSIS_AXLE_SPACING_MM            204.7f
#define CHASSIS_WHEEL_DIAMETER_MM           65.0f
#define CHASSIS_TIRE_CONTACT_WIDTH_MM       25.5f
#define CHASSIS_PI                           3.14159265f

/*
 * MG513XP28_12V: nominal 13 PPR motor-shaft Hall encoder and 1:28 gearbox.
 *
 * Calibrated on 2026-07-25 from four 10-wheel-revolution trials per side.
 * Right: 14673, 14665, 14674, 14706 -> 1467.95 counts/revolution.
 * Left:  14680, 14683, 14671, 14665 -> 1467.475 counts/revolution.
 */
#define MOTOR_GEAR_RATIO                    28.0f
#define ENCODER_SENSOR_NOMINAL_PPR          13.0f
#define ENCODER_LEFT_COUNTS_PER_WHEEL_REV  1467U
#define ENCODER_RIGHT_COUNTS_PER_WHEEL_REV 1468U

#define MOTOR_CTRL_PERIOD_MS                10U
#define MOTOR_PWM_MAX                      100
#define MOTOR_MAX_RPM                      350.0f
#define MOTOR_RPM_FILTER_ALPHA               0.25f
#define MOTOR_CLOSED_LOOP_PWM_LIMIT          40.0f
#define MOTOR_REVERSE_COAST_MS             1000U
#define MOTOR_DIRECTION_SETUP_MS             10U
#define MOTOR_REVERSE_SAFE_RPM                2.0f

/*
 * Re-verified by the user on 2026-07-28 after restoring the 5 V encoder
 * supply: vehicle-forward wheel rotation was negative on both corrected
 * channels with the 2026072504 flags. These flags invert both channels so
 * vehicle-forward is positive and reverse is negative.
 */
#define MOTOR_LEFT_OUTPUT_REVERSED           0
#define MOTOR_RIGHT_OUTPUT_REVERSED          0
#define ENCODER_LEFT_REVERSED                1
#define ENCODER_RIGHT_REVERSED               0

#define MOTOR_PID_DEFAULT_KP                 0.7f
#define MOTOR_PID_DEFAULT_KI                20.0f
#define MOTOR_PID_DEFAULT_KD                 0.0f

/* Conservative first-commissioning limits. */
#define CHASSIS_MAX_LINEAR_MM_S             300
#define CHASSIS_MAX_ANGULAR_MRAD_S         2000
#define TEST_OPEN_PWM_HARD_LIMIT             20
#define TEST_DEFAULT_OPEN_PWM                15
#define TEST_DEFAULT_DURATION_MS           5000U
#define TEST_MAX_DURATION_MS              30000U
#define TEST_ENCODER_PROGRESS_WINDOW_MS      750U
#define TEST_ENCODER_MIN_PROGRESS_COUNTS       2
#define TEST_ENCODER_MIN_RUN_DURATION_MS     2000U
#define TEST_MAX_ABS_DISTANCE_MM            2000.0f
#define TEST_MAX_ABS_TURN_ANGLE_DEG          720.0f

/*
 * Formal competition course:
 * - two 1500 mm straights;
 * - two radius-500 mm semicircles;
 * - nominal centerline length 6141.6 mm;
 * - 65 mm wheels need 90.2 RPM average to finish in 20 seconds.
 *
 * Q1's 143 RPM straight / 110 RPM curve envelope retains time for the
 * guarded launch and final reverse-marker stop without a step command.
 * Q3/Q4/Q5 keep their independent lower profiles below.
 */
#define STANDALONE_KEY1_START                    0
#define LINE_FOLLOW_DEFAULT_BASE_RPM         120.0f
#define LINE_FOLLOW_Q1_BASE_RPM              143.0f
#define LINE_FOLLOW_Q1_CURVE_MIN_BASE_RPM    110.0f
#define LINE_FOLLOW_Q1_MAX_WHEEL_RPM         176.0f
#define LINE_FOLLOW_Q3_CRUISE_RPM             100.0f
#define LINE_FOLLOW_Q3_APPROACH_RPM            45.0f
#define LINE_FOLLOW_Q3_APPROACH_START_MM     1250.0f
#define LINE_FOLLOW_Q3_FINAL_START_MM        1450.0f
#define LINE_FOLLOW_Q45_BASE_RPM               70.0f
#define LINE_FOLLOW_MIN_BASE_RPM              30.0f
#define LINE_FOLLOW_MAX_BASE_RPM             130.0f
#define LINE_FOLLOW_MAX_WHEEL_RPM            160.0f
#define LINE_FOLLOW_MIN_WHEEL_RPM             15.0f
#define LINE_FOLLOW_DEFAULT_STEERING_KP        12.0f
#define LINE_FOLLOW_DEFAULT_STEERING_KD         6.0f
#define LINE_FOLLOW_MAX_STEERING_KP            25.0f
#define LINE_FOLLOW_MAX_STEERING_KD            20.0f
#define LINE_FOLLOW_MAX_D_CORRECTION_RPM       12.0f
#define LINE_FOLLOW_MAX_CORRECTION_RPM         45.0f
#define LINE_FOLLOW_CURVE_MIN_BASE_RPM        100.0f
#define LINE_FOLLOW_LAUNCH_BASE_RPM            55.0f
#define LINE_FOLLOW_LAUNCH_DISTANCE_MM        300.0f
#define LINE_FOLLOW_START_BASE_RPM              30.0f
#define LINE_FOLLOW_START_DURATION_MS         1000U
#define LINE_FOLLOW_START_STRAIGHT_MM         100.0f
#define LINE_FOLLOW_POSITION_FILTER_ALPHA       0.25f
#define LINE_FOLLOW_ACCEL_SLEW_RPM_PER_S       80.0f
#define LINE_FOLLOW_DECEL_SLEW_RPM_PER_S      250.0f
#define LINE_FOLLOW_Q3_ACCEL_RPM_PER_S          30.0f
#define LINE_FOLLOW_Q3_DECEL_RPM_PER_S          30.0f
#define LINE_FOLLOW_Q45_ACCEL_RPM_PER_S         14.0f
#define LINE_FOLLOW_Q45_DECEL_RPM_PER_S         14.0f
#define LINE_FOLLOW_STEERING_SLEW_RPM_PER_S    300.0f
#define LINE_FOLLOW_SENSOR_GRACE_MS           100U
#define LINE_FOLLOW_START_VALID_SAMPLES          5U
#define LINE_FOLLOW_START_SENSOR_TIMEOUT_MS    500U
#define LINE_FOLLOW_LINE_LOSS_HOLD_MS           50U
#define LINE_FOLLOW_LINE_LOSS_RECOVERY_MS      300U
#define LINE_FOLLOW_RECOVERY_BASE_RPM          45.0f
#define LINE_FOLLOW_RECOVERY_CORRECTION_RPM    28.0f
#define LINE_FOLLOW_TEST_DURATION_MS          5000U
#define LINE_FOLLOW_LAP_DURATION_MS          40000U
#define LINE_FOLLOW_MAX_DURATION_MS         120000U

/*
 * Formal competition-course Q1 finish:
 * - starting on the transverse A bar still uses 100 mm of symmetric
 *   low-speed drive before normal centroid steering;
 * - the private two-turn/distance estimate starts a bounded forward
 *   alignment segment near A; the independent distance bound starts the same
 *   segment if the turn estimate is unavailable;
 * - the vehicle follows into the next straight, settles at true zero, then
 *   reverses slowly with equal wheel targets;
 * - ordinary centered line uses one or two channels. A is accepted only when
 *   at least three channels are simultaneously active in three consecutive
 *   fresh sensor samples;
 * - reverse distance and time bounds stop safely if A is not recognized;
 *   an intact, fresh sensor may report zero active channels while the front
 *   array trails the line during this bounded straight reverse.
 * Q3/Q4/Q5 do not enter this reverse sequence. Their shared gentle profile
 * slews the straight/common wheel component at 50 RPM/s while retaining a
 * faster steering-difference response, then ramps to zero before disarming.
 */
#define LINE_FINISH_ARM_DISTANCE_MM           150.0f
#define LINE_COURSE_STRAIGHT_MM              1500.0f
#define LINE_COURSE_RADIUS_MM                 500.0f
#define LINE_COURSE_EXPECTED_LAP_MM          6141.6f
#define LINE_TURN1_MIN_DISTANCE_MM           1300.0f
#define LINE_TURN1_MIN_ABS_YAW_DEG            100.0f
#define LINE_TURN2_MIN_DISTANCE_MM           4500.0f
#define LINE_TURN2_MIN_ABS_YAW_DEG            250.0f
#define LINE_FINISH_TURN_ASSIST_DISTANCE_MM  5970.0f
#define LINE_FINISH_MAX_LAP_DISTANCE_MM      6070.0f
#define LINE_FINISH_APPROACH_RPM                20.0f
#define LINE_FINISH_FORWARD_ALIGN_RPM           60.0f
#define LINE_FINISH_FORWARD_ALIGN_MIN_MM        150.0f
#define LINE_FINISH_FORWARD_ALIGN_MAX_MM        250.0f
#define LINE_FINISH_CENTER_MAX_POSITION_X1000  1000
#define LINE_FINISH_CENTER_SAMPLES               8U
#define LINE_FINISH_SETTLE_MIN_MS              200U
#define LINE_FINISH_SETTLE_MAX_MS             1200U
#define LINE_FINISH_REVERSE_RPM                 25.0f
#define LINE_FINISH_REVERSE_ARM_MM              40.0f
#define LINE_FINISH_REVERSE_MAX_MM             400.0f
#define LINE_FINISH_REVERSE_TIMEOUT_MS        6000U
#define LINE_FINISH_MARKER_RECENT_MS           120U
#define LINE_FINISH_MARKER_MIN_ACTIVE_CHANNELS    3U
#define LINE_FINISH_MARKER_CONFIRM_SAMPLES       3U
#define LINE_FINISH_GENTLE_STOP_MAX_MS         8000U

#endif
