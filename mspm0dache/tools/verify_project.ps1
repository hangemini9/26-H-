$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$gmake = "C:\ti\ccs\utils\bin\gmake.exe"
$compilerBin = "C:\ti\ccs\tools\compiler\ti-cgt-armllvm_5.1.1.LTS\bin"
$nm = Join-Path $compilerBin "tiarmnm.exe"
$size = Join-Path $compilerBin "tiarmsize.exe"

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

foreach ($tool in @($gmake, $nm, $size)) {
    if (-not (Test-Path -LiteralPath $tool)) {
        throw "Required tool not found: $tool"
    }
}

Push-Location $projectRoot
try {
    Write-Host "[1/5] Clean build"
    Invoke-Checked $gmake "clean"

    Write-Host "[2/5] TEST_MODE build"
    Invoke-Checked $gmake "-j4"

    Write-Host "[3/5] Safe release build"
    Invoke-Checked $gmake "release" "-j4"

    Write-Host "[4/5] Project structure and configuration"
    $requiredFiles = @(
        "README.md",
        "COMMISSIONING.md",
        "AGENT.MD",
        "AGENTS.md",
        "bluetooth.c",
        "bluetooth.h",
        "line_sensor.c",
        "line_sensor.h",
        "oled_display.c",
        "oled_display.h",
        "question_timer.c",
        "question_timer.h",
        "mc02_link.c",
        "mc02_link.h",
        "chassis_config.h",
        "mspm0dache.syscfg",
        "mspm0dache.jdebug",
        "targetConfigs\mspm0g3507_xds110.ccxml",
        "tools\flash_xds110.ps1",
        "ticlang\mspm0dache.out",
        "ticlang\mspm0dache_release.out",
        "docs\R3X_chassis_320x240mm.pdf"
    )
    foreach ($path in $requiredFiles) {
        if (-not (Test-Path -LiteralPath $path)) {
            throw "Required file missing: $path"
        }
    }
    $rootMarkdownFiles = @(Get-ChildItem -LiteralPath "." -File -Filter "*.md")
    if ($rootMarkdownFiles.Count -lt 5) {
        throw "Expected the English handoff files and the Chinese commissioning guide."
    }

    Get-Content -LiteralPath ".vscode\tasks.json" -Raw |
        ConvertFrom-Json | Out-Null
    Get-Content -LiteralPath ".vscode\c_cpp_properties.json" -Raw |
        ConvertFrom-Json | Out-Null
    Get-Content -LiteralPath ".theia\launch.json" -Raw |
        ConvertFrom-Json | Out-Null
    [xml](Get-Content -LiteralPath `
        "targetConfigs\mspm0g3507_xds110.ccxml" -Raw) | Out-Null
    $testModeSource = Get-Content -LiteralPath "test_mode.c" -Raw
    $chassisConfigSource = Get-Content -LiteralPath "chassis_config.h" -Raw
    $mainSource = Get-Content -LiteralPath "main.c" -Raw
    $buildSource = Get-Content -LiteralPath "ticlang\makefile" -Raw
    $oledSource = Get-Content -LiteralPath "oled_display.c" -Raw
    $timerSource = Get-Content -LiteralPath "question_timer.c" -Raw
    if ($testModeSource -notmatch "FW_BUILD_ID\s+2026080104U") {
        throw "Unexpected TI firmware build ID"
    }
    foreach ($profilePattern in @(
        "LINE_FOLLOW_Q3_ACCEL_RPM_PER_S\s+30\.0f",
        "LINE_FOLLOW_Q3_DECEL_RPM_PER_S\s+30\.0f",
        "LINE_FOLLOW_Q45_ACCEL_RPM_PER_S\s+14\.0f",
        "LINE_FOLLOW_Q45_DECEL_RPM_PER_S\s+14\.0f",
        "LINE_FINISH_GENTLE_STOP_MAX_MS\s+8000U"
    )) {
        if ($chassisConfigSource -notmatch $profilePattern) {
            throw "Unexpected Q3-Q5 gentle motion profile"
        }
    }
    foreach ($requiredPattern in @(
        "TEST_LINE_FOLLOW",
        "TEST_STATUS_LINE_SENSOR_FAULT",
        "LINE_FOLLOW_SENSOR_GRACE_MS",
        "LINE_FOLLOW_LAP_DURATION_MS",
        "LINE_LAP_PHASE_LEAVE_START",
        "LINE_LAP_PHASE_SEARCH_FINISH",
        "LINE_LAP_PHASE_FORWARD_ALIGN",
        "LINE_LAP_PHASE_SETTLE",
        "LINE_LAP_PHASE_REVERSE_SEARCH",
        "LINE_LAP_PHASE_MARKER_DETECTED",
        "LINE_LAP_PHASE_GENTLE_STOP",
        "average_forward_distance_mm",
        "recent_finish_marker_state"
    )) {
        if ($testModeSource -notmatch [regex]::Escape($requiredPattern)) {
            throw "Line-follow implementation is missing $requiredPattern"
        }
    }
    foreach ($requiredPattern in @(
        "LINE_FINISH_ARM_DISTANCE_MM",
        "LINE_TURN1_MIN_DISTANCE_MM",
        "LINE_TURN1_MIN_ABS_YAW_DEG",
        "LINE_TURN2_MIN_DISTANCE_MM",
        "LINE_TURN2_MIN_ABS_YAW_DEG",
        "LINE_COURSE_STRAIGHT_MM",
        "LINE_COURSE_RADIUS_MM",
        "LINE_COURSE_EXPECTED_LAP_MM",
        "LINE_FINISH_TURN_ASSIST_DISTANCE_MM",
        "LINE_FINISH_MAX_LAP_DISTANCE_MM",
        "LINE_FINISH_APPROACH_RPM",
        "LINE_FINISH_FORWARD_ALIGN_RPM",
        "LINE_FINISH_FORWARD_ALIGN_MIN_MM",
        "LINE_FINISH_FORWARD_ALIGN_MAX_MM",
        "LINE_FINISH_CENTER_MAX_POSITION_X1000",
        "LINE_FINISH_CENTER_SAMPLES",
        "LINE_FINISH_SETTLE_MIN_MS",
        "LINE_FINISH_SETTLE_MAX_MS",
        "LINE_FINISH_REVERSE_RPM",
        "LINE_FINISH_REVERSE_ARM_MM",
        "LINE_FINISH_REVERSE_MAX_MM",
        "LINE_FINISH_REVERSE_TIMEOUT_MS",
        "LINE_FINISH_MARKER_RECENT_MS",
        "LINE_FINISH_MARKER_MIN_ACTIVE_CHANNELS",
        "LINE_FINISH_MARKER_CONFIRM_SAMPLES",
        "LINE_FOLLOW_Q3_CRUISE_RPM",
        "LINE_FOLLOW_Q3_APPROACH_RPM",
        "LINE_FOLLOW_Q3_APPROACH_START_MM",
        "LINE_FOLLOW_Q3_FINAL_START_MM",
        "LINE_FOLLOW_Q45_BASE_RPM",
        "LINE_FOLLOW_DEFAULT_STEERING_KD",
        "LINE_FOLLOW_MAX_D_CORRECTION_RPM",
        "LINE_FOLLOW_POSITION_FILTER_ALPHA",
        "LINE_FOLLOW_ACCEL_SLEW_RPM_PER_S",
        "LINE_FOLLOW_DECEL_SLEW_RPM_PER_S",
        "LINE_FOLLOW_CURVE_MIN_BASE_RPM",
        "LINE_FOLLOW_LAUNCH_BASE_RPM",
        "LINE_FOLLOW_LAUNCH_DISTANCE_MM",
        "LINE_FOLLOW_START_BASE_RPM",
        "LINE_FOLLOW_START_DURATION_MS",
        "LINE_FOLLOW_START_STRAIGHT_MM",
        "LINE_FOLLOW_START_VALID_SAMPLES",
        "LINE_FOLLOW_START_SENSOR_TIMEOUT_MS",
        "LINE_FOLLOW_LINE_LOSS_HOLD_MS",
        "LINE_FOLLOW_LINE_LOSS_RECOVERY_MS",
        "LINE_FOLLOW_RECOVERY_BASE_RPM",
        "LINE_FOLLOW_RECOVERY_CORRECTION_RPM"
    )) {
        if ($chassisConfigSource -notmatch [regex]::Escape($requiredPattern)) {
            throw "Competition finish configuration is missing $requiredPattern"
        }
    }
    foreach ($formalCoursePattern in @(
        "LINE_FOLLOW_DEFAULT_BASE_RPM\s+120\.0f",
        "LINE_FOLLOW_Q1_BASE_RPM\s+143\.0f",
        "LINE_FOLLOW_Q1_CURVE_MIN_BASE_RPM\s+110\.0f",
        "LINE_FOLLOW_Q1_MAX_WHEEL_RPM\s+176\.0f",
        "LINE_FOLLOW_Q3_CRUISE_RPM\s+100\.0f",
        "LINE_FOLLOW_Q3_APPROACH_RPM\s+45\.0f",
        "LINE_FOLLOW_Q3_APPROACH_START_MM\s+1250\.0f",
        "LINE_FOLLOW_Q3_FINAL_START_MM\s+1450\.0f",
        "LINE_FOLLOW_Q45_BASE_RPM\s+70\.0f",
        "LINE_FOLLOW_CURVE_MIN_BASE_RPM\s+100\.0f",
        "STANDALONE_KEY1_START\s+0",
        "LINE_FOLLOW_LAP_DURATION_MS\s+40000U",
        "LINE_FOLLOW_START_STRAIGHT_MM\s+100\.0f",
        "LINE_COURSE_STRAIGHT_MM\s+1500\.0f",
        "LINE_COURSE_RADIUS_MM\s+500\.0f",
        "LINE_COURSE_EXPECTED_LAP_MM\s+6141\.6f",
        "LINE_TURN1_MIN_DISTANCE_MM\s+1300\.0f",
        "LINE_TURN1_MIN_ABS_YAW_DEG\s+100\.0f",
        "LINE_TURN2_MIN_DISTANCE_MM\s+4500\.0f",
        "LINE_TURN2_MIN_ABS_YAW_DEG\s+250\.0f",
        "LINE_FINISH_TURN_ASSIST_DISTANCE_MM\s+5970\.0f",
        "LINE_FINISH_MAX_LAP_DISTANCE_MM\s+6070\.0f",
        "LINE_FINISH_FORWARD_ALIGN_RPM\s+60\.0f",
        "LINE_FINISH_FORWARD_ALIGN_MIN_MM\s+150\.0f",
        "LINE_FINISH_FORWARD_ALIGN_MAX_MM\s+250\.0f",
        "LINE_FINISH_REVERSE_RPM\s+25\.0f",
        "LINE_FINISH_REVERSE_ARM_MM\s+40\.0f",
        "LINE_FINISH_REVERSE_MAX_MM\s+400\.0f",
        "LINE_FINISH_MARKER_MIN_ACTIVE_CHANNELS\s+3U",
        "LINE_FINISH_MARKER_CONFIRM_SAMPLES\s+3U"
    )) {
        if ($chassisConfigSource -notmatch $formalCoursePattern) {
            throw "Formal-course configuration is missing $formalCoursePattern"
        }
    }
    foreach ($startPattern in @(
        "s_start_sensor_valid_samples",
        "line->sample_sequence == s_start_sensor_sequence",
        "LINE_FOLLOW_START_VALID_SAMPLES",
        "g_test.elapsed_ms < LINE_FOLLOW_START_DURATION_MS"
    )) {
        if ($testModeSource -notmatch [regex]::Escape($startPattern)) {
            throw "Fresh-sensor slow-start implementation is missing $startPattern"
        }
    }
    foreach ($finishPattern in @(
        "update_lap_turn_assist",
        "s_turn_count >= 2U",
        "LINE_FINISH_TURN_ASSIST_DISTANCE_MM",
        "LINE_LAP_PHASE_FORWARD_ALIGN",
        "LINE_LAP_PHASE_SETTLE",
        "LINE_LAP_PHASE_REVERSE_SEARCH",
        "line->active_count >=",
        "LINE_FINISH_MARKER_MIN_ACTIVE_CHANNELS",
        "LINE_FOLLOW_Q3_ACCEL_RPM_PER_S",
        "LINE_FOLLOW_Q3_DECEL_RPM_PER_S",
        "LINE_FOLLOW_Q45_ACCEL_RPM_PER_S",
        "LINE_FOLLOW_Q45_DECEL_RPM_PER_S",
        "LINE_FOLLOW_STEERING_SLEW_RPM_PER_S",
        "LINE_FINISH_GENTLE_STOP_MAX_MS",
        "QuestionTimer_Start(s_supervisor_question_id)",
        "TEST_COMMON_MOTION_ACCELERATING",
        "bounded_reverse_search",
        "recent_finish_marker_state",
        "set_line_reverse_targets",
        "set_line_zero_targets",
        "set_line_straight_targets",
        "LINE_FOLLOW_START_STRAIGHT_MM",
        "s_filtered_line_position = position",
        "s_previous_line_position = position"
    )) {
        if ($testModeSource -notmatch [regex]::Escape($finishPattern)) {
            throw "Turn-assisted reverse-marker finish logic is missing $finishPattern"
        }
    }
    foreach ($mc02Pattern in @(
        "START_SOURCE_MC02",
        "TestMode_TakeButtonEvent",
        "TestMode_SupervisorPrepareQuestion",
        "TestMode_SupervisorStartQuestion",
        "s_supervisor_question_id",
        "LINE_COURSE_STRAIGHT_MM",
        "competition_base_rpm",
        "LINE_FOLLOW_Q3_APPROACH_START_MM",
        "s_supervisor_question_id == 2U",
        "s_button_events",
        "TestMode_IsEmergencyStopLatched",
        "TEST_STATUS_EMERGENCY_STOP"
    )) {
        if ($testModeSource -notmatch [regex]::Escape($mc02Pattern)) {
            throw "MC02-supervised test-mode integration is missing $mc02Pattern"
        }
    }
    foreach ($powerPattern in @(
        "BUTTON6_MASK",
        "set_power_enabled",
        "TestMode_IsPowerEnabled",
        "QuestionTimer_SetPowerEnabled",
        "s_power_enabled == 0U",
        "g_test.mc02.online != 0U",
        "MC02_IDLE_DISARMED_STATE"
    )) {
        if ($testModeSource -notmatch [regex]::Escape($powerPattern)) {
            throw "K6 power-permission implementation is missing $powerPattern"
        }
    }
    foreach ($displayPattern in @(
        "OLED_POWER_PAGE",
        "OLED_POWER_VALUE_COLUMN",
        "glyph_p",
        "glyph_w",
        "glyph_r",
        "s_power_render_pending",
        "OLED_RECONNECT_PERIOD_MS",
        "OLED_BOOT_DELAY_MS",
        "s_initial_connect_pending",
        "connect_and_draw_static_ui",
        "render_time_value",
        "value_pixels",
        "seconds % 1000U",
        "s_render_count"
    )) {
        if ($oledSource -notmatch [regex]::Escape($displayPattern)) {
            throw "OLED PWR display implementation is missing $displayPattern"
        }
    }
    foreach ($timerPattern in @(
        "QuestionTimer_SetPowerEnabled",
        "s_state.power_enabled",
        "OledDisplay_GetDisplayedSeconds",
        "OledDisplay_GetRenderCount"
    )) {
        if ($timerSource -notmatch [regex]::Escape($timerPattern)) {
            throw "Question timer power telemetry is missing $timerPattern"
        }
    }
    if ($mainSource -match "Bluetooth_(Init|Tick)" -or
        $buildSource -match '(?m)^\s*bluetooth\.c\s*\\') {
        throw "Bluetooth must remain excluded while the JDY-31 is unplugged"
    }
    foreach ($mc02Pattern in @(
        "MC02Link_Init",
        "MC02Link_Tick"
    )) {
        if ($mainSource -notmatch [regex]::Escape($mc02Pattern)) {
            throw "main.c is missing $mc02Pattern"
        }
    }
    if ($buildSource -notmatch '(?m)^\s*mc02_link\.c\s*\\') {
        throw "mc02_link.c is not linked"
    }
    foreach ($displaySource in @(
        "oled_display.c",
        "question_timer.c"
    )) {
        if ($buildSource -notmatch
            "(?m)^\s*$([regex]::Escape($displaySource))\s*\\") {
            throw "$displaySource is not linked"
        }
    }

    $generatedHeader = Get-Content -LiteralPath "ti_msp_dl_config.h" -Raw
    $generatedSource = Get-Content -LiteralPath "ti_msp_dl_config.c" -Raw
    foreach ($requiredSymbol in @(
        "MOTOR_PWM_INST",
        "QEI_RIGHT_INST",
        "GPIO_MOTOR_PORT",
        "GPIO_ENCODER_LEFT_PORT",
        "GPIO_BUTTON_PANEL_A_PORT",
        "GPIO_BUTTON_PANEL_B_PORT",
        "LINE_SENSOR_I2C_INST",
        "MC02_UART_INST"
    )) {
        if ($generatedHeader -notmatch [regex]::Escape($requiredSymbol)) {
            throw "Generated configuration is missing $requiredSymbol"
        }
    }
    foreach ($requiredPinPattern in @(
        "GPIO_QEI_RIGHT_PHA_PORT\s+GPIOB",
        "GPIO_QEI_RIGHT_PHA_PIN\s+DL_GPIO_PIN_6",
        "GPIO_QEI_RIGHT_PHB_PORT\s+GPIOB",
        "GPIO_QEI_RIGHT_PHB_PIN\s+DL_GPIO_PIN_7",
        "GPIO_ENCODER_LEFT_ENCODER_LEFT_A_PIN\s+\(DL_GPIO_PIN_16\)",
        "GPIO_ENCODER_LEFT_ENCODER_LEFT_B_PIN\s+\(DL_GPIO_PIN_0\)",
        "GPIO_BUTTON_PANEL_A_PORT\s+\(GPIOA\)",
        "GPIO_BUTTON_PANEL_A_BUTTON1_PIN\s+\(DL_GPIO_PIN_10\)",
        "GPIO_BUTTON_PANEL_A_BUTTON2_PIN\s+\(DL_GPIO_PIN_11\)",
        "GPIO_BUTTON_PANEL_B_PORT\s+\(GPIOB\)",
        "GPIO_BUTTON_PANEL_B_BUTTON3_PIN\s+\(DL_GPIO_PIN_13\)",
        "GPIO_BUTTON_PANEL_B_BUTTON4_PIN\s+\(DL_GPIO_PIN_20\)",
        "GPIO_BUTTON_PANEL_A_BUTTON5_PIN\s+\(DL_GPIO_PIN_31\)",
        "GPIO_BUTTON_PANEL_A_BUTTON6_PIN\s+\(DL_GPIO_PIN_28\)",
        "GPIO_BUTTON_PANEL_B_BUTTON7_PIN\s+\(DL_GPIO_PIN_12\)",
        "GPIO_BUTTON_PANEL_B_BUTTON8_PIN\s+\(DL_GPIO_PIN_15\)",
        "LINE_SENSOR_I2C_INST\s+I2C1",
        "GPIO_LINE_SENSOR_I2C_SCL_PIN\s+DL_GPIO_PIN_2",
        "GPIO_LINE_SENSOR_I2C_SDA_PIN\s+DL_GPIO_PIN_3",
        "LINE_SENSOR_I2C_BUS_SPEED_HZ\s+100000",
        "MC02_UART_INST\s+UART1",
        "GPIO_MC02_UART_RX_PIN\s+DL_GPIO_PIN_9",
        "GPIO_MC02_UART_TX_PIN\s+DL_GPIO_PIN_8",
        "MC02_UART_BAUD_RATE\s+\(115200\)"
    )) {
        if ($generatedHeader -notmatch $requiredPinPattern) {
            throw "Generated configuration mismatch: $requiredPinPattern"
        }
    }
    foreach ($buttonName in @(
        "GPIO_BUTTON_PANEL_A_BUTTON1_IOMUX",
        "GPIO_BUTTON_PANEL_A_BUTTON2_IOMUX",
        "GPIO_BUTTON_PANEL_B_BUTTON3_IOMUX",
        "GPIO_BUTTON_PANEL_B_BUTTON4_IOMUX",
        "GPIO_BUTTON_PANEL_A_BUTTON5_IOMUX",
        "GPIO_BUTTON_PANEL_A_BUTTON6_IOMUX",
        "GPIO_BUTTON_PANEL_B_BUTTON7_IOMUX",
        "GPIO_BUTTON_PANEL_B_BUTTON8_IOMUX"
    )) {
        if ($generatedSource -notmatch
            "$buttonName[\s\S]{0,160}DL_GPIO_RESISTOR_PULL_UP") {
            throw "$buttonName is not configured with an internal pull-up"
        }
    }
    if ($generatedHeader -match "BLUETOOTH_UART") {
        throw "Bluetooth UART is still present in generated configuration"
    }
    if ($generatedHeader -match "BUZZER|KEY4") {
        throw "Unexpected legacy peripheral found in generated configuration"
    }

    function Get-Crc16CcittFalse {
        param([byte[]] $Data)
        [uint16] $crc = 0xFFFF
        foreach ($byte in $Data) {
            $crc = [uint16] ($crc -bxor ([uint16] $byte -shl 8))
            for ($bit = 0; $bit -lt 8; $bit++) {
                if (($crc -band 0x8000) -ne 0) {
                    $crc = [uint16] ((($crc -shl 1) -bxor 0x1021) -band 0xFFFF)
                } else {
                    $crc = [uint16] (($crc -shl 1) -band 0xFFFF)
                }
            }
        }
        return $crc
    }
    [byte[]] $safeStopBody = @(
        0x02, 0x21, 0x10, 0x00, 0x01, 0x00,
        0x2A, 0x00, 0x00, 0x00, 0x07, 0x00, 0x02, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    )
    if ((Get-Crc16CcittFalse $safeStopBody) -ne 0xC612) {
        throw "MC02 canonical SAFE_STOP CRC self-test failed"
    }
    $mc02Source = Get-Content -LiteralPath "mc02_link.c" -Raw
    foreach ($requiredPattern in @(
        "MC02_HEARTBEAT_TIMEOUT_MS",
        "MC02_Q1_OPTIONS",
        "MC02_Q1_DEFAULT_TIMEOUT_MS",
        "MC02_Q2_OPTIONS",
        "MC02_Q2_DEFAULT_TIMEOUT_MS",
        "MC02_Q3_OPTIONS",
        "MC02_Q45_OPTIONS",
        "MC02Link_Crc16",
        "command_seen",
        "QuestionTimer_Start(question_id)",
        "TestMode_SupervisorStop",
        "MC02_STATE_STOPPING",
        "MC02_STATE_EMERGENCY_STOP_LATCHED",
        "MC02_EVENT_A_MARKER_DETECTED",
        "MC02_EVENT_B_MARKER_PASSED",
        "MC02_FLAG_B_MARKER_SEEN",
        "MC02_FLAG_BUTTON_PANEL_OK",
        "MC02_FLAG_COMMON_ACCEL_ACTIVE",
        "MC02_FLAG_COMMON_DECEL_ACTIVE",
        "MC02_UART_INST_IRQHandler"
    )) {
        if ($mc02Source -notmatch [regex]::Escape($requiredPattern)) {
            throw "MC02 link implementation is missing $requiredPattern"
        }
    }

    Write-Host "[5/5] ELF symbols and memory usage"
    $symbols = & $nm "ticlang\mspm0dache.out"
    if ($LASTEXITCODE -ne 0) {
        throw "tiarmnm failed with exit code $LASTEXITCODE"
    }
    $symbolText = $symbols -join "`n"
    foreach ($requiredSymbol in @(
        "g_imu",
        "g_test",
        "LineSensor_Tick",
        "OledDisplay_Tick",
        "QuestionTimer_Tick",
        "MC02Link_Tick",
        "MC02Link_Crc16"
    )) {
        if ($symbolText -notmatch "(?m)\b$requiredSymbol`$") {
            throw "ELF is missing Ozone symbol $requiredSymbol"
        }
    }
    if ($symbolText -match "(?m)\bBluetooth_Tick`$") {
        throw "Bluetooth code is still linked into the TEST_MODE ELF"
    }

    Invoke-Checked $size "ticlang\mspm0dache.out"
    Invoke-Checked $size "ticlang\mspm0dache_release.out"

    Write-Host ""
    Write-Host "PASS: mspm0dache offline verification completed." `
        -ForegroundColor Green
    Write-Host "Hardware flashing and calibration are intentionally not claimed."
}
finally {
    Pop-Location
}
