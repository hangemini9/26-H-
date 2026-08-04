$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot

function Invoke-Checked {
    param(
        [Parameter(Mandatory = $true)]
        [string] $Program,
        [Parameter(ValueFromRemainingArguments = $true)]
        [string[]] $Arguments
    )
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$Program failed with exit code $LASTEXITCODE"
    }
}

function Get-Crc16CcittFalse {
    param([byte[]] $Data)
    [uint16] $crc = 0xFFFF
    foreach ($byte in $Data) {
        $crc = [uint16] ($crc -bxor ([uint16] $byte -shl 8))
        for ($bit = 0; $bit -lt 8; $bit++) {
            if (($crc -band 0x8000) -ne 0) {
                $crc = [uint16] (
                    (($crc -shl 1) -bxor 0x1021) -band 0xFFFF)
            } else {
                $crc = [uint16] (($crc -shl 1) -band 0xFFFF)
            }
        }
    }
    return $crc
}

Push-Location $projectRoot
try {
    foreach ($preset in @("Debug", "Release", "Vehicle", "DelayTest")) {
        Write-Host "Building $preset"
        Invoke-Checked "cmake" "--preset" $preset
        Invoke-Checked "cmake" "--build" "--preset" $preset "--clean-first"
    }

    foreach ($required in @(
        "Core\Src\gimbal_app.c",
        "Core\Src\jetson_link.c",
        "Core\Src\ti_link.c",
        "Core\Inc\jetson_link.h",
        "docs\JETSON_MC02_PROTOCOL_V3.md",
        "docs\MC02_TI_PROTOCOL_V2.md",
        "docs\FIVE_QUESTION_CONTROL.md",
        "docs\DELAY_TEST_FIRMWARE.md",
        "gimbal_MC02_Vehicle.jdebug",
        "build\Debug\gimbal_MC02.elf",
        "build\Release\gimbal_MC02.elf",
        "build\Vehicle\gimbal_MC02.elf",
        "build\DelayTest\gimbal_MC02.elf"
    )) {
        if (-not (Test-Path -LiteralPath $required)) {
            throw "Required file missing: $required"
        }
    }

    $config = Get-Content -LiteralPath "Core\Inc\gimbal_config.h" -Raw
    foreach ($pattern in @(
        "GIMBAL_BUILD_ID\s+2026080107UL",
        "GIMBAL_POWER_ON_TIMEOUT_MS\s+300000U",
        "GIMBAL_PIPE_PER_MOTOR_RATIO\s+0\.044f",
        "GIMBAL_COMP_MAX_MOTOR_ANGLE_DEG\s+45\.0f",
        "GIMBAL_COMP_MAX_PIPE_ANGLE_DEG\s+2\.0f",
        "GIMBAL_COMP_MAX_SLEW_RAD_S\s+3\.00f",
        "GIMBAL_BALL_KP_S2\s+4\.0f",
        "GIMBAL_BALL_KD_S\s+2\.5f",
        "GIMBAL_GRAVITY_M_S2\s+9\.80665f",
        "GIMBAL_ROLLING_INVERSE_FACTOR\s+1\.4f",
        "GIMBAL_CHASSIS_FORWARD_TO_X_SIGN\s+1\.0f",
        "GIMBAL_CHASSIS_ACCEL_FF_GAIN\s+1\.0f",
        "GIMBAL_TOTAL_DELAY_MS\s+125U",
        "GIMBAL_TOTAL_DELAY_MAX_MS\s+250U",
        "GIMBAL_PREDICTED_POSITION_LIMIT_UM\s+130000L",
        "GIMBAL_VELOCITY_HISTORY_DEPTH\s+8U",
        "GIMBAL_VELOCITY_WINDOW_MS\s+70U",
        "GIMBAL_VELOCITY_WINDOW_MIN_MS\s+50U",
        "GIMBAL_VELOCITY_WINDOW_MAX_MS\s+120U",
        "GIMBAL_ROUTE_SETTLE_POSITION_UM\s+5000L",
        "GIMBAL_ROUTE_SETTLE_VELOCITY_UM_S\s+30000L",
        "GIMBAL_ROUTE_SETTLE_MS\s+3000U",
        "GIMBAL_ROUTE_POST_STOP_HOLD_MS\s+5000U",
        "GIMBAL_ROUTE_SETTLE_TIMEOUT_MS\s+15000U",
        "GIMBAL_Q2_POSITIVE_TARGET_UM\s+50000L",
        "GIMBAL_Q2_NEGATIVE_TARGET_UM\s+-50000L",
        "GIMBAL_Q2_FINAL_SETTLE_MS\s+1000U",
        "GIMBAL_Q2_EARLIEST_COMPLETE_MS\s+5000U",
        "GIMBAL_Q2_TIMEOUT_MS\s+15000U",
        "GIMBAL_Q2_POSITIVE_BREAKAWAY_PIPE_MDEG\s+2000L",
        "GIMBAL_Q2_NEGATIVE_BREAKAWAY_PIPE_MDEG\s+1500L",
        "GIMBAL_Q2_POSITIVE_BREAKAWAY_MS\s+350U",
        "GIMBAL_Q2_NEGATIVE_BREAKAWAY_MS\s+250U",
        "GIMBAL_Q2_POSITIVE_BREAKAWAY_MAX\s+2U",
        "GIMBAL_Q2_NEGATIVE_BREAKAWAY_MAX\s+1U",
        "GIMBAL_Q2_STALL_ARM_DELAY_MS\s+650U",
        "GIMBAL_Q2_STALL_WINDOW_MS\s+150U",
        "GIMBAL_Q2_STALL_SPAN_UM\s+2000L",
        "GIMBAL_Q2_STATIONARY_MS\s+120U",
        "GIMBAL_Q2_MIN_DRIVE_PIPE_MDEG\s+1200L",
        "GIMBAL_Q2_POSITIVE_NO_BRAKE_END_UM\s+30000L",
        "GIMBAL_Q2_POSITIVE_KEEP_DRIVE_MDEG\s+600L",
        "GIMBAL_Q2_POSITIVE_LOW_SPEED_MDEG\s+1800L",
        "GIMBAL_Q2_NEGATIVE_DRIVE_CAP_MDEG\s+1000L",
        "GIMBAL_Q2_REBOUND_BRAKE_UM_S\s+30000L",
        "GIMBAL_Q2_FINAL_CAPTURE_ERROR_UM\s+20000L",
        "GIMBAL_Q2_FINAL_CAPTURE_PIPE_MDEG\s+600L",
        "GIMBAL_DELAY_TEST_PIPE_ANGLE_MDEG\s+-1500L",
        "GIMBAL_DELAY_TEST_HOLD_MS\s+500U",
        "GIMBAL_DELAY_TEST_STABLE_WINDOW_MS\s+300U",
        "GIMBAL_DELAY_TEST_STABLE_SPAN_UM\s+2000L",
        "GIMBAL_DELAY_TEST_STABLE_SAMPLES\s+8U",
        "GIMBAL_DELAY_TEST_STATUS_PERIOD_MS\s+10U"
    )) {
        if ($config -notmatch $pattern) {
            throw "Competition configuration mismatch: $pattern"
        }
    }

    $gimbal = Get-Content -LiteralPath "Core\Src\gimbal_app.c" -Raw
    foreach ($pattern in @(
        "GIMBAL_ROLLING_INVERSE_FACTOR",
        "GIMBAL_CHASSIS_ACCEL_FF_GAIN",
        "ti_link_chassis_acceleration_m_s2",
        "control_chassis_forward_to_x_sign",
        "control_chassis_feedforward_mdeg",
        "competition_predict_position_um",
        "competition_velocity_um_s",
        "s_velocity_history_um",
        "s_velocity_history_ms",
        "s_velocity_last_sample_ms",
        "s_velocity_filtered_um_s",
        "GIMBAL_VELOCITY_WINDOW_MS",
        "GIMBAL_VELOCITY_WINDOW_MIN_MS",
        "GIMBAL_VELOCITY_WINDOW_MAX_MS",
        "competition_q2_update_motion",
        "s_q2_stationary_ms",
        "GIMBAL_Q2_STATIONARY_MS",
        "ball_is_stationary\s*!=\s*0U",
        "\(float\)velocity_um_s\s*/\s*" +
        "1000000\.0f",
        "\(int64_t\)velocity_um_s\s*\*\s*" +
        "\(int64_t\)horizon_ms",
        "s_competition_target_um\s*-\s*predicted_position_um",
        "s_level_zero_rad\s*-\s*\(desired_pipe_rad / pipe_per_motor\)",
        "COMPETITION_Q2_POSITIVE",
        "COMPETITION_Q2_NEGATIVE",
        "s_vision\.position_um\s*>=\s*" +
        "GIMBAL_Q2_POSITIVE_TARGET_UM",
        "competition_q2_begin_negative",
        "GIMBAL_Q2_FINAL_SETTLE_MS",
        "GIMBAL_Q2_EARLIEST_COMPLETE_MS",
        "s_q2_breakaway_deadline_ms\s*=\s*" +
        "now_ms \+ duration_ms",
        "competition_q2_update_stall",
        "GIMBAL_Q2_POSITIVE_BREAKAWAY_PIPE_MDEG",
        "GIMBAL_Q2_POSITIVE_BREAKAWAY_MS",
        "competition_q2_start_breakaway\(\s*" +
        "GIMBAL_Q2_NEGATIVE_BREAKAWAY_PIPE_MDEG",
        "GIMBAL_Q2_NEGATIVE_BREAKAWAY_MS",
        "s_q2_breakaway_used_this_leg",
        "GIMBAL_Q2_POSITIVE_BREAKAWAY_MAX",
        "GIMBAL_Q2_NEGATIVE_BREAKAWAY_MAX",
        "GIMBAL_Q2_STALL_SPAN_UM",
        "GIMBAL_Q2_MIN_DRIVE_PIPE_MDEG",
        "GIMBAL_Q2_POSITIVE_NO_BRAKE_END_UM",
        "GIMBAL_Q2_POSITIVE_KEEP_DRIVE_MDEG",
        "GIMBAL_Q2_POSITIVE_LOW_SPEED_MDEG",
        "GIMBAL_Q2_NEGATIVE_DRIVE_CAP_MDEG",
        "GIMBAL_Q2_REBOUND_BRAKE_UM_S",
        "s_q2_breakaway_deadline_ms\s*==\s*0U",
        "GIMBAL_Q2_FINAL_CAPTURE_ERROR_UM",
        "GIMBAL_Q2_FINAL_CAPTURE_PIPE_MDEG",
        "s_q2_final_capture_latched\s*=\s*1U",
        "q2_breakaway_active",
        "q2_stall_elapsed_ms",
        "gimbal_app_question_complete",
        "GIMBAL_VISION_LOST_MS",
        "GIMBAL_DELAY_REQUEST_VALIDATE_EMPTY",
        "GIMBAL_DELAY_REQUEST_ACK_MECHANICAL",
        "GIMBAL_DELAY_REQUEST_MEASURE",
        "delay_test_start_pulse",
        "delay_test_stability_on_sample",
        "s_delay_test_stable_ready"
    )) {
        if ($gimbal -notmatch $pattern) {
            throw "Competition controller invariant missing: $pattern"
        }
    }
    foreach ($removedToken in @(
        "VISION_FLAG_VELOCITY_VALID",
        "s_estimated_velocity_um_s",
        "s_previous_position_um",
        "s_previous_position_ms",
        "s_have_previous_position",
        "GIMBAL_Q2_MIN_DRIVE_VELOCITY_UM_S",
        "GIMBAL_Q2_POSITIVE_LOW_SPEED_UM_S",
        "GIMBAL_Q2_POSITIVE_PREDICTED_BRAKE_MDEG",
        "GIMBAL_Q2_POSITIVE_CROSSING_NUDGE_MDEG",
        "GIMBAL_Q2_POSITIVE_BRAKE_TRIGGER_UM",
        "GIMBAL_Q2_POSITIVE_BRAKE_MIN_SPEED_UM_S",
        "GIMBAL_Q2_POSITIVE_BRAKE_MDEG",
        "GIMBAL_Q2_NEGATIVE_KEEP_DRIVE_END_UM",
        "GIMBAL_Q2_NEGATIVE_KEEP_DRIVE_MDEG",
        "GIMBAL_Q2_NEGATIVE_BRAKE_ARM_UM",
        "GIMBAL_Q2_FINAL_CAPTURE_EXIT_UM",
        "applied_accel_m_s2",
        "predicted_velocity_um_s"
    )) {
        if ($gimbal -match [regex]::Escape($removedToken)) {
            throw "Step B/B2 cleanup is incomplete: $removedToken remains"
        }
    }

    $gimbalHeader =
        Get-Content -LiteralPath "Core\Inc\gimbal_app.h" -Raw
    foreach ($token in @(
        "control_predicted_velocity_um_s",
        "control_applied_pipe_mdeg"
    )) {
        if ($gimbalHeader -match [regex]::Escape($token)) {
            throw "Out-of-scope debug telemetry remains: $token"
        }
    }
    foreach ($token in @(
        "control_velocity_window_ms",
        "q2_stationary_ms"
    )) {
        if ($gimbalHeader -notmatch [regex]::Escape($token)) {
            throw "Step B/B2 debug telemetry missing: $token"
        }
    }
    $powerRenewals = [regex]::Matches(
        $gimbal,
        "s_power_off_deadline_ms\s*=\s*" +
        "now_ms \+ GIMBAL_POWER_ON_TIMEOUT_MS"
    ).Count
    if ($powerRenewals -lt 4) {
        throw "DelayTest operator-attention timeout renewals are incomplete"
    }

    $jetson = Get-Content -LiteralPath "Core\Src\jetson_link.c" -Raw
    foreach ($token in @(
        "JETSON_PROTOCOL_VERSION",
        "JETSON_MSG_HEARTBEAT",
        "JETSON_MSG_VISION_SAMPLE",
        "JETSON_MSG_GIMBAL_STATUS",
        "JETSON_MSG_SYSTEM_EVENT",
        "jetson_link_crc16",
        "ti_link_run_id"
    )) {
        if ($jetson -notmatch [regex]::Escape($token)) {
            throw "Jetson V3 implementation is missing $token"
        }
    }
    foreach ($pattern in @(
        "sample\.processing_latency_us\s*=\s*read_u32\(&payload\[8\]\)",
        "sample\.position_um\s*=\s*read_i32\(&payload\[12\]\)"
    )) {
        if ($jetson -notmatch $pattern) {
            throw "Jetson V3 payload layout mismatch: $pattern"
        }
    }

    $ti = Get-Content -LiteralPath "Core\Src\ti_link.c" -Raw
    foreach ($token in @(
        "TI_Q1_TIMEOUT_MS",
        "TI_Q2_TIMEOUT_MS",
        "TI_Q3_TIMEOUT_MS",
        "TI_Q45_TIMEOUT_MS",
        "begin_question",
        "gimbal_app_begin_question",
        "gimbal_app_start_question",
        "button_id == 6U",
        "s_power_enabled",
        "button_id == 2U",
        "GIMBAL_TI_STOP_PRESERVE_FAULT",
        "TI_ACCEL_ESTIMATE_WINDOW_MS",
        "TI_ACCEL_DEADBAND_M_S2",
        "TI_Q3_ACCEL_COMMAND_FLOOR_M_S2",
        "TI_Q45_ACCEL_COMMAND_FLOOR_M_S2",
        "TI_ACCEL_TAIL_MS",
        "TI_FLAG_COMMON_ACCEL_ACTIVE",
        "TI_FLAG_COMMON_DECEL_ACTIVE",
        "s_left_rpm_x10 = read_i16(&payload[20])",
        "s_right_rpm_x10 = read_i16(&payload[22])",
        "update_chassis_acceleration",
        "TI_SUPERVISOR_WAIT_BALL_SETTLE",
        "gimbal_app_ball_within_route_settle_limits",
        "GIMBAL_ROUTE_SETTLE_MS",
        "GIMBAL_ROUTE_POST_STOP_HOLD_MS",
        "GIMBAL_ROUTE_SETTLE_TIMEOUT_MS"
    )) {
        if ($ti -notmatch [regex]::Escape($token)) {
            throw "Five-question TI supervisor is missing $token"
        }
    }
    if ($ti -notmatch "TI_Q45_ACCEL_COMMAND_FLOOR_M_S2\s+0\.048f") {
        throw "Q4/Q5 acceleration feed-forward floor is not 0.048 m/s2"
    }

    $ioc = Get-Content -LiteralPath "gimbal_MC02.ioc" -Raw
    foreach ($pattern in @(
        "PA9\.Signal=USART1_TX",
        "PA10\.Signal=USART1_RX"
    )) {
        if ($ioc -notmatch $pattern) {
            throw "MC02 USART1 pin mapping mismatch: $pattern"
        }
    }

    $wiring = Get-Content -LiteralPath `
        "docs\MC02_WIRING_CHECKLIST.md" -Raw
    foreach ($pattern in @(
        "\| 1 \| USART1_TX / PA9 \|",
        "\| 2 \| USART1_RX / PA10 \|",
        "\| 3 \| GND \|"
    )) {
        if ($wiring -notmatch $pattern) {
            throw "MC02 USART1 connector table mismatch: $pattern"
        }
    }

    [byte[]] $heartbeatBody = @(
        0x03, 0x01, 0x08, 0x00, 0x01, 0x00,
        0x78, 0x56, 0x34, 0x12, 0x02, 0x07, 0x00, 0x00
    )
    if ((Get-Crc16CcittFalse $heartbeatBody) -ne 0xF12F) {
        throw "Jetson V3 heartbeat CRC self-test failed"
    }

    $vehicleCommands =
        Get-Content -LiteralPath "build\Vehicle\compile_commands.json" -Raw
    if ($vehicleCommands -notmatch "GIMBAL_ENABLE_MANUAL_CALRUN=0") {
        throw "Vehicle build did not remove manual CALRUN"
    }
    if ($vehicleCommands -notmatch "GIMBAL_ENABLE_DELAY_TEST=0") {
        throw "Vehicle build unexpectedly enabled DelayTest"
    }
    if ($vehicleCommands -notmatch '(^|[ "])-g3([ "])') {
        throw "Vehicle build does not retain Ozone DWARF type information"
    }
    $vehicleOzone =
        Get-Content -LiteralPath "gimbal_MC02_Vehicle.jdebug" -Raw
    if ($vehicleOzone -notmatch
        'File\.Open\("build/Vehicle/gimbal_MC02\.elf"\)') {
        throw "Vehicle Ozone project is not bound to the Vehicle ELF"
    }
    foreach ($token in @(
        "VehicleCommissioningSafe",
        "VehicleCommissioning08",
        "VehicleCommissioning08Kd20",
        "VehicleCommissioningAggressiveQ2",
        "VehicleDisableDelayComp",
        "VehicleRestoreMeasuredDelay"
    )) {
        if ($vehicleOzone -notmatch [regex]::Escape($token)) {
            throw "Vehicle Ozone project is missing helper $token"
        }
    }

    foreach ($preset in @("Debug", "Release")) {
        $commands = Get-Content -LiteralPath `
            "build\$preset\compile_commands.json" -Raw
        if ($commands -notmatch "GIMBAL_ENABLE_DELAY_TEST=0") {
            throw "$preset build unexpectedly enabled DelayTest"
        }
    }

    $delayCommands =
        Get-Content -LiteralPath `
            "build\DelayTest\compile_commands.json" -Raw
    if ($delayCommands -notmatch "GIMBAL_ENABLE_DELAY_TEST=1") {
        throw "DelayTest build did not enable the dedicated test"
    }
    if ($delayCommands -notmatch "GIMBAL_ENABLE_MANUAL_CALRUN=0") {
        throw "DelayTest build did not remove manual CALRUN"
    }

    $size = Get-Command "arm-none-eabi-size.exe" -ErrorAction Stop
    Invoke-Checked $size.Source "build\Debug\gimbal_MC02.elf"
    Invoke-Checked $size.Source "build\Release\gimbal_MC02.elf"
    Invoke-Checked $size.Source "build\Vehicle\gimbal_MC02.elf"
    Invoke-Checked $size.Source "build\DelayTest\gimbal_MC02.elf"

    Write-Host ""
    Write-Host "PASS: gimbal_MC02 full offline verification completed." `
        -ForegroundColor Green
    Write-Host "Hardware tuning and powered motion are not claimed."
}
finally {
    Pop-Location
}
