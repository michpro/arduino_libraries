/**
 * @file        OLED_Constants.h
 * @brief       Hardware constants, command definitions, and configuration types for OLED SSD1306/SH1106 displays.
 *
 * @copyright   SPDX-FileCopyrightText: Copyright 2026 Michal Protasowicki
 * @license     SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

namespace OLED
{
    /**
     * @brief       Enumeration of physical display types and resolutions.
     */
    enum class display_t : uint_fast8_t
    {
        SH1106_128x64,                                              ///< 1.3"  SH1106 based display
        SH1107_128x64,                                              ///< 1.54" SH1107 based display
        SSD1315_128x64,                                             ///< 0.96" SSD1315 based display
        SSD1309_128x64,                                             ///< 1.54" SSD1309 based display
        SSD1306_128x64,                                             ///< 0.96" SSD1306 based display
        SSD1306_128x32,                                             ///< 0.91" SSD1306 based display
        SSD1306_96x16,                                              ///< 0.69" SSD1306 based display
        SSD1306_72x40,                                              ///< 0.42" SSD1306 based display
        SSD1312_64x128,                                             ///< 1.09" SSD1312 based display
        SSD1306_64x48,                                              ///< 0.66" SSD1306 based display
        SSD1306_64x32,                                              ///< 0.49" SSD1306 based display
        CH1115_48x88                                                ///< 0.5"  CH1115 based display
    };

    /**
     * @brief       OLED controller command bytes.
     */
    enum class command_t : uint8_t
    {
        NOP                                         = 0xE3,         ///< No operation
        MEMORYMODE                                  = 0x20,         ///< Set memory addressing mode
        COLUMNADDR                                  = 0x21,         ///< Set column address
        PAGEADDR                                    = 0x22,         ///< Set page address
        SETCONTRAST                                 = 0x81,         ///< Set contrast control
        CHARGEPUMP                                  = 0x8D,         ///< Set charge pump
        SEGREMAP                                    = 0xA0,         ///< Set segment remap
        DISPLAYALLON_RESUME                         = 0xA4,         ///< Resume to RAM content display
        DISPLAYALLON                                = 0xA5,         ///< Entire display ON
        NORMALDISPLAY                               = 0xA6,         ///< Normal display mode
        INVERTDISPLAY                               = 0xA7,         ///< Inverted display mode
        SETMULTIPLEX                                = 0xA8,         ///< Set multiplex ratio
        DISPLAYOFF                                  = 0xAE,         ///< Display OFF (sleep mode)
        DISPLAYON                                   = 0xAF,         ///< Display ON (normal mode)
        COMSCANINC                                  = 0xC0,         ///< Set COM output scan direction increment
        COMSCANDEC                                  = 0xC8,         ///< Set COM output scan direction decrement
        SETDISPLAYOFFSET                            = 0xD3,         ///< Set display offset
        SETDISPLAYCLOCKDIV                          = 0xD5,         ///< Set display clock divide ratio/oscillator frequency
        SETPRECHARGE                                = 0xD9,         ///< Set pre-charge period
        SETCOMPINS                                  = 0xDA,         ///< Set COM pins hardware configuration
        SETVCOMDETECT                               = 0xDB,         ///< Set VCOMH deselect level

        SETLOWCOLUMN                                = 0x00,         ///< Set lower column start address for page addressing mode
        SETHIGHCOLUMN                               = 0x10,         ///< Set higher column start address for page addressing mode
        SETSTARTLINE                                = 0x40,         ///< Set display RAM display start line register
        SETSTARTPAGE                                = 0xB0,         ///< Set page start address for page addressing mode

        EXTERNALVCC                                 = 0x01,         ///< External display voltage source
        SWITCHCAPVCC                                = 0x02,         ///< General display voltage from 3.3V

        RIGHT_HORIZONTAL_SCROLL                     = 0x26,         ///< Init right horizontal scroll
        LEFT_HORIZONTAL_SCROLL                      = 0x27,         ///< Init left horizontal scroll
        VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL        = 0x29,         ///< Init vertical and right horizontal scroll
        VERTICAL_AND_LEFT_HORIZONTAL_SCROLL         = 0x2A,         ///< Init vertical and left horizontal scroll
        DEACTIVATE_SCROLL                           = 0x2E,         ///< Stop scrolling
        ACTIVATE_SCROLL                             = 0x2F,         ///< Start scrolling
        SET_VERTICAL_SCROLL_AREA                    = 0xA3,         ///< Set vertical scroll area

        CONTENT_SCROLL_RIGHT                        = 0x2C,         ///< Content scroll right
        CONTENT_SCROLL_LEFT                         = 0x2D,         ///< Content scroll left

        SH1106_SET_PUMP_VOLTAGE                     = 0x30,         ///< Set pump voltage (SH1106 only)
        SH1106_SET_PUMP_MODE                        = 0xAD,         ///< Set pump mode (SH1106 only)
        SH1106_PUMP_ON                              = 0x8B,         ///< Turn pump ON (SH1106 only)
        SH1106_PUMP_OFF                             = 0x8A          ///< Turn pump OFF (SH1106 only)
    };

    /**
     * @brief       Available voltage generation methods.
     */
    enum class voltageSrc_t : uint_fast8_t
    {
        V_EXTERNAL = (uint_fast8_t)command_t::EXTERNALVCC,          ///< External VCC supplied
        V_INTERNAL = (uint_fast8_t)command_t::SWITCHCAPVCC          ///< Internal charge pump enabled
    };

    /**
     * @brief       State of a single pixel.
     */
    enum class pixel_t : uint_fast8_t
    {
        OFF     = 0x00,                                             ///< Draw 'off' (background) pixel
        ON      = 0x01,                                             ///< Draw 'on' (foreground) pixel
        INVERSE = 0x02                                              ///< Invert current pixel state
    };

    /**
     * @brief       Available scroll directions.
     */
    enum class scroll_dir_t : uint_fast8_t
    {
        RIGHT,                                                      ///< Horizontal right
        LEFT,                                                       ///< Horizontal left
        DIAG_RIGHT,                                                 ///< Diagonal right
        DIAG_LEFT,                                                  ///< Diagonal left
        UP,                                                         ///< Vertical up (SSD1315 only)
        DOWN                                                        ///< Vertical down (SSD1315 only)
    };

