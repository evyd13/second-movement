/*
 * MIT License
 *
 * Copyright (c) 2022 Joey Castillo
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
#include "set_time_face.h"
#include "watch.h"
#include "watch_utility.h"
#include "zones.h"
#include "watch_common_display.h"

#define SET_TIME_FACE_NUM_SETTINGS (7)

static int32_t current_offset;
static uint8_t setting_mode_character_index = 0;
static bool allow_advance_page;
static bool advance_page;

#define SET_TIME_FACE_BACK_ON_ADJUST true

static inline void button_beep() {
    // play a beep as confirmation for a button press (if applicable)
    if (movement_button_should_sound()) watch_buzzer_play_note_with_volume(BUZZER_NOTE_C7, 50, movement_button_volume());
}

static uint32_t _get_keypad_input(
        uint8_t minimum_value,
        uint8_t maximum_value,
        uint8_t current_value,
        uint8_t* character_index,
        uint8_t value_modifier) {
        //value modifier: what needs to be added to current_value to match what is displayed, for year this is 20.

    uint8_t new_value;
    uint8_t input = movement_get_keypad_number_pressed();
    uint8_t _current_value = current_value + value_modifier;
    uint8_t remainder = _current_value % 10;

    if (*character_index == 0) {
        new_value = input * 10 + remainder;
    } else {
        new_value = _current_value - remainder + input;
    }

    if (new_value > maximum_value && *character_index == 0) {
        new_value = (new_value/10)*10;
    };
    if (new_value > maximum_value) return _current_value - value_modifier;
    if (new_value < minimum_value) return _current_value - value_modifier;

    button_beep();
    allow_advance_page = true;
    *character_index = !*character_index;
    return new_value - value_modifier;
}

static void _handle_keypad_input(watch_date_time_t date_time, uint8_t current_page) {
    switch (current_page) {
        uint8_t min_value = 1;
        uint8_t max_value = 99;
        case 0: // year
            min_value = RTC_REFERENCE_YEAR % 100;
            max_value = 0b111111;
            date_time.unit.year = _get_keypad_input(
                min_value,
                max_value,
                date_time.unit.year,
                &setting_mode_character_index,
                min_value);
            break;
        case 1: // month
            min_value = 1;
            max_value = 12;
            date_time.unit.month = _get_keypad_input(
                min_value,
                max_value,
                date_time.unit.month,
                &setting_mode_character_index, 0);
            break;
        case 2: // day
            min_value = 1;
            max_value = watch_utility_days_in_month(date_time.unit.month, date_time.unit.year + WATCH_RTC_REFERENCE_YEAR);
            date_time.unit.day = _get_keypad_input(
                min_value,
                max_value,
                date_time.unit.day,
                &setting_mode_character_index, 0);
            break;
        case 4: // hour
            min_value = movement_clock_mode_24h() ? 0 : 1;
            max_value = movement_clock_mode_24h() ? 23 : 12;
            date_time.unit.hour = _get_keypad_input(
                min_value,
                max_value,
                date_time.unit.hour,
                &setting_mode_character_index,
                movement_clock_mode_24h() ? 0 : (date_time.unit.hour > 12) ? -12 : 0);
            break;
        case 5: // minute
            min_value = 0;
            max_value = 59;
            date_time.unit.minute = _get_keypad_input(
                min_value,
                max_value,
                date_time.unit.minute,
                &setting_mode_character_index, 0);
            break;
        case 6: // second
            if(movement_get_key_pressed() == KEYPAD_KEY_K0) {
                // Thanks @alesgenova for this code!
                uint32_t counter = watch_rtc_get_counter();
                uint32_t freq = watch_rtc_get_frequency();
                uint32_t half_freq = freq >> 1;
                uint32_t subsecond_mask = freq - 1;
                uint32_t subseconds = counter & subsecond_mask;

                uint32_t delta;

                // Like a casio watch we reset the seconds to 0 to the nearest minute
                bool to_next_minute = date_time.unit.second >= 30;

                if (subseconds >= half_freq) {
                    delta = (subseconds - half_freq) * 8;
                } else {
                    delta = (half_freq + subseconds) * 8;
                }

                date_time.unit.second = 0;

                if (to_next_minute) {
                    uint32_t timestamp = watch_utility_date_time_to_unix_time(date_time, 0);
                    timestamp += 60;
                    date_time = watch_utility_date_time_from_unix_time(timestamp, 0);
                }

                watch_rtc_enable(false);
                delay_ms(delta);
                watch_rtc_enable(true);
                movement_move_to_face(0);
            }
            break;
    }
    movement_set_local_date_time(date_time);
}

void set_time_face_setup(uint8_t watch_face_index, void ** context_ptr) {
    (void) watch_face_index;
    if (*context_ptr == NULL) *context_ptr = malloc(sizeof(uint8_t));
}

void set_time_face_activate(void *context) {
    *((uint8_t *)context) = 0;
    movement_request_tick_frequency(4);
    current_offset = movement_get_current_timezone_offset();
    setting_mode_character_index = 0;
    allow_advance_page = 0;
    advance_page = 0;
}

bool set_time_face_loop(movement_event_t event, void *context) {
    uint8_t current_page = *((uint8_t *)context);
    watch_date_time_t date_time = movement_get_local_date_time();

    switch (event.event_type) {
        case EVENT_TIMEOUT:
            movement_move_to_face(0);
            break;

        case EVENT_ACTIVATE:
        case EVENT_TICK:
            break;

        case EVENT_ADJUST_BUTTON_DOWN:
            if (SET_TIME_FACE_BACK_ON_ADJUST) {
                movement_move_to_face(0);
                break;
            }

        case EVENT_KEYPAD_BUTTON_DOWN:
            if (movement_is_number_pressed() && (current_page > 3 || current_page < 3)) {
                _handle_keypad_input(date_time, current_page);
                if (!setting_mode_character_index && allow_advance_page) {
                    advance_page = true;
                }
            } else if (movement_get_key_pressed() == KEYPAD_KEY_PLUS && current_page > 3) {
                movement_set_clock_mode_24h(!movement_clock_mode_24h());
                button_beep();
            } else if (movement_get_key_pressed() == KEYPAD_KEY_DECIMAL && current_page > 3) {
                if (!movement_clock_mode_24h()) {
                    if (date_time.unit.hour > 12) {
                        date_time.unit.hour -= 12;
                    } else {
                        date_time.unit.hour += 12;
                    }
                    movement_set_local_date_time(date_time);
                    button_beep();
                }
            } else if (current_page == 3) {
                int timezone_index_offset = 0;
                if (movement_get_key_pressed() == KEYPAD_KEY_PLUS){
                    timezone_index_offset=1;
                } else if (movement_get_key_pressed() == KEYPAD_KEY_MINUS) {
                    if (movement_get_timezone_index() > 0){timezone_index_offset=-1;}
                    else {timezone_index_offset=NUM_ZONE_NAMES-1;}
                }
                movement_set_timezone_index(movement_get_timezone_index() + timezone_index_offset);
                if (movement_get_timezone_index() >= NUM_ZONE_NAMES) movement_set_timezone_index(0);
                current_offset = movement_get_current_timezone_offset_for_zone(movement_get_timezone_index());
            }
            break;

        case EVENT_MODE_LONG_PRESS:
            movement_force_led_off();
            movement_move_to_next_face();
            return true;

        case EVENT_MODE_BUTTON_DOWN:
            if (current_page + 1 != SET_TIME_FACE_NUM_SETTINGS) {
                switch(current_page) {
                    case 2:
                        if (setting_mode_character_index == 0) break;
                    case 3:
                    case 6:
                        setting_mode_character_index = 1;
                        break;
                }
                setting_mode_character_index = !setting_mode_character_index;
                if (!setting_mode_character_index) {
                    advance_page = true;
                }
                break;
            }

        default:
            return movement_default_loop_handler(event);
    }
    if (advance_page && event.subsecond % 2 != 0) {
        advance_page = false;
        allow_advance_page = false;
        current_page = (current_page + 1) % SET_TIME_FACE_NUM_SETTINGS;
        *((uint8_t *)context) = current_page;
        setting_mode_character_index = 0;
    } else if (advance_page && event.subsecond % 2 == 0) {
        return true;
    }

    char buf[11];
    watch_display_text(WATCH_POSITION_TOP, "  ");
    if (current_page == 3) {
        char buf_index[3];
        sprintf(buf_index, "%02d", movement_get_timezone_index()+1);
        watch_display_text(WATCH_POSITION_TOP, buf_index);
        if (event.subsecond % 2 == 0) {
            uint8_t hours = abs(current_offset) / 3600;
            uint8_t minutes = (abs(current_offset) % 3600) / 60;

            sprintf(buf, "%02d %02d   ", hours % 100, minutes % 100);
            watch_set_colon();
        } else {
            sprintf(buf, "%-8s", watch_utility_time_zone_name_at_index(movement_get_timezone_index()));
            watch_clear_colon();
        }
    } else if (current_page < 3) {
        watch_clear_colon();
        watch_clear_indicator(WATCH_INDICATOR_AM);
        watch_clear_indicator(WATCH_INDICATOR_PM);
        sprintf(buf, "%2d %02d-%02d", date_time.unit.year + 20, date_time.unit.month, date_time.unit.day);
    } else {
        watch_set_colon();
        if (movement_clock_mode_24h()) {
            watch_clear_indicator(WATCH_INDICATOR_AM);
            watch_clear_indicator(WATCH_INDICATOR_PM);
            sprintf(buf, "%02d %02d %02d", date_time.unit.hour, date_time.unit.minute, date_time.unit.second);
        } else {
            sprintf(buf, "%02d %02d %02d", (date_time.unit.hour % 12) ? (date_time.unit.hour % 12) : 12, date_time.unit.minute, date_time.unit.second);
            if (date_time.unit.hour > 12) {
                watch_clear_indicator(WATCH_INDICATOR_AM);
                watch_set_indicator(WATCH_INDICATOR_PM);
            } else {
                watch_clear_indicator(WATCH_INDICATOR_PM);
                watch_set_indicator(WATCH_INDICATOR_AM);
            }
        }
    }

    watch_display_text(WATCH_POSITION_BOTTOM, buf);

    // blink up the parameter we're setting
    if (event.subsecond % 2 == 0) {
        switch (current_page) {
            case 0:
            case 4:
                watch_display_character('_', 2 + setting_mode_character_index);
                break;
            case 1:
            case 5:
                watch_display_character('_', 5 + setting_mode_character_index);
                break;
            case 2:
                watch_display_character('_', 8 + setting_mode_character_index);
                break;
            case 6:
                watch_display_text(WATCH_POSITION_SECONDS, "__");
                break;
        }
    }

    return true;
}

void set_time_face_resign(void *context) {
    (void) context;
    movement_store_settings();
    movement_request_tick_frequency(1);
}
