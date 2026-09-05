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
static bool is_editing;

/* 
 * Which button do we want to use for advancing the
 * timezone selection? Makes a big difference in UX!
 */
 #define SET_TIME_FACE_TIMEZONE_MODE false
#define SET_TIME_FACE_TIMEZONE_KEYPAD true

// TODOEEF: KEYPAD INPUT!! maybe make one function for multiple faces? hint: pointers
static void _handle_keypad_input(watch_date_time_t date_time, uint8_t current_page) {
    switch (current_page) {
        case 0: // year
            date_time.unit.year = (date_time.unit.year + 1) % 60;
            break;
        case 1: // month
            date_time.unit.month = (date_time.unit.month % 12) + 1;
            break;
        case 2: // day
            date_time.unit.day = (date_time.unit.day % watch_utility_days_in_month(date_time.unit.month, date_time.unit.year + WATCH_RTC_REFERENCE_YEAR)) + 1;
            break;
        case 4: // hour
            date_time.unit.hour = (date_time.unit.hour + 1) % 24;
            break;
        case 5: // minute
            date_time.unit.minute = (date_time.unit.minute + 1) % 60;
            break;
        case 6: // second
            date_time.unit.second = 0;
            break;
    }
    movement_set_local_date_time(date_time);



    // if(movement_is_number_pressed()) {
    //     uint8_t maximum_value = movement_clock_mode_24h() ? 23 : 12;
    //     uint8_t current_value = get_displayed_hour(state->hour);
    //     uint8_t new_value;
    //     uint8_t input = movement_get_keypad_number_pressed();
    //     uint8_t remainder = current_value % 10;

    //     if (state->setting_mode_character_index == 0) {
    //         new_value = input * 10 + remainder;
    //     } else {
    //         new_value = current_value - remainder + input;
    //     }

    //     if (new_value > maximum_value) {
    //         new_value = (new_value/10)*10;
    //     };
    //     if (new_value > maximum_value) break;
    //     state->hour = apply_pm_to_value(state, new_value);
        
    //     if (state->setting_mode_character_index == 0) {
    //         state->setting_mode_character_index = 1;
    //     } else {
    //         state->setting_mode_character_index = 0;
    //         state->setting_mode = ALARM_FACE_SETTING_MODE_SETTING_MINUTE;
    //     }
    // }
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
    is_editing = 1;
}

bool set_time_face_loop(movement_event_t event, void *context) {
    uint8_t current_page = *((uint8_t *)context);
    watch_date_time_t date_time = movement_get_local_date_time();

    switch (event.event_type) {
        case EVENT_ACTIVATE:
        case EVENT_TICK:
            break;
        case EVENT_ADJUST_BUTTON_DOWN:
            is_editing = !is_editing;
        case EVENT_KEYPAD_BUTTON_DOWN:
            if (movement_is_number_pressed()) {
                _handle_keypad_input(date_time, current_page);
            } else if (
                (movement_get_key_pressed() == KEYPAD_KEY_PLUS || \
                movement_get_key_pressed() == KEYPAD_KEY_DECIMAL) \
                && current_page > 3) {

                movement_set_clock_mode_24h(!movement_clock_mode_24h());
                if (date_time.unit.hour > 12) {
                    watch_clear_indicator(WATCH_INDICATOR_AM);
                    watch_set_indicator(WATCH_INDICATOR_PM);
                } else {
                    watch_clear_indicator(WATCH_INDICATOR_PM);
                    watch_set_indicator(WATCH_INDICATOR_AM);
                }
            } else if (current_page == 3 && SET_TIME_FACE_TIMEZONE_KEYPAD) {
                int timezone_index_offset = 0;
                if (movement_get_key_pressed() == KEYPAD_KEY_PLUS){timezone_index_offset=1;}
                if (movement_get_key_pressed() == KEYPAD_KEY_MINUS && movement_get_timezone_index() > 1){timezone_index_offset=-1;}
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
            if (!is_editing) {
                switch(current_page) {
                    case 0:
                    case 1:
                    case 2:
                        current_page = 3;
                        break;
                    case 3:
                        current_page = 4;
                        break;
                    case 4:
                    case 5:
                    case 6:
                        return movement_default_loop_handler(event);
                }
                setting_mode_character_index = 0;
                *((uint8_t *)context) = current_page;
                break;
            } else if (current_page == 3 && SET_TIME_FACE_TIMEZONE_MODE) {
                movement_set_timezone_index(movement_get_timezone_index() + 1);
                if (movement_get_timezone_index() >= NUM_ZONE_NAMES) movement_set_timezone_index(0);
                current_offset = movement_get_current_timezone_offset_for_zone(movement_get_timezone_index());
                break;
            } else if (current_page + 1 != SET_TIME_FACE_NUM_SETTINGS) {
                switch(current_page) {
                    case 2:
                        if (setting_mode_character_index == 0) break;
                    case 3:
                    case 6:
                        setting_mode_character_index = 1;
                        is_editing = 1;
                        break;
                }
                setting_mode_character_index = !setting_mode_character_index;
                if (!setting_mode_character_index) {
                    current_page = (current_page + 1) % SET_TIME_FACE_NUM_SETTINGS;
                    setting_mode_character_index = 0;
                    *((uint8_t *)context) = current_page;
                }
                break;
            }
        default:
            return movement_default_loop_handler(event);
    }

    char buf[11];
    watch_display_text(WATCH_POSITION_TOP, "  ");
    if (current_page == 3) {
        watch_display_text(WATCH_POSITION_TOP, "--");
        if (current_offset < 0) watch_display_text(WATCH_POSITION_TOP, "< ");
        else if (current_offset > 0) watch_display_text(WATCH_POSITION_TOP, " >");
        if (event.subsecond % 2 == 0 && is_editing) {
            uint8_t hours = abs(current_offset) / 3600;
            uint8_t minutes = (abs(current_offset) % 3600) / 60;

            sprintf(buf, "%2d %02d   ", hours % 100, minutes % 100);
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
            sprintf(buf, "%2d %02d %02d", date_time.unit.hour, date_time.unit.minute, date_time.unit.second);
        } else {
            sprintf(buf, "%2d %02d %02d", (date_time.unit.hour % 12) ? (date_time.unit.hour % 12) : 12, date_time.unit.minute, date_time.unit.second);
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
    if (event.subsecond % 2 == 0 && is_editing) {
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