//*******************************************************************
//*                                                                 *
//*                        Hardware Constants                       *
//*                                                                 *
//*******************************************************************

    /**
     * @brief       Global configuration constants and initialization data.
     */
    namespace Constants
    {
        /** @brief I2C command control byte indicator. */
        constexpr uint8_t I2C_CONTROL_BYTE_CMD      {0x00};

        /** @brief I2C data control byte indicator. */
        constexpr uint8_t I2C_CONTROL_BYTE_DATA     {0x40};

        /** @brief Default I2C address for SSD1306 (typically 0x3C). */
        constexpr uint8_t I2C_ADDR_3C               {0x3C};

        /** @brief Alternative I2C address for SSD1306 (typically 0x3D). */
        constexpr uint8_t I2C_ADDR_3D               {0x3D};

        /** @brief Bitmask for the Most Significant Bit. */
        constexpr uint8_t BIT_MASK_MSB              {0x80};

        /** @brief Value representing an unassigned or unused pin. */
        constexpr int_fast8_t PIN_NONE              {-1};

        // Contrast constants
        constexpr uint8_t CONTRAST_EXT_16           {0x10};         ///< Contrast for 16px high display (external VCC)
        constexpr uint8_t CONTRAST_INT_16           {0xAF};         ///< Contrast for 16px high display (internal VCC)
        constexpr uint8_t CONTRAST_ALL_32           {0x8F};         ///< Contrast for 32px high display
        constexpr uint8_t CONTRAST_ALL_40           {0x8F};         ///< Contrast for 40px high display
        constexpr uint8_t CONTRAST_ALL_48           {0x8F};         ///< Contrast for 48px high display
        constexpr uint8_t CONTRAST_INT_64           {0xCF};         ///< Contrast for 64px high display (internal VCC)
        constexpr uint8_t CONTRAST_INT_64_SSD1306   {0xFF};         ///< Max contrast for 64px SSD1306 (internal VCC)
        constexpr uint8_t CONTRAST_EXT_64           {0x9F};         ///< Contrast for 64px high display (external VCC)
        constexpr uint8_t CONTRAST_ALL_128          {0x8F};         ///< Contrast for 128px high display
        constexpr uint8_t CONTRAST_DEFAULT          {0x7F};         ///< Default fall-back contrast
        constexpr uint8_t CONTRAST_SH1106           {0x80};         ///< Default contrast for SH1106 and CH1115 displays

        // Init delays
        /** @brief Short reset delay duration in milliseconds. */
        constexpr int_fast16_t RESET_DELAY_MS_SHORT {1};

        /** @brief Long reset delay duration in milliseconds. */
        constexpr int_fast16_t RESET_DELAY_MS_LONG  {10};

        // Display configuration values
        constexpr uint8_t INTERNAL_PRECHARGE        {0xF1};         ///< Precharge period for internal VCC
        constexpr uint8_t EXTERNAL_PRECHARGE        {0x22};         ///< Precharge period for external VCC
        constexpr uint8_t CHARGE_PUMP_INT           {0x14};         ///< Charge pump setting for internal VCC
        constexpr uint8_t CHARGE_PUMP_EXT           {0x10};         ///< Charge pump setting for external VCC

        constexpr uint8_t COMPINS_SMALL             {0x02};         ///< COM pins configuration for small height displays
        constexpr uint8_t COMPINS_LARGE             {0x12};         ///< COM pins configuration for large height displays
        constexpr uint8_t VCOMDETECT_SMALL          {0x20};         ///< VCOM detect level for small height displays
        constexpr uint8_t VCOMDETECT_LARGE          {0x40};         ///< VCOM detect level for large height displays

        // Scroll Constants
        constexpr uint8_t SCROLL_DUMMY_NONE         {0x00};         //< Dummy byte for disabling scroll modifier
        constexpr uint8_t SCROLL_DUMMY_ENABLE_X     {0x01};         ///< Dummy byte for enabling scroll modifier
        constexpr uint8_t SCROLL_DUMMY_FF           {0xFF};         ///< Dummy byte FF
        constexpr uint8_t SCROLL_OFFSET_1           {0x01};         ///< Scroll offset value 1
        constexpr uint8_t SCROLL_OFFSET_63          {0x3F};         ///< Scroll offset value 63 (downwards by 1)

        // Initialization specific parameters
        constexpr uint8_t CLOCKDIV_DEFAULT          {0x80};         ///< Default clock divide ratio
        constexpr uint8_t OFFSET_DEFAULT            {0x00};         ///< Default display offset
        constexpr uint8_t SEGREMAP_ENABLE           {0x01};         ///< Segment remap flag enable
        constexpr uint8_t MEMORYMODE_HORIZ          {0x00};         ///< Horizontal memory addressing mode
        constexpr uint8_t ADDR_START                {0x00};         ///< Zero index start for page/column bounding
        constexpr uint8_t SH1106_PUMP_VOLT_80V      {0x02};         ///< SH1106 Charge pump 8.0V configuration
        constexpr uint8_t OFFSET_CH1115             {0x08};         ///< Initial row offset for CH1115
        constexpr uint8_t CH1115_OFFSET_DEFAULT     {0x04};         ///< Default column offset for CH1115

        /**
         * @brief Common initialization sequence sent to the display controller upon startup.
         */
        static const uint8_t PROGMEM initConst[]
        {
            (uint8_t)command_t::DISPLAYOFF,
            (uint8_t)command_t::SETDISPLAYCLOCKDIV,    CLOCKDIV_DEFAULT,
            (uint8_t)command_t::SETDISPLAYOFFSET,      OFFSET_DEFAULT,
            (uint8_t)command_t::SEGREMAP | SEGREMAP_ENABLE,
            (uint8_t)command_t::COMSCANDEC,
            (uint8_t)command_t::DISPLAYALLON_RESUME,
            (uint8_t)command_t::NORMALDISPLAY,
        };
    }
}
