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
        uint8_t com : 2;
        uint8_t seg : 6;
    } address;
    uint8_t value;
} segment_mapping_t;

// Value to indicate that a segment does not exist
static const uint8_t segment_does_not_exist = 0xff;


// Union representing 16 segment mappings, A-H
typedef union weekday_digit_mapping_t {
    //positions 0 and 1
    segment_mapping_t segment[16];
    uint32_t value; //2x16=32
} weekday_digit_mapping_t;

// Union representing 8 segment mappings, A-H
typedef union calculator_digit_mapping_t {
    // positions 2-10
    segment_mapping_t segment[8];
    uint64_t value; //8x8=64
} calculator_digit_mapping_t;

static const uint16_t weekday_character_set[] =
{
    // uppercase is left (pos 0), lowercase is right (pos 1)
    // all alphabetical characters will show as uppercase.
//  0b-----JIH.GFEDCBA
    0b0000000000000000, // [space]
    0b0000000000000110, // !
    0b0000000000100010, // "
    0b0000000001100011, // # (degree symbol, hash mark doesn't fit)
    0b0000000001101101, // $ (S without the center segment)
    0b0000000001010010, // % (unused)
    0b0000000001000100, // & ("lowercase 7" for positions 4 and 6 and as right half of "m")
    0b0000000000100000, // '
    0b0000000000101001, // (
    0b0000000000001011, // )
    0b0000000000100001, // *
    0b0000000001110000, // + (segments E, F and G; looks like ┣╸)
    0b0000000000010000, // ,
    0b0000000001000000, // -
    0b0000000000000000, // .
    0b0000000001010010, // /
    0b0000000000111111, // 0
    0b0000000000000110, // 1
    0b0000000001011011, // 2
    0b0000000001001111, // 3
    0b0000000001100110, // 4
    0b0000000001101101, // 5
    0b0000000001111101, // 6
    0b0000000000000111, // 7
    0b0000000001111111, // 8
    0b0000000001101111, // 9
    0b0000000000001001, // : (unused)
    0b0000000000001101, // ; (unused)
    0b0000000001100001, // <
    0b0000000001001000, // =
    0b0000000001000011, // >
    0b0000000001010011, // ?
    0b1111111111111111, // @ (all segments on)
    0b0000000001110111, // A
    0b0000010101111111, // B (special for weekday)
    0b0000000000111001, // C
    0b0000010100111111, // D (special for weekday)
    0b0000000001111001, // E
    0b0000000001110001, // F
    0b0000000000111101, // G
    0b0000000001110110, // H
    0b0000010100111001, // I (special for weekday)
    0b0000000000011110, // J
    0b0000000001110101, // K
    0b0000000000111000, // L
    0b0000001100110111, // M (special for weekday)
    0b0000000000110111, // N
    0b0000000000111111, // O
    0b0000000001110011, // P
    0b0000000001100111, // Q
    0b0000000000110011, // R (special for weekday)
    0b0000000001101101, // S
    0b0000000100110001, // T (special for weekday)
    0b0000000000111110, // U
    0b0000000000111110, // V
    0b0000011000111110, // W (special for weekday)
    0b0000000001110110, // X
    0b0000000001101110, // Y
    0b0000000001011011, // Z
    0b0000000000111001, // [
    0b0000000001100100, // backslash
    0b0000000000001111, // ]
    0b0000000000100011, // ^
    0b0000000000001000, // _
    0b0000000000000010, // `
    0b0000000001110111, // a
    0b0000000001111100, // b (special for weekday)
    0b0000000000111001, // c
    0b0000000001011110, // d (special for weekday)
    0b0000000001111001, // e
    0b0000000001110001, // f
    0b0000000000111101, // g
    0b0000000001110110, // h
    0b0000000000110000, // i
    0b0000000000011110, // j
    0b0000000001110101, // k
    0b0000000000111000, // l
    0b0000000000010101, // m
    0b0000000000110111, // n
    0b0000000000111111, // o
    0b0000000001110011, // p
    0b0000000001100111, // q
    0b0000000101110111, // r (special for weekday)
    0b0000000001101101, // s
    0b0000000100110001, // t (special for weekday)
    0b0000000000111110, // u
    0b0000000000111110, // v
    0b0000000000101010, // w
    0b0000000001110110, // x
    0b0000000001101110, // y
    0b0000000001011011, // z
    0b0000000000010110, // { (open brace doesn't really work; overriden to represent the two character ligature "il")
    0b0000000000110110, // | (overriden to represent the two character ligature "ll")
    0b0000000000110100, // } (overriden to represent the two character ligature "li")
    0b0000000001000000, // ~
};

