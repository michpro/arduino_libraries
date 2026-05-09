/**
 * @file        OLED_SSD1306.cpp
 * @brief       Arduino library for monochrome OLED displays based on SSD1306.
 *
 * @copyright   SPDX-FileCopyrightText: Copyright 2026 Michal Protasowicki
 *              Based on Adafruit SSD1306 Library, Copyright (c) 2012 Adafruit Industries
 * @license     SPDX-License-Identifier: MIT
 */

//*******************************************************************
//*                                                                 *
//*                            Includes                             *
//*                                                                 *
//*******************************************************************

#ifdef __AVR__
#include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
#   include <pgmspace.h>
#else
#   define pgm_read_byte(addr) (*(const unsigned char *)(addr))    // PROGMEM workaround for non-AVR
#endif

#if !defined(__ARM_ARCH) && !defined(ENERGIA) && !defined(ESP8266) && !defined(ESP32) && !defined(__arc__)
#   include <util/delay.h>
#endif

#include "OLED_SSD1306.h"
#include "gfx/gfx.h"

//*******************************************************************
//*                                                                 *
//*                            Macros                               *
//*                                                                 *
//*******************************************************************

#if defined(I2C_BUFFER_LENGTH)
#   define WIRE_MAX min(256, I2C_BUFFER_LENGTH)                     // Particle or similar Wire lib
#elif defined(BUFFER_LENGTH)
#   define WIRE_MAX min(256, BUFFER_LENGTH)                         // AVR or similar Wire lib
#elif defined(SERIAL_BUFFER_SIZE)
#   define WIRE_MAX min(255, SERIAL_BUFFER_SIZE - 1)                // Newer Wire uses RingBuffer
#else
#   define WIRE_MAX 32                                              // Use common Arduino core default
#endif

#define SSD1306_SWAP(a, b) (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b)))   // No-temp-var swap operation

#ifdef HAVE_PORTREG
#   define SSD1306_SELECT          *_csPort &= ~_csPinMask
#   define SSD1306_DESELECT        *_csPort |= _csPinMask
#   define SSD1306_MODE_COMMAND    *_dcPort &= ~_dcPinMask
#   define SSD1306_MODE_DATA       *_dcPort |= _dcPinMask
#else
#   define SSD1306_SELECT          digitalWrite(_csPin, LOW)
#   define SSD1306_DESELECT        digitalWrite(_csPin, HIGH)
#   define SSD1306_MODE_COMMAND    digitalWrite(_dcPin, LOW)
#   define SSD1306_MODE_DATA       digitalWrite(_dcPin, HIGH)
#endif

#if defined(SPI_HAS_TRANSACTION)
#   define SPI_TRANSACTION_START   _spi->beginTransaction(_spiSettings)
#   define SPI_TRANSACTION_END     _spi->endTransaction()
#else
#   define SPI_TRANSACTION_START
#   define SPI_TRANSACTION_END
#endif

#define TRANSACTION_START                                           \
    if (nullptr == _wire)                                           \
    {                                                               \
        if (nullptr != _spi)                                        \
        {                                                           \
            SPI_TRANSACTION_START;                                  \
        }                                                           \
        SSD1306_SELECT;                                             \
    }
#define TRANSACTION_END                                             \
    if (nullptr == _wire)                                           \
    {                                                               \
        SSD1306_DESELECT;                                           \
        if (nullptr != _spi)                                        \
        {                                                           \
            SPI_TRANSACTION_END;                                    \
        }                                                           \
    }


//*******************************************************************
//*                                                                 *
//*                        class implementation                     *
//*                                                                 *
//*******************************************************************

