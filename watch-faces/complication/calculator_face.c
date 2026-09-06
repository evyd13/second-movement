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

static const watch_date_time_t distant_future = {
    .unit = {0, 0, 0, 1, 1, 63}
};

#define CALCULATOR_DISPLAY_LENGTH 10 //8 digits, a decimal point and \0

static double x; // 1
static double y; // 2
static double z; // result
static calculator_mode_t mode; // mode
static bool is_constant;
static bool is_showing_answer;
static char current_input[CALCULATOR_DISPLAY_LENGTH] = { 0 };

static inline void button_beep() {
    // play a beep as confirmation for a button press (if applicable)
    if (movement_button_should_sound()) watch_buzzer_play_note_with_volume(BUZZER_NOTE_C7, 50, movement_button_volume());
}

void _clear_input(void) {
    for (uint8_t i = 0; i < CALCULATOR_DISPLAY_LENGTH; i++) {
        current_input[i] = 0;
    }
    current_input[0] = '0';
}

char* double_to_string(double value) {
    char buf[sizeof(value)] = { 0 };

}

void save_current_input(double* destination) {
    *destination = atof(current_input);
}

void _calculator_init(void) {
    x = 0.00;
    y = 0.00;
    z = 2356.543543;
    mode = CALCULATOR_MODE_NONE;
    is_constant = false;
    is_showing_answer = false;
    movement_schedule_background_task(distant_future);
    _clear_input();
}

void handle_equals_key(void) {
    if (is_constant) {
        
    } else {
        mode = CALCULATOR_MODE_NONE;
    }
    button_beep();
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
    
    sprintf(buf, string_has_decimal_point(current_input) ? "%9s" : "%8s.", current_input);
    
    watch_display_text(WATCH_POSITION_BOTTOM, buf);
    _calculator_render_mode_icons();
}

void handle_keypad_number_input(void) {
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
    // Limit reached, return
    if (strlen(current_input) == (string_has_decimal_point(current_input) ? CALCULATOR_DISPLAY_LENGTH - 1 : CALCULATOR_DISPLAY_LENGTH - 2)) return;

    if (strlen(current_input) == 1 && current_input[0] == '0') {
        // first character typed!
        current_input[c == '.' ? 1 : 0] = c;
    } else {
        //other characters are appended
        if (string_has_decimal_point(current_input) && c == '.') return;
        strncat(current_input, &c, 1);
    }
    button_beep();
}

void handle_modifier_input(void) {
    switch(movement_get_key_pressed()) {
        case KEYPAD_KEY_DIVIDE:
            if (mode == CALCULATOR_MODE_DIVISION) { is_constant = !is_constant;}
            else {mode = CALCULATOR_MODE_DIVISION; is_constant = false;}
            break;
        case KEYPAD_KEY_TIMES:
            if (mode == CALCULATOR_MODE_MULTIPLICATION) { is_constant = !is_constant;}
            else {mode = CALCULATOR_MODE_MULTIPLICATION; is_constant = false;}
            break;
        case KEYPAD_KEY_MINUS:
            if (mode == CALCULATOR_MODE_SUBTRACTION) { is_constant = !is_constant;}
            else {mode = CALCULATOR_MODE_SUBTRACTION; is_constant = false;}
            break;
        case KEYPAD_KEY_PLUS:
            if (mode == CALCULATOR_MODE_ADDITION) { is_constant = !is_constant;}
            else {mode = CALCULATOR_MODE_ADDITION; is_constant = false;}
            break;
    }
    button_beep();
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
            break;
        case EVENT_LOW_ENERGY_UPDATE:
            break;
        case EVENT_MODE_BUTTON_DOWN:
            if (!(strlen(current_input) == 1 && current_input[0] == '0')) {
                _clear_input();
                button_beep();
                break;
            } else if (mode != CALCULATOR_MODE_NONE) {
                mode = CALCULATOR_MODE_NONE;
                _calculator_init();
                button_beep();
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
