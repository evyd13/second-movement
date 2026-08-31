/*
 * MIT License
 *
 * Copyright (c) 2020 Joey Castillo
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

#include "watch_slcd.h"
#include "watch_common_display.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

uint8_t IndicatorSegments[10] = {
    SLCD_SEGID(0, 0), // WATCH_INDICATOR_SIGNAL
    SLCD_SEGID(1, 0), // WATCH_INDICATOR_BELL
    SLCD_SEGID(2, 4), // WATCH_INDICATOR_PM
    SLCD_SEGID(2, 1), // WATCH_INDICATOR_AM

    SLCD_SEGID(2, 0), // WATCH_INDICATOR_K
    SLCD_SEGID(2, 30), // WATCH_INDICATOR_DIVIDE
    SLCD_SEGID(2, 10), // WATCH_INDICATOR_TIMES
    SLCD_SEGID(2, 13), // WATCH_INDICATOR_MINUS
    SLCD_SEGID(2, 16), // WATCH_INDICATOR_PLUS

    SLCD_SEGID(1, 8), // WATCH_INDICATOR_COLON
};

void watch_display_character(uint8_t character, uint8_t position) {
    // if (character == '.') {
    //     // TODOEEF: position -1 and apply decimal
    //     character = '_'; // we can use the bottom segment; make dot an underscore
    // }
    // TODOEEF: filter characters for pos 1,2 and the other

    digit_mapping_t segmap;
    uint8_t segdata;

    segmap = Classic_LCD_Display_Mapping[position];
    segdata = Classic_LCD_Character_Set[character - 0x20];

    for (int i = 0; i < 16; i++) {
        if (segmap.segment[i].value == segment_does_not_exist) {
            // Segment does not exist; skip it.
            segdata = segdata >> 1;
            continue;
        }
        uint8_t com = segmap.segment[i].address.com;
        uint8_t seg = segmap.segment[i].address.seg;

        if (segdata & 1) {
            watch_set_pixel(com, seg);
        }
        else {
            watch_clear_pixel(com, seg);
        }

        segdata = segdata >> 1;
    }
    // TODOEEF: SPECIAL CASE FOR T IN POS 1    
    // if (character == 'T' && position == 1) watch_set_pixel(1, 12); // add descender
}

void watch_display_character_lp_seconds(uint8_t character, uint8_t position) {
    // Will only work for digits and for positions  8 and 9 - but less code & checks to reduce power consumption

    digit_mapping_t segmap;
    uint8_t segdata;

    /// TODO: See optimization note above.

    segmap = Classic_LCD_Display_Mapping[position];
    segdata = Classic_LCD_Character_Set[character - 0x20];

    for (int i = 0; i < 16; i++) {
        if (segmap.segment[i].value == segment_does_not_exist) {
            // Segment does not exist; skip it.
            segdata = segdata >> 1;
            continue;
        }
        uint8_t com = segmap.segment[i].address.com;
        uint8_t seg = segmap.segment[i].address.seg;

        if (segdata & 1) {
            watch_set_pixel(com, seg);
            
        }
        else {
            watch_clear_pixel(com, seg);
        }

        segdata = segdata >> 1;
    }
}

void watch_display_string(const char *string, uint8_t position) {
    size_t i = 0;
    while(string[i] != 0) {
        watch_display_character(string[i], position + i);
        i++;
        if (position + i >= 10) break;
    }
}

void watch_display_text(watch_position_t location, const char *string) {
    switch (location) {
        case WATCH_POSITION_TOP:
            watch_display_character(string[0], 0);
            if (string[1]) {
                watch_display_character(string[1], 1);
            }
            break;
        case WATCH_POSITION_BOTTOM:
            {
                int i = 0;
                while (string[i] != 0) {
                    watch_display_character(string[i], 2 + i);
                    i++;
                }
        }
            break;
        case WATCH_POSITION_HOURS:
            watch_display_character(string[0], 2);
            if (string[1]) {
                watch_display_character(string[1], 3);
            }
            break;
        case WATCH_POSITION_MINUTES:
            watch_display_character(string[0], 5);
            if (string[1]) {
                watch_display_character(string[1], 6);
            }
            break;
        case WATCH_POSITION_SECONDS:
            watch_display_character(string[0], 8);
            if (string[1]) {
                watch_display_character(string[1], 9);
            }
            break;
        case WATCH_POSITION_FULL:
            // This is deprecated, but we use it for the legacy behavior.
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
            watch_display_string(string, 0);
            #pragma GCC diagnostic pop
    }
}

void watch_display_float_with_best_effort(float value, const char *units) {
    char buf[8];
    char buf_fallback[8];
    const char *blank_units = "  ";

    if (value < -99.9) {
        watch_display_text(WATCH_POSITION_BOTTOM, "Undrflow");
        return;
    } else if (value > 199.99) {
        watch_display_text(WATCH_POSITION_BOTTOM, "Overflow");
        return;
    }

    uint16_t value_times_100 = abs((int)round(value * 100.0));

    if (value < 0 && value_times_100 != 0) {
        if (value_times_100 > 999) {
            snprintf(buf, sizeof(buf), "-%4.1f%s", -value, units ? units : blank_units);
            snprintf(buf_fallback, sizeof(buf_fallback), "%s", buf);
        } else {
            snprintf(buf, sizeof(buf), "-%03d%s", value_times_100 % 1000u, units ? units : blank_units);
            snprintf(buf_fallback, sizeof(buf_fallback), "-%3.1f%s", -value, units ? units : blank_units);
        }
    } else if (value_times_100 > 9999) {
        snprintf(buf, sizeof(buf), "%5u%s", value_times_100, units ? units : blank_units);
        snprintf(buf_fallback, sizeof(buf_fallback), "%4.1f%s", value, units ? units : blank_units);
    } else if (value_times_100 > 999) {
        snprintf(buf, sizeof(buf), "%4u%s", value_times_100, units ? units : blank_units);
        snprintf(buf_fallback, sizeof(buf_fallback), "%4.1f%s", value, units ? units : blank_units);
    } else {
        snprintf(buf, sizeof(buf), " %03u%s", value_times_100 % 1000u, units ? units : blank_units);
        snprintf(buf_fallback, sizeof(buf_fallback), "%4.2f%s", value, units ? units : blank_units);
    }

    watch_display_text(WATCH_POSITION_BOTTOM, buf_fallback);
}

void watch_set_colon(void) {
    watch_set_pixel(1, 8);
}

void watch_clear_colon(void) {
    watch_clear_pixel(1, 8);
}

void watch_set_indicator(watch_indicator_t indicator) {
    uint32_t value = IndicatorSegments[indicator];
    uint8_t com = SLCD_COMNUM(value);
    uint8_t seg = SLCD_SEGNUM(value);
    watch_set_pixel(com, seg);
}

void watch_clear_indicator(watch_indicator_t indicator) {
    uint32_t value = IndicatorSegments[indicator];
    uint8_t com = SLCD_COMNUM(value);
    uint8_t seg = SLCD_SEGNUM(value);
    watch_clear_pixel(com, seg);
}

void watch_clear_all_indicators(void) {
    /// TODO: Optimize this? Can be 3-4 writes to SDATAL registers
    watch_clear_indicator(WATCH_INDICATOR_SIGNAL);
    watch_clear_indicator(WATCH_INDICATOR_BELL);
    watch_clear_indicator(WATCH_INDICATOR_PM);
    watch_clear_indicator(WATCH_INDICATOR_AM);
    watch_clear_indicator(WATCH_INDICATOR_K);
    watch_clear_indicator(WATCH_INDICATOR_DIVIDE);
    watch_clear_indicator(WATCH_INDICATOR_TIMES);
    watch_clear_indicator(WATCH_INDICATOR_PLUS);
    watch_clear_indicator(WATCH_INDICATOR_MINUS);
}
