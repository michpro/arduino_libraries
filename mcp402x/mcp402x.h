/**
 * \file    mcp402x.h
 * \brief   Library to drive MCP402x devices - non-volatile,
 *          6-bit (64 wiper steps) digital potentiometers
 *          with a simple up/down serial interface.
 *
 * \copyright SPDX-FileCopyrightText: Copyright 2022-2023 Michal Protasowicki
 *
 * \license SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include <Arduino.h>

namespace Mcp402xNS
{
    /**
     * \brief Maximum value of wiper resistance (according to chip's datasheet).
    **/
    constexpr uint8_t   maxValue        {0x3F};

    /**
     * \brief Minimum value of wiper resistance (according to chip's datasheet).
    **/
    constexpr uint8_t   minValue        {0x00};

    /**
     *  \brief Structure containing the context of MCP402x chip settings.
     * 
     *  \param csPin            chip select input pin. Active LOW enables the serial commands.
     *  \param udPin            U/D input pin is used to increment or decrement the wiper on the digital potentiometer.
     *  \param currentValue     Potentiometer wiper current value.
     *                          Attention!!! When using non-volatile chip mode,
     *                          the value may be invalid after initialization (always equal to 0x00).
     *                          In NV mode, you must maintain this value yourself, as it cannot be read from the chip.
     *  \param isInitialized    A variable indicating whether the initialization allowing communication
     *                          with the chip has been performed - the init() method has been called.
    **/
    typedef struct
    {
        uint8_t         csPin           {0x02};
        uint8_t         udPin           {0x03};
        uint8_t         currentValue    {0x00};
        bool            isInitialized   {false};
    } context_t;
    
}

class Mcp402x
{
public:
    Mcp402x(void) = delete;

    /**
     *  \brief Mcp402x class constructor.
     * 
     *  \param ctx[in] a reference to structure containing settings for chip.
    **/
    Mcp402x(Mcp402xNS::context_t &ctx);

    /**
     *  \brief Mcp402x class destructor.
    **/
    virtual ~Mcp402x(void);

    /**
     *  \brief Method that initializes MCP402x chip based on data from the 'ctx' structure.
     * 
     *  \return returns true if initialized, false otherwise.
    **/
    bool init(void);

    /**
     *  \brief Method that initializes MCP402x chip based on data from passed 'ctx' structure.
     *
     *  \param ctx[in] a reference to structure containing settings for chip
     *
     *  \return returns true if initialized, false otherwise.
    **/
    bool init(Mcp402xNS::context_t &ctx);

    /**
     *  \brief A method that allows you to indicate context with settings for the chip. 
     *
     *  \param ctx[in] a reference to structure containing settings for chip.
    **/
    void setCtx(Mcp402xNS::context_t &ctx);

    /**
     *  \brief A method that checks whether chip with current context has been initialized.
     *
     *  \return returns true if initialized, false otherwise.
    **/
    bool isInitialized(void);

    /**
     *  \brief  A method, used after initialization of NV-mode chip context,
     *          to update the potentiometer wiper value stored by the object.
     *          It allows you to synchronize the value stored in context 
     *          (restored from external non-volatile memory) with chip wiper state setting,
     *          which we can't read from it.
     *
     *  \param[in] value Value to store in current context.
     * 
     *  \return true if successful writing to the context, false otherwise.
    **/
    bool updateWiperValue(const uint8_t value);

    /**
     *  \brief  A method to increase position of potentiometer wiper. 
     * 
     *  \return true if position has been changed, false otherwise. 
    **/
    bool up(void);

    /**
     *  \brief  A method to decrease position of potentiometer wiper. 
     *
     *  \return true if position has been changed, false otherwise. 
    **/
    bool down(void);

    /**
     *  \brief  A method that sets the state of potentiometer wiper position to a selected value. 
     *
     *  \param[in] value Potentiometer wiper value to set.
     * 
     *  \return true when value has been changed, false otherwise. 
    **/
    bool set(uint8_t value);

    /**
     *  \brief  A method that reads the status of potentiometer wiper position from current context.
     *
     *  \return Value of current state of potentiometer wiper, read from context.
     *          When context is uninitialized, it always returns Mcp402xNS::minValue.
    **/
    uint8_t get(void);

    /**
     *  \brief  A method that saves the current value of potentiometer wiper
     *          in the chip's internal non-volatile memory.
     *
     *  \return Value of current state of potentiometer wiper, read from context.
     *          When context is uninitialized, it always returns Mcp402xNS::minValue.
    **/
    uint8_t keepNonVolatile(void);

protected:
    static const unsigned int   pulseDelay  {1};
    static const unsigned int   minCsTime   {5};

    Mcp402xNS::context_t       *_ctx        {nullptr};

    /**
     *  \brief  An internal class type specifying direction of potentiometer wiper.
    **/
    typedef enum Direction : bool
    {
        DOWN    = false,
        UP      = true,
    } direction_t;

    /**
     *  \brief  An internal class type that determines whether potentiometer wiper value
     *          is to be stored in non-volatile memory of the chip.
    **/
    typedef enum KeepNV : bool
    {
        NO      = false,
        YES     = true,
    } keepNV_t;

    /**
     *  \brief  Method that sends n pulses to U/D pin of MCP402x chip
     *          to change the resistance value.
     *
     *  \param pulses[in]       number of pulses to be sent
     *  \param dir[in]          potentiometer wiper direction
     *  \param nonVolatile[in]  whether to save data in internal non-volatile memory of chip.
    **/
    void pulse(const uint8_t pulses, const direction_t dir, const keepNV_t nonVolatile);
};
