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

////< @file watch_slcd.h

#include "watch.h"

/** @addtogroup slcd Segment LCD Display
  * @brief This section covers functions related to the Segment LCD display driver, which is responsible
  *        for displaying strings of characters and indicators on the main watch display.
  * @details The segment LCD controller consumes about 3 microamperes of power with no segments on, and
  *          about 4 microamperes with all segments on. There is also a slight power impact associated
  *          with updating the screen (about 1 microampere to update at 1 Hz). For the absolute lowest
  *          power operation, update the display only when its contents have changed, and disable the
  *          SLCD peripheral when the screen is not in use.
  *          For a map of all common and segment pins, see <a href="segmap.html">segmap.html</a>. You can
  *          hover over any segment in that diagram to view the common and segment pins associated with
  *          each segment of the display.
  */
/// @{

#define SLCD_SEGID(com, seg) (((com) << 5) | (seg))
#define SLCD_COMNUM(segid) (((segid) >> 5) & 0x07)
#define SLCD_SEGNUM(segid) ((segid) & 0x1F)

/// An enum listing the icons and indicators available on the watch.
typedef enum {
    WATCH_INDICATOR_SIGNAL = 0, ///< The signal indicator indicating alarm is set; also useful for indicating that sensors are on.
    WATCH_INDICATOR_BELL,       ///< The small bell indicating hourly chime is on.
    WATCH_INDICATOR_PM,         ///< The PM indicator, indicating that a time is in the afternoon.
    WATCH_INDICATOR_AM,         ///< The PM indicator, indicating that a time is before noon.
    // Calculator indicators
    WATCH_INDICATOR_K,          ///< The constant indicator, indicating that the calculator is in constant mode.
    WATCH_INDICATOR_DIVIDE,
    WATCH_INDICATOR_TIMES,
    WATCH_INDICATOR_MINUS,
    WATCH_INDICATOR_PLUS,

    // You can generally address the colon using dedicated functions, but it's also available here if needed.
    WATCH_INDICATOR_COLON,      ///< The colon between hours and minutes.
} watch_indicator_t;

/// An enum listing the locations on the display where text can be placed.
typedef enum {
    WATCH_POSITION_FULL = 0,    ///< Display 10 characters to the full screen, in the standard F-91W layout.
    WATCH_POSITION_TOP,         ///< Display 2 characters at the top of the screen.
    WATCH_POSITION_BOTTOM,      ///< Display 8 characters at the bottom of the screen, the main line.
    WATCH_POSITION_HOURS,       ///< Display 2 characters in the hours portion of the main line.
    WATCH_POSITION_MINUTES,     ///< Display 2 characters in the minutes portion of the main line.
    WATCH_POSITION_SECONDS,     ///< Display 2 characters in the seconds portion of the main line.
} watch_position_t;


/** @brief Enables the Segment LCD display.
  * Call this before attempting to set pixels or display strings.
  */
void watch_enable_display(void);

/** @brief Disables the Segment LCD display.
  */
void watch_disable_display(void);

/** @brief Sets a pixel. Use this to manually set a pixel with a given common and segment number.
  *        See <a href="segmap.html">segmap.html</a>.
  * @param com the common pin, numbered from 0-2.
  * @param seg the segment pin, numbered from 0-30.
  */
void watch_set_pixel(uint8_t com, uint8_t seg);

/** @brief Clears a pixel. Use this to manually clear a pixel with a given common and segment number.
  *        See <a href="segmap.html">segmap.html</a>.
  * @param com the common pin, numbered from 0-2.
  * @param seg the segment pin, numbered from 0-30.
  */
void watch_clear_pixel(uint8_t com, uint8_t seg);

/** @brief Clears all segments of the display, including incicators and the colon.
  */
void watch_clear_display(void);

/** @brief Displays a string at the given position, starting from the top left. There are ten digits.
           A space in any position will clear that digit.
  * @deprecated Use `watch_display_text` instead.
  * @param string A null-terminated string.
  * @param position The position where you wish to start displaying the string. The day of week digits
  *                 are positions 0 and 1; the day of month digits are positions 2 and 3, and the main
  *                 clock line occupies positions 4-9.
  * @note This method does not clear the display; if for example you display a two-character string at
          position 0, positions 2-9 will retain whatever state they were previously displaying.
  */
void watch_display_string(const char *string, uint8_t position) __attribute__ ((deprecated("Use watch_display_text instead.")));

/**
 * @brief Displays a string at the provided location.
 * @param location @see watch_position_t, the location where you wish to display the string.
 * @param string A null-terminated string with two characters to display.
 */
void watch_display_text(watch_position_t location, const char *string);


