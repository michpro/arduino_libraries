/**
 * \file max7219.h
 * \brief MAX7219 (Serially Interfaced, 8-Digit LED Display Drivers) base library.
 *
 * \copyright SPDX-FileCopyrightText: Copyright 2022 Michal Protasowicki
 *
 * \license SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include <Arduino.h>

namespace Max7219Base
{
    constexpr uint8_t maxDigits     {0x08};
    constexpr uint8_t maxIntensity  {0x0F};

    /**
     *  \brief Structure containing the context of MAX7219 chip(s) settings.
     * 
     *  \param csbPin           'load-data' pin. The last 16 bits of serial data are latched on csbPin’s rising edge
     *  \param clkPin           'serial-clock' pin. On CLK’s rising edge, data is shifted into the internal shift register.
     *                          On CLK’s falling edge, data is clocked out of DOUT.
     *  \param dataPin          'serial-data' pin. Data is loaded into the internal 16-bit shift register on CLK’s rising edge
     *  \param scanDigits       scan limit (number of scanned digits) - range [1 - 8]
     *  \param intensity        lighting intensity range [0x00 - 0x0F]; 0x0F is maximum brightness
     *  \param numDevices       number of MAX7219 chips in the chain - range [1 - 255]
     *  \param activeDevice     number of active MAX chip in the chain to which the next sent data will be written
     *  \param decodeBcd        false - explicit; true - "code B", i.e. [0-9EHLP\-]
     *  \param isInitialized    indicates whether class has set appropriate IO pins
     *                          to communicate with MAX7219 chips chain and initialized them.
    **/
    typedef struct
    {
        uint8_t csbPin          {2};
        uint8_t clkPin          {3};
        uint8_t dataPin         {4};
        uint8_t scanDigits      {8};
        uint8_t intensity       {0x0F};
        uint8_t numDevices      {1};
        uint8_t activeDevice    {1};
        boolean decodeBcd       {false};
        boolean isInitialized   {false};
    } context;
}

class Max7219
{
public:
    Max7219(void) = delete;
    Max7219(Max7219Base::context &ctx);
    virtual ~Max7219(void);

    void    init(void);
    void    init(Max7219Base::context &ctx);
    void    release(void);
    void    setCtx(Max7219Base::context &ctx);
    boolean isInitialized(void);
    boolean isChainBusy(void);
    void    setIntensity(const uint8_t intensity);
    void    test(void);
    void    shutdown(void);
    void    activate(void);
    void    clear(void);
    void    clear(const uint8_t position);
    void    write(const uint8_t position, const uint8_t value);

protected:
    const uint32_t clockDelay   {1};

    enum registers : uint16_t
    {
        regNoOp         = 0x0000,
        regDecodeMode   = 0x0900,
        regIntensity    = 0x0A00,
        regScanLimit    = 0x0B00,
        regShutdown     = 0x0C00,
        regDisplayTest  = 0x0F00,
    };

    Max7219Base::context       *_ctx;

    void sendCmd(const uint16_t cmd);
    void shiftOutByte(uint8_t val);
    inline void setScanDigits(const uint8_t digits);
};
