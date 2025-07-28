/**
 * @file    max7219.cpp
 * @brief   MAX7219 (Serially Interfaced, 8-Digit LED Display Drivers) base library implementation.
 *
 * This file provides the implementation of the Max7219 class for controlling MAX7219 chips.
 *
 * @copyright SPDX-FileCopyrightText: Copyright 2022-2023 Michal Protasowicki
 *
 * @license SPDX-License-Identifier: MIT
 */

#include "max7219.h"

Max7219::Max7219(Max7219NS::context_t &ctx) : _ctx(&ctx)
{
}

Max7219::~Max7219(void)
{
}

bool Max7219::init(void)
{
    bool result {false};

    if (nullptr != _ctx)
    {
        // Ensure at least one device is configured
        if (0 == _ctx->numDevices)
        {
            _ctx->numDevices = 1;
        }
        // Reset activeDevice if chain is busy, marking it ready
        if (isChainBusy())
        {
            _ctx->activeDevice = _ctx->numDevices;
        }

        // Configure IO pins as outputs
        pinMode(_ctx->csbPin, OUTPUT);
        pinMode(_ctx->clkPin, OUTPUT);
        pinMode(_ctx->dataPin, OUTPUT);

        // Set initial pin states
        digitalWrite(_ctx->csbPin, HIGH);
        digitalWrite(_ctx->dataPin, LOW);
        digitalWrite(_ctx->clkPin, LOW);

        // Initialize chips in shutdown mode
        shutdown();

        // Configure decode mode for all chips
        do
        {
            sendCmd(regDecodeMode + (_ctx->decodeBcd ? 0xFF : 0x00));
        } while (isChainBusy());

        // Set initial intensity and scan limit
        setIntensity(_ctx->intensity);
        setScanDigits(_ctx->scanDigits);

        // Clear display and activate chips
        clear();
        activate();

        _ctx->isInitialized = true;
        result = true;
    }

    return result;
}

bool Max7219::init(Max7219NS::context_t &ctx)
{
    _ctx = &ctx;
    return init();
}

bool Max7219::release(void)
{
    bool result {false};

    if (nullptr != _ctx)
    {
        // Set pins low before switching to input to avoid pull-up issues
        digitalWrite(_ctx->csbPin, LOW);
        digitalWrite(_ctx->dataPin, LOW);
        digitalWrite(_ctx->clkPin, LOW);

        // Release pins by setting them to input mode
        pinMode(_ctx->csbPin, INPUT);
        pinMode(_ctx->clkPin, INPUT);
        pinMode(_ctx->dataPin, INPUT);

        _ctx->isInitialized = false;
        result = true;
    }

    return result;
}

void Max7219::setCtx(Max7219NS::context_t &ctx)
{
    _ctx = &ctx;
}

bool Max7219::isInitialized(void)
{
    return (nullptr != _ctx) ? _ctx->isInitialized : false;
}

bool Max7219::isChainBusy(void)
{
    return (nullptr != _ctx) ? (_ctx->activeDevice != _ctx->numDevices) : false;
}

bool Max7219::setIntensity(const uint8_t intensity)
{
    bool result {false};

    if (nullptr != _ctx)
    {
        // Send intensity command to all chips
        do
        {
            sendCmd(regIntensity + (intensity % 0x0F));
        } while (isChainBusy());
        result = true;
    }
    return result;
}

bool Max7219::test(const bool doTest)
{
    bool result {false};

    if (nullptr != _ctx)
    {
        uint16_t testcmd {doTest ? (regDisplayTest | 0x0001) : regDisplayTest};
        do
        {
            sendCmd(testcmd);
        } while (isChainBusy());
        result = true;
    }
    return result;
}

bool Max7219::shutdown(void)
{
    bool result {false};

    if (nullptr != _ctx)
    {
        // Send shutdown command to all chips
        do
        {
            sendCmd(regShutdown);
        } while (isChainBusy());
        result = true;
    }
    return result;
}

bool Max7219::activate(void)
{
    bool result {false};

    if (nullptr != _ctx)
    {
        // Send activate command to all chips
        do
        {
            sendCmd(regShutdown | 0x0001);
        } while (isChainBusy());
        result = true;
    }
    return result;
}

bool Max7219::clear(void)
{
    bool result {false};

    if (nullptr != _ctx)
    {
        // Clear all digit positions
        for (uint8_t position = 0; position < Max7219NS::maxDigits; position++)
        {
            do
            {
                clear(position);
            } while (isChainBusy());
        }
        result = true;
    }
    return result;
}

bool Max7219::clear(const uint8_t position)
{
    // Clear specific position using appropriate value based on decode mode
    return write(position, (_ctx->decodeBcd ? 0x0F : 0x00));
}

bool Max7219::write(const uint8_t position, const uint8_t value)
{
    bool result {false};

    if (nullptr != _ctx)
    {
        // Construct command: position (address) shifted to high byte, value in low byte
        uint16_t cmd {(uint16_t)((position & 0x07) + 1) <<8};
        sendCmd(cmd + value);
        result = true;
    }
    return result;
}

// *****************************************************************
// *                       protected methods                       *
// *****************************************************************

void Max7219::shiftOutByte(uint8_t val)
{
    // Shift out each bit of the byte
    for (uint8_t bit = 0; bit < 8; bit++)
    {
        digitalWrite(_ctx->clkPin, LOW);
        delayMicroseconds(clockDelay);
        digitalWrite(_ctx->dataPin, (val & 0x80) != 0);
        val <<= 1;
        digitalWrite(_ctx->clkPin, HIGH);
        delayMicroseconds(clockDelay);
    }
}

void Max7219::sendCmd(const uint16_t cmd)
{
    // Lower CSB pin for the first command in a sequence
    if (_ctx->activeDevice == _ctx->numDevices)
    {
        digitalWrite(_ctx->csbPin, LOW);
        delayMicroseconds(clockDelay);
    }
    // Send address and data bytes
    shiftOutByte((uint8_t)((cmd >> 8) & 0xFF));
    shiftOutByte((uint8_t)(cmd & 0xFF));

    // Raise CSB pin when done with the last chip
    if (1 == _ctx->activeDevice)
    {
        digitalWrite(_ctx->csbPin, HIGH);
        delayMicroseconds(clockDelay);
        _ctx->activeDevice = _ctx->numDevices;          // Reset for next sequence
    }
    else
    {
        _ctx->activeDevice--;                           // Move to next chip in chain
    }
}

inline void Max7219::setScanDigits(const uint8_t digits)
{
    // Set scan limit for all chips
    do
    {
        sendCmd(regScanLimit + (digits > 0 ? ((digits - 1) & 0x07) : 0x00));
    } while (isChainBusy());
}
