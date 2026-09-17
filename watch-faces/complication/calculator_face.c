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
#include "calculator_face.h"
#include "watch.h"
#include "watch_utility.h"
#include <float.h>
#include <limits.h>

static const watch_date_time_t distant_future = {
    .unit = {0, 0, 0, 1, 1, 63}
};

#define CALCULATOR_DISPLAY_LENGTH 10 //8 digits, a decimal point and \0

static double x; // 1
static double y; // 2
static double z; // result
static calculator_mode_t mode; // mode
static calculator_display_mode_t display_mode;
static bool is_constant;
static long int e_number;
static bool should_clear_input;
static bool has_x_input;
static char current_display[CALCULATOR_DISPLAY_LENGTH+2] = { 0 };

static inline void button_beep() {
    // play a beep as confirmation for a button press (if applicable)
    if (movement_button_should_sound()) watch_buzzer_play_note_with_volume(BUZZER_NOTE_C7, 50, movement_button_volume());
}

void _clear_display(void) {
    for (uint8_t i = 0; i < CALCULATOR_DISPLAY_LENGTH; i++) {
        current_display[i] = 0;
    }
    current_display[0] = '0';
    display_mode = CALCULATOR_DISPLAY_NONE;
    should_clear_input = false;
}

long int str_to_int(char *str) {
   char *endptr;
   long int num;

   num = strtol(str, &endptr, 10);
   if (endptr == str) {
      return 0; // no digits found
   } else if (*endptr != '\0') {
      return 0; // invalid character
   } else {
      return num;
   }
   return 0;
} 



void show_answer() {
    if (z > 99999999.99999999 || z < - 9999999.99999999) {
        int8_t index_plus_character = 0;
        sprintf(current_display, "%8G", z);
        for (uint8_t i = 0; i < CALCULATOR_DISPLAY_LENGTH; i++) {
            if (current_display[i] == 'e') current_display[i] = 'E';
            if (current_display[i] == '+') index_plus_character = i;
        }
        if (index_plus_character != 0) {
            char buf1[CALCULATOR_DISPLAY_LENGTH] = {0};
            char buf2[CALCULATOR_DISPLAY_LENGTH] = {0};
            strncpy(buf1, current_display, index_plus_character);
            strncpy(buf2, current_display + index_plus_character + 1, CALCULATOR_DISPLAY_LENGTH-index_plus_character+2);
            e_number = str_to_int(buf2);
            sprintf(current_display, "%s ", buf1);
        }
        if (strstr(current_display, "INF") != NULL) {
            display_mode = CALCULATOR_DISPLAY_ERROR;
        }
    } else {
        char buf[18] = {0};
        char buf1[CALCULATOR_DISPLAY_LENGTH] = {0};
        sprintf(buf, "%.7f", z);
        // 0.00000001
        // 0.00010000
        // 0.10000000
        uint8_t last_non_zero_digit_index = 0;
        for (uint8_t i = 0; i < 18; i++) {
            if (buf[i] != '0' && buf[i] >= 32 && buf[i] != '.') {
                last_non_zero_digit_index = i;
                break;
            }
        }
        if (last_non_zero_digit_index > 0) {
            strncpy(buf1, buf, last_non_zero_digit_index+1);
            sprintf(current_display, "%s", buf1);
        } else {
            //integer
            sprintf(current_display, "%d", (int) z);
        }
    }
}

void save_current_display(double* destination) {
    *destination = strtod(current_display, "");
}

void _calculator_init(void) {
    x = 0.00;
    y = 0.00;
    z = 0.00;
    e_number = 0;

    mode = CALCULATOR_MODE_NONE;
    is_constant = false;
    should_clear_input = false;
    has_x_input = false;

    _clear_display();
}

void calculator_face_setup(uint8_t watch_face_index, void ** context_ptr) {
    (void) watch_face_index;
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(calculator_state_t));
        memset(*context_ptr, 0, sizeof(calculator_state_t));
    }
}

void calculator_face_activate(void *context) {
    calculator_state_t *calculator_state = (calculator_state_t *)context;
    _calculator_init();
}

bool string_has_decimal_point(char* s) {
    char *res = strchr(s, '.');
    return res != NULL;
}

