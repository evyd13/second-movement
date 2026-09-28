/*
 * MIT License
 *
 * Copyright (c) 2022 Andreas Nebinger, building on Wesley Ellis’ countdown_face.c
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include "timer_face.h"
#include "watch.h"
#include "watch_utility.h"

static const uint32_t _default_timer_values[] = {0x000100, 0x000300, 0x000500, 0x000A00, 0x000F00, 0x001400, 0x001E00, 0x002D00, 0x00001}; // default timers: 1 min, 3 min, 5 min, 10 min, 15 min, 20 min, 30 min, 45 min, 1 hour

// sound sequence for a single beeping sequence
static const int8_t _sound_seq_beep[] = {BUZZER_NOTE_C8, 3, BUZZER_NOTE_REST, 3, -2, 2, BUZZER_NOTE_C8, 5, BUZZER_NOTE_REST, 25, 0};
static const int8_t _sound_seq_start[] = {BUZZER_NOTE_C8, 2, 0};

static uint8_t _beeps_to_play;    // temporary counter for ring signals playing

static inline void button_beep() {
    // play a beep as confirmation for a button press (if applicable)
    if (movement_button_should_sound()) watch_buzzer_play_note_with_volume(BUZZER_NOTE_C7, 50, movement_button_volume());
}

static void _signal_callback() {
    if (_beeps_to_play) {
        _beeps_to_play--;
        watch_buzzer_play_sequence((int8_t *)_sound_seq_beep, _signal_callback);
    }
}

static void _start(timer_state_t *state, bool with_beep) {
    if (state->timers[state->current_timer].value == 0) return;
    watch_date_time_t now = watch_rtc_get_date_time();
    state->now_ts = watch_utility_date_time_to_unix_time(now, movement_get_current_timezone_offset());
    if (state->mode == pausing)
        state->target_ts = state->now_ts + state->paused_left;
    else
        state->target_ts = watch_utility_offset_timestamp(state->now_ts, 
                                                          state->timers[state->current_timer].unit.hours, 
                                                          state->timers[state->current_timer].unit.minutes, 
                                                          state->timers[state->current_timer].unit.seconds);
    watch_date_time_t target_dt = watch_utility_date_time_from_unix_time(state->target_ts, movement_get_current_timezone_offset());
    state->mode = running;
    movement_schedule_background_task_for_face(state->watch_face_index, target_dt);
    if (with_beep) watch_buzzer_play_sequence((int8_t *)_sound_seq_start, NULL);
    movement_request_tick_frequency(2);
}

static void _draw(timer_state_t *state, uint8_t subsecond) {
    char bottom_time[10];
    char timer_id[3];
    uint32_t delta;
    div_t result;
    uint8_t h, min, sec;

    switch (state->mode) {
        case pausing:
            watch_set_colon();
            if (state->pausing_seconds != 1)
                // not 1st iteration (or 256th): do not write anything
                return;
            // fall through
        case running:
            delta = state->target_ts - state->now_ts;
            result = div(delta, 60);
            sec = result.rem;
            result = div(result.quot, 60);
            min = result.rem;
            h = result.quot;
            sprintf(bottom_time, "%02u%02u%02u", h, min, sec);
            break;
        case setting:
            sprintf(bottom_time, "%02u%02u%02u", state->timers[state->current_timer].unit.hours,
                    state->timers[state->current_timer].unit.minutes,
                    state->timers[state->current_timer].unit.seconds);
            watch_set_colon();
            break;
        case waiting:
            sprintf(bottom_time, "%02u%02u%02u", state->timers[state->current_timer].unit.hours,
                    state->timers[state->current_timer].unit.minutes,
                    state->timers[state->current_timer].unit.seconds);
            watch_set_colon();
            break;
    }

    sprintf(timer_id, "%2u", state->current_timer + 1);
    if (state->mode == setting && subsecond % 2) {
        // blink the current settings value
        bottom_time[(state->settings_state) * 2] = bottom_time[(state->settings_state * 2) + 1] = ' ';
    }
    watch_display_text_with_fallback(WATCH_POSITION_BOTTOM, bottom_time, bottom_time);
    watch_display_text_with_fallback(WATCH_POSITION_TOP_RIGHT, timer_id, timer_id);

    // set lap indicator when we have a looping timer
    if (state->timers[state->current_timer].unit.repeat) watch_set_indicator(WATCH_INDICATOR_LAP);
    else watch_clear_indicator(WATCH_INDICATOR_LAP);
}

static void _reset(timer_state_t *state) {
    state->mode = waiting;
    movement_cancel_background_task_for_face(state->watch_face_index);
    movement_request_tick_frequency(1);
}

static void _set_next_valid_timer(timer_state_t *state) {
    if ((state->timers[state->current_timer].value & 0xFFFFFF) == 0) {
        uint8_t i = state->current_timer;
        do {
            i = (i + 1) % TIMER_SLOTS;
        } while ((state->timers[i].value & 0xFFFFFF) == 0 && i != state->current_timer);
        state->current_timer = i;
    }
}

static void _resume_setting(timer_state_t *state) {
    state->settings_state = 0;
    state->mode = waiting;
    movement_request_tick_frequency(1);
    _set_next_valid_timer(state);
}

static void _settings_increment(timer_state_t *state) {
    switch(state->settings_state) {
        case 0:
            state->timers[state->current_timer].unit.hours = (state->timers[state->current_timer].unit.hours + 1) % 24;
            break;
        case 1:
            state->timers[state->current_timer].unit.minutes = (state->timers[state->current_timer].unit.minutes + 1) % 60;
            break;
        case 2:
            state->timers[state->current_timer].unit.seconds = (state->timers[state->current_timer].unit.seconds + 1) % 60;
            break;
        default:
            // should never happen
            break;
    }
    return;
}

static void _abort_quick_cycle(timer_state_t *state) {
    if (state->quick_cycle) {
        state->quick_cycle = false;
        movement_request_tick_frequency(4);
    }
}

static inline bool _check_for_signal() {
    if (_beeps_to_play) {
        _beeps_to_play = 0;
        return true;
    }
    return false;
}

void timer_face_setup(uint8_t watch_face_index, void ** context_ptr) {

    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(timer_state_t));
        timer_state_t *state = (timer_state_t *)*context_ptr;
        memset(*context_ptr, 0, sizeof(timer_state_t));
        state->watch_face_index = watch_face_index;
        for (uint8_t i = 0; i < sizeof(_default_timer_values) / sizeof(uint32_t); i++) {
            state->timers[i].value = _default_timer_values[i];
        }
    }
}

void timer_face_activate(void *context) {
    timer_state_t *state = (timer_state_t *)context;
    watch_display_text_with_fallback(WATCH_POSITION_TOP_LEFT, "TMR", "TR");
    watch_set_colon();
    if(state->mode == running) {
        watch_date_time_t now = watch_rtc_get_date_time();
        state->now_ts = watch_utility_date_time_to_unix_time(now, movement_get_current_timezone_offset());
    } else {
        state->pausing_seconds = 1;
        _beeps_to_play = 0;
    }
}

bool timer_face_loop(movement_event_t event, void *context) {
    timer_state_t *state = (timer_state_t *)context;
    uint8_t subsecond = event.subsecond;

    switch (event.event_type) {
        case EVENT_ACTIVATE:
            _draw(state, event.subsecond);
            break;
        case EVENT_TICK:
            if (state->mode == running) {
                if (subsecond % 2) {
                    watch_set_colon();
                    state->now_ts++;
                } else {
                    watch_clear_colon();
                }
            }
            else if (state->mode == pausing) state->pausing_seconds++;
            else if (state->quick_cycle) {
                if (HAL_GPIO_BTN_ALARM_read()) {
                    _settings_increment(state);
                    subsecond = 0;
                } else _abort_quick_cycle(state);
            }
            _draw(state, subsecond);
            break;
        case EVENT_LIGHT_BUTTON_DOWN:
            if (state->mode != setting) movement_illuminate_led();
            switch (state->mode) {
                case pausing:
                    _reset(state);
                    button_beep();
                    watch_set_colon();
                    subsecond = 0;
                    break;
                case waiting:
                    state->timers[state->current_timer].unit.repeat ^= 1;
                    break;
                case running:
                    movement_illuminate_led();
                    break;
                case setting:
                    if (state->settings_state == 2) {
                        _resume_setting(state);
                        button_beep();
                    }
                    state->settings_state = (state->settings_state + 1) % 3;
                    break;
                default:
                    break;
            }
            _draw(state, subsecond);
            break;
        case EVENT_LIGHT_LONG_PRESS:
            if (state->mode == waiting) {
                state->timers[state->current_timer].unit.repeat ^= 1; // revert change made
                // initiate settings
                state->mode = setting;
                state->settings_state = 0;
                movement_request_tick_frequency(4);
            } else if (state->mode == setting) {
                _resume_setting(state);
                button_beep();
            }
            _draw(state, subsecond);
            break;
        case EVENT_ALARM_BUTTON_UP:
            _abort_quick_cycle(state);
            if (_check_for_signal()) break;;
            switch (state->mode) {
                case waiting:
                    _start(state, true);
                    button_beep();
                    subsecond = 0;
                    break;
                default:
                    break;
            }
            _draw(state, subsecond);
            break;
        case EVENT_ALARM_BUTTON_DOWN:
            _abort_quick_cycle(state);
            if (_check_for_signal()) break;;
            switch (state->mode) {
                case running:
                    state->mode = pausing;
                    state->pausing_seconds = 0;
                    state->paused_left = state->target_ts - state->now_ts;
                    movement_cancel_background_task();
                    movement_request_tick_frequency(1);
                    watch_set_colon();
                    subsecond = 0;
                    button_beep();
                    break;
                case pausing:
                    _start(state, false);
                    button_beep();
                    subsecond = 0;
                    break;
                case waiting:
                    break;
                case setting:
                    _settings_increment(state);
                    subsecond = 0;
                    break;
            }
            _draw(state, subsecond);
            break;
        case EVENT_ALARM_LONG_PRESS:
            switch(state->mode) {
                case setting:
                    switch (state->settings_state) {
                        case 0:
                        case 1:
                        case 2:
                            state->quick_cycle = true;
                            movement_request_tick_frequency(8);
                            break;
                        default:
                            break;
                    }
                    break;
                case waiting:
                    uint8_t last_timer = state->current_timer;
                    state->current_timer = (state->current_timer + 1) % TIMER_SLOTS;
                    _set_next_valid_timer(state);
                    // start the time immediately if there is only one valid timer slot
                    if (last_timer == state->current_timer) {
                        _start(state, true); 
                        subsecond = 0;
                    }
                    button_beep();
                    break;
                case running:
                    state->mode = pausing;
                    state->pausing_seconds = 0;
                    state->paused_left = state->target_ts - state->now_ts;
                    movement_cancel_background_task();
                    movement_request_tick_frequency(1);
                    watch_set_colon();
                    subsecond = 0;
                    button_beep();
                    break;
                case pausing:
                    _start(state, false);
                    button_beep();
                    subsecond = 0;
                    break;
                default:
                    break;
            }
            _draw(state,subsecond);
            break;
        case EVENT_BACKGROUND_TASK:
            // play the alarm
            _beeps_to_play = 4;
            watch_buzzer_play_sequence((int8_t *)_sound_seq_beep, _signal_callback);
            _reset(state);
            if (state->timers[state->current_timer].unit.repeat) {
                _start(state, false);
                subsecond = 0;
            }
            break;
        case EVENT_ALARM_LONG_UP:
            _abort_quick_cycle(state);
            break;
        case EVENT_MODE_LONG_PRESS:
        case EVENT_TIMEOUT:
            _abort_quick_cycle(state);
            movement_move_to_face(0);
            break;
        default:
            movement_default_loop_handler(event);
            break;
    }

    return true;
}

void timer_face_resign(void *context) {
    timer_state_t *state = (timer_state_t *)context;
    if (state->mode == setting) {
        state->settings_state = 0;
        state->mode = waiting;
    }
}
