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

#include <emscripten.h>
#include "watch_adc.h"

void watch_enable_adc(void) {}

void watch_enable_analog_input(const uint16_t pin) {}

uint16_t watch_get_analog_pin_level(const uint16_t pin) {
    return 32767; // pretend it's half of VCC
}

void watch_set_analog_num_samples(uint16_t samples) {}

void watch_set_analog_sampling_length(uint8_t cycles) {}

void watch_set_analog_reference_voltage(uint8_t reference) {}

uint16_t watch_get_vcc_voltage(void) {
    uint16_t battery_voltage;
#if __EMSCRIPTEN__
    battery_voltage = 1000 * EM_ASM_DOUBLE({
        return volt_c || 3;
    });
#else
    battery_voltage = watch_get_vcc_voltage();
#endif
    // TODO: (a2) hook to UI
    return battery_voltage;
}

inline void watch_disable_analog_input(const uint16_t pin) {}

inline void watch_disable_adc(void) {}
