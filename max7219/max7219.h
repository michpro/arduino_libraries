/**
 * \file max7219.h
 * \brief MAX7219 (Serially Interfaced, 8-Digit LED Display Drivers) base library.
 *
 * \copyright SPDX-FileCopyrightText: Copyright 2022-2023 Michal Protasowicki
 *
 * \license SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include <Arduino.h>

namespace Max7219NS
{
    /**
     * \brief Maximum number of eight-bit display segments supported by a single chip.
    **/
    constexpr uint8_t maxDigits     {0x08};

    /**
     * \brief Maximum value of brightness of display segments.
    **/
    constexpr uint8_t maxIntensity  {0x0F};

    /**
     * \brief Structure containing the context of MAX7219 chip(s) settings.
     *
     * \param csbPin            'load-data' pin. The last 16 bits of serial data are latched on csbPin’s rising edge
     * \param clkPin            'serial-clock' pin. On CLK’s rising edge, data is shifted into the internal shift register.
     *                          On CLK’s falling edge, data is clocked out of DOUT.
     * \param dataPin           'serial-data' pin. Data is loaded into the internal 16-bit shift register on CLK’s rising edge
     * \param scanDigits        scan limit (number of scanned digits) - range [1 - 8]
     * \param intensity         lighting intensity range [0x00 - 0x0F]; 0x0F is maximum brightness
     * \param numDevices        number of MAX7219 chips in the chain - range [1 - 255]
     * \param activeDevice      number of active MAX chip in the chain to which the next sent data will be written
     * \param decodeBcd         false - explicit; true - "code B", i.e. [0-9EHLP\-]
     * \param isInitialized     indicates whether class has set appropriate IO pins
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
        bool    decodeBcd       {false};
        bool    isInitialized   {false};
    } context_t;
}

class Max7219
{
public:
    Max7219(void) = delete;

    /**
     * \brief Max7219 class constructor.
     *
     * \param ctx a reference to structure containing settings for chip(s) chain
    **/
    Max7219(Max7219NS::context_t &ctx);

    /**
     * \brief Max7219 class destructor.
    **/
    virtual ~Max7219(void);

    /**
     * \brief Method that initializes MAX7219 chip(s) chain based on data from the 'ctx' structure.
     *
     * \return returns true if initialized, false otherwise.
    **/
    bool init(void);

    /**
     * \brief Method that initializes MAX7219 chip(s) chain based on data from passed 'ctx' structure.
     *
     * \param ctx[in] a reference to structure containing settings for chip(s) chain
     *
     * \return returns true if initialized, false otherwise.
    **/
    bool init(Max7219NS::context_t &ctx);

    /**
     * \brief   A method of releasing IO pins indicated by the current context
     *          used to communicate with MAX7219 chip(s) chain.
     *
     * \return true if successful, otherwise false.
    **/
    bool release(void);

    /**
     * \brief   A method that sets a reference to a new context. It allows you
     *          to handle many MAX7219 chip(s) chains connected to
     *          different IO pins with use of one instance of the class.
     *
     * \param ctx[in] a reference to structure containing settings for chip
    **/
    void setCtx(Max7219NS::context_t &ctx);

    /**
     * \brief   A method that returns initialization state of the MAX7219 chip(s) chain,
     *          for the current context.
     *
     * \return true when initialized, false otherwise
    **/
    bool isInitialized(void);

    /**
     * \brief   A method that returns a flag indicating whether data
     *          has already been written to all MAX7219 chips in the chain.
     *
     * \return  true if not all chips have received data yet,
     *          false when all chips are finished writing and chain is ready for next sequence.
    **/
    bool isChainBusy(void);

    /**
     * \brief   Method that determines intensity of illumination of segments
     *          connected to all MAX7219 chips in chain.
     *
     * \param intensity[in] lighting intensity range [0x00 - 0x0F]; 0x0F is maximum brightness
     *
     * \return true if successful, otherwise false.
    **/
    bool setIntensity(const uint8_t intensity);

    /**
     * \brief A method for testing the correct operation of the MAX7219 chip(s) chain.
     *
     * \param doTest[in]    if true then turn on chip test (all segments on),
     *                      if false, turn off chip test (turn off all segments)
     *
     * \return
    **/
    bool test(const bool doTest);

    /**
     * \brief A method that puts all MAX7219 chips in chain into shutdown mode.
     *
     * \return true if successful, otherwise false.
    **/
    bool shutdown(void);

    /**
     * \brief A method that puts all MAX7219 chips in chain into active mode.
     *
     * \return true if successful, otherwise false.
    **/
    bool activate(void);

    /**
     * \brief A method that turns off the lighting of all segments in chain.
     *
     * \return true if successful, otherwise false.
    **/
    bool clear(void);

    /**
     * \brief A method that turns off the lighting of a selected group of segments for currently active chip.
     *
     * \param position[in] position of the selected segment group to be cleared
     *
     * \return true if successful, otherwise false.
    **/
    bool clear(const uint8_t position);

    /**
     * \brief   Method that sets the state of the appropriate segment group for active MAX7219 chip.
     *          First write is for last MAX7219 chip in chain.
     *
     * \param position[in]  position of the selected segment group to set the state
     * \param value[in]     a byte with the state data of segments in the group representing.
     *
     * \return true if successful, otherwise false.
    **/
    bool write(const uint8_t position, const uint8_t value);

protected:
    static const uint32_t   clockDelay  {1};

    Max7219NS::context_t   *_ctx        {nullptr};

    /**
     * \brief A type specifying addresses of internal chip registers.
    **/
    typedef enum : uint16_t
    {
        regNoOp         = 0x0000,
        regDecodeMode   = 0x0900,
        regIntensity    = 0x0A00,
        regScanLimit    = 0x0B00,
        regShutdown     = 0x0C00,
        regDisplayTest  = 0x0F00,
    } registers_t;

    /**
     * \brief A method that sends a data byte to the MAX7219 chip.
     *
     * \param val[in] data byte to be sent to MAX7219 chip
    **/
    void shiftOutByte(uint8_t val);

    /**
     * \brief Method that sends command [2 bytes] to currently active MAX7219 chip.
     *
     * \param cmd[in] [2 bytes] representing command being sent
    **/
    void sendCmd(const uint16_t cmd);

    /**
     * \brief Method for setting the number of displayed segment groups (digits) for all chips in chain.
     *
     * \param digits[in] number of displayed groups (digits)
    **/
    inline void setScanDigits(const uint8_t digits);
};
