/*
 * MIT License
 *
 * Copyright (c) 2024 Joey Castillo
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
#include "all_segments_face.h"
#include "watch_common_display.h"
#include "watch.h"

void all_segments_face_setup(uint8_t watch_face_index, void ** context_ptr) {
    (void) watch_face_index;
    (void) context_ptr;
}

void all_segments_face_activate(void *context) {
    (void) context;
    watch_display_text(WATCH_POSITION_TOP, "@@");
    watch_display_text(WATCH_POSITION_BOTTOM, "@@@@@@@@");
    watch_set_indicator(WATCH_INDICATOR_SIGNAL);
    watch_set_indicator(WATCH_INDICATOR_BELL);
    watch_set_indicator(WATCH_INDICATOR_PM);
    watch_set_indicator(WATCH_INDICATOR_AM);
    watch_set_indicator(WATCH_INDICATOR_K);
    watch_set_indicator(WATCH_INDICATOR_DIVIDE);
    watch_set_indicator(WATCH_INDICATOR_TIMES);
    watch_set_indicator(WATCH_INDICATOR_PLUS);
    watch_set_indicator(WATCH_INDICATOR_MINUS);
    watch_display_text(WATCH_POSITION_BOTTOM, "@@@@@@@@");
}

bool all_segments_face_loop(movement_event_t event, void *context) {
    

    (void) context;

    movement_default_loop_handler(event);

    return true;
}

void all_segments_face_resign(void *context) {
    (void) context;
}