static const uint8_t calculator_character_set[] =
{
// TODOEEF: find a way to apply a dot to any character for calculator, OR?
//  0b.GFEDCBA
    0b00000000, // [space]
    0b10000110, // !
    0b00100010, // "
    0b01100011, // # (degree symbol, hash mark doesn't fit)
    0b01101101, // $ (S without the center segment)
    0b11010010, // % (unused)
    0b01000100, // & ("lowercase 7" for positions 4 and 6 and as right half of "m")
    0b00100000, // '
    0b00101001, // (
    0b00001011, // )
    0b00100001, // *
    0b01110000, // + (segments E, F and G; looks like ┣╸)
    0b00010000, // ,
    0b01000000, // -
    0b10000000, // .
    0b01010010, // /
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111, // 9
    0b00001001, // : (unused)
    0b00001101, // ; (unused)
    0b01100001, // <
    0b01001000, // =
    0b01000011, // >
    0b11010011, // ?
    0b11111111, // @ (all segments on)
    0b01011111, // A
    0b01111100, // B
    0b00111001, // C
    0b01011110, // D
    0b01111001, // E
    0b01110001, // F
    0b00111101, // G
    0b01110110, // H
    0b00110000, // I
    0b00011110, // J
    0b01110101, // K
    0b00111000, // L
    0b00010101, // M
    0b00110111, // N
    0b00111111, // O
    0b01110011, // P
    0b01100111, // Q
    0b00110011, // R
    0b01101101, // S
    0b01111000, // T
    0b00111110, // U
    0b00111110, // V
    0b00101010, // W
    0b01110110, // X
    0b01101110, // Y
    0b01011011, // Z
    0b00111001, // [
    0b01100100, // backslash
    0b00001111, // ]
    0b00100011, // ^
    0b00001000, // _
    0b00000010, // `
    0b01011111, // a
    0b01111100, // b
    0b01011000, // c
    0b01011110, // d
    0b01111011, // e
    0b01110001, // f
    0b01101111, // g
    0b01110100, // h
    0b00010000, // i
    0b00001100, // j 
    0b01110101, // k
    0b00110000, // l
    0b00010100, // m
    0b01010100, // n
    0b01011100, // o
    0b01110011, // p
    0b01100111, // q
    0b01010000, // r
    0b01101101, // s
    0b01111000, // t
    0b00011100, // u
    0b00011100, // v
    0b00010100, // w
    0b01110110, // x
    0b01101110, // y
    0b01011011, // z
    0b00010110, // { (open brace doesn't really work; overriden to represent the two character ligature "il")
    0b00110110, // | (overriden to represent the two character ligature "ll")
    0b00110100, // } (overriden to represent the two character ligature "li")
    0b01000000, // ~
};

static const weekday_digit_mapping_t Weekday_Digits_LCD_Mapping[] = {
    // // Positions 0 and 1 are the Weekday or Mode digits
    {
        .segment = {
            { .address = { .com = 0, .seg = 28 } }, // A
            { .address = { .com = 1, .seg = 28 } }, // B
            { .address = { .com = 2, .seg = 27 } }, // C
            { .address = { .com = 2, .seg = 22 } }, // D
            { .address = { .com = 2, .seg = 29 } }, // E
            { .address = { .com = 1, .seg = 29 } }, // F
            { .address = { .com = 2, .seg = 28 } }, // G
            { .value = segment_does_not_exist }, // DOT
            { .address = { .com = 0, .seg = 29 } }, // H
            { .address = { .com = 0, .seg = 30 } }, // I
            { .address = { .com = 2, .seg = 19 } }, // J
            { .value = segment_does_not_exist }, // fill byte
            { .value = segment_does_not_exist }, // fill byte
            { .value = segment_does_not_exist }, // fill byte
            { .value = segment_does_not_exist }, // fill byte
            { .value = segment_does_not_exist }, // fill byte
        },
    },
    {
        .segment = {
            { .address = { .com = 0, .seg = 26 } }, // A
            { .address = { .com = 0, .seg = 25 } }, // B
            { .address = { .com = 1, .seg = 25 } }, // C
            { .address = { .com = 2, .seg = 25 } }, // D
            { .address = { .com = 2, .seg = 26 } }, // E
            { .address = { .com = 1, .seg = 27 } }, // F
            { .address = { .com = 1, .seg = 26 } }, // G
            { .value = segment_does_not_exist }, // DOT
            { .address = { .com = 0, .seg = 27 } }, // H
            { .value = segment_does_not_exist }, // fill byte
            { .value = segment_does_not_exist }, // fill byte
            { .value = segment_does_not_exist }, // fill byte
            { .value = segment_does_not_exist }, // fill byte
            { .value = segment_does_not_exist }, // fill byte
            { .value = segment_does_not_exist }, // fill byte
            { .value = segment_does_not_exist }, // fill byte
        },
    },
};