namespace OLED
{

SSD1306::SSD1306(display_t display, TwoWire *wire, int_fast8_t rstPin)
    : GFX(0, 0), _spi(nullptr), _wire(wire ? wire : &Wire), _buffer(nullptr), _i2cAddr(0x00), _vccState(0x00), _mosiPin(-1), _clkPin(-1), _dcPin(-1), _csPin(-1), _rstPin(rstPin), _contrast(0x00)
{
    reinitGFX(display);
}

SSD1306::SSD1306(display_t display, int_fast8_t mosiPin, int_fast8_t sclkPin, int_fast8_t dcPin, int_fast8_t rstPin, int_fast8_t csPin)
    : GFX(0, 0), _spi(nullptr), _wire(nullptr), _buffer(nullptr), _i2cAddr(0x00), _vccState(0x00), _mosiPin(mosiPin), _clkPin(sclkPin), _dcPin(dcPin), _csPin(csPin), _rstPin(rstPin), _contrast(0x00)
{
    reinitGFX(display);
}

SSD1306::SSD1306(display_t display, SPIClass *spi_ptr, int_fast8_t dcPin, int_fast8_t rstPin, int_fast8_t csPin, uint32_t bitrate)
    : GFX(0, 0), _spi(spi_ptr ? spi_ptr : &SPI), _wire(nullptr), _buffer(nullptr), _i2cAddr(0x00), _vccState(0x00), _mosiPin(-1), _clkPin(-1), _dcPin(dcPin), _csPin(csPin), _rstPin(rstPin), _contrast(0x00)
{
    reinitGFX(display);
#ifdef SPI_HAS_TRANSACTION
    _spiSettings = SPISettings(bitrate, MSBFIRST, SPI_MODE0);
#endif
}

SSD1306::~SSD1306(void)
{
    if (nullptr != _buffer)
    {
        free(_buffer);
        _buffer = nullptr;
    }
}

inline void SSD1306::reinitGFX(display_t display)
{
    _device = DEV_SSD1306;
    _colOffset = 0;

    switch (display)
    {
        case display_t::SSD1315_128x64:
            _baseWidth = 128;
            _baseHeight = 64;
            _device = DEV_SSD1315;
            break;
        case display_t::SH1106_128x64:
            _colOffset = 2;
        case display_t::SH1107_128x64:
            _baseWidth = 128;
            _baseHeight = 64;
            _device = DEV_SH1106;
            break;
        case display_t::SSD1306_128x32:
            _baseWidth = 128;
            _baseHeight = 32;
            break;
        case display_t::SSD1306_96x16:
            _baseWidth = 96;
            _baseHeight = 16;
            break;
        case display_t::SSD1306_72x40:
            _baseWidth = 72;
            _baseHeight = 40;
            _colOffset = 28;
            break;
        case display_t::SSD1306_64x48:
            _baseWidth = 64;
            _baseHeight = 48;
            _colOffset = 32;
            break;
        case display_t::SSD1306_64x32:
            _baseWidth = 64;
            _baseHeight = 32;
            _colOffset = 32;
            break;
        case display_t::CH1115_48x88:
            _baseWidth = 88;
            _baseHeight = 48;
            _device = DEV_CH1115;
            _colOffset = Constants::CH1115_OFFSET_DEFAULT;
            _rotation = 1;
            break;
        case display_t::SSD1312_64x128:
            _baseWidth = 128;
            _baseHeight = 64;
            _device = DEV_SSD1312;
            _rotation = 3;
            break;
        case display_t::SSD1306_128x64:
        case display_t::SSD1309_128x64:
        default:
            _baseWidth = 128;
            _baseHeight = 64;
            break;
    }

    if (0 != (_rotation % 2))
    {
        _width = _baseHeight;
        _height = _baseWidth;
    } else
    {
        _width = _baseWidth;
        _height = _baseHeight;
    }
}

inline void SSD1306::setContrastValue(device_t device, bool isIntVcc)
{
    if ((DEV_SSD1306 == device) || (DEV_SSD1315 == device) || (DEV_SSD1312 == device))
    {
        switch (_baseHeight)
        {
        case 16:
            _contrast = isIntVcc ? Constants::CONTRAST_INT_16 : Constants::CONTRAST_EXT_16;
            break;
        case 32:
            _contrast = Constants::CONTRAST_ALL_32;
            break;
        case 40:
            _contrast = Constants::CONTRAST_ALL_40;
            break;
        case 48:
            _contrast = Constants::CONTRAST_ALL_48;
            break;
        case 64:
            _contrast = isIntVcc ? ((DEV_SSD1306 == device) ? Constants::CONTRAST_INT_64_SSD1306 : Constants::CONTRAST_INT_64) : Constants::CONTRAST_EXT_64;
            break;
        case 88:
        case 128:
            _contrast = Constants::CONTRAST_ALL_128;
            break;
        default:
            _contrast = Constants::CONTRAST_DEFAULT;
            break;
        }
    } else
    {
        // For SH1106 and CH1115
        _contrast = Constants::CONTRAST_SH1106;
    }
}

//*******************************************************************
//*                                                                 *
//*                       protected methods                         *
//*                                                                 *
//*******************************************************************

inline void SSD1306::SPIwrite(uint_fast8_t dataByte)
{
    if (nullptr != _spi)
    {
        (void)_spi->transfer(dataByte);
    } else
    {
        for (uint_fast8_t bit = Constants::BIT_MASK_MSB; 0 != bit; bit >>= 1)
        {
        #ifdef HAVE_PORTREG
            if (0 != (dataByte & bit))
            {
                *_mosiPort |= _mosiPinMask;
            } else
            {
                *_mosiPort &= ~_mosiPinMask;
            }
            *_clkPort |= _clkPinMask;                               // Clock high
            *_clkPort &= ~_clkPinMask;                              // Clock low
        #else
            digitalWrite(_mosiPin, (0 != (dataByte & bit)) ? HIGH : LOW);
            digitalWrite(_clkPin, HIGH);
            digitalWrite(_clkPin, LOW);
        #endif
        }
    }
}

void SSD1306::sendOneCommand(uint_fast8_t command)
{
    if (nullptr != _wire)
    {
        _wire->beginTransmission(_i2cAddr);
        _wire->write(Constants::I2C_CONTROL_BYTE_CMD);
        _wire->write(command);
        _wire->endTransmission();
    } else
    {
        SSD1306_MODE_COMMAND;
        SPIwrite(command);
    }
}

void SSD1306::sendCommandList(const uint8_t *commandsArray, uint_fast8_t count, bool isProgMem)
{
    if (nullptr != _wire)
    {
        uint_fast16_t bytesOut {1};

        _wire->beginTransmission(_i2cAddr);
        _wire->write(Constants::I2C_CONTROL_BYTE_CMD);

        while (0 != count--)
        {
            if (bytesOut >= WIRE_MAX)
            {
                _wire->endTransmission();
                _wire->beginTransmission(_i2cAddr);
                _wire->write(Constants::I2C_CONTROL_BYTE_CMD);
                bytesOut = 1;
            }
            _wire->write(isProgMem ? pgm_read_byte(commandsArray) : *commandsArray);
            commandsArray++;
            bytesOut++;
        }
        _wire->endTransmission();
    } else
    {
        SSD1306_MODE_COMMAND;
        while (0 != count--)
        {
            SPIwrite(isProgMem ? pgm_read_byte(commandsArray) : *commandsArray);
            commandsArray++;
        }
    }
}

void SSD1306::sendDataPayload(const uint8_t *data, uint_fast16_t count)
{
    if (nullptr != _wire)
    {
        uint_fast16_t bytesOut {1};

        _wire->beginTransmission(_i2cAddr);
        _wire->write((uint8_t)0x40);

        while (0 != count--)
        {
            if (bytesOut >= WIRE_MAX)
            {
                _wire->endTransmission();
                _wire->beginTransmission(_i2cAddr);
                _wire->write((uint8_t)0x40);
                bytesOut = 1;
            }
            _wire->write(nullptr != data ? *data++ : 0x00);
            bytesOut++;
        }
        _wire->endTransmission();
    } else
    {
        SSD1306_MODE_DATA;
        while (0 != count--)
        {
            SPIwrite(nullptr != data ? *data++ : 0x00);
        }
    }
}

//*******************************************************************
//*                                                                 *
//*                        public methods                           *
//*                                                                 *
//*******************************************************************

void SSD1306::sendCommand(uint_fast8_t c)
{
    TRANSACTION_START
    sendOneCommand(c);
    TRANSACTION_END
}

bool SSD1306::begin(voltageSrc_t vcc, uint_fast8_t addr, bool reset, bool periphBegin)
{
    bool result {true};

    if (nullptr == _buffer)
    {
        _buffer = (uint8_t *)malloc(_baseWidth * ((_baseHeight + 7) / 8));
        if (nullptr == _buffer)
        {
            result = false;
        }
    }

    if (true == result)
    {
        clearDisplay();

        _vccState = (uint_fast8_t)vcc;
        bool isIntVcc {voltageSrc_t::V_INTERNAL == vcc};

        if (nullptr != _wire)
        {
            _i2cAddr = (0 != addr) ? addr : ((32 == _baseHeight || 16 == _baseHeight || 40 == _baseHeight) ? 0x3C : 0x3D);
            if (true == periphBegin)
            {
                _wire->begin();
            }
        } else
        {
            pinMode(_dcPin, OUTPUT);
            pinMode(_csPin, OUTPUT);
        #ifdef HAVE_PORTREG
            _dcPort = (PortReg *)portOutputRegister(digitalPinToPort(_dcPin));
            _dcPinMask = digitalPinToBitMask(_dcPin);
            _csPort = (PortReg *)portOutputRegister(digitalPinToPort(_csPin));
            _csPinMask = digitalPinToBitMask(_csPin);
        #endif
            SSD1306_DESELECT;
            if (nullptr != _spi)
            {
                if (true == periphBegin)
                {
                    _spi->begin();
                }
            } else
            {
                pinMode(_mosiPin, OUTPUT);
                pinMode(_clkPin, OUTPUT);
            #ifdef HAVE_PORTREG
                _mosiPort = (PortReg *)portOutputRegister(digitalPinToPort(_mosiPin));
                _mosiPinMask = digitalPinToBitMask(_mosiPin);
                _clkPort = (PortReg *)portOutputRegister(digitalPinToPort(_clkPin));
                _clkPinMask = digitalPinToBitMask(_clkPin);
                *_clkPort &= ~_clkPinMask;
            #else
                digitalWrite(_clkPin, LOW);
            #endif
            }
        }

        if ((true == reset) && (_rstPin >= 0))
        {
            pinMode(_rstPin, OUTPUT);
            digitalWrite(_rstPin, HIGH);
            delay(1);
            digitalWrite(_rstPin, LOW);
            delay(10);
            digitalWrite(_rstPin, HIGH);
        }

        TRANSACTION_START

        bool isPageAddrDevice     {((DEV_SH1106 == _device) || (DEV_CH1115 == _device))};
        bool isSH1106ChargePump   {(DEV_SH1106 == _device)};

        uint8_t cmdStart        {(uint8_t)command_t::SETSTARTLINE};
        uint8_t valPrecharge    {(uint8_t)(isIntVcc ? Constants::INTERNAL_PRECHARGE : Constants::EXTERNAL_PRECHARGE)};
        uint8_t cmdChargePumpA  {(uint8_t)(!isSH1106ChargePump ? (uint8_t)command_t::CHARGEPUMP : (isIntVcc ? (uint8_t)command_t::SH1106_SET_PUMP_MODE : (uint8_t)command_t::NOP))};
        uint8_t valChargePump   {(uint8_t)(isIntVcc ? Constants::CHARGE_PUMP_INT : Constants::CHARGE_PUMP_EXT)};
        uint8_t cmdChargePumpB  {(uint8_t)(!isSH1106ChargePump ? valChargePump : (uint8_t)command_t::SH1106_PUMP_ON)};
        uint8_t cmdChargePumpC  {(uint8_t)(!isSH1106ChargePump ? (uint8_t)command_t::NOP : (isIntVcc ? ((uint8_t)command_t::SH1106_SET_PUMP_VOLTAGE | Constants::SH1106_PUMP_VOLT_80V) : (uint8_t)command_t::NOP))};
        uint8_t cmdMemoryMode   {(uint8_t)(!isPageAddrDevice ? (uint8_t)command_t::MEMORYMODE : (uint8_t)command_t::NOP)};
        uint8_t valMemoryMode   {(uint8_t)(!isPageAddrDevice ? Constants::MEMORYMODE_HORIZ : (uint8_t)command_t::NOP)};
        uint8_t cmdScroll       {(uint8_t)(!isPageAddrDevice ? (uint8_t)command_t::DEACTIVATE_SCROLL : (uint8_t)command_t::NOP)};
        uint8_t valComPins      {(uint8_t)((((_baseHeight <= 32) && (64 != _baseWidth)) || (DEV_CH1115 == _device)) ? Constants::COMPINS_SMALL : Constants::COMPINS_LARGE)};
        uint8_t cmdComScan      {(uint8_t)(DEV_SSD1312 == _device ? (uint8_t)command_t::COMSCANINC : (uint8_t)command_t::COMSCANDEC)};

        uint8_t valMux          {(uint8_t)(_baseHeight - 1)};
        uint8_t valOffset       {(uint8_t)((DEV_CH1115 == _device) ? Constants::OFFSET_CH1115 : Constants::OFFSET_DEFAULT)};

        setContrastValue(_device, isIntVcc);

        uint8_t initMut[]
        {
            (uint8_t)command_t::SETMULTIPLEX,     valMux,
            (uint8_t)command_t::SETDISPLAYOFFSET, valOffset,
            cmdStart,
            (uint8_t)command_t::SETPRECHARGE,   valPrecharge,
            cmdChargePumpA,
            cmdChargePumpB,
            cmdChargePumpC,
            cmdMemoryMode,                      valMemoryMode,
            (uint8_t)command_t::SETCOMPINS,     valComPins,
            (uint8_t)command_t::SETCONTRAST,    (uint8_t)_contrast,
            (uint8_t)command_t::SETVCOMDETECT,  (uint8_t)((_baseHeight > 16) ? Constants::VCOMDETECT_LARGE : Constants::VCOMDETECT_SMALL),
            cmdScroll,
            cmdComScan,
            (uint8_t)command_t::DISPLAYON
        };

        sendCommandList(Constants::initConst, sizeof(Constants::initConst));

        sendCommandList(initMut, sizeof(initMut), false);

        // Clear all internal hardware memory to prevent garbage at edges during scrolling
        uint_fast8_t pages  {(uint_fast8_t)((_baseHeight + 7) / 8)};
        if (isPageAddrDevice)
        {
            // Always clear all 8 hardware pages to prevent random garbage on displays with offset mapping
            for (uint_fast8_t p = 0; p < 8; ++p)
            {
                sendOneCommand((uint8_t)command_t::SETSTARTPAGE | p);
                sendOneCommand((uint8_t)command_t::SETLOWCOLUMN | 0);
                sendOneCommand((uint8_t)command_t::SETHIGHCOLUMN | 0);

                sendDataPayload(nullptr, 132);
            }
        }
        else
        {
            uint8_t start_page {Constants::ADDR_START};
            uint8_t end_page {(uint8_t)(pages - 1)};
            uint8_t bounds[]
            {
                (uint8_t)command_t::PAGEADDR,   start_page, end_page,
                (uint8_t)command_t::COLUMNADDR, Constants::ADDR_START, (uint8_t)((_baseWidth <= 128) ? 127 : (_baseWidth - 1))
            };
            sendCommandList(bounds, sizeof(bounds), false);
            for (uint_fast8_t p = 0; p < pages; ++p)
            {
                sendDataPayload(nullptr, 128);
            }
        }

        TRANSACTION_END
    }

    return result;
}

void SSD1306::drawPixel(int_fast16_t x, int_fast16_t y, uint_fast8_t color)
{
    if ((x >= 0) && (x < _width) && (y >= 0) && (y < _height))
    {
        switch (_rotation)
        {
            case 1:
                SSD1306_SWAP(x, y);
                x = _baseWidth - x - 1;
                break;
            case 2:
                x = _baseWidth - x - 1;
                y = _baseHeight - y - 1;
                break;
            case 3:
                SSD1306_SWAP(x, y);
                y = _baseHeight - y - 1;
                break;
        }
        switch (color)
        {
            case (uint_fast8_t)pixel_t::ON:
                _buffer[x + ((uint_fast16_t)y >> 3) * _baseWidth] |= (1 << (y & 7));
                break;
            case (uint_fast8_t)pixel_t::OFF:
                _buffer[x + ((uint_fast16_t)y >> 3) * _baseWidth] &= ~(1 << (y & 7));
                break;
            case (uint_fast8_t)pixel_t::INVERSE:
                _buffer[x + ((uint_fast16_t)y >> 3) * _baseWidth] ^= (1 << (y & 7));
                break;
        }

        if (x < _windowX1) _windowX1 = x;
        if (x > _windowX2) _windowX2 = x;
        if (y < _windowY1) _windowY1 = y;
        if (y > _windowY2) _windowY2 = y;
    }
}

void SSD1306::clearDisplay(void)
{
    if (nullptr != _buffer)
    {
        memset(_buffer, 0, _baseWidth * ((_baseHeight + 7) / 8));
        _windowX1 = 0;
        _windowY1 = 0;
        _windowX2 = _baseWidth - 1;
        _windowY2 = _baseHeight - 1;
    }
}

void SSD1306::drawFastHLine(int_fast16_t x, int_fast16_t y, int_fast16_t width, uint_fast8_t color)
{
    bool bSwap {false};

    switch (_rotation)
    {
        case 1:
            bSwap = true;
            SSD1306_SWAP(x, y);
            x = _baseWidth - x - 1;
            break;
        case 2:
            x = _baseWidth - x - 1;
            y = _baseHeight - y - 1;
            x -= (width - 1);
            break;
        case 3:
            bSwap = true;
            SSD1306_SWAP(x, y);
            y = _baseHeight - y - 1;
            y -= (width - 1);
            break;
    }

    if (true == bSwap)
    {
        drawFastVLineInternal(x, y, width, color);
    } else
    {
        drawFastHLineInternal(x, y, width, color);
    }
}

void SSD1306::drawFastHLineInternal(int_fast16_t x, int_fast16_t y, int_fast16_t width, uint_fast8_t color)
{
    if ((y >= 0) && (y < _baseHeight))
    {
        if (0 > x)
        {
            width += x;
            x = 0;
        }
        if ((x + width) > _baseWidth)
        {
            width = (_baseWidth - x);
        }
        if (0 < width)
        {
            uint8_t        *pBuf    {&_buffer[((uint_fast16_t)y >> 3) * _baseWidth + x]};
            uint_fast8_t    mask    {(uint_fast8_t)(1 << (y & 7))};

            if (x < _windowX1) _windowX1 = x;
            if ((x + width - 1) > _windowX2) _windowX2 = x + width - 1;
            if (y < _windowY1) _windowY1 = y;
            if (y > _windowY2) _windowY2 = y;

            switch (color)
            {
                case (uint_fast8_t)pixel_t::ON:
                    while (0 != width--)
                    {
                        *pBuf++ |= mask;
                    }
                    break;
                case (uint_fast8_t)pixel_t::OFF:
                    mask = ~mask;
                    while (0 != width--)
                    {
                        *pBuf++ &= mask;
                    }
                    break;
                case (uint_fast8_t)pixel_t::INVERSE:
                    while (0 != width--)
                    {
                        *pBuf++ ^= mask;
                    }
                    break;
            }
        }
    }
}

void SSD1306::drawFastVLine(int_fast16_t x, int_fast16_t y, int_fast16_t height, uint_fast8_t color)
{
    bool bSwap {false};

    switch (_rotation)
    {
        case 1:
            bSwap = true;
            SSD1306_SWAP(x, y);
            x = _baseWidth - x - 1;
            x -= (height - 1);
            break;
        case 2:
            x = _baseWidth - x - 1;
            y = _baseHeight - y - 1;
            y -= (height - 1);
            break;
        case 3:
            bSwap = true;
            SSD1306_SWAP(x, y);
            y = _baseHeight - y - 1;
            break;
    }

    if (true == bSwap)
    {
        drawFastHLineInternal(x, y, height, color);
    } else
    {
        drawFastVLineInternal(x, y, height, color);
    }
}

void SSD1306::drawFastVLineInternal(int_fast16_t x, int_fast16_t yPos, int_fast16_t rawHeight, uint_fast8_t color)
{
    if ((x >= 0) && (x < _baseWidth))
    {
        if (0 > yPos)
        {
            rawHeight += yPos;
            yPos = 0;
        }
        if ((yPos + rawHeight) > _baseHeight)
        {
            rawHeight = (_baseHeight - yPos);
        }
        if (0 < rawHeight)
        {
            uint_fast8_t    y       {(uint_fast8_t)yPos};
            uint_fast8_t    h       {(uint_fast8_t)rawHeight};
            uint8_t        *pBuf    {&_buffer[(y >> 3) * _baseWidth + x]};
            uint_fast8_t    mod     {(uint_fast8_t)(y & 7)};

            if (x < _windowX1) _windowX1 = x;
            if (x > _windowX2) _windowX2 = x;
            if (y < _windowY1) _windowY1 = y;
            if ((y + h - 1) > _windowY2) _windowY2 = y + h - 1;

            if (0 != mod)
            {
                mod = 8 - mod;
                static const uint8_t PROGMEM    premask[8]      {0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE};
                uint_fast8_t                    mask            {pgm_read_byte(&premask[mod])};

                if (h < mod)
                {
                    mask &= (0xFF >> (mod - h));
                }

                switch (color)
                {
                    case (uint_fast8_t)pixel_t::ON:
                        *pBuf |= mask;
                        break;
                    case (uint_fast8_t)pixel_t::OFF:
                        *pBuf &= ~mask;
                        break;
                    case (uint_fast8_t)pixel_t::INVERSE:
                        *pBuf ^= mask;
                        break;
                }
                pBuf += _baseWidth;
            }

            if (mod <= h)
            {
                h -= mod;
                if (8 <= h)
                {
                    if ((uint_fast8_t)pixel_t::INVERSE == color)
                    {
                        do
                        {
                            *pBuf ^= 0xFF;
                            pBuf += _baseWidth;
                            h -= 8;
                        } while (h >= 8);
                    } else
                    {
                        uint_fast8_t val {(uint_fast8_t)(((uint_fast8_t)pixel_t::OFF != color) ? 0xFF : 0x00)};
                        do
                        {
                            *pBuf = val;
                            pBuf += _baseWidth;
                            h -= 8;
                        } while (h >= 8);
                    }
                }

                if (0 != h)
                {
                    mod = h & 7;
                    static const uint8_t PROGMEM    postmask[8]     {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F};
                    uint_fast8_t                    mask            {pgm_read_byte(&postmask[mod])};

                    switch (color)
                    {
                        case (uint_fast8_t)pixel_t::ON:
                            *pBuf |= mask;
                            break;
                        case (uint_fast8_t)pixel_t::OFF:
                            *pBuf &= ~mask;
                            break;
                        case (uint_fast8_t)pixel_t::INVERSE:
                            *pBuf ^= mask;
                            break;
                    }
                }
            }
        }
    }
}

bool SSD1306::getPixel(int_fast16_t x, int_fast16_t y)
{
    bool result {false};

    if ((x >= 0) && (x < _width) && (y >= 0) && (y < _height))
    {
        switch (_rotation)
        {
            case 1:
                SSD1306_SWAP(x, y);
                x = _baseWidth - x - 1;
                break;
            case 2:
                x = _baseWidth - x - 1;
                y = _baseHeight - y - 1;
                break;
            case 3:
                SSD1306_SWAP(x, y);
                y = _baseHeight - y - 1;
                break;
        }
        result = (0 != (_buffer[x + ((uint_fast16_t)y >> 3) * _baseWidth] & (1 << (y & 7))));
    }

    return result;
}

uint8_t *SSD1306::getBuffer(void)
{
    return _buffer;
}

void SSD1306::display(void)
{
    if ((_windowX1 <= _windowX2) && (_windowY1 <= _windowY2))
    {
        uint_fast8_t start_page {(uint_fast8_t)(_windowY1 >> 3)};
        uint_fast8_t end_page   {(uint_fast8_t)(_windowY2 >> 3)};
        uint_fast8_t start_col  {(uint_fast8_t)(_windowX1 + _colOffset)};
        uint_fast8_t end_col    {(uint_fast8_t)(_windowX2 + _colOffset)};

        TRANSACTION_START

        if ((DEV_SH1106 == _device) || (DEV_CH1115 == _device))
        {
            // SH1106 Page Addressing Fallback
            for (uint_fast8_t p = start_page; p <= end_page; ++p)
            {
                sendOneCommand((uint8_t)command_t::SETSTARTPAGE | p);
                sendOneCommand((uint8_t)command_t::SETLOWCOLUMN | (start_col & 0x0F));
                sendOneCommand((uint8_t)command_t::SETHIGHCOLUMN | (start_col >> 4));

                uint8_t      *ptr      {&_buffer[p * _baseWidth + _windowX1]};
                uint_fast8_t  cols     {(uint_fast8_t)((_windowX2 - _windowX1) + 1)};

                sendDataPayload(ptr, cols);
            }
        }
        else
        {
            // SSD1306 Horizontal / Bounding Box Optimized Mode
            uint8_t bounds[]
            {
                (uint8_t)command_t::PAGEADDR, start_page, end_page,
                (uint8_t)command_t::COLUMNADDR, start_col, end_col
            };
            sendCommandList(bounds, sizeof(bounds), false);

        #if defined(ESP8266)
            yield();
        #endif

            for (uint_fast8_t p = start_page; p <= end_page; ++p)
            {
                uint8_t      *ptr  {&_buffer[p * _baseWidth + _windowX1]};
                uint_fast8_t  cols {(uint_fast8_t)((_windowX2 - _windowX1) + 1)};

                sendDataPayload(ptr, cols);
            }
        }

        TRANSACTION_END

        // Reset display bounding box dimensions to maximum limits
        _windowX1 = 255;
        _windowY1 = 255;
        _windowX2 = 0;
        _windowY2 = 0;

    #if defined(ESP8266)
        yield();
    #endif
    }
}

void SSD1306::startScroll(scroll_dir_t dir, uint_fast8_t startPage, uint_fast8_t stopPage)
{
    if ((DEV_SH1106 != _device) && (DEV_CH1115 != _device))
    {
        TRANSACTION_START
        sendOneCommand((uint8_t)command_t::DEACTIVATE_SCROLL);

        uint8_t cmds[9];
        uint8_t len {0};

        switch (dir)
        {
            case scroll_dir_t::RIGHT:
            case scroll_dir_t::LEFT:
                cmds[0] = (dir == scroll_dir_t::RIGHT) ? (uint8_t)command_t::RIGHT_HORIZONTAL_SCROLL : (uint8_t)command_t::LEFT_HORIZONTAL_SCROLL;
                cmds[1] = Constants::SCROLL_DUMMY_NONE;
                cmds[2] = (uint8_t)startPage;
                cmds[3] = Constants::SCROLL_DUMMY_NONE;
                cmds[4] = (uint8_t)stopPage;
                cmds[5] = Constants::SCROLL_DUMMY_NONE;
                cmds[6] = Constants::SCROLL_DUMMY_FF;
                len = 7;
                break;
            case scroll_dir_t::DIAG_RIGHT:
            case scroll_dir_t::DIAG_LEFT:
                if (DEV_SSD1315 == _device)
                {
                    cmds[0] = (dir == scroll_dir_t::DIAG_RIGHT) ? (uint8_t)command_t::VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL : (uint8_t)command_t::VERTICAL_AND_LEFT_HORIZONTAL_SCROLL;
                    cmds[1] = Constants::SCROLL_DUMMY_ENABLE_X;
                    cmds[2] = (uint8_t)startPage;
                    cmds[3] = Constants::SCROLL_DUMMY_NONE;
                    cmds[4] = (uint8_t)stopPage;
                    cmds[5] = Constants::SCROLL_OFFSET_1;
                    cmds[6] = Constants::SCROLL_DUMMY_NONE;
                    cmds[7] = (uint8_t)(_baseWidth - 1);
                    len = 8;
                }
                else
                {
                    cmds[0] = (uint8_t)command_t::SET_VERTICAL_SCROLL_AREA;
                    cmds[1] = Constants::SCROLL_DUMMY_NONE;
                    cmds[2] = (uint8_t)_baseHeight;
                    cmds[3] = (dir == scroll_dir_t::DIAG_RIGHT) ? (uint8_t)command_t::VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL : (uint8_t)command_t::VERTICAL_AND_LEFT_HORIZONTAL_SCROLL;
                    cmds[4] = Constants::SCROLL_DUMMY_NONE;
                    cmds[5] = (uint8_t)startPage;
                    cmds[6] = Constants::SCROLL_DUMMY_NONE;
                    cmds[7] = (uint8_t)stopPage;
                    cmds[8] = Constants::SCROLL_OFFSET_1;
                    len = 9;
                }
                break;
            case scroll_dir_t::UP:
            case scroll_dir_t::DOWN:
                if (DEV_SSD1315 == _device)
                {
                    cmds[0] = (uint8_t)command_t::VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL;
                    cmds[1] = Constants::SCROLL_DUMMY_NONE;
                    cmds[2] = (uint8_t)startPage;
                    cmds[3] = Constants::SCROLL_DUMMY_NONE;
                    cmds[4] = (uint8_t)stopPage;
                    cmds[5] = (dir == scroll_dir_t::UP) ? Constants::SCROLL_OFFSET_1 : Constants::SCROLL_OFFSET_63;
                    cmds[6] = Constants::SCROLL_DUMMY_NONE;
                    cmds[7] = (uint8_t)(_baseWidth - 1);
                    len = 8;
                }
                break;
        }

        if (0 < len)
        {
            sendCommandList(cmds, len, false);
            sendOneCommand((uint8_t)command_t::ACTIVATE_SCROLL);
        }

        TRANSACTION_END
    }
}

void SSD1306::stopScroll(void)
{
    if ((DEV_SH1106 != _device) && (DEV_CH1115 != _device))
    {
        TRANSACTION_START
        sendOneCommand((uint8_t)command_t::DEACTIVATE_SCROLL);
        TRANSACTION_END
    }
}

void SSD1306::contentScrollRight(void)
{
    if (DEV_SSD1315 == _device)
    {
        TRANSACTION_START
        sendOneCommand((uint8_t)command_t::CONTENT_SCROLL_RIGHT);
        TRANSACTION_END
    }
}

void SSD1306::contentScrollLeft(void)
{
    if (DEV_SSD1315 == _device)
    {
        TRANSACTION_START
        sendOneCommand((uint8_t)command_t::CONTENT_SCROLL_LEFT);
        TRANSACTION_END
    }
}

void SSD1306::invertDisplay(bool invert)
{
    TRANSACTION_START
    sendOneCommand((true == invert) ? (uint8_t)command_t::INVERTDISPLAY : (uint8_t)command_t::NORMALDISPLAY);
    TRANSACTION_END
}

void SSD1306::dim(bool isDimmed)
{
    TRANSACTION_START
    sendOneCommand((uint8_t)command_t::SETCONTRAST);
    sendOneCommand((true == isDimmed) ? 0 : _contrast);
    TRANSACTION_END
}

} // namespace OLED