void _calculator_render_mode_icons(void) {
    if (mode == CALCULATOR_MODE_DIVISION) watch_set_indicator(WATCH_INDICATOR_DIVIDE);
    else watch_clear_indicator(WATCH_INDICATOR_DIVIDE);
    
    if (mode == CALCULATOR_MODE_MULTIPLICATION) watch_set_indicator(WATCH_INDICATOR_TIMES);
    else watch_clear_indicator(WATCH_INDICATOR_TIMES);
    
    if (mode == CALCULATOR_MODE_SUBTRACTION) watch_set_indicator(WATCH_INDICATOR_MINUS);
    else watch_clear_indicator(WATCH_INDICATOR_MINUS);

    if (mode == CALCULATOR_MODE_ADDITION) watch_set_indicator(WATCH_INDICATOR_PLUS);
    else watch_clear_indicator(WATCH_INDICATOR_PLUS);

    if (is_constant) watch_set_indicator(WATCH_INDICATOR_K);
    else watch_clear_indicator(WATCH_INDICATOR_K);
}

void _calculator_render_display(void) {
    char buf[CALCULATOR_DISPLAY_LENGTH] = { 0 };
    sprintf(buf, string_has_decimal_point(current_display) ? "%9s" : "%8s.", current_display);
    
    if (display_mode == CALCULATOR_DISPLAY_ERROR) {
        watch_display_text(WATCH_POSITION_BOTTOM, "Error   ");
    } else {
        watch_display_text(WATCH_POSITION_BOTTOM, buf);
    }
    
    _calculator_render_mode_icons();
}

void handle_keypad_number_input(void) {
    if (e_number > 0) return;
    if (should_clear_input) _clear_display();
    // handle input to current_display
    char c;
    switch(movement_get_key_pressed()) {
        case KEYPAD_KEY_K0:
            c = '0'; break;
        case KEYPAD_KEY_K1:
            c = '1'; break;
        case KEYPAD_KEY_K2:
            c = '2'; break;
        case KEYPAD_KEY_K3:
            c = '3'; break;
        case KEYPAD_KEY_K4:
            c = '4'; break;
        case KEYPAD_KEY_K5:
            c = '5'; break;
        case KEYPAD_KEY_K6:
            c = '6'; break;
        case KEYPAD_KEY_K7:
            c = '7'; break;
        case KEYPAD_KEY_K8:
            c = '8'; break;
        case KEYPAD_KEY_K9:
            c = '9'; break;
        case KEYPAD_KEY_DECIMAL:
            c = '.'; break;
    }
    if (display_mode == CALCULATOR_DISPLAY_ANS && !is_constant) return;

    // don't allow more input than fits
    if (strlen(current_display) == (string_has_decimal_point(current_display) ? CALCULATOR_DISPLAY_LENGTH - 1 : CALCULATOR_DISPLAY_LENGTH - 2)) return;

    if (strlen(current_display) == 1 && current_display[0] == '0') {
        // first character typed!
        current_display[c == '.' ? 1 : 0] = c;
    } else {
        //other characters are appended
        if (string_has_decimal_point(current_display) && c == '.') return;
        strncat(current_display, &c, 1);
    }
    button_beep();
}

void calculate_answer(void) {
    switch(mode) {
        case CALCULATOR_MODE_DIVISION:
            z = x / y;
            break;
        case CALCULATOR_MODE_MULTIPLICATION:
            z = x * y;
            break;
        case CALCULATOR_MODE_SUBTRACTION:
            z = x - y;
            break;
        case CALCULATOR_MODE_ADDITION:
            z = x + y;
            break;
    }
}

void handle_equals_key(void) {
    if (e_number > 0) return;
    save_current_display(&y);
    calculate_answer();

    display_mode = CALCULATOR_DISPLAY_ANS;
    should_clear_input = true;
    if (!is_constant) mode = CALCULATOR_MODE_NONE;
    if (!is_constant) x = 0.00;
    show_answer();
    button_beep();
}