/**
 * @brief Displays a floating point number as best we can on whatever LCD is available.
 * @details The custom LCD can energize a decimal point in the same position as the colon. With the leading 1,
 *          we can display numbers from -99.99 to 199.99 with one or two digits of precision, depending on the
 *          number and the LCD in use (classic LCD, usually 1 digit; custom LCD, usually 2). The original F-91W
 *          LCD has no decimal point, so we use a hyphen or underscore instead. For numbers <9.99, the decimal
 *          point will appear as an underscore on the custom LCD as well, since the position doesn't line up.
 *          Also note that sometimes, to offer at least one digit of precision, the second character of your
 *          units may be truncated (i.e. -12.4#F will become -12.4#).
 * @param value A floating point number from -99.99 to 199.99 to display on the main line of the display.
 * @param units A 1-2 character string to display in the seconds position. Second character may be truncated.
 */
void watch_display_float_with_best_effort(float value, const char *units);

/** @brief Turns the colon segment on.
  */
void watch_set_colon(void);

/** @brief Turns the colon segment off.
  */
void watch_clear_colon(void);
void watch_set_indicator(watch_indicator_t indicator);

/** @brief Clears an indicator on the LCD. Use this to turn off one of the indicator segments.
  * @param indicator One of the indicator segments from the enum. @see watch_indicator_t
  */
void watch_clear_indicator(watch_indicator_t indicator);

/** @brief Clears all indicator segments.
  * @see watch_indicator_t
  */
void watch_clear_all_indicators(void);

/** @brief Blinks a single character in position 7. Does not affect other positions.
  * @details Six of the seven segments in position 7 (and only position 7) are capable of autonomous
  *          blinking. This blinking does not require any CPU resources, and will continue even in
  *          STANDBY and Sleep mode (but not Deep Sleep mode, since that mode turns off the LCD).
  * @param character The character you wish to blink.
  * @param duration The duration of the on/off cycle in milliseconds, from 50 to ~4250 ms.
  * @note Segment B of position 7 cannot blink autonomously, so not all characters will work well.
  *       Supported characters for blinking on classic LCD :
  *        * Punctuation: underscore, apostrophe, comma, hyphen, equals sign, tilde (top segment only)
  *        * Numbers: 5, 6, ampersand (lowercase 7)
  *        * Letters: b, C, c, E, F, h, i, L, l, n, o, S, t
  *       Supported characters for blinking on custom LCD :
  *        * Only the segments making up the capital letter 'C' (ADEF)
  */
void watch_start_character_blink(char character, uint32_t duration);

/** @brief Blinks an indicator on the custom LCD.
  * @details You can blink the LAP, ARROWS, SLEEP and COLON indicators on the custom LCD.
  *          Alas you cannot autonomously blink any indicators on the original F-91W LCD.
  * @param indicator The indicator you wish to blink.
  * @param duration The duration of the on/off cycle in milliseconds, from 50 to ~4250 ms.
  */
void watch_start_indicator_blink_if_possible(watch_indicator_t indicator, uint32_t duration);

/** @brief Stops and clears all blinking segments.
  * @details This will stop all blinking in position 7, and clear all segments in that digit.
  *          On the Pro LCD, this will also stop the blinking of all indicators.
  */
void watch_stop_blink(void);

/** @brief Begins a two-segment "tick-tock" animation in position 8 on the classic LCD, or energizes
 *         the crescent moon icon on the custom LCD to indicate a sleep mode.
  * @details On the classic Casio LCD, six of the seven segments in position 8 can animate themselves
  *          autonomously. This animation is very basic, and consists of moving a bit pattern forward
  *          or backward in a shift register whose positions map to fixed segments on the LCD. Given
  *          this constraint, an animation across all six segments does not make sense; so the watch
  *          library offers only a simple "tick/tock" in segments D and E. This animation does not
  *          require any CPU resources, and will continue even in STANDBY and Sleep mode (but not Deep
  *          Sleep mode, since that mode turns off the LCD).
  * @param duration The duration of each frame in ms. 500 milliseconds produces a classic tick/tock.
  * @note On the custom LCD, this function illuminates the crescent moon icon with no animation. The
  *       duration parameter is unused and ignored.
  */
void watch_start_sleep_animation(uint32_t duration);

/** @brief Checks if the sleep state is currently shown
  * @return true if the animation is running on classic, or the sleep indicator is energized on custom.
  *         false otherwise.
  */
bool watch_sleep_animation_is_running(void);

/** @brief Stops the tick/tock animation and clears all animating segments.
  * @details On the classic LCD this will stop the animation and clear all segments in position 8.
  *          On the custom LCD, it will turn off the crescent moon indicator.
  */
void watch_stop_sleep_animation(void);
/// @}
