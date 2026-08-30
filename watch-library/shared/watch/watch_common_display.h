/*
 * MIT License
 *
 * Copyright (c) 2020-2024 Joey Castillo
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

// Union representing a single segment mapping
// COM occupies two bits, SEG occupes the rest.
typedef union segment_mapping_t {
    struct {
        uint8_t com : 3;
        uint8_t seg : 5;
    } address;
    uint16_t value;
} segment_mapping_t;

// Value to indicate that a segment does not exist
static const uint16_t segment_does_not_exist = 0xffff;

// Union representing 16 segment mappings, A-J
typedef union digit_mapping_t {
    segment_mapping_t segment[16];
    uint64_t value;
} digit_mapping_t;


// Original CA-53W Casio LCD
static const uint16_t Classic_LCD_Character_Set[] =
{
// TODOEEF: find a way to apply a dot to any character for calculator, OR?
//  0bABCDEFG.HIJ-----
    0b0000000000000000, // [space]
    0b0110000100000000, // !
    0b0100010000000000, // "
    0b1100011000000000, // # (degree symbol, hash mark doesn't fit)
    0b1011011000000000, // $ (S without the center segment)
    0b0100101100000000, // % (unused)
    0b0010001000000000, // & ("lowercase 7" for positions 4 and 6 and as right half of "m")
    0b0000010000000000, // '
    0b1001010000000000, // (
    0b1101000000000000, // )
    0b1000010000000000, // *
    0b0000111000000000, // + (segments E, F and G; looks like ┣╸)
    0b0000100000000000, // ,
    0b0000001000000000, // -
    0b0000000100000000, // .
    0b0100101000000000, // /
    0b1111110000000000, // 0
    0b0110000000000000, // 1
    0b1101101000000000, // 2
    0b1111001000000000, // 3
    0b0110010000000000, // 4
    0b1011011000000000, // 5
    0b1011111000000000, // 6
    0b1110000000000000, // 7
    0b1111111000000000, // 8
    0b1111011000000000, // 9
    0b1001000000000000, // : (unused)
    0b1011000000000000, // ; (unused)
    0b1000011000000000, // <
    0b0001001000000000, // =
    0b1100001000000000, // >
    0b1100101100000000, // ?
    0b1111111111111111, // @ (all segments on)
    0b1111101000000000, // A
    0b0011111000000000, // B
    0b1001110000000000, // C
    0b1111110010100000, // D (only works in position 0)
    0b1001111000000000, // E
    0b1000111000000000, // F
    0b1011110000000000, // G
    0b0110111000000000, // H
    0b1001110010100000, // I (only works in position 0)
    0b0111100000000000, // J
    0b1010111000000000, // K
    0b0001110000000000, // L
    0b1110110001100000, // M (only works in position 0)
    0b1110110000000000, // N
    0b1111110000000000, // O
    0b1100111000000000, // P
    0b1101011000000000, // Q
    0b1110111010000000, // R (only works in position 1)
    0b1011011000000000, // S
    0b1000110000100000, // T (only works in position 0)
    0b0111110000000000, // U
    0b0111110000000000, // V
    0b0111110011000000, // W (only works in position 0)
    0b0110111000000000, // X
    0b0111011000000000, // Y
    0b1101101000000000, // Z
    0b1001110000000000, // [
    0b0010011000000000, // backslash
    0b1111000000000000, // ]
    0b1100010000000000, // ^
    0b0001000000000000, // _
    0b0100000000000000, // `
    0b1111101000000000, // a
    0b0011111000000000, // b
    0b0001101000000000, // c
    0b0111101000000000, // d
    0b1101111000000000, // e
    0b1000111000000000, // f
    0b1111011000000000, // g
    0b0010111000000000, // h
    0b0000100000000000, // i
    0b0011000000000000, // j 
    0b1010111000000000, // k
    0b0000110000000000, // l
    0b1110110001100000, // m (only works in position 0)
    0b0010101000000000, // n
    0b0011101000000000, // o
    0b1100111000000000, // p
    0b1110011000000000, // q
    0b0000101000000000, // r
    0b1011011000000000, // s
    0b0001111000000000, // t
    0b0100011000000000, // u
    0b0100011000000000, // v
    0b0111110011000000, // w (only works in position 0)
    0b0110111000000000, // x
    0b0111011000000000, // y
    0b1101101000000000, // z
    0b0110100000000000, // { (open brace doesn't really work; overriden to represent the two character ligature "il")
    0b0110110000000000, // | (overriden to represent the two character ligature "ll")
    0b0010110000000000, // } (overriden to represent the two character ligature "li")
    0b0000001000000000, // ~
};

static const digit_mapping_t Classic_LCD_Display_Mapping[] = {
    // Positions 0 and 1 are the Weekday or Mode digits
    {
        .segment = {
            { .address = { .com = 1, .seg = 29 } }, // A
            { .address = { .com = 2, .seg = 29 } }, // B
            { .address = { .com = 3, .seg = 28 } }, // C
            { .address = { .com = 3, .seg = 23 } }, // D
            { .address = { .com = 3, .seg = 30 } }, // E
            { .address = { .com = 2, .seg = 30 } }, // F
            { .address = { .com = 3, .seg = 29 } }, // G
            { .value = segment_does_not_exist }, // DOT
            { .address = { .com = 3, .seg = 20 } }, // H
            { .address = { .com = 1, .seg = 31 } }, // I
            { .address = { .com = 1, .seg = 30 } }, // J
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
        },
    },
    {
        .segment = {
            { .address = { .com = 1, .seg = 27 } }, // A
            { .address = { .com = 1, .seg = 26 } }, // B
            { .address = { .com = 2, .seg = 26 } }, // C
            { .address = { .com = 3, .seg = 26 } }, // D
            { .address = { .com = 3, .seg = 27 } }, // E
            { .address = { .com = 2, .seg = 28 } }, // F
            { .address = { .com = 2, .seg = 27 } }, // G
            { .value = segment_does_not_exist }, // DOT
            { .address = { .com = 1, .seg = 28 } }, // H
            { .value = segment_does_not_exist }, // I
            { .value = segment_does_not_exist }, // J
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
        },
    },
    // Positions 2 and 3 are the hour digits
    {
        .segment = {
            { .address = { .com = 3, .seg = 3 } }, // A
            { .address = { .com = 3, .seg = 4 } }, // B
            { .address = { .com = 2, .seg = 4 } }, // C
            { .address = { .com = 1, .seg = 3 } }, // D
            { .address = { .com = 1, .seg = 2 } }, // E
            { .address = { .com = 2, .seg = 2 } }, // F
            { .address = { .com = 2, .seg = 3 } }, // G
            { .address = { .com = 1, .seg = 4 } }, // DOT
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
        },
    },
    {
        .segment = {
            { .address = { .com = 3, .seg = 6 } }, // A
            { .address = { .com = 2, .seg = 7 } }, // B
            { .address = { .com = 1, .seg = 7 } }, // C
            { .address = { .com = 1, .seg = 6 } }, // D
            { .address = { .com = 1, .seg = 5 } }, // E
            { .address = { .com = 2, .seg = 5 } }, // F
            { .address = { .com = 2, .seg = 6 } }, // G
            { .address = { .com = 1, .seg = 8 } }, // DOT
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
        },
    },
    // Positions 4 is located at the hour-minute seperator (:)
    {
        .segment = {
            { .address = { .com = 2, .seg = 31 } }, // A
            { .address = { .com = 3, .seg = 10 } }, // B
            { .address = { .com = 2, .seg = 10 } }, // C
            { .address = { .com = 1, .seg = 9 } }, // D
            { .address = { .com = 2, .seg = 8 } }, // E
            { .address = { .com = 3, .seg = 8 } }, // F
            { .address = { .com = 3, .seg = 9 } }, // G
            { .address = { .com = 1, .seg = 10 } }, // DOT
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
        },
    },
    // Positions 5 and 6 are the minute digits
    {
        .segment = {
            { .address = { .com = 3, .seg = 12 } }, // A
            { .address = { .com = 3, .seg = 13 } }, // B
            { .address = { .com = 2, .seg = 13 } }, // C
            { .address = { .com = 1, .seg = 12 } }, // D
            { .address = { .com = 1, .seg = 11 } }, // E
            { .address = { .com = 2, .seg = 11 } }, // F
            { .address = { .com = 2, .seg = 12 } }, // G
            { .address = { .com = 1, .seg = 13 } }, // DOT
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
        },
    },
    {
        .segment = {
            { .address = { .com = 3, .seg = 15 } }, // A
            { .address = { .com = 3, .seg = 16 } }, // B
            { .address = { .com = 2, .seg = 16 } }, // C
            { .address = { .com = 1, .seg = 15 } }, // D
            { .address = { .com = 1, .seg = 14 } }, // E
            { .address = { .com = 2, .seg = 14 } }, // F
            { .address = { .com = 2, .seg = 15 } }, // G
            { .address = { .com = 1, .seg = 16 } }, // DOT
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
        },
    },
    // Positions 7 is located between minutes and seconds, and it is not used in the watch face.
    {
        .segment = {
            { .address = { .com = 3, .seg = 18 } }, // A
            { .address = { .com = 3, .seg = 19 } }, // B
            { .address = { .com = 2, .seg = 19 } }, // C
            { .address = { .com = 1, .seg = 18 } }, // D
            { .address = { .com = 1, .seg = 17 } }, // E
            { .address = { .com = 2, .seg = 17 } }, // F
            { .address = { .com = 2, .seg = 18 } }, // G
            { .address = { .com = 1, .seg = 19 } }, // DOT
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
        },
    },
    // Positions 8 and 9 are the seconds digits
    {
        .segment = {
            { .address = { .com = 3, .seg = 21 } }, // A
            { .address = { .com = 3, .seg = 22 } }, // B
            { .address = { .com = 2, .seg = 22 } }, // C
            { .address = { .com = 1, .seg = 21 } }, // D
            { .address = { .com = 1, .seg = 20 } }, // E
            { .address = { .com = 2, .seg = 20 } }, // F
            { .address = { .com = 2, .seg = 21 } }, // G
            { .address = { .com = 1, .seg = 22 } }, // DOT
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
        },
    },
    {
        .segment = {
            { .address = { .com = 3, .seg = 24 } }, // A
            { .address = { .com = 3, .seg = 25 } }, // B
            { .address = { .com = 2, .seg = 25 } }, // C
            { .address = { .com = 1, .seg = 24 } }, // D
            { .address = { .com = 1, .seg = 23 } }, // E
            { .address = { .com = 2, .seg = 23 } }, // F
            { .address = { .com = 2, .seg = 24 } }, // G
            { .address = { .com = 1, .seg = 25 } }, // DOT
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
            { .value = segment_does_not_exist },    // fill byte
        },
    },
};

void watch_display_character(uint8_t character, uint8_t position);
void watch_display_character_lp_seconds(uint8_t character, uint8_t position);

