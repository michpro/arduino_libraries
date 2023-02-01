/**
 * \file    mcp402x.cpp
 * \brief   Library to drive MCP402X devices - non-volatile,
 *          6-bit (64 wiper steps) digital potentiometers
 *          with a simple up/down serial interface.
 *
 * \copyright SPDX-FileCopyrightText: Copyright 2022-2023 Michal Protasowicki
 *
 * \license SPDX-License-Identifier: MIT
 *
 */

#include "mcp402x.h"

Mcp402x::Mcp402x(Mcp402xNS::context_t &ctx) : _ctx(&ctx)
{
}

Mcp402x::~Mcp402x(void)
{
}

bool Mcp402x::init(void)
{
    bool result {false};

    if (nullptr != _ctx)
    {
        pinMode(_ctx->csPin, OUTPUT);
        pinMode(_ctx->udPin, OUTPUT);
        digitalWrite(_ctx->csPin, HIGH);
        digitalWrite(_ctx->udPin, HIGH);
        _ctx->currentValue = 0x00;
        _ctx->isInitialized = true;
        result = true;
    }

    return result;
}

bool Mcp402x::init(Mcp402xNS::context_t &ctx)
{
    setCtx(ctx);

    return init();
}

void Mcp402x::setCtx(Mcp402xNS::context_t &ctx)
{
    _ctx = &ctx;
}

bool Mcp402x::isInitialized(void)
{
    bool result {false};

    if (nullptr != _ctx)
    {
        result = _ctx->isInitialized;
    }

    return result;
}

bool Mcp402x::updateWiperValue(const uint8_t value)
{
    bool result {false};

    if ((nullptr != _ctx) && _ctx->isInitialized && (value < Mcp402xNS::maxValue))
    {
        _ctx->currentValue = value;
        result = true;
    }

    return result;
}

bool Mcp402x::up(void)
{
    bool result {false};

    if ((nullptr != _ctx) && _ctx->isInitialized && (_ctx->currentValue < Mcp402xNS::maxValue))
    {
        pulse(1, UP, keepNV_t::NO);
        _ctx->currentValue++;
        result= true;
    }

    return result;
}

bool Mcp402x::down(void)
{
    bool result {false};

    if ((nullptr != _ctx) && _ctx->isInitialized && (_ctx->currentValue > Mcp402xNS::minValue))
    {
        pulse(1, DOWN, keepNV_t::NO);
        _ctx->currentValue--;
        result= true;
    }

    return result;
}

bool Mcp402x::set(uint8_t value)
{
    bool result {false};

    if ((nullptr != _ctx) && _ctx->isInitialized)
    {
        uint8_t     diff    {0};
        direction_t dir     {UP};

        if (value > Mcp402xNS::maxValue)
        {
            value = Mcp402xNS::maxValue;
        }

        if (value > _ctx->currentValue)
        {
            diff = value - _ctx->currentValue;
        }
        else if (value < _ctx->currentValue)
        {
            diff = _ctx->currentValue - value;
            dir = DOWN;
        }

        if (diff != 0)
        {
            pulse(diff, dir, keepNV_t::NO);
            _ctx->currentValue = value;
            result = true;
        }
    }

    return result;
}

uint8_t Mcp402x::get(void)
{ 
    return (nullptr != _ctx) ? _ctx->currentValue : Mcp402xNS::minValue;
}

uint8_t Mcp402x::keepNonVolatile(void)
{
    if ((nullptr != _ctx) && _ctx->isInitialized)
    {
        pulse(1, UP, keepNV_t::YES);
    }

    return get();
}

// *****************************************************************
// *                                                               *
// *                       protected methods                       *
// *                                                               *
// *****************************************************************

void Mcp402x::pulse(const uint8_t pulses, const direction_t dir, const keepNV_t nonVolatile)
{
    digitalWrite(_ctx->udPin, ((DOWN == dir) ? LOW : HIGH));
    delayMicroseconds(minCsTime);
    digitalWrite(_ctx->csPin, LOW);

    for (uint8_t pulse = 0; pulse < pulses; pulse++)
    {
        if (UP == dir)
        {
            delayMicroseconds(pulseDelay);
            digitalWrite(_ctx->udPin, LOW);
        }
        if (NO == nonVolatile)
        {
            delayMicroseconds(pulseDelay);
            digitalWrite(_ctx->udPin, HIGH);
        }
        if (DOWN == dir)
        {
            delayMicroseconds(pulseDelay);
            digitalWrite(_ctx->udPin, LOW);
        }
    }

    delayMicroseconds(minCsTime);
    digitalWrite(_ctx->csPin, HIGH);
}
