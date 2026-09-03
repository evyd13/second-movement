/*
 * MIT License
 *
 * Copyright (c) 2022-2024 Joey Castillo
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
#include "world_clock_face.h"
#include "watch.h"
#include "watch_utility.h"
#include "watch_common_display.h"
#include "filesystem.h"
#include "zones.h"

static int world_clock_instances;

static inline void button_beep() {
    // play a beep as confirmation for a button press (if applicable)
    if (movement_button_should_sound()) watch_buzzer_play_note_with_volume(BUZZER_NOTE_C7, 50, movement_button_volume());
}

static void persist_world_clock_settings(world_clock_state_t *state) {
    world_clock_settings_t maybe_settings;
    char filename[13];

    maybe_settings.reg = 0xFFFFFFFF;
    sprintf(filename, "wclk_%03d.u32", state->clock_index);

    filesystem_read_file(filename, (char *) &maybe_settings.reg, sizeof(world_clock_settings_t));
    if (state->settings.reg != maybe_settings.reg) {
        filesystem_write_file(filename, (char *) &state->settings.reg, sizeof(world_clock_settings_t));
    }
}

static void _update_timezone_offset(world_clock_state_t *state) {
    state->current_offset = movement_get_current_timezone_offset_for_zone(state->settings.bit.timezone_index);
}

void world_clock_face_setup(uint8_t watch_face_index, void ** context_ptr) {
    (void) watch_face_index;
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(world_clock_state_t));
        memset(*context_ptr, 0, sizeof(world_clock_state_t));
        world_clock_state_t *state = (world_clock_state_t *)*context_ptr;
        state->clock_index = world_clock_instances++;

        // load settings from file if it exists
        char filename[13];
        sprintf(filename, "wclk_%03d.u32", state->clock_index);
        if (filesystem_file_exists(filename)) {
            filesystem_read_file(filename, (char *) &state->settings.reg, sizeof(world_clock_settings_t));
        } else {
            state->settings.bit.timezone_index = UTZ_UTC;
        }
    }
}

void world_clock_face_activate(void *context) {
    world_clock_state_t *state = (world_clock_state_t *)context;

    state->current_screen = 0;
    _update_timezone_offset(state);

    if (watch_sleep_animation_is_running()) {
        watch_stop_sleep_animation();
        watch_stop_blink();
    }
}

static bool world_clock_face_do_display_mode(movement_event_t event, world_clock_state_t *state) {
    char buf[11];

    uint32_t previous_date_time;
    watch_date_time_t date_time;
    switch (event.event_type) {
        case EVENT_ACTIVATE:
            if (movement_clock_mode_24h()) watch_set_indicator(WATCH_INDICATOR_AM);
            watch_set_colon();
            state->previous_date_time = 0xFFFFFFFF;
            // fall through
        case EVENT_TICK:
        case EVENT_LOW_ENERGY_UPDATE:
            date_time = movement_get_date_time_in_zone(state->settings.bit.timezone_index);
            previous_date_time = state->previous_date_time;
            state->previous_date_time = date_time.reg;
            if ((date_time.reg >> 6) == (previous_date_time >> 6) && event.event_type != EVENT_LOW_ENERGY_UPDATE) {
                // everything before seconds is the same, don't waste cycles setting those segments.
                watch_display_character('0' + date_time.unit.second / 10, 8);
                watch_display_character('0' + date_time.unit.second % 10, 9);
                break;
            } else if ((date_time.reg >> 12) == (previous_date_time >> 12) && event.event_type != EVENT_LOW_ENERGY_UPDATE) {
                // everything before minutes is the same.
                sprintf(buf, "%02d%02d", date_time.unit.minute, date_time.unit.second);
                watch_display_text(WATCH_POSITION_MINUTES, buf);
                watch_display_text(WATCH_POSITION_SECONDS, buf + 2);
                if (date_time.unit.minute % 15 == 0) {
                    _update_timezone_offset(state);
                }
            } else {
                // other stuff changed; let's do it all.
                if (!movement_clock_mode_24h()) {
                    // if we are in 12 hour mode, do some cleanup.
                    if (date_time.unit.hour < 12) {
                        watch_clear_indicator(WATCH_INDICATOR_PM);
                    } else {
                        watch_set_indicator(WATCH_INDICATOR_PM);
                    }
                    date_time.unit.hour %= 12;
                    if (date_time.unit.hour == 0) date_time.unit.hour = 12;
                }
                sprintf(buf, "%2d%02d%02d", date_time.unit.hour, date_time.unit.minute, date_time.unit.second);
                watch_display_text(WATCH_POSITION_TOP, "DT");
                watch_display_text(WATCH_POSITION_HOURS, buf);
                watch_display_text(WATCH_POSITION_MINUTES, buf + 2);
                if (event.event_type == EVENT_LOW_ENERGY_UPDATE) {
                    if (!watch_sleep_animation_is_running()) {
                        watch_display_text(WATCH_POSITION_SECONDS, "  ");
                        watch_start_sleep_animation(500);
                        watch_start_indicator_blink_if_possible(WATCH_INDICATOR_COLON, 500);
                    }
                } else {
                    watch_display_text(WATCH_POSITION_SECONDS, buf + 4);
                }
            }
            break;
        case EVENT_ADJUST_BUTTON_DOWN:
            movement_request_tick_frequency(4);
            state->current_screen = 1;
            break;
        default:
            return movement_default_loop_handler(event);
    }

    return true;
}

static bool _world_clock_face_do_settings_mode(movement_event_t event, world_clock_state_t *state) {

    switch (event.event_type) {
        case EVENT_MODE_BUTTON_DOWN:
            if (state->current_screen == 1) {
                state->settings.bit.timezone_index++;
                if (state->settings.bit.timezone_index >= NUM_ZONE_NAMES) state->settings.bit.timezone_index = 0;
                button_beep();
            }
            break;
        case EVENT_ADJUST_BUTTON_DOWN:
            state->current_screen++;
            if (state->current_screen > 1) {
                movement_request_tick_frequency(1);
                _update_timezone_offset(state);
                state->current_screen = 0;
                persist_world_clock_settings(state);
                watch_clear_display();
                event.event_type = EVENT_ACTIVATE;
                return world_clock_face_do_display_mode(event, state);
            }
            button_beep();
            break;
        case EVENT_TIMEOUT:
            persist_world_clock_settings(state);
            movement_move_to_face(0);
            break;
        default:
            break;
    }

    char buf[9];

    watch_clear_colon();
    sprintf(buf, "%s  ", watch_utility_time_zone_name_at_index(state->settings.bit.timezone_index));
    watch_clear_indicator(WATCH_INDICATOR_PM);

    // blink up the parameter we're setting
    if (event.subsecond % 2) {
        switch (state->current_screen) {
            case 1:
                memcpy(buf, "        ", 8);
                break;
        }
    }

    watch_display_text(WATCH_POSITION_BOTTOM, buf);

    return true;
}

bool world_clock_face_loop(movement_event_t event, void *context) {
    world_clock_state_t *state = (world_clock_state_t *)context;

    if (state->current_screen == 0) {
        return world_clock_face_do_display_mode(event, state);
    } else {
        return _world_clock_face_do_settings_mode(event, state);
    }
}

void world_clock_face_resign(void *context) {
    (void) context;
}
