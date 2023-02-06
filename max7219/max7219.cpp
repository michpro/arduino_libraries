/**
 * \file max7219.cpp
 * \brief MAX7219 (Serially Interfaced, 8-Digit LED Display Drivers) base library.
 *
 * \copyright SPDX-FileCopyrightText: Copyright 2022-2023 Michal Protasowicki
 *
 * \license SPDX-License-Identifier: MIT
 *
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
        if (0 == _ctx->numDevices)
        {
            _ctx->numDevices = 1;
        }
        if (isChainBusy())
        {
            _ctx->activeDevice = _ctx->numDevices;                      // now chain is ready for operation
        }

        pinMode(_ctx->csbPin, OUTPUT);
        pinMode(_ctx->clkPin, OUTPUT);
        pinMode(_ctx->dataPin, OUTPUT);

        digitalWrite(_ctx->csbPin, HIGH);
        digitalWrite(_ctx->dataPin, LOW);
        digitalWrite(_ctx->clkPin, LOW);

        shutdown();
        do
        {
            sendCmd(regDecodeMode + (_ctx->decodeBcd ? 0xFF : 0x00));   // set decode mode
        } while (isChainBusy());

        setIntensity(_ctx->intensity);
        setScanDigits(_ctx->scanDigits);
        clear();
        activate();

        _ctx->isInitialized = true;
        result = true;
    };

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
        digitalWrite(_ctx->csbPin, LOW);                            // prevents the pull-up resistor from turning on
                                                                    // after pin is configured as an input
        digitalWrite(_ctx->dataPin, LOW);
        digitalWrite(_ctx->clkPin, LOW);

        pinMode(_ctx->csbPin, INPUT);
        pinMode(_ctx->clkPin, INPUT);
        pinMode(_ctx->dataPin, INPUT);

        _ctx->isInitialized = false;
        result = true;
    };

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
        do
        {
            sendCmd(regShutdown | 0x0001);                          // set normal operation flag in the shutdown register
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
    return write(position, (_ctx->decodeBcd ? 0x0F : 0x00));
}

bool Max7219::write(const uint8_t position, const uint8_t value)
{
    bool result {false};

    if (nullptr != _ctx)
    {
        uint16_t cmd {(uint16_t)((position & 0x07) + 1) << 8};      // wrap around position and digit 0 is at address 1

        sendCmd(cmd + value);

        result = true;
    }

    return result;
}

// *****************************************************************
// *                                                               *
// *                       protected methods                       *
// *                                                               *
// *****************************************************************

void Max7219::shiftOutByte(uint8_t val)
{
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
    if (_ctx->activeDevice == _ctx->numDevices)
    {
        digitalWrite(_ctx->csbPin, LOW);
        delayMicroseconds(clockDelay);
    }
    shiftOutByte((uint8_t)((cmd >> 8) & 0xFF));
    shiftOutByte((uint8_t)(cmd & 0xFF));
    
    if (1 == _ctx->activeDevice)
    {
        digitalWrite(_ctx->csbPin, HIGH);
        delayMicroseconds(clockDelay);
        _ctx->activeDevice = _ctx->numDevices;
    } else
    {
        _ctx->activeDevice--;
    }
}

inline void Max7219::setScanDigits(const uint8_t digits)
{
    do
    {
        sendCmd(regScanLimit + (digits > 0 ? ((digits - 1) & 0x07) : 0x00));
    } while (isChainBusy());
}
