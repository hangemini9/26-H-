#ifndef QUESTION_TIMER_H
#define QUESTION_TIMER_H

#include <stdint.h>

typedef struct {
    uint8_t running;
    uint8_t question_id;
    uint8_t power_enabled;
    uint8_t oled_connected;
    uint8_t oled_address;
    uint32_t elapsed_ms;
    uint32_t elapsed_seconds;
    uint32_t oled_error_count;
    uint32_t oled_requested_seconds;
    uint32_t oled_displayed_seconds;
    uint32_t oled_render_count;
} QuestionTimerState;

void QuestionTimer_Init(void);
void QuestionTimer_Start(uint8_t question_id);
void QuestionTimer_Stop(void);
void QuestionTimer_SetPowerEnabled(uint8_t enabled);
void QuestionTimer_Tick(void);
const QuestionTimerState *QuestionTimer_GetState(void);

#endif