//TODOEEF: swap seg08 (colon) and seg01 so that the colon can blink
static const calculator_digit_mapping_t Calculator_Digits_LCD_Mapping[] = {
    // Positions 2 and 3 are the hour digits
    {
        .segment = {
            { .address = { .com = 2, .seg = 2 } }, // A
            { .address = { .com = 2, .seg = 3 } }, // B
            { .address = { .com = 1, .seg = 3 } }, // C
            { .address = { .com = 0, .seg = 2 } }, // D
            { .address = { .com = 0, .seg = 8 } }, // E
            { .address = { .com = 1, .seg = 8 } }, // F
            { .address = { .com = 1, .seg = 2 } }, // G
            { .address = { .com = 0, .seg = 3 } }, // DOT
        },
    },
    {
        .segment = {
            { .address = { .com = 2, .seg = 5 } }, // A
            { .address = { .com = 1, .seg = 6 } }, // B
            { .address = { .com = 0, .seg = 6 } }, // C
            { .address = { .com = 0, .seg = 5 } }, // D
            { .address = { .com = 0, .seg = 4 } }, // E
            { .address = { .com = 1, .seg = 4 } }, // F
            { .address = { .com = 1, .seg = 5 } }, // G
            { .address = { .com = 0, .seg = 7 } }, // DOT
        },
    },
    // Positions 4 is located at the hour-minute seperator (:)
    {
        .segment = {
            { .address = { .com = 1, .seg = 30 } }, // A
            { .address = { .com = 2, .seg = 9 } }, // B
            { .address = { .com = 1, .seg = 9 } }, // C
            { .address = { .com = 0, .seg = 1 } }, // D
            { .address = { .com = 1, .seg = 7 } }, // E
            { .address = { .com = 2, .seg = 7 } }, // F
            { .address = { .com = 2, .seg = 1 } }, // G
            { .address = { .com = 0, .seg = 9 } }, // DOT
        },
    },
    // Positions 5 and 6 are the minute digits
    {
        .segment = {
            { .address = { .com = 2, .seg = 11 } }, // A
            { .address = { .com = 2, .seg = 12 } }, // B
            { .address = { .com = 1, .seg = 12 } }, // C
            { .address = { .com = 0, .seg = 11 } }, // D
            { .address = { .com = 0, .seg = 10 } }, // E
            { .address = { .com = 1, .seg = 10 } }, // F
            { .address = { .com = 1, .seg = 11 } }, // G
            { .address = { .com = 0, .seg = 12 } }, // DOT
        },
    },
    {
        .segment = {
            { .address = { .com = 2, .seg = 14 } }, // A
            { .address = { .com = 2, .seg = 15 } }, // B
            { .address = { .com = 1, .seg = 15 } }, // C
            { .address = { .com = 0, .seg = 14 } }, // D
            { .address = { .com = 0, .seg = 13 } }, // E
            { .address = { .com = 1, .seg = 13 } }, // F
            { .address = { .com = 1, .seg = 14 } }, // G
            { .address = { .com = 0, .seg = 15 } }, // DOT
        },
    },
    // Positions 7 is located between minutes and seconds, and it is not used in the watch face.
    {
        .segment = {
            { .address = { .com = 2, .seg = 17 } }, // A
            { .address = { .com = 2, .seg = 18 } }, // B
            { .address = { .com = 1, .seg = 18 } }, // C
            { .address = { .com = 0, .seg = 17 } }, // D
            { .address = { .com = 0, .seg = 16 } }, // E
            { .address = { .com = 1, .seg = 16 } }, // F
            { .address = { .com = 1, .seg = 17 } }, // G
            { .address = { .com = 0, .seg = 18 } }, // DOT
        },
    },
    // Positions 8 and 9 are the seconds digits
    {
        .segment = {
            { .address = { .com = 2, .seg = 20 } }, // A
            { .address = { .com = 2, .seg = 21 } }, // B
            { .address = { .com = 1, .seg = 21 } }, // C
            { .address = { .com = 0, .seg = 20 } }, // D
            { .address = { .com = 0, .seg = 19 } }, // E
            { .address = { .com = 1, .seg = 19 } }, // F
            { .address = { .com = 1, .seg = 20 } }, // G
            { .address = { .com = 0, .seg = 21 } }, // DOT
        },
    },
    {
        .segment = {
            { .address = { .com = 2, .seg = 23 } }, // A
            { .address = { .com = 2, .seg = 24 } }, // B
            { .address = { .com = 1, .seg = 24 } }, // C
            { .address = { .com = 0, .seg = 23 } }, // D
            { .address = { .com = 0, .seg = 22 } }, // E
            { .address = { .com = 1, .seg = 22 } }, // F
            { .address = { .com = 1, .seg = 23 } }, // G
            { .address = { .com = 0, .seg = 24 } }, // DOT
        },
    },
};

void watch_display_character(uint8_t character, uint8_t position);