void handle_modifier_input(void) {
    if (e_number > 0) return;
    calculator_mode_t old_mode = mode;
    calculator_mode_t new_mode;
    switch(movement_get_key_pressed()) {
        case KEYPAD_KEY_DIVIDE:
            new_mode = CALCULATOR_MODE_DIVISION;
            break;
        case KEYPAD_KEY_TIMES:
            new_mode = CALCULATOR_MODE_MULTIPLICATION;
            break;
        case KEYPAD_KEY_MINUS:
            new_mode = CALCULATOR_MODE_SUBTRACTION;
            break;
        case KEYPAD_KEY_PLUS:
            new_mode = CALCULATOR_MODE_ADDITION;
            break;
        default:
            return;
    }
    if (old_mode == new_mode) {
        is_constant = true;
        if (is_constant && display_mode == CALCULATOR_DISPLAY_ANS) {
            is_constant = false;
            save_current_display(&x);
            has_x_input = true;
            should_clear_input = true;
        }
    } else if (old_mode != new_mode) {
        if (is_constant && display_mode == CALCULATOR_DISPLAY_ANS) {
            is_constant = false;
            has_x_input = true;
            should_clear_input = true;
        } else if (has_x_input && !should_clear_input) {
            save_current_display(&y);
            calculate_answer();
            show_answer();
            x = 0.00;
            y = 1.00;
        }
        is_constant = false;
        save_current_display(&x);
        has_x_input = true;
        if (display_mode != CALCULATOR_DISPLAY_NONE) {
            display_mode = CALCULATOR_DISPLAY_NONE;
        }
        should_clear_input = true;
    }
    mode = new_mode;
    button_beep();
}

bool handle_button_mode(void) {
    if (display_mode == CALCULATOR_DISPLAY_ANS && e_number > 0) {
        uint8_t character_index;
        for (uint8_t i = 0; i < CALCULATOR_DISPLAY_LENGTH; i++) {
            if (current_display[i] == 'E') {character_index = i;break;}
        }
        if (character_index) {
            display_mode = CALCULATOR_DISPLAY_E;
            char temp_string[3] = {0};
            sprintf(temp_string, "%02d", e_number);
            current_display[character_index] = temp_string[0];
            current_display[character_index+1] = temp_string[1];
        }
        return true;
    } else if ( display_mode == CALCULATOR_DISPLAY_ERROR || \
                (display_mode == CALCULATOR_DISPLAY_ANS && e_number == 0) || \
                display_mode == CALCULATOR_DISPLAY_E) {
        // remove error OR ANS or E
        _calculator_init();
        button_beep();
        return true;
    } else if (!(strlen(current_display) == 1 && current_display[0] == '0')) {
        _calculator_init();
        button_beep();
        return true;
    }
    return false; //fall through
}

bool calculator_face_loop(movement_event_t event, void *context) {
    calculator_state_t *calculator_state = (calculator_state_t *)context;

    switch (event.event_type) {
        case EVENT_ACTIVATE:
            _calculator_init();
            break;
        case EVENT_TICK:
            break;
        case EVENT_KEYPAD_BUTTON_DOWN:
            movement_schedule_background_task(distant_future);
            switch(movement_get_key_pressed()) {
                case KEYPAD_KEY_K0:
                case KEYPAD_KEY_K1:
                case KEYPAD_KEY_K2:
                case KEYPAD_KEY_K3:
                case KEYPAD_KEY_K4:
                case KEYPAD_KEY_K5:
                case KEYPAD_KEY_K6:
                case KEYPAD_KEY_K7:
                case KEYPAD_KEY_K8:
                case KEYPAD_KEY_K9:
                case KEYPAD_KEY_DECIMAL:
                    handle_keypad_number_input();
                    break;
                case KEYPAD_KEY_EQUALS:
                    handle_equals_key();
                    break;
                case KEYPAD_KEY_DIVIDE:
                case KEYPAD_KEY_TIMES:
                case KEYPAD_KEY_MINUS:
                case KEYPAD_KEY_PLUS:
                    handle_modifier_input();
                    break;
                default:
                    break;
            }
            break;
        case EVENT_TIMEOUT:
            movement_move_to_face(0);
            break;
        case EVENT_LOW_ENERGY_UPDATE:
            break;
        case EVENT_MODE_BUTTON_DOWN:
            if (handle_button_mode()) {
                break;
            }
        default:
            return movement_default_loop_handler(event);
    }

    _calculator_render_display();

    return true;
}

void calculator_face_resign(void *context) {
    (void) context;
    movement_cancel_background_task();
    movement_request_tick_frequency(1); // we need to manually blink some pixels
}
