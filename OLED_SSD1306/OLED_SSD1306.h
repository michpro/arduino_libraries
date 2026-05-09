/**
 * @file        OLED_SSD1306.h
 * @brief       Arduino library for monochrome OLED displays based on SSD1306.
 *
 * @copyright   SPDX-FileCopyrightText: Copyright 2026 Michal Protasowicki
 *              Based on Adafruit SSD1306 Library, Copyright (c) 2012 Adafruit Industries
 * @license     SPDX-License-Identifier: MIT
 */

#pragma once

#if defined(ARDUINO_STM32_FEATHER)
typedef class HardwareSPI SPIClass;
#endif

//*******************************************************************
//*                                                                 *
//*                            Includes                             *
//*                                                                 *
//*******************************************************************

#include "gfx\gfx.h"
#include <SPI.h>
#include <Wire.h>
#include "OLED_Constants.h"

#if defined(__AVR__)
    typedef volatile uint8_t    PortReg;
    typedef uint8_t             PortMask;
#   define HAVE_PORTREG
#elif defined(__SAM3X8E__)
    typedef volatile RwReg      PortReg;
    typedef uint32_t            PortMask;
#   define HAVE_PORTREG
#elif (defined(__arm__) || defined(ARDUINO_FEATHER52)) && !defined(ARDUINO_ARCH_MBED) && !defined(ARDUINO_ARCH_RP2040)
    typedef volatile uint32_t   PortReg;
    typedef uint32_t            PortMask;
#   define HAVE_PORTREG
#endif

namespace OLED
{
    //*******************************************************************
    //*                                                                 *
    //*                      Class Declaration                          *
    //*                                                                 *
    //*******************************************************************

    /**
     * @brief Class that stores state and functions for interacting with SSD1306 OLED displays.
     */
    class SSD1306 : public GFX
    {
    public:
        /**
         * @brief Constructor for I2C-connected SSD1306 displays.
         *
         * @param[in] display Display type configuration.
         * @param[in] wire    Pointer to TwoWire instance (e.g. &Wire).
         * @param[in] rstPin Reset pin (using Arduino pin numbering), or PIN_NONE if not used.
         */
        SSD1306(display_t display, TwoWire *wire = &Wire, int_fast8_t rstPin = Constants::PIN_NONE);

        /**
         * @brief Constructor for SPI-connected SSD1306 displays (software SPI).
         *
         * @param[in] display   Display type configuration.
         * @param[in] mosiPin  MOSI pin.
         * @param[in] sclkPin  SCLK pin.
         * @param[in] dcPin    Data/Command pin.
         * @param[in] rstPin   Reset pin, or PIN_NONE if not used.
         * @param[in] csPin    Chip Select pin.
         */
        SSD1306(display_t display, int_fast8_t mosiPin, int_fast8_t sclkPin, int_fast8_t dcPin, int_fast8_t rstPin, int_fast8_t csPin);

        /**
         * @brief Constructor for SPI-connected SSD1306 displays (hardware SPI).
         *
         * @param[in] display   Display type configuration.
         * @param[in] spi       Pointer to SPIClass instance.
         * @param[in] dcPin    Data/Command pin.
         * @param[in] rstPin   Reset pin, or PIN_NONE if not used.
         * @param[in] csPin    Chip Select pin.
         * @param[in] bitrate   SPI clock rate.
         */
        SSD1306(display_t display, SPIClass *spi, int_fast8_t dcPin, int_fast8_t rstPin, int_fast8_t csPin, uint32_t bitrate = 8000000UL);

        /**
         * @brief Destructor.
         */
        ~SSD1306(void) override;

        /**
         * @brief Initialize the display peripheral and internal buffers.
         *
         * @param[in] vcc         Voltage source configuration.
         * @param[in] i2cAddr     I2C address (0 defaults to device typical).
         * @param[in] reset       Whether to perform hardware reset.
         * @param[in] periphBegin Whether to call Wire.begin() / SPI.begin().
         * @return true on success, false on failure (e.g., allocation error).
         */
        bool begin(voltageSrc_t vcc = voltageSrc_t::V_INTERNAL, uint_fast8_t i2cAddr = 0, bool reset = true, bool periphBegin = true);

