/**
 * \file max7219.cpp
 * \brief MAX7219 (Serially Interfaced, 8-Digit LED Display Drivers) base library.
 *
 * \copyright SPDX-FileCopyrightText: Copyright 2022 Michal Protasowicki
 *
 * \license SPDX-License-Identifier: MIT
 *
 */

#include "max7219.h"

/**
 *  \brief Max7219 class constructor.
 * 
 *  \param ctx a reference to structure containing settings for chip(s) chain
**/
Max7219::Max7219(Max7219Base::context &ctx) : _ctx(&ctx)
{
}

/**
 *  \brief Max7219 class destructor.
**/
Max7219::~Max7219(void)
{
}

/**
 *  \brief Method that initializes MAX7219 chip(s) chain based on data from the 'ctx' structure.
**/
void Max7219::init(void)
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
}

/**
 *  \brief Method that initializes MAX7219 chip(s) chain based on data from passed 'ctx' structure.
 *
 *  \param ctx a reference to structure containing settings for chip(s) chain
**/
void Max7219::init(Max7219Base::context &ctx)
{
    _ctx = &ctx;
    init();
}

/**
 *  \brief  A method of releasing IO pins indicated by the current context
 *          used to communicate with MAX7219 chip(s) chain. 
**/
void Max7219::release(void)
{
    digitalWrite(_ctx->csbPin, LOW);                                // prevents the pull-up resistor from turning on
                                                                    // after pin is configured as an input
    digitalWrite(_ctx->dataPin, LOW);
    digitalWrite(_ctx->clkPin, LOW);

    pinMode(_ctx->csbPin, INPUT);
    pinMode(_ctx->clkPin, INPUT);
    pinMode(_ctx->dataPin, INPUT);

    _ctx->isInitialized = false;
}

/**
 *  \brief  A method that sets a reference to a new context. It allows you
 *          to handle many MAX7219 chip(s) chains connected to
 *          different IO pins with use of one instance of the class. 
 *
 *  \param ctx a reference to structure containing settings for chip
**/
void Max7219::setCtx(Max7219Base::context &ctx)
{
    _ctx = &ctx;
}

/**
 *  \brief  A method that returns initialization state of the MAX7219 chip(s) chain,
 *          for the current context. 
 *
 *  \return true when initialized, false otherwise
**/
boolean Max7219::isInitialized(void)
{
    return _ctx->isInitialized;
}

/**
 *  \brief  A method that returns a flag indicating whether data
 *          has already been written to all MAX7219 chips in the chain. 
 *
 *  \return true if not all chips have received data yet,
 *          false when all chips are finished writing and chain is ready for next sequence.
**/
boolean Max7219::isChainBusy(void)
{
    return (_ctx->activeDevice != _ctx->numDevices);
}

/**
 *  \brief  Method that determines intensity of illumination of segments
 *          connected to all MAX7219 chips in chain.
 * 
 *  \param intensity lighting intensity range [0x00 - 0x0F]; 0x0F is maximum brightness
**/
void Max7219::setIntensity(const uint8_t intensity)
{
    do
    {
        sendCmd(regIntensity + (intensity % 0x0F));
    } while (isChainBusy());
}

/**
 *  \brief A method for testing the correct operation of the MAX7219 chip(s) chain.
 * 
 *  \param doTest   if true then turn on chip test (all segments on),
 *                  if false, turn off chip test (turn off all segments)
**/
void Max7219::test(const boolean doTest)
{
    uint16_t testcmd {doTest ? (regDisplayTest | 0x0001) : regDisplayTest};

    do
    {
        sendCmd(testcmd);
    } while (isChainBusy());
}

/**
 *  \brief A method that puts all MAX7219 chips in chain into shutdown mode.
**/
void Max7219::shutdown(void)
{
    do
    {
        sendCmd(regShutdown);
    } while (isChainBusy());
}

/**
 *  \brief A method that puts all MAX7219 chips in chain into active mode.
**/
void Max7219::activate(void)
{
    do
    {
        sendCmd(regShutdown | 0x0001);                              // set normal operation flag in the shutdown register
    } while (isChainBusy());
}

/**
 *  \brief A method that turns off the lighting of all segments in chain.
**/
void Max7219::clear(void)
{
    for (uint8_t position = 0; position < Max7219Base::maxDigits; position++)
    {
        do
        {
            clear(position);
        } while (isChainBusy());
    }
}

/**
 *  \brief A method that turns off the lighting of a selected group of segments for currently active chip.
 *
 *  \param position-position of the selected segment group to be cleared
**/
void Max7219::clear(const uint8_t position)
{
    write(position, (_ctx->decodeBcd ? 0x0F : 0x00));
}

/**
 *  \brief  Method that sets the state of the appropriate segment group for active MAX7219 chip.
 *          First write is for last MAX7219 chip in chain.
 *
 *  \param position position of the selected segment group to set the state
 *  \param value    a byte with the state data of segments in the group representing.
**/
void Max7219::write(const uint8_t position, const uint8_t value)
{
    uint16_t cmd {(uint16_t)((position & 0x07) + 1) << 8};          // wrap around position and digit 0 is at address 1

    sendCmd(cmd + value);
}

// *****************************************************************
// *                                                               *
// *                       protected methods                       *
// *                                                               *
// *****************************************************************

/**
 *  \brief A method that sends a data byte to the MAX7219 chip.
 *
 *  \param val data byte to be sent to MAX7219 chip
**/
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

/**
 *  \brief Method that sends command [2 bytes] to currently active MAX7219 chip.
 *
 *  \param cmd [2 bytes] representing command being sent
**/
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

/**
 *  \brief Method for setting the number of displayed segment groups (digits) for all chips in chain.
 *
 *  \param digits number of displayed groups (digits)
**/
inline void Max7219::setScanDigits(const uint8_t digits)
{
    do
    {
        sendCmd(regScanLimit + (digits > 0 ? ((digits - 1) & 0x07) : 0x00));
    } while (isChainBusy());
}
