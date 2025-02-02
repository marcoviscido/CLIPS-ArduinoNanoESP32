/*
 * MIT License
 *
 * Copyright (c) 2025 Marco Viscido
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

#ifndef _H_CLIPS_DIGITAL_IO_H

#pragma once

#define _H_CLIPS_DIGITAL_IO_H

#include "Arduino.h"
#include "clips.h"

struct KeyValue
{
    const char *key;
    int value;
};

#if defined(BOARD_HAS_PIN_REMAP) && !defined(BOARD_USES_HW_GPIO_NUMBERS)
/**
 * Inverse Arduino style definitions (API uses Dx)
 */
static KeyValue pinsLookupTable[] = {
    {"D0", 0},
    {"RX", 0},
    {"D1", 1},
    {"TX", 1},
    {"D2", 2},
    {"D3", 3},
    {"CTS", 3},
    {"D4", 4},
    {"DSR", 4},
    {"D5", 5},
    {"D6", 6},
    {"D7", 7},
    {"PIN_I2S_SCK", 7},
    {"D8", 8},
    {"PIN_I2S_FS", 8},
    {"D9", 9},
    {"PIN_I2S_SD", 9},
    {"PIN_I2S_SD_OUT", 9},
    {"PIN_I2S_SD_IN", 10},
    {"D10", 10},
    {"SS", 10},
    {"D11", 11},
    {"MOSI", 11},
    {"D12", 12},
    {"MISO", 12},
    // LED_BUILTIN reserved
    // {"D13", 13},
    // {"SCK", 13},
    // {"LED_BUILTIN", 13},
    {"LED_RED", 14},
    {"LEDR", 14},
    {"LED_GREEN", 15},
    {"LEDG", 15},
    {"RTS", 16},
    {"LED_BLUE", 16},
    {"LEDB", 16},
    {"DTR", 17},
    {"A0", 17},
    {"A1", 18},
    {"A2", 19},
    {"A3", 20},
    {"A4", 21},
    {"SDA", 21},
    {"A5", 22},
    {"SCL", 22},
    {"A6", 23},
    {"A7", 24},
    {"LED_BLUE", 16}
};
#else
/**
 * Inverse ESP32-style pin definitions (when API uses GPIOx)
 */
static KeyValue pinsLookupTable[] = {
    {"D0", 44},
    {"RX", 44},
    {"D1", 43},
    {"TX", 43},
    {"D2", 5},
    {"D3", 6},
    {"CTS", 6},
    {"D4", 7},
    {"DSR", 7},
    {"D5", 8},
    {"D6", 9},
    {"D7", 10},
    {"PIN_I2S_SCK", 10},
    {"D8", 17},
    {"PIN_I2S_FS", 17},
    {"D9", 18},
    {"PIN_I2S_SD", 18},
    {"PIN_I2S_SD_OUT", 18},
    {"PIN_I2S_SD_IN", 21},
    {"D10", 21},
    {"SS", 21},
    {"D11", 38},
    {"MOSI", 38},
    {"D12", 47},
    {"MISO", 47},
    // LED_BUILTIN reserved
    // {"D13", 48},
    // {"SCK", 48},
    // {"LED_BUILTIN", 48},
    {"LED_RED", 46},
    {"LEDR", 46},
    {"LED_GREEN", 0},
    {"LEDG", 0},
    {"RTS", 45},
    {"LED_BLUE", 45},
    {"LEDB", 45},
    {"DTR", 1},
    {"A0", 1},
    {"A1", 2},
    {"A2", 3},
    {"A3", 4},
    {"A4", 11},
    {"SDA", 11},
    {"A5", 12},
    {"SCL", 12},
    {"A6", 13},
    {"A7", 14},
    {"LED_BLUE", 45}
};
#endif

static const int pinsLookupTableSize = sizeof(pinsLookupTable) / sizeof(pinsLookupTable[0]);

void DigitalReadFunction(Environment *, UDFContext *, UDFValue *);
void DigitalWriteFunction(Environment *, UDFContext *, UDFValue *);
void PinModeFunction(Environment *, UDFContext *, UDFValue *);
void PinResetFunction(Environment *, UDFContext *, UDFValue *);
void SyncPinStateFunction(Environment *, void *);

#endif