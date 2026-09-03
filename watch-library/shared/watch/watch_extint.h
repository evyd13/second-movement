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

#pragma once

////< @file watch_extint.h

#include "watch.h"
#include "eic.h"

typedef enum {
    KEYPAD_KEY_NONE = 0,
    KEYPAD_KEY_K0,
    KEYPAD_KEY_K1,
    KEYPAD_KEY_K2,
    KEYPAD_KEY_K3,
    KEYPAD_KEY_K4,
    KEYPAD_KEY_K5,
    KEYPAD_KEY_K6,
    KEYPAD_KEY_K7,
    KEYPAD_KEY_K8,
    KEYPAD_KEY_K9,
    KEYPAD_KEY_DECIMAL,
    KEYPAD_KEY_DIVIDE,
    KEYPAD_KEY_TIMES,
    KEYPAD_KEY_MINUS,
    KEYPAD_KEY_PLUS,
    KEYPAD_KEY_EQUALS,
} movement_keypad_key_t;

#ifndef MOVEMENT_BACKLIGHT_KEY
  #define MOVEMENT_BACKLIGHT_KEY KEYPAD_KEY_PLUS
#endif

typedef enum {
    KEYPAD_ROW_NONE = 0,
    KEYPAD_ROW_0,
    KEYPAD_ROW_1,
    KEYPAD_ROW_2,
    KEYPAD_ROW_3,
} movement_keypad_row_t;

typedef enum {
    KEYPAD_COL_NONE = 0,
    KEYPAD_COL_0,
    KEYPAD_COL_1,
    KEYPAD_COL_2,
    KEYPAD_COL_3,
} movement_keypad_col_t;


/** @addtogroup buttons Buttons & External Interrupts
  * @brief This section covers functions related to the three buttons: Light, Mode and Alarm, as well as
  *        external interrupts from devices on the nine-pin connector.
  * @details The buttons are the core input UI of the watch, and the way the user will interact with
  *          your application. They are active high, pulled down by the microcontroller, and triggered
  *          when one of the "pushers" brings a tab from the metal frame into contact with the edge
  *          of the board. Note that the buttons can only wake the watch from STANDBY mode, at least as
  *          of the current SAM L22 silicon revision. The external interrupt controller runs in STANDBY
  *          mode, but it does not run in BACKUP mode; to wake from BACKUP, buttons will not cut it.
  */
/// @{

/// @brief Enables the external interrupt controller.
void watch_enable_external_interrupts(void);

/// @brief Disables the external interrupt controller.
void watch_disable_external_interrupts(void);

/** @brief Configures an external interrupt callback on one of the external interrupt pins.
  * @details You can set one interrupt callback per pin, and you can monitor for a rising condition,
  *          a falling condition, or both. If you just want to detect a button press, register your
  *          interrupt with INTERRUPT_TRIGGER_RISING; if you want to detect an active-low interrupt
  *          signal from a device on the nine-pin connector, use INTERRUPT_TRIGGER_FALLING. If you
  *          want to detect both rising and falling conditions (i.e. button down and button up), use
  *          INTERRUPT_TRIGGER_BOTH and use watch_get_pin_level to check the pin level in your callback
  *          to determine which condition caused the interrupt.
  * @param pin One of BTN_LIGHT, BTN_MODE, BTN_ALARM, A0, A1, A2, A3 or A4. If the pin parameter matches one of
  *            the three button pins, this function will also enable an internal pull-down resistor. If
  *            the pin parameter is A0-A4, you are responsible for setting any required pull configuration
  *            using watch_enable_pull_up or watch_enable_pull_down.
  * @param callback The function you wish to have called when the button is pressed.
  * @param trigger The condition on which you wish to trigger: rising, falling or both.
  * @note Pins A2 and A4 can also generate interrupts via the watch_register_extwake_callback function, which
  *       will allow them to trigger even when the watch is in deep sleep mode.
  * @warning Pin A2 shares an interrupt channel with the Alarm button; use caution when configuring both.
  */
void watch_register_interrupt_callback(const uint8_t pin, watch_cb_t callback, eic_interrupt_trigger_t trigger);

/// @}
