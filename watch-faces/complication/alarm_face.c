/*
 * MIT License
 *
 * Copyright (c) 2022 Josh Berson, building on Wesley Ellis’ countdown_face.c
 * Copyright (c) 2025 Joey Castillo
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
#include "alarm_face.h"
#include "watch.h"
#include "watch_utility.h"
#include "watch_common_display.h"

//
// Private
//

static void _alarm_face_display_alarm_time(alarm_face_state_t *state) {
    uint8_t hour = state->hour;

    if ( movement_clock_mode_24h() ) {
        watch_clear_indicator(WATCH_INDICATOR_PM);
        watch_clear_indicator(WATCH_INDICATOR_AM);
    } else {
        if ( hour >= 12 ) {
            watch_set_indicator(WATCH_INDICATOR_PM);
            watch_clear_indicator(WATCH_INDICATOR_AM);
        } else {
            watch_set_indicator(WATCH_INDICATOR_AM);
        watch_clear_indicator(WATCH_INDICATOR_PM);
        }
        hour = hour % 12 ? hour % 12 : 12;
    }

    static char lcdbuf[7];
    sprintf(lcdbuf, "%2d%02d", hour, state->minute);

    watch_display_text(WATCH_POSITION_HOURS, lcdbuf);
    watch_display_text(WATCH_POSITION_MINUTES, lcdbuf+2);
}

static inline void button_beep() {
    // play a beep as confirmation for a button press (if applicable)
    if (movement_button_should_sound()) watch_buzzer_play_note_with_volume(BUZZER_NOTE_C7, 50, movement_button_volume());
}

static void alarm_indicate(watch_indicator_t indicator, bool on) {
    if (on) {
        watch_set_indicator(indicator);
    } else {
        watch_clear_indicator(indicator);
    }
}
static void alarm_indicate_time_signal(alarm_face_state_t *state) {
    alarm_indicate(WATCH_INDICATOR_BELL, movement_time_signal_enabled());
}

static void alarm_toggle_time_signal(alarm_face_state_t *state) {
    movement_set_time_signal_enabled(!movement_time_signal_enabled());
    alarm_indicate_time_signal(state);
}

void end_setting(alarm_face_state_t *state) {
    // If we're setting the minute, advance back to normal mode and cancel fast tick.
    state->setting_mode = ALARM_FACE_SETTING_MODE_NONE;
    movement_request_tick_frequency(1);
    // beep to confirm setting.
    button_beep();
    // also turn the alarm on since they just set it.
    state->alarm_is_on = 1;
    movement_set_alarm_enabled(true);
    watch_set_indicator(WATCH_INDICATOR_SIGNAL);
    _alarm_face_display_alarm_time(state);
}

void toggle_am_pm(alarm_face_state_t *state) {
    state->hour = (state->hour < 12) ? state->hour + 12 : state->hour - 12;
    _alarm_face_display_alarm_time(state);
}

//
// Exported
//

void alarm_face_setup(uint8_t watch_face_index, void **context_ptr) {
    (void) watch_face_index;

    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(alarm_face_state_t));
        alarm_face_state_t *state = (alarm_face_state_t *)*context_ptr;
        memset(*context_ptr, 0, sizeof(alarm_face_state_t));

        // default to an 8:00 AM alarm time.
        state->hour = 8;
    }
}

void alarm_face_activate(void *context) {
    alarm_face_state_t *state = (alarm_face_state_t *)context;
    state->setting_mode = ALARM_FACE_SETTING_MODE_NONE;
    state->setting_mode_character_index = 0;
    alarm_indicate_time_signal(state);
}
void alarm_face_resign(void *context) {
    (void) context;
}

bool alarm_face_loop(movement_event_t event, void *context) {
    alarm_face_state_t *state = (alarm_face_state_t *)context;

    switch (event.event_type) {
        case EVENT_ACTIVATE:
            watch_display_text(WATCH_POSITION_TOP, "AL");
            if (state->alarm_is_on) watch_set_indicator(WATCH_INDICATOR_SIGNAL);
            watch_set_colon();
            _alarm_face_display_alarm_time(state);
            break;
        case EVENT_TICK:
            // No action needed for tick events in normal mode; we displayed our stuff in EVENT_ACTIVATE.
            if (state->setting_mode == ALARM_FACE_SETTING_MODE_NONE)
                break;

            // but in settings mode, we need to blink up the parameter we're setting.
            _alarm_face_display_alarm_time(state);
            if (event.subsecond % 2 == 0) {
                uint8_t position = 0;
                switch (state->setting_mode) {
                    case ALARM_FACE_SETTING_MODE_SETTING_HOUR:
                        position = 2 + state->setting_mode_character_index;
                        break;
                    case ALARM_FACE_SETTING_MODE_SETTING_MINUTE:
                        position = 5 + state->setting_mode_character_index;
                        break;
                }
                watch_display_character('_', position);
            }
            break;
        case EVENT_KEYPAD_BUTTON_DOWN:
            switch (state->setting_mode) {
                case ALARM_FACE_SETTING_MODE_NONE:
                    switch(movement_get_key_pressed()) {
                        case KEYPAD_KEY_K4:
                            state->alarm_is_on ^= 1;
                            if ( state->alarm_is_on ) {
                                watch_set_indicator(WATCH_INDICATOR_SIGNAL);
                                movement_set_alarm_enabled(true);
                            } else {
                                watch_clear_indicator(WATCH_INDICATOR_SIGNAL);
                                movement_set_alarm_enabled(false);
                            }
                            break;
                        case KEYPAD_KEY_TIMES:
                            alarm_toggle_time_signal(state);
                            break;
                        default:
                            break;
                    }

                    break;
                case ALARM_FACE_SETTING_MODE_SETTING_HOUR:
                    switch(movement_get_key_pressed()) {
                        uint8_t maximum_value = movement_clock_mode_24h() ? 23 : 12;
                        case KEYPAD_KEY_DECIMAL:
                            toggle_am_pm(state);
                            break;
                        default:
                            if(movement_is_number_pressed()) {
                                switch(movement_get_keypad_number_pressed()) {
                                    case 3:
                                    case 4:
                                    case 5:
                                    case 6:
                                    case 7:
                                    case 8:
                                    case 9:
                                    
                                        break;
                                    case 0:
                                        break;
                                    case 1:
                                        break;
                                    case 2:
                                        break;
                                }
                            }
                            break;
                    }
                    break;
                case ALARM_FACE_SETTING_MODE_SETTING_MINUTE:
                    switch(movement_get_key_pressed()) {
                        case KEYPAD_KEY_DECIMAL:
                            toggle_am_pm(state);
                            break;
                        default:
                            break;
                    }
                    break;
            }
            if (movement_get_key_pressed() == MOVEMENT_BACKLIGHT_KEY) {
                movement_illuminate_led();
            }
            _alarm_face_display_alarm_time(state);
            break;
        case EVENT_MODE_BUTTON_UP:
            switch (state->setting_mode) {
                case ALARM_FACE_SETTING_MODE_SETTING_HOUR:
                    // If we're setting the hour, advance to minute set mode.
                    switch (state->setting_mode_character_index) {
                        case 0:
                            state->setting_mode_character_index = 1;
                            break;
                        default:
                            state->setting_mode_character_index = 0;
                            state->setting_mode = ALARM_FACE_SETTING_MODE_SETTING_MINUTE;
                            break;
                    }
                    break;
                case ALARM_FACE_SETTING_MODE_SETTING_MINUTE:
                    switch (state->setting_mode_character_index) {
                        case 0:
                            state->setting_mode_character_index = 1;
                            break;
                        default:
                            state->setting_mode_character_index = 0;
                            // If we're setting the minute, advance back to normal mode and cancel fast tick.
                            state->setting_mode = ALARM_FACE_SETTING_MODE_NONE;
                            movement_request_tick_frequency(1);
                            // beep to confirm setting.
                            button_beep();
                            // also turn the alarm on since they just set it.
                            state->alarm_is_on = 1;
                            movement_set_alarm_enabled(true);
                            watch_set_indicator(WATCH_INDICATOR_SIGNAL);
                            _alarm_face_display_alarm_time(state);
                            break;
                    }
                    break;
                default:
                    movement_move_to_next_face();
                    break;
            }
            break;
        case EVENT_ADJUST_BUTTON_UP:
            if (state->setting_mode == ALARM_FACE_SETTING_MODE_NONE) {
                // long press in normal mode: move to hour setting mode, request fast tick.
                state->setting_mode = ALARM_FACE_SETTING_MODE_SETTING_HOUR;
                movement_request_tick_frequency(4);
                button_beep();
            } else {
                end_setting(state);
            }
            break;
        case EVENT_BACKGROUND_TASK:
            movement_play_alarm();
                // 2022-07-23: Thx @joeycastillo for the dedicated “alarm” signal
            break;
        case EVENT_TIMEOUT:
            movement_move_to_face(0);
            break;
        case EVENT_LOW_ENERGY_UPDATE:
            break;
        default:
            movement_default_loop_handler(event);
            break;
    }

    return true;
}

movement_watch_face_advisory_t alarm_face_advise(void *context) {
    alarm_face_state_t *state = (alarm_face_state_t *)context;
    movement_watch_face_advisory_t retval = { 0 };

    if ( state->alarm_is_on ) {
        watch_date_time_t now = movement_get_local_date_time();
        retval.wants_background_task = (state->hour==now.unit.hour && state->minute==now.unit.minute);
        // We’re at the mercy of the advise handler
        // In Safari, the emulator triggers at the ›end‹ of the minute
        // Converting to Unix timestamps and taking a difference between now and wake
        // is not an easy win — because the timestamp for wake has to rely on now
        // for its date. So first we’d have to see if the TOD of wake is after that
        // of now. If it is, take tomorrow’s date, calculating month and year rollover
        // if need be.
    }

    return retval;
}
