/**
 * @file    max7219.h
 * @brief   MAX7219 (Serially Interfaced, 8-Digit LED Display Drivers) base library.
 *
 * This header file defines the interface for controlling MAX7219 chips, which are
 * serially interfaced drivers for 8-digit 7-segment LED displays or LED matrices.
 *
 * @copyright SPDX-FileCopyrightText: Copyright 2022-2023 Michal Protasowicki
 *
 * @license SPDX-License-Identifier: MIT
 */

#pragma once

#include <Arduino.h>

namespace Max7219NS
{
    /**
     * @brief Maximum number of eight-bit display segments supported by a single chip.
     */
    constexpr uint8_t maxDigits     {0x08};

    /**
     * @brief Maximum value of brightness of display segments.
     */
    constexpr uint8_t maxIntensity  {0x0F};

    /**
     * @brief Structure containing the context of MAX7219 chip(s) settings.
     *
     * This structure holds all necessary configuration data for a chain of MAX7219 chips,
     * including pin assignments and operational settings.
     *
     * @param csbPin            The 'load-data' pin. The last 16 bits of serial data are latched on csbPin’s rising edge.
     * @param clkPin            The 'serial-clock' pin. On CLK’s rising edge, data is shifted into the internal shift register.
     *                          On CLK’s falling edge, data is clocked out of DOUT.
     * @param dataPin           The 'serial-data' pin. Data is loaded into the internal 16-bit shift register on CLK’s rising edge.
     * @param scanDigits        The scan limit (number of scanned digits) - range [1 - 8]. Determines how many digits are displayed.
     * @param intensity         The lighting intensity - range [0x00 - 0x0F]; 0x0F is maximum brightness.
     * @param numDevices        The number of MAX7219 chips in the chain - range [1 - 255].
     * @param activeDevice      The number of the active MAX7219 chip in the chain to which the next sent data will be written.
     * @param decodeBcd         Flag to enable BCD decoding. If true, the chip interprets data as BCD codes for digits 0-9 and some letters.
     *                          If false, data is treated as raw segment data.
     * @param isInitialized     Indicates whether the class has set the appropriate IO pins to communicate with the MAX7219 chips chain and initialized them.
     */
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
     * @brief Max7219 class constructor.
     *
     * Creates an instance of the Max7219 class to manage a chain of MAX7219 chips.
     *
     * @param ctx A reference to the context_t structure containing settings for the chip(s) chain.
     */
    Max7219(Max7219NS::context_t &ctx);

    /**
     * @brief Max7219 class destructor.
     *
     * Cleans up resources used by the Max7219 instance.
     */
    virtual ~Max7219(void);

    /**
     * @brief Initializes the MAX7219 chip(s) chain based on the current context.
     *
     * Sets up the IO pins and configures the chips with settings from the context.
     * Must be called before using other methods.
     *
     * @return True if initialization succeeds, false otherwise.
     */
    bool init(void);

    /**
     * @brief Initializes the MAX7219 chip(s) chain with a new context.
     *
     * Updates the internal context to the provided one and initializes the chain.
     *
     * @param ctx A reference to the context_t structure containing settings for the chip(s) chain.
     * @return True if initialization succeeds, false otherwise.
     */
    bool init(Max7219NS::context_t &ctx);

    /**
     * @brief Releases the IO pins used by the MAX7219 chip(s) chain.
     *
     * Sets the pins to input mode, disconnecting from the chips, and marks the chain as uninitialized.
     *
     * @return True if release succeeds, false otherwise.
     */
    bool release(void);

    /**
     * @brief Sets a new context for the Max7219 instance.
     *
     * Allows changing the configuration to manage a different chain of MAX7219 chips.
     * Requires calling init() afterward to apply the new context.
     *
     * @param ctx A reference to the new context_t structure.
     */
    void setCtx(Max7219NS::context_t &ctx);

    /**
     * @brief Checks if the MAX7219 chip(s) chain is initialized.
     *
     * @return True if the chain is initialized, false otherwise.
     */
    bool isInitialized(void);

    /**
     * @brief Checks if the chain is busy writing data.
     *
     * Indicates whether all chips in the chain have received their data in a sequence.
     *
     * @return True if the chain is busy, false if all chips have received data.
     */
    bool isChainBusy(void);

    /**
     * @brief Sets the display intensity for all chips in the chain.
     *
     * Adjusts the brightness of the connected LED segments.
     *
     * @param intensity The intensity level, ranging from 0x00 (minimum) to 0x0F (maximum).
     * @return True if the operation succeeds, false otherwise.
     */
    bool setIntensity(const uint8_t intensity);

    /**
     * @brief Activates or deactivates display test mode.
     *
     * In test mode, all segments are lit to verify functionality.
     *
     * @param doTest If true, enables test mode; if false, disables it.
     * @return True if the operation succeeds, false otherwise.
     */
    bool test(const bool doTest);

    /**
     * @brief Puts all chips in the chain into shutdown mode.
     *
     * Turns off the display and reduces power consumption.
     *
     * @return True if the operation succeeds, false otherwise.
     */
    bool shutdown(void);

    /**
     * @brief Activates all chips in the chain from shutdown mode.
     *
     * Restores normal operation and turns the display back on.
     *
     * @return True if the operation succeeds, false otherwise.
     */
    bool activate(void);

    /**
     * @brief Clears the entire display on all chips in the chain.
     *
     * Turns off all segments across all connected chips.
     *
     * @return True if the operation succeeds, false otherwise.
     */
    bool clear(void);

    /**
     * @brief Clears a specific digit position on the active chip.
     *
     * Turns off all segments at the specified position on the currently active chip.
     *
     * @param position The digit position to clear (0 to 7).
     * @return True if the operation succeeds, false otherwise.
     */
    bool clear(const uint8_t position);

    /**
     * @brief Writes data to a specific digit position on the active chip.
     *
     * Sends data to control the segments at the specified position.
     *
     * @param position The digit position to write to (0 to 7).
     * @param value The data to write; BCD code if decodeBcd is true, raw segment data otherwise.
     * @return True if the operation succeeds, false otherwise.
     */
    bool write(const uint8_t position, const uint8_t value);

protected:
    static const uint32_t   clockDelay  {1};        // Delay in microseconds for clock signal timing.
    Max7219NS::context_t   *_ctx        {nullptr};  // Pointer to the current context structure.

    /**
     * @brief Enum specifying addresses of internal MAX7219 chip registers.
     */
    typedef enum : uint16_t
    {
        regNoOp         = 0x0000,                   // No operation.
        regDecodeMode   = 0x0900,                   // Decode mode register.
        regIntensity    = 0x0A00,                   // Intensity control register.
        regScanLimit    = 0x0B00,                   // Scan limit register.
        regShutdown     = 0x0C00,                   // Shutdown mode register.
        regDisplayTest  = 0x0F00,                   // Display test register.
    } registers_t;

    /**
     * @brief Sends a single byte of data to the MAX7219 chip.
     *
     * Shifts out the byte bit-by-bit over the data pin, synchronized with the clock pin.
     *
     * @param val The byte to send.
     */
    void shiftOutByte(uint8_t val);

    /**
     * @brief Sends a 16-bit command to the currently active MAX7219 chip.
     *
     * Manages the chip-select pin and shifts out the command bytes.
     *
     * @param cmd The 16-bit command (address and data).
     */
    void sendCmd(const uint16_t cmd);

    /**
     * @brief Sets the number of displayed digits for all chips in the chain.
     *
     * Configures the scan limit register on all chips.
     *
     * @param digits The number of digits to scan (1 to 8).
     */
    inline void setScanDigits(const uint8_t digits);
};
