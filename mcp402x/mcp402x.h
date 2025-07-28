/**
 * @file    mcp402x.h
 * @brief   Library to drive MCP402X devices - non-volatile, 6-bit (64 wiper steps) digital potentiometers
 *          with a simple up/down serial interface.
 *
 * This header file defines the interface for controlling MCP402X digital potentiometers using an Arduino.
 * It provides a class with methods to initialize the chip, adjust the wiper position, retrieve the current
 * position, and save settings to non-volatile memory.
 *
 * @copyright SPDX-FileCopyrightText: Copyright 2022-2023 Michal Protasowicki
 * @license SPDX-License-Identifier: MIT
 */

#pragma once

#include <Arduino.h>

namespace Mcp402xNS
{
    /**
     * @brief Maximum value of wiper resistance (according to chip's datasheet).
     */
    constexpr uint8_t   maxValue        {0x3F};

    /**
     * @brief Minimum value of wiper resistance (according to chip's datasheet).
     */
    constexpr uint8_t   minValue        {0x00};

    /**
     * @brief Structure containing the context of MCP402X chip settings.
     *
     * This structure holds the configuration and state of the MCP402X chip, including pin assignments
     * and the current wiper value.
     */
    typedef struct
    {
        uint8_t         csPin           {0x02};         // Chip select input pin (active LOW enables serial commands).
        uint8_t         udPin           {0x03};         // Up/Down pin to increment or decrement the wiper.
        uint8_t         currentValue    {0x00};         // Current wiper value (0 to 63). In non-volatile mode, this
                                                        // may be invalid after init (defaults to 0x00) and must be
                                                        // maintained manually as it cannot be read from the chip.
        bool            isInitialized   {false};        // Indicates if the chip has been initialized via init().
    } context_t;
}

class Mcp402x
{
public:
    Mcp402x(void) = delete;

    /**
     * @brief Constructor for Mcp402x class.
     *
     * Initializes the Mcp402x object with a reference to a context structure.
     *
     * @param ctx[in] Reference to the context structure containing chip settings.
     */
    Mcp402x(Mcp402xNS::context_t &ctx);

    /**
     * @brief Destructor for Mcp402x class.
     *
     * Cleans up resources used by the Mcp402x object.
     */
    virtual ~Mcp402x(void);

    /**
     * @brief Initializes the MCP402X chip using the current context.
     *
     * Configures the GPIO pins as outputs and sets initial states (CS high, U/D high).
     *
     * @return true if initialization is successful, false otherwise (e.g., if context is null).
     */
    bool init(void);

    /**
     * @brief Initializes the MCP402X chip with a new context.
     *
     * Updates the internal context and initializes the chip accordingly.
     *
     * @param ctx[in]   Reference to the new context structure containing chip settings.
     * @return          true if initialization is successful, false otherwise.
     */
    bool init(Mcp402xNS::context_t &ctx);

    /**
     * @brief Sets a new context for the MCP402X chip.
     *
     * Updates the internal context pointer without initializing the chip.
     *
     * @param ctx[in] Reference to the new context structure containing chip settings.
     */
    void setCtx(Mcp402xNS::context_t &ctx);

    /**
     * @brief Checks if the chip has been initialized.
     *
     * @return true if the chip is initialized (init() was called successfully), false otherwise.
     */
    bool isInitialized(void);

    /**
     * @brief Updates the wiper value stored in the context.
     *
     * Useful for synchronizing the context with the chip’s state in non-volatile mode, where the
     * actual wiper position cannot be read directly from the chip.
     *
     * @param value[in] New wiper value to store in the context (0 to 63).
     * @return          true if the value was updated successfully, false otherwise (e.g., if uninitialized).
     */
    bool updateWiperValue(const uint8_t value);

    /**
     * @brief Increments the wiper position by one step.
     *
     * Moves the wiper up if it’s not already at the maximum value (63).
     *
     * @return true if the wiper was incremented, false if already at max or uninitialized.
     */
    bool up(void);

    /**
     * @brief Decrements the wiper position by one step.
     *
     * Moves the wiper down if it’s not already at the minimum value (0).
     *
     * @return true if the wiper was decremented, false if already at min or uninitialized.
     */
    bool down(void);

    /**
     * @brief Sets the wiper to a specific value.
     *
     * Adjusts the wiper position to the specified value (clamped to 0-63).
     *
     * @param value[in] Desired wiper value (0 to 63; values > 63 are set to 63).
     * @return          true if the wiper value was changed, false otherwise (e.g., if uninitialized).
     */
    bool set(uint8_t value);

    /**
     * @brief Retrieves the current wiper value from the context.
     *
     * @return Current wiper value (0 to 63) if initialized, Mcp402xNS::minValue (0) otherwise.
     */
    uint8_t get(void);

    /**
     * @brief Saves the current wiper value to the chip’s non-volatile memory.
     *
     * Ensures the wiper position persists across power cycles.
     *
     * @return Current wiper value after the save attempt; Mcp402xNS::minValue if uninitialized.
     */
    uint8_t keepNonVolatile(void);

protected:
    static const unsigned int   pulseDelay  {1};        // Delay between pulses in microseconds.
    static const unsigned int   minCsTime   {5};        // Minimum chip select time in microseconds.
    Mcp402xNS::context_t       *_ctx        {nullptr};  // Pointer to the chip’s context.

    /**
     * @brief Enumeration for wiper direction.
     */
    typedef enum Direction : bool
    {
        DOWN    = false,                                // Decrease wiper position.
        UP      = true                                  // Increase wiper position.
    } direction_t;

    /**
     * @brief Enumeration for non-volatile memory option.
     */
    typedef enum KeepNV : bool
    {
        NO      = false,                                // Do not save to non-volatile memory.
        YES     = true                                  // Save to non-volatile memory.
    } keepNV_t;

    /**
     * @brief Sends a series of pulses to adjust the wiper position.
     *
     * Controls the U/D and CS pins to move the wiper or save to non-volatile memory.
     *
     * @param pulses[in]        Number of pulses to send (steps to move the wiper).
     * @param dir[in]           Direction to move the wiper (UP or DOWN).
     * @param nonVolatile[in]   Whether to save the new position to non-volatile memory.
     */
    void pulse(const uint8_t pulses, const direction_t dir, const keepNV_t nonVolatile);
};