        /**
         * @brief Push the internal memory buffer to the display.
         */
        void display(void);

        /**
         * @brief Clear the internal memory buffer.
         */
        void clearDisplay(void);

        /**
         * @brief Invert the display colors.
         *
         * @param[in] invert true to invert, false for normal rendering.
         */
        void invertDisplay(bool invert) override;

        /**
         * @brief Dim the display contrast.
         *
         * @param[in] isDimmed true to lower contrast.
         */
        void dim(bool isDimmed);

        /**
         * @brief Draw a single pixel to the buffer.
         *
         * @param[in] x     X coordinate.
         * @param[in] y     Y coordinate.
         * @param[in] color Pixel color (from OLED::Pixel enum).
         */
        void drawPixel(int_fast16_t x, int_fast16_t y, uint_fast8_t color) override;

        /**
         * @brief Draw a fast horizontal line.
         *
         * @param[in] xPos  Starting X coordinate.
         * @param[in] yPos  Starting Y coordinate.
         * @param[in] width Line length in pixels.
         * @param[in] color Line color.
         */
        void drawFastHLine(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t width, uint_fast8_t color) override;

        /**
         * @brief Draw a fast vertical line.
         *
         * @param[in] xPos   Starting X coordinate.
         * @param[in] yPos   Starting Y coordinate.
         * @param[in] height Line height in pixels.
         * @param[in] color  Line color.
         */
        void drawFastVLine(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t height, uint_fast8_t color) override;

        /**
         * @brief Activate hardware scrolling.
         *
         * @param[in] dir       Scroll direction.
         * @param[in] startPage Start page address.
         * @param[in] stopPage  Stop page address.
         */
        void startScroll(scroll_dir_t dir, uint_fast8_t startPage, uint_fast8_t stopPage);

        /**
         * @brief Deactivate all scrolling.
         */
        void stopScroll(void);

        /**
         * @brief Scroll content completely to the right.
         */
        void contentScrollRight(void);

        /**
         * @brief Scroll content completely to the left.
         */
        void contentScrollLeft(void);

        /**
         * @brief Check if the device is SSD1315.
         *
         * @return true if SSD1315, false otherwise.
         */
        bool isSSD1315(void) const
        {
            return (DEV_SSD1315 == _device);
        }

        /**
         * @brief Send a generic command to the display.
         *
         * @param[in] command The byte to send as command.
         */
        void sendCommand(uint_fast8_t command);

        /**
         * @brief Get the boolean pixel state at a coordinate.
         *
         * @param[in] x X coordinate.
         * @param[in] y Y coordinate.
         * @return true if pixel is set, false if unset.
         */
        bool getPixel(int_fast16_t x, int_fast16_t y);

        /**
         * @brief Get a pointer to the internal drawing buffer.
         *
         * @return Pointer to the buffer.
         */
        uint8_t *getBuffer(void);

    protected:
        typedef enum : uint_fast8_t
        {
            DEV_SSD1306,
            DEV_SSD1312,
            DEV_SSD1315,
            DEV_SH1106,
            DEV_CH1115
        } device_t;

        /**
         * @brief Write data byte over SPI.
         *
         * @param[in] dataByte Data byte to write.
         */
        inline void SPIwrite(uint_fast8_t dataByte) __attribute__((always_inline));

        /**
         * @brief Internal fast horizontal line drawing method without rotation handling.
         *
         * @param[in] xPos  Starting X coordinate.
         * @param[in] yPos  Starting Y coordinate.
         * @param[in] width Line length in pixels.
         * @param[in] color Line color.
         */
        void drawFastHLineInternal(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t width, uint_fast8_t color);

        /**
         * @brief Internal fast vertical line drawing method without rotation handling.
         *
         * @param[in] xPos   Starting X coordinate.
         * @param[in] yPos   Starting Y coordinate.
         * @param[in] height Line height in pixels.
         * @param[in] color  Line color.
         */
        void drawFastVLineInternal(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t height, uint_fast8_t color);

