#include "question_timer.h"

#include "oled_display.h"
#include "platform.h"

static QuestionTimerState s_state;
static uint32_t s_start_ms;

void QuestionTimer_Init(void)
{
    s_state.running = 0U;
    s_state.question_id = 0U;
    s_state.power_enabled = 0U;
    s_state.elapsed_ms = 0U;
    s_state.elapsed_seconds = 0U;
    s_state.oled_requested_seconds = 0U;
    s_state.oled_displayed_seconds = 0U;
    s_state.oled_render_count = 0U;
    s_start_ms = Platform_Millis();
    OledDisplay_Init();
    s_state.oled_connected = OledDisplay_IsConnected();
    s_state.oled_address = OledDisplay_GetAddress();
    s_state.oled_error_count = OledDisplay_GetErrorCount();
    s_state.oled_requested_seconds =
        OledDisplay_GetRequestedSeconds();
    s_state.oled_displayed_seconds =
        OledDisplay_GetDisplayedSeconds();
    s_state.oled_render_count = OledDisplay_GetRenderCount();
}

void QuestionTimer_Start(uint8_t question_id)
{
    if (question_id < 1U || question_id > 5U) {
        return;
    }
    s_start_ms = Platform_Millis();
    s_state.running = 1U;
    s_state.question_id = question_id;
    s_state.elapsed_ms = 0U;
    s_state.elapsed_seconds = 0U;
}

void QuestionTimer_Stop(void)
{
    if (s_state.running != 0U) {
        s_state.elapsed_ms =
            (uint32_t) (Platform_Millis() - s_start_ms);
        s_state.elapsed_seconds = s_state.elapsed_ms / 1000U;
    }
    s_state.running = 0U;
}

void QuestionTimer_SetPowerEnabled(uint8_t enabled)
{
    s_state.power_enabled = enabled != 0U ? 1U : 0U;
}

void QuestionTimer_Tick(void)
{
    if (s_state.running != 0U) {
        s_state.elapsed_ms =
            (uint32_t) (Platform_Millis() - s_start_ms);
        s_state.elapsed_seconds = s_state.elapsed_ms / 1000U;
    }
    OledDisplay_Tick(
        s_state.elapsed_seconds, s_state.power_enabled);
    s_state.oled_connected = OledDisplay_IsConnected();
    s_state.oled_address = OledDisplay_GetAddress();
    s_state.oled_error_count = OledDisplay_GetErrorCount();
    s_state.oled_requested_seconds =
        OledDisplay_GetRequestedSeconds();
    s_state.oled_displayed_seconds =
        OledDisplay_GetDisplayedSeconds();
    s_state.oled_render_count = OledDisplay_GetRenderCount();
}

const QuestionTimerState *QuestionTimer_GetState(void)
{
    return &s_state;
}
