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

 // take a look at this: https://github.com/joeycastillo/Sensor-Watch/blob/main/movement/watch_faces/complication/rpn_calculator_alt_face.c

#include <stdlib.h>
#include <string.h>
#include "calculator_face.h"
#include "watch.h"
#include "watch_utility.h"

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

bool calculator_face_loop(movement_event_t event, void *context) {
    calculator_state_t *calculator_state = (calculator_state_t *)context;

    switch (event.event_type) {
        case EVENT_ACTIVATE:
            watch_display_text(WATCH_POSITION_BOTTOM, "       0.");
            // fall through
        case EVENT_TICK:
            break;
        case EVENT_KEYPAD_BUTTON_DOWN:
            break;
        case EVENT_ADJUST_BUTTON_DOWN:
            break;
        case EVENT_TIMEOUT:
            break;
        case EVENT_LOW_ENERGY_UPDATE:
            break;
        default:
            return movement_default_loop_handler(event);
    }

    return true;
}

void calculator_face_resign(void *context) {
    (void) context;

    // regardless of whether we're running or stopped, cancel the task
    // that was keeping us awake while on screen.
    movement_cancel_background_task();
}