        /**
         * @brief Send a single command byte.
         *
         * @param[in] command Command byte.
         */
        void sendOneCommand(uint_fast8_t command);

        /**
         * @brief Send a list of commands stored in PROGMEM or RAM.
         *
         * @param[in] commandsArray Pointer to the array of commands.
         * @param[in] count         Number of commands in the array.
         * @param[in] isProgMem     true if array is in PROGMEM, false if in RAM.
         */
        void sendCommandList(const uint8_t *commandsArray, uint_fast8_t count, bool isProgMem = true);

        /**
         * @brief Send a block of data to display RAM.
         *
         * @param[in] data  Pointer to data buffer (or nullptr to send zeros).
         * @param[in] count Number of bytes to send.
         */
        void sendDataPayload(const uint8_t *data, uint_fast16_t count);

        /**
         * @brief Reinitialize the graphics context for a specific display device.
         *
         * @param[in] display Display type configuration.
         */
        inline void reinitGFX(display_t display) __attribute__((always_inline));

        /**
         * @brief Set internal contrast value based on device type and VCC source.
         *
         * @param[in] device   Display device type.
         * @param[in] isIntVcc true if internal VCC source is used.
         */
        inline void setContrastValue(device_t device, bool isIntVcc) __attribute__((always_inline));

        SPIClass       *_spi            {nullptr};                  ///< Pointer to hardware SPI instance
        TwoWire        *_wire           {nullptr};                  ///< Pointer to I2C interface instance
        uint8_t        *_buffer         {nullptr};                  ///< Pointer to display memory buffer
        uint_fast8_t    _i2cAddr        {0x00};                     ///< Configured I2C address
        uint_fast8_t    _vccState       {0x00};                     ///< Status of the voltage source

        uint_fast8_t    _windowX1       {255};                      ///< Bounding box minimum X
        uint_fast8_t    _windowY1       {255};                      ///< Bounding box minimum Y
        uint_fast8_t    _windowX2       {0};                        ///< Bounding box maximum X
        uint_fast8_t    _windowY2       {0};                        ///< Bounding box maximum Y

        int_fast8_t     _mosiPin        {Constants::PIN_NONE};      ///< Software SPI MOSI pin
        int_fast8_t     _clkPin         {Constants::PIN_NONE};      ///< Software SPI Clock pin
        int_fast8_t     _dcPin          {Constants::PIN_NONE};      ///< Data/Command pin
        int_fast8_t     _csPin          {Constants::PIN_NONE};      ///< Chip Select pin
        int_fast8_t     _rstPin         {Constants::PIN_NONE};      ///< Reset pin

        device_t        _device         {DEV_SSD1306};              ///< Type of OLED display controller
        uint_fast8_t    _colOffset      {0};                        ///< Column offset for centering RAM on display

    #ifdef HAVE_PORTREG
        PortReg        *_mosiPort       {nullptr};                  ///< Port register for software SPI MOSI
        PortReg        *_clkPort        {nullptr};                  ///< Port register for software SPI Clock
        PortReg        *_dcPort         {nullptr};                  ///< Port register for Data/Command pin
        PortReg        *_csPort         {nullptr};                  ///< Port register for Chip Select pin
        PortMask        _mosiPinMask    {0};                        ///< Pin mask for software SPI MOSI
        PortMask        _clkPinMask     {0};                        ///< Pin mask for software SPI Clock
        PortMask        _dcPinMask      {0};                        ///< Pin mask for Data/Command pin
        PortMask        _csPinMask      {0};                        ///< Pin mask for Chip Select pin
    #endif
        uint_fast8_t    _contrast       {0x00};                     ///< Normal contrast setting for this device
    #if defined(SPI_HAS_TRANSACTION)
        SPISettings     _spiSettings;                               ///< SPI settings initialized for transactions. Allow sub-class to change
    #endif
    };
}
