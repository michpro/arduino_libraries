/**
 * @file        gfx.cpp
 * @brief       Core graphics library for OLED displays, providing a common set of graphics primitives.
 *
 * This is the core graphics library for OLED displays, providing a common
 * set of graphics primitives (points, lines, circles, etc.). It needs to be
 * paired with a hardware-specific library for each display device
 * (to handle the lower-level functions).
 *
 * @copyright   SPDX-FileCopyrightText: Copyright 2026 Michal Protasowicki
 *              Based on Adafruit GFX Library, Copyright (c) 2013 Adafruit Industries
 * @license     SPDX-License-Identifier: MIT
 */

//*******************************************************************
//*                                                                 *
//*                            Includes                             *
//*                                                                 *
//*******************************************************************

#include "gfx.h"
#include "glcdfont.h"

#ifdef __AVR__
#   include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
#   include <pgmspace.h>
#endif

// Many (but maybe not all) non-AVR board installs define macros
// for compatibility with existing PROGMEM-reading AVR code.
// Do our own checks and defines here for good measure...

#ifndef pgm_read_byte
#   define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif
#ifndef pgm_read_word
#   define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif
#ifndef pgm_read_dword
#   define pgm_read_dword(addr) (*(const unsigned long *)(addr))
#endif

// Pointers are a peculiar case...typically 16-bit on AVR boards, 32 bits elsewhere. Try to accommodate both...

#if !defined(__INT_MAX__) || (__INT_MAX__ > 0xFFFF)
#   define pgm_read_pointer(addr) ((void *)pgm_read_dword(addr))
#else
#   define pgm_read_pointer(addr) ((void *)pgm_read_word(addr))
#endif

inline GFXglyph *pgm_read_glyph_ptr(const GFXfont *gfxFont, uint_fast8_t c)
{
    GFXglyph *result {nullptr};
#ifdef __AVR__
    result = &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[c]);
#else
    // expression in __AVR__ section may generate "dereferencing type-punned pointer
    // will break strict-aliasing rules" warning. In fact, on other platforms (such as STM32)
    // there is no need to do this pointer magic as program memory may be read in a usual way
    // So expression may be simplified
    result = gfxFont->glyph + c;
#endif //__AVR__
    return result;
}

inline uint8_t *pgm_read_bitmap_ptr(const GFXfont *gfxFont)
{
    uint8_t *result {nullptr};
#ifdef __AVR__
    result = (uint8_t *)pgm_read_pointer(&gfxFont->bitmap);
#else
    // expression in __AVR__ section generates "dereferencing type-punned pointer
    // will break strict-aliasing rules" warning In fact, on other platforms (such as STM32)
    // there is no need to do this pointer magic as program memory may be read in a usual way
    // So expression may be simplified
    result = gfxFont->bitmap;
#endif //__AVR__
    return result;
}

#ifndef min
#   define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef GFX_SWAP_INT16
#   define GFX_SWAP_INT16(a, b)                                     \
    {                                                               \
        int_fast16_t t {a};                                         \
        a = b;                                                      \
        b = t;                                                      \
    }
#endif


//*******************************************************************
//*                                                                 *
//*                        class implementation                     *
//*                                                                 *
//*******************************************************************

/**
 * @brief Instatiate a GFX context for graphics! Can only be done by a superclass
 *
 * @param[in] width Display width, in pixels
 * @param[in] height Display height, in pixels
 */
GFX::GFX(int_fast16_t width, int_fast16_t height) : _baseWidth(width), _baseHeight(height)
{
    _width = _baseWidth;
    _height = _baseHeight;
    _rotation = 0;
    _cursorY = 0;
    _cursorX = 0;
    _textSizeX = 1;
    _textSizeY = 1;
    _textColor = 0xFFFF;
    _textBgColor = 0xFFFF;
    _wrap = true;
    _gfxFont = nullptr;
}

/**
 * @brief Write a line. (Bresenham's algorithm)
 *
 * @param[in] x0 Start point x coordinate
 * @param[in] y0 Start point y coordinate
 * @param[in] x1 End point x coordinate
 * @param[in] y1 End point y coordinate
 * @param[in] color 1-bit Color to draw with
 */
void GFX::writeLine(int_fast16_t x0, int_fast16_t y0, int_fast16_t x1, int_fast16_t y1, uint_fast8_t color)
{
#if defined(ESP8266)
    yield();
#endif
    int_fast16_t steep {abs(y1 - y0) > abs(x1 - x0)};
    if (0 != steep)
    {
        GFX_SWAP_INT16(x0, y0);
        GFX_SWAP_INT16(x1, y1);
    }

    if (x0 > x1)
    {
        GFX_SWAP_INT16(x0, x1);
        GFX_SWAP_INT16(y0, y1);
    }

    int_fast16_t dx     {x1 - x0};
    int_fast16_t dy     {abs(y1 - y0)};

    int_fast16_t err    {dx / 2};
    int_fast16_t ystep  {(y0 < y1) ? 1 : -1};

    for (; x0 <= x1; x0++)
    {
        if (0 != steep)
        {
            writePixel(y0, x0, color);
        } else
        {
            writePixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0)
        {
            y0 += ystep;
            err += dx;
        }
    }
}

/**
 * @brief Start a display-writing routine, overwrite in subclasses.
 */
void GFX::startWrite(void)
{
}

/**
 * @brief Write a pixel, overwrite in subclasses if startWrite is defined!
 *
 * @param[in] xPos x coordinate
 * @param[in] yPos y coordinate
 * @param[in] color 1-bit Color to fill with
 */
void GFX::writePixel(int_fast16_t xPos, int_fast16_t yPos, uint_fast8_t color)
{
    drawPixel(xPos, yPos, color);
}

/**
 * @brief Write a perfectly vertical line, overwrite in subclasses if startWrite is defined!
 *
 * @param[in] xPos Top-most x coordinate
 * @param[in] yPos Top-most y coordinate
 * @param[in] height Height in pixels
 * @param[in] color 1-bit Color to fill with
 */
void GFX::writeFastVLine(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t height, uint_fast8_t color)
{
    // Overwrite in subclasses if startWrite is defined!
    // Can be just writeLine(xPos, yPos, xPos, yPos+height-1, color); or writeFillRect(xPos, yPos, 1, height, color);
    drawFastVLine(xPos, yPos, height, color);
}

/**
 * @brief Write a perfectly horizontal line, overwrite in subclasses if startWrite is defined!
 *
 * @param[in] xPos Left-most x coordinate
 * @param[in] yPos Left-most y coordinate
 * @param[in] width Width in pixels
 * @param[in] color 1-bit Color to fill with
 */
void GFX::writeFastHLine(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t width, uint_fast8_t color)
{
    // Overwrite in subclasses if startWrite is defined!
    // Example: writeLine(xPos, yPos, xPos+width-1, yPos, color); or writeFillRect(xPos, yPos, width, 1, color);
    drawFastHLine(xPos, yPos, width, color);
}

/**
 * @brief Write a rectangle completely with one color, overwrite in subclasses if startWrite is defined!
 *
 * @param[in] xPos Top left corner x coordinate
 * @param[in] yPos Top left corner y coordinate
 * @param[in] width Width in pixels
 * @param[in] height Height in pixels
 * @param[in] color 1-bit Color to fill with
 */
void GFX::writeFillRect(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t width, int_fast16_t height, uint_fast8_t color)
{
    // Overwrite in subclasses if desired!
    fillRect(xPos, yPos, width, height, color);
}

/**
 * @brief End a display-writing routine, overwrite in subclasses if startWrite is defined!
 */
void GFX::endWrite(void)
{
}

/**
 * @brief Draw a perfectly vertical line (this is often optimized in a subclass!)
 *
 * @param[in] xPos Top-most x coordinate
 * @param[in] yPos Top-most y coordinate
 * @param[in] height Height in pixels
 * @param[in] color 1-bit Color to fill with
 */
void GFX::drawFastVLine(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t height, uint_fast8_t color)
{
    startWrite();
    writeLine(xPos, yPos, xPos, (yPos + height - 1), color);
    endWrite();
}

/**
 * @brief Draw a perfectly horizontal line (this is often optimized in a subclass!)
 *
 * @param[in] xPos Left-most x coordinate
 * @param[in] yPos Left-most y coordinate
 * @param[in] width Width in pixels
 * @param[in] color 1-bit Color to fill with
 */
void GFX::drawFastHLine(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t width, uint_fast8_t color)
{
    startWrite();
    writeLine(xPos, yPos, (xPos + width - 1), yPos, color);
    endWrite();
}

/**
 * @brief Fill a rectangle completely with one color. Update in subclasses if desired!
 *
 * @param[in] xPos Top left corner x coordinate
 * @param[in] yPos Top left corner y coordinate
 * @param[in] width Width in pixels
 * @param[in] height Height in pixels
 * @param[in] color 1-bit Color to fill with
 */
void GFX::fillRect(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t width, int_fast16_t height, uint_fast8_t color)
{
    startWrite();
    for (int_fast16_t i = xPos; i < (xPos + width); i++)
    {
        writeFastVLine(i, yPos, height, color);
    }
    endWrite();
}

/**
 * @brief Fill the screen completely with one color. Update in subclasses if desired!
 *
 * @param[in] color 1-bit Color to fill with
 */
void GFX::fillScreen(uint_fast8_t color)
{
    fillRect(0, 0, _width, _height, color);
}

/**
 * @brief Draw a line
 *
 * @param[in] x0 Start point x coordinate
 * @param[in] y0 Start point y coordinate
 * @param[in] x1 End point x coordinate
 * @param[in] y1 End point y coordinate
 * @param[in] color 1-bit Color to draw with
 */
void GFX::drawLine(int_fast16_t x0, int_fast16_t y0, int_fast16_t x1, int_fast16_t y1, uint_fast8_t color)
{
    // Update in subclasses if desired!
    if (x0 == x1)
    {
        if (y0 > y1)
        {
            GFX_SWAP_INT16(y0, y1);
        }
        drawFastVLine(x0, y0, (y1 - y0 + 1), color);
    } else if (y0 == y1)
    {
        if (x0 > x1)
        {
            GFX_SWAP_INT16(x0, x1);
        }
        drawFastHLine(x0, y0, (x1 - x0 + 1), color);
    } else
    {
        startWrite();
        writeLine(x0, y0, x1, y1, color);
        endWrite();
    }
}

/**
 * @brief Draw a circle outline
 *
 * @param[in] x0 Center-point x coordinate
 * @param[in] y0 Center-point y coordinate
 * @param[in] r Radius of circle
 * @param[in] color 1-bit Color to draw with
 */
void GFX::drawCircle(int_fast16_t x0, int_fast16_t y0, int_fast16_t r, uint_fast8_t color)
{
#if defined(ESP8266)
    yield();
#endif
    int_fast16_t f      {1 - r};
    int_fast16_t ddF_x  {1};
    int_fast16_t ddF_y  {-2 * r};
    int_fast16_t x      {0};
    int_fast16_t y      {r};

    startWrite();
    writePixel(x0,       (y0 + r), color);
    writePixel(x0,       (y0 - r), color);
    writePixel((x0 + r),  y0,      color);
    writePixel((x0 - r),  y0,      color);

    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        writePixel((x0 + x), (y0 + y), color);
        writePixel((x0 - x), (y0 + y), color);
        writePixel((x0 + x), (y0 - y), color);
        writePixel((x0 - x), (y0 - y), color);
        writePixel((x0 + y), (y0 + x), color);
        writePixel((x0 - y), (y0 + x), color);
        writePixel((x0 + y), (y0 - x), color);
        writePixel((x0 - y), (y0 - x), color);
    }
    endWrite();
}

/**
 * @brief Quarter-circle drawer, used to do circles and roundrects
 *
 * @param[in] x0 Center-point x coordinate
 * @param[in] y0 Center-point y coordinate
 * @param[in] r Radius of circle
 * @param[in] cornerName Mask bit #1, #2, #4, and #8 to indicate which quarters of the circle we're doing
 * @param[in] color 1-bit Color to draw with
 */
void GFX::drawCircleHelper(int_fast16_t x0, int_fast16_t y0, int_fast16_t r, uint_fast8_t cornerName, uint_fast8_t color)
{
    int_fast16_t f      {1 - r};
    int_fast16_t ddF_x  {1};
    int_fast16_t ddF_y  {-2 * r};
    int_fast16_t x      {0};
    int_fast16_t y      {r};

    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        if (0 != (cornerName & 0x4))
        {
            writePixel((x0 + x), (y0 + y), color);
            writePixel((x0 + y), (y0 + x), color);
        }
        if (0 != (cornerName & 0x2))
        {
            writePixel((x0 + x), (y0 - y), color);
            writePixel((x0 + y), (y0 - x), color);
        }
        if (0 != (cornerName & 0x8))
        {
            writePixel((x0 - y), (y0 + x), color);
            writePixel((x0 - x), (y0 + y), color);
        }
        if (0 != (cornerName & 0x1))
        {
            writePixel((x0 - y), (y0 - x), color);
            writePixel((x0 - x), (y0 - y), color);
        }
    }
}

/**
 * @brief Draw a circle with filled color
 *
 * @param[in] x0 Center-point x coordinate
 * @param[in] y0 Center-point y coordinate
 * @param[in] r Radius of circle
 * @param[in] color 1-bit Color to fill with
 */
void GFX::fillCircle(int_fast16_t x0, int_fast16_t y0, int_fast16_t r, uint_fast8_t color)
{
    startWrite();
    writeFastVLine(x0, (y0 - r), (2 * r + 1), color);
    fillCircleHelper(x0, y0, r, 3, 0, color);
    endWrite();
}

/**
 * @brief Half-circle drawer with fill, used for circles and roundrects
 *
 * @param[in] x0 Center-point x coordinate
 * @param[in] y0 Center-point y coordinate
 * @param[in] r Radius of circle
 * @param[in] corners Mask bits indicating which sides of the circle we are doing, left (1) and/or right (2)
 * @param[in] delta Offset from center-point, used for round-rects
 * @param[in] color 1-bit Color to fill with
 */
void GFX::fillCircleHelper(int_fast16_t x0, int_fast16_t y0, int_fast16_t r, uint_fast8_t corners, int_fast16_t delta, uint_fast8_t color)
{
    int_fast16_t f        {1 - r};
    int_fast16_t ddF_x    {1};
    int_fast16_t ddF_y    {-2 * r};
    int_fast16_t x        {0};
    int_fast16_t y        {r};
    int_fast16_t px       {x};
    int_fast16_t py       {y};

    delta++;                                                        // Avoid some +1's in the loop

    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        // These checks avoid double-drawing certain lines, important
        // for the SSD1306 library which has an INVERT drawing mode.
        if (x < (y + 1))
        {
            if (0 != (corners & 1))
            {
                writeFastVLine(x0 + x, y0 - y, 2 * y + delta, color);
            }
            if (0 != (corners & 2))
            {
                writeFastVLine(x0 - x, y0 - y, 2 * y + delta, color);
            }
        }
        if (y != py)
        {
            if (0 != (corners & 1))
            {
                writeFastVLine(x0 + py, y0 - px, 2 * px + delta, color);
            }
            if (0 != (corners & 2))
            {
                writeFastVLine(x0 - py, y0 - px, 2 * px + delta, color);
            }
            py = y;
        }
        px = x;
    }
}

/**
 * @brief Draw a rectangle with no fill color
 *
 * @param[in] xPos Top left corner x coordinate
 * @param[in] yPos Top left corner y coordinate
 * @param[in] width Width in pixels
 * @param[in] height Height in pixels
 * @param[in] color 1-bit Color to draw with
 */
void GFX::drawRect(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t width, int_fast16_t height, uint_fast8_t color)
{
    startWrite();
    writeFastHLine(xPos,                yPos,               width,  color);
    writeFastHLine(xPos,               (yPos + height - 1), width,  color);
    writeFastVLine(xPos,                yPos,               height, color);
    writeFastVLine((xPos + width - 1),  yPos,               height, color);
    endWrite();
}

//*******************************************************************
//*                                                                 *
//*                        Ellipse Functions                        *
//*                                                                 *
//*******************************************************************

/**
 * @brief Draw an ellipse outline
 *
 * @param[in] x0 Center-point x coordinate
 * @param[in] y0 Center-point y coordinate
 * @param[in] rw Horizontal radius of ellipse
 * @param[in] rh Vertical radius of ellipse
 * @param[in] color 1-bit Color to draw with
 */
void GFX::drawEllipse(int_fast16_t x0, int_fast16_t y0, int_fast16_t rw, int_fast16_t rh, uint_fast8_t color)
{
#if defined(ESP8266)
    yield();
#endif
    // Bresenham's ellipse algorithm
    int_fast16_t x        {0};
    int_fast16_t y        {rh};
    int32_t      rw2      {(int32_t)rw * rw};
    int32_t      rh2      {(int32_t)rh * rh};
    int32_t      twoRw2   {2 * rw2};
    int32_t      twoRh2   {2 * rh2};

    int32_t      decision {rh2 - (rw2 * rh) + (rw2 / 4)};

    startWrite();

    // region 1
    while ((twoRh2 * x) < (twoRw2 * y))
    {
        writePixel((x0 + x), (y0 + y), color);
        writePixel((x0 - x), (y0 + y), color);
        writePixel((x0 + x), (y0 - y), color);
        writePixel((x0 - x), (y0 - y), color);
        x++;
        if (0 > decision)
        {
            decision += rh2 + (twoRh2 * x);
        } else
        {
            decision += rh2 + (twoRh2 * x) - (twoRw2 * y);
            y--;
        }
    }

    // region 2
    decision = ((rh2 * (2 * x + 1) * (2 * x + 1)) >> 2) + (rw2 * (y - 1) * (y - 1)) - (rw2 * rh2);
    while (0 <= y)
    {
        writePixel((x0 + x), (y0 + y), color);
        writePixel((x0 - x), (y0 + y), color);
        writePixel((x0 + x), (y0 - y), color);
        writePixel((x0 - x), (y0 - y), color);
        y--;
        if (0 < decision)
        {
            decision += rw2 - (twoRw2 * y);
        } else
        {
            decision += rw2 + (twoRh2 * x) - (twoRw2 * y);
            x++;
        }
    }

    endWrite();
}

/**
 * @brief Draw an ellipse with filled colour
 *
 * @param[in] x0 Center-point x coordinate
 * @param[in] y0 Center-point y coordinate
 * @param[in] rw Horizontal radius of ellipse
 * @param[in] rh Vertical radius of ellipse
 * @param[in] color 1-bit Color to draw with
 */
void GFX::fillEllipse(int_fast16_t x0, int_fast16_t y0, int_fast16_t rw, int_fast16_t rh, uint_fast8_t color)
{
#if defined(ESP8266)
    yield();
#endif
    // Bresenham's ellipse algorithm
    int_fast16_t x        {0};
    int_fast16_t y        {rh};
    int32_t      rw2      {(int32_t)rw * rw};
    int32_t      rh2      {(int32_t)rh * rh};
    int32_t      twoRw2   {2 * rw2};
    int32_t      twoRh2   {2 * rh2};

    int32_t      decision {rh2 - (rw2 * rh) + (rw2 / 4)};

    startWrite();

    // region 1
    while ((twoRh2 * x) < (twoRw2 * y))
    {
        x++;
        if (0 > decision)
        {
            decision += rh2 + (twoRh2 * x);
        } else
        {
            decision += rh2 + (twoRh2 * x) - (twoRw2 * y);
            writeFastHLine((x0 - (x - 1)), (y0 + y), (2 * (x - 1) + 1), color);
            writeFastHLine((x0 - (x - 1)), (y0 - y), (2 * (x - 1) + 1), color);
            y--;
        }
    }

    // region 2
    decision = ((rh2 * (2 * x + 1) * (2 * x + 1)) >> 2) + (rw2 * (y - 1) * (y - 1)) - (rw2 * rh2);
    while (0 <= y)
    {
        writeFastHLine((x0 - x), (y0 + y), (2 * x + 1), color);
        writeFastHLine((x0 - x), (y0 - y), (2 * x + 1), color);

        y--;
        if (0 < decision)
        {
            decision += rw2 - (twoRw2 * y);
        } else
        {
            decision += rw2 + (twoRh2 * x) - (twoRw2 * y);
            x++;
        }
    }

    endWrite();
}

/**
 * @brief Draw a rounded rectangle with no fill color
 *
 * @param[in] xPos Top left corner x coordinate
 * @param[in] yPos Top left corner y coordinate
 * @param[in] width Width in pixels
 * @param[in] height Height in pixels
 * @param[in] radius Radius of corner rounding
 * @param[in] color 1-bit Color to draw with
 */
void GFX::drawRoundRect(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t width, int_fast16_t height, int_fast16_t radius, uint_fast8_t color)
{
    int_fast16_t maxLimit {((width < height) ? width : height) / 2};    // 1/2 minor axis

    if (radius > maxLimit)
    {
        radius = maxLimit;
    }
    // smarter version
    startWrite();
    writeFastHLine((xPos + radius),      yPos,               (width - 2 * radius), color);      // Top
    writeFastHLine((xPos + radius),     (yPos + height - 1), (width - 2 * radius), color);      // Bottom
    writeFastVLine( xPos,               (yPos + radius),     (height - 2 * radius), color);     // Left
    writeFastVLine((xPos + width - 1),  (yPos + radius),     (height - 2 * radius), color);     // Right
    // draw four corners
    drawCircleHelper((xPos + radius),               (yPos + radius),                radius, 1, color);
    drawCircleHelper((xPos + width - radius - 1),   (yPos + radius),                radius, 2, color);
    drawCircleHelper((xPos + width - radius - 1),   (yPos + height - radius - 1),   radius, 4, color);
    drawCircleHelper((xPos + radius),               (yPos + height - radius - 1),   radius, 8, color);
    endWrite();
}

/**
 * @brief Draw a rounded rectangle with fill color
 *
 * @param[in] xPos Top left corner x coordinate
 * @param[in] yPos Top left corner y coordinate
 * @param[in] width Width in pixels
 * @param[in] height Height in pixels
 * @param[in] radius Radius of corner rounding
 * @param[in] color 1-bit Color to draw/fill with
 */
void GFX::fillRoundRect(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t width, int_fast16_t height, int_fast16_t radius, uint_fast8_t color)
{
    int_fast16_t maxLimit {((width < height) ? width : height) / 2};    // 1/2 minor axis

    if (radius > maxLimit)
    {
        radius = maxLimit;
    }
    // smarter version
    startWrite();
    writeFillRect((xPos + radius), yPos, (width - 2 * radius), height, color);
    // draw four corners
    fillCircleHelper((xPos + width - radius - 1),   (yPos + radius), radius, 1, (height - 2 * radius - 1), color);
    fillCircleHelper((xPos + radius),               (yPos + radius), radius, 2, (height - 2 * radius - 1), color);
    endWrite();
}

/**
 * @brief Draw a triangle with no fill color
 *
 * @param[in] x0 Vertex #0 x coordinate
 * @param[in] y0 Vertex #0 y coordinate
 * @param[in] x1 Vertex #1 x coordinate
 * @param[in] y1 Vertex #1 y coordinate
 * @param[in] x2 Vertex #2 x coordinate
 * @param[in] y2 Vertex #2 y coordinate
 * @param[in] color 1-bit Color to draw with
 */
void GFX::drawTriangle(int_fast16_t x0, int_fast16_t y0, int_fast16_t x1, int_fast16_t y1, int_fast16_t x2, int_fast16_t y2, uint_fast8_t color)
{
    drawLine(x0, y0, x1, y1, color);
    drawLine(x1, y1, x2, y2, color);
    drawLine(x2, y2, x0, y0, color);
}

//*******************************************************************
//*                                                                 *
//*                      Rotated Rect Functions                     *
//*                                                                 *
//*******************************************************************

/**
 * @brief Rotate a point in standard position
 *
 * @param[in,out] x0 x coordinate of point to rotate. This is passed by reference
 *                   and updated upon return
 * @param[in,out] y0 y coordinate of point to rotate. This is passed by reference
 *                   and updated upon return
 * @param[in] angleDeg angle to rotate the point by (degrees)
 */
// Precomputed sine values scaled by 128 (to bypass floating-point library inclusion)
// Stored in PROGMEM. Length: 91 (for 0 to 90 degrees).
static const uint8_t PROGMEM gfx_sinLUT_128[91]
{
    0,   2,   4,   7,   9,   11,  13,  16,  18,  20,
    22,  24,  27,  29,  31,  33,  36,  38,  40,  42,
    44,  46,  48,  50,  52,  54,  56,  58,  60,  62,
    64,  66,  68,  70,  72,  74,  75,  77,  79,  81,
    82,  84,  86,  87,  89,  91,  92,  94,  95,  97,
    98,  99,  101, 102, 104, 105, 106, 107, 109, 110,
    111, 112, 113, 114, 115, 116, 117, 118, 119, 119,
    120, 121, 122, 122, 123, 124, 124, 125, 125, 126,
    126, 126, 127, 127, 127, 128, 128, 128, 128, 128,
    128
};

static int_fast16_t fixed_sin128(int_fast16_t angleDeg)
{
    int_fast16_t angle  {angleDeg % 360};
    int_fast16_t result {0};

    if (0 > angle)
    {
        angle += 360;
    }

    if (90 >= angle)
    {
        result = pgm_read_byte(&gfx_sinLUT_128[angle]);
    } else if (180 >= angle)
    {
        result = pgm_read_byte(&gfx_sinLUT_128[180 - angle]);
    } else if (270 >= angle)
    {
        result = -pgm_read_byte(&gfx_sinLUT_128[angle - 180]);
    } else
    {
        result = -pgm_read_byte(&gfx_sinLUT_128[360 - angle]);
    }

    return result;
}

static int_fast16_t fixed_cos128(int_fast16_t angleDeg)
{
    return fixed_sin128(angleDeg + 90);
}

void GFX::rotatePoint(int_fast16_t &x0, int_fast16_t &y0, int_fast16_t angleDeg)
{
    int_fast16_t s {fixed_sin128(angleDeg)};
    int_fast16_t c {fixed_cos128(angleDeg)};

    // Rotate point using fixed point arithmetic (scaled by 128)
    // >> 7 is equivalent to dividing by 128. We add 64 for rounding.
    int_fast16_t new_x {(int_fast16_t)(((int32_t)x0 * c - (int32_t)y0 * s + 64) >> 7)};
    int_fast16_t new_y {(int_fast16_t)(((int32_t)x0 * s + (int32_t)y0 * c + 64) >> 7)};

    x0 = new_x;
    y0 = new_y;
}

/**
 * @brief Draw a rotated rectangle
 *
 * @param[in] cenX x coordinate of center of rectangle.
 * @param[in] cenY y coordinate of center of rectangle.
 * @param[in] width width of rectangle
 * @param[in] height height of rectangle
 * @param[in] angleDeg angle of rotation of rectangle
 * @param[in] color 1-bit Color to fill/draw with
 */
void GFX::drawRotatedRect(int_fast16_t cenX, int_fast16_t cenY, int_fast16_t width, int_fast16_t height, int_fast16_t angleDeg, uint_fast8_t color)
{
    if ((1 <= width) && (1 <= height))
    {
        int_fast16_t limitW      {width - 1};
        int_fast16_t limitH      {height - 1};

        int_fast16_t halfW   {limitW / 2};                          // Midpoint should always be integer
        int_fast16_t halfH   {limitH / 2};

        int_fast16_t x0      {limitW - halfW};                      // bottom-right
        int_fast16_t y0      {limitH - halfH};
        int_fast16_t x1      {-halfW};                              // bottom-left
        int_fast16_t y1      {limitH - halfH};
        int_fast16_t x2      {-halfW};                              // top-left
        int_fast16_t y2      {-halfH};
        int_fast16_t x3      {limitW - halfW};                      // top-right
        int_fast16_t y3      {-halfH};

        rotatePoint(x0, y0, angleDeg);
        rotatePoint(x1, y1, angleDeg);
        rotatePoint(x2, y2, angleDeg);
        rotatePoint(x3, y3, angleDeg);

        x0 += cenX;
        x1 += cenX;
        x2 += cenX;
        x3 += cenX;

        y0 += cenY;
        y1 += cenY;
        y2 += cenY;
        y3 += cenY;

        drawLine(x0, y0, x1, y1, color);                            // bottom right to bottom left
        drawLine(x1, y1, x2, y2, color);                            // bottom left to top left
        drawLine(x2, y2, x3, y3, color);                            // top left to top right
        drawLine(x3, y3, x0, y0, color);                            // top right to bottom right
    }
}

/**
 * @brief Draw a filled rotated rectangle
 *
 * @param[in] cenX x coordinate of center of rectangle.
 * @param[in] cenY y coordinate of center of rectangle.
 * @param[in] width width of rectangle
 * @param[in] height height of rectangle
 * @param[in] angleDeg angle of rotation of rectangle
 * @param[in] color 1-bit Color to fill/draw with
 */
void GFX::fillRotatedRect(int_fast16_t cenX, int_fast16_t cenY, int_fast16_t width, int_fast16_t height, int_fast16_t angleDeg, uint_fast8_t color)
{
    if ((1 <= width) && (1 <= height))
    {
        int_fast16_t limitW      {width - 1};
        int_fast16_t limitH      {height - 1};

        int_fast16_t halfW   {limitW / 2};                          // Midpoint should always be integer
        int_fast16_t halfH   {limitH / 2};

        int_fast16_t x0      {limitW - halfW};                      // bottom-right
        int_fast16_t y0      {limitH - halfH};
        int_fast16_t x1      {-halfW};                              // bottom-left
        int_fast16_t y1      {limitH - halfH};
        int_fast16_t x2      {-halfW};                              // top-left
        int_fast16_t y2      {-halfH};
        int_fast16_t x3      {limitW - halfW};                      // top-right
        int_fast16_t y3      {-halfH};

        rotatePoint(x0, y0, angleDeg);
        rotatePoint(x1, y1, angleDeg);
        rotatePoint(x2, y2, angleDeg);
        rotatePoint(x3, y3, angleDeg);

        x0 += cenX;
        x1 += cenX;
        x2 += cenX;
        x3 += cenX;

        y0 += cenY;
        y1 += cenY;
        y2 += cenY;
        y3 += cenY;

        fillTriangle(x0, y0, x1, y1, x2, y2, color);
        fillTriangle(x2, y2, x3, y3, x0, y0, color);
    }
}

/**
 * @brief Draw a triangle with color-fill
 *
 * @param[in] x0 Vertex #0 x coordinate
 * @param[in] y0 Vertex #0 y coordinate
 * @param[in] x1 Vertex #1 x coordinate
 * @param[in] y1 Vertex #1 y coordinate
 * @param[in] x2 Vertex #2 x coordinate
 * @param[in] y2 Vertex #2 y coordinate
 * @param[in] color 1-bit Color to fill/draw with
 */
void GFX::fillTriangle(int_fast16_t x0, int_fast16_t y0, int_fast16_t x1, int_fast16_t y1, int_fast16_t x2, int_fast16_t y2, uint_fast8_t color)
{
    int_fast16_t a      {0};
    int_fast16_t b      {0};
    int_fast16_t y      {0};
    int_fast16_t last   {0};

    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1)
    {
        GFX_SWAP_INT16(y0, y1);
        GFX_SWAP_INT16(x0, x1);
    }
    if (y1 > y2)
    {
        GFX_SWAP_INT16(y2, y1);
        GFX_SWAP_INT16(x2, x1);
    }
    if (y0 > y1)
    {
        GFX_SWAP_INT16(y0, y1);
        GFX_SWAP_INT16(x0, x1);
    }

    startWrite();
    if (y0 == y2)
    { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if (x1 < a)
        {
            a = x1;
        } else if (x1 > b)
        {
            b = x1;
        }
        if (x2 < a)
        {
            a = x2;
        } else if (x2 > b)
        {
            b = x2;
        }
        writeFastHLine(a, y0, b - a + 1, color);
    }
    else
    {
        int_fast16_t    dx01    {x1 - x0};
        int_fast16_t    dy01    {y1 - y0};
        int_fast16_t    dx02    {x2 - x0};
        int_fast16_t    dy02    {y2 - y0};
        int_fast16_t    dx12    {x2 - x1};
        int_fast16_t    dy12    {y2 - y1};
        int32_t         sa      {0};
        int32_t         sb      {0};

        // For upper part of triangle, find scanline crossings for segments 0-1 and 0-2.
        // If y1=y2 (flat-bottomed triangle), the scanline y1 is included here (and second loop will be skipped,
        // avoiding a / 0 error there), otherwise scanline y1 is skipped here and handled in the second loop...
        // which also avoids a /0 error here if y0=y1 (flat-topped triangle).
        last = (y1 == y2)   ? y1                                    // Include y1 scanline
                            : (y1 - 1);                             // Skip it;

        for (y = y0; y <= last; y++)
        {
            a = x0 + sa / dy01;
            b = x0 + sb / dy02;
            sa += dx01;
            sb += dx02;

            if (a > b)
            {
                GFX_SWAP_INT16(a, b);
            }
            writeFastHLine(a, y, (b - a + 1), color);
        }

        // For lower part of triangle, find scanline crossings for segments 0-2 and 1-2. This loop is skipped if y1=y2.
        sa = (int32_t)dx12 * (y - y1);
        sb = (int32_t)dx02 * (y - y0);
        for (; y <= y2; y++)
        {
            a = x1 + sa / dy12;
            b = x0 + sb / dy02;
            sa += dx12;
            sb += dx02;

            if (a > b)
            {
                GFX_SWAP_INT16(a, b);
            }
            writeFastHLine(a, y, (b - a + 1), color);
        }
    }
    endWrite();
}

// BITMAP / XBITMAP / GRAYSCALE / RGB BITMAP FUNCTIONS ---------------------

/**
 * @brief Draw a PROGMEM-resident 1-bit image at the specified (x,y) position,
 *        using the specified foreground color (unset bits are transparent).
 *
 * @param[in] xPos Top left corner x coordinate
 * @param[in] yPos Top left corner y coordinate
 * @param[in] bitmap byte array with monochrome bitmap
 * @param[in] width Width of bitmap in pixels
 * @param[in] height Height of bitmap in pixels
 * @param[in] color 1-bit Color to draw with
 */
void GFX::drawBitmap(int_fast16_t xPos, int_fast16_t yPos, const uint8_t bitmap[], int_fast16_t width, int_fast16_t height, uint_fast8_t color)
{
    int_fast16_t byteWidth    {(width + 7) / 8};                    // Bitmap scanline pad = whole byte
    uint_fast8_t b            {0};

    startWrite();
    for (int_fast16_t j = 0; j < height; j++, yPos++)
    {
        for (int_fast16_t i = 0; i < width; i++)
        {
            if (i & 7)
            {
                b <<= 1;
            } else
            {
                b = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
            }
            if (b & 0x80)
            {
                writePixel((xPos + i), yPos, color);
            }
        }
    }
    endWrite();
}

/**
 * @brief Draw a PROGMEM-resident 1-bit image at the specified (x,y) position,
 *        using the specified foreground (for set bits) and background (unset bits) colors.
 *
 * @param[in] xPos Top left corner x coordinate
 * @param[in] yPos Top left corner y coordinate
 * @param[in] bitmap byte array with monochrome bitmap
 * @param[in] width Width of bitmap in pixels
 * @param[in] height Height of bitmap in pixels
 * @param[in] color 1-bit Color to draw pixels with
 * @param[in] bgColor 1-bit Color to draw background with
 */
void GFX::drawBitmap(int_fast16_t xPos, int_fast16_t yPos, const uint8_t bitmap[], int_fast16_t width, int_fast16_t height, uint_fast8_t color, uint_fast8_t bgColor)
{
    int_fast16_t byteWidth  {(width + 7) / 8};
    uint_fast8_t b          {0};

    startWrite();
    for (int_fast16_t j = 0; j < height; j++, yPos++)
    {
        for (int_fast16_t i = 0; i < width; i++)
        {
            if (i & 7)
            {
                b <<= 1;
            } else
            {
                b = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
            }
            writePixel(xPos + i, yPos, (b & 0x80) ? color : bgColor);
        }
    }
    endWrite();
}

/**
 * @brief Draw a RAM-resident 1-bit image at the specified (x,y) position,
 *        using the specified foreground color (unset bits are transparent).
 *
 * @param[in] xPos Top left corner x coordinate
 * @param[in] yPos Top left corner y coordinate
 * @param[in] bitmap byte array with monochrome bitmap
 * @param[in] width Width of bitmap in pixels
 * @param[in] height Height of bitmap in pixels
 * @param[in] color 1-bit Color to draw with
 */
void GFX::drawBitmap(int_fast16_t xPos, int_fast16_t yPos, uint8_t *bitmap, int_fast16_t width, int_fast16_t height, uint_fast8_t color)
{
    int_fast16_t byteWidth  {(width + 7) / 8};
    uint_fast8_t b          {0};

    startWrite();
    for (int_fast16_t j = 0; j < height; j++, yPos++)
    {
        for (int_fast16_t i = 0; i < width; i++)
        {
            if (i & 7)
            {
                b <<= 1;
            } else
            {
                b = bitmap[j * byteWidth + i / 8];
            }
            if (b & 0x80)
            {
                writePixel((xPos + i), yPos, color);
            }
        }
    }
    endWrite();
}

/**
 * @brief Draw a RAM-resident 1-bit image at the specified (x,y) position,
 *        using the specified foreground (for set bits) and background (unset bits) colors.
 *
 * @param[in] xPos Top left corner x coordinate
 * @param[in] yPos Top left corner y coordinate
 * @param[in] bitmap byte array with monochrome bitmap
 * @param[in] width Width of bitmap in pixels
 * @param[in] height Height of bitmap in pixels
 * @param[in] color 1-bit Color to draw pixels with
 * @param[in] bgColor 1-bit Color to draw background with
 */
void GFX::drawBitmap(int_fast16_t xPos, int_fast16_t yPos, uint8_t *bitmap, int_fast16_t width, int_fast16_t height, uint_fast8_t color, uint_fast8_t bgColor)
{
    int_fast16_t byteWidth  {(width + 7) / 8};
    uint_fast8_t b          {0};

    startWrite();
    for (int_fast16_t j = 0; j < height; j++, yPos++)
    {
        for (int_fast16_t i = 0; i < width; i++)
        {
            if (i & 7)
            {
                b <<= 1;
            } else
            {
                b = bitmap[j * byteWidth + i / 8];
            }
            writePixel((xPos + i), yPos, (b & 0x80) ? color : bgColor);
        }
    }
    endWrite();
}

/**
 * @brief Draw PROGMEM-resident XBitMap Files (*.xbm), exported from GIMP.
 *        Usage: Export from GIMP to *.xbm, rename *.xbm to *.c and open in editor.
 *        C Array can be directly used with this function.
 *        There is no RAM-resident version of this function; if generating bitmaps
 *        in RAM, use the format defined by drawBitmap() and call that instead.
 *
 * @param[in] xPos Top left corner x coordinate
 * @param[in] yPos Top left corner y coordinate
 * @param[in] bitmap byte array with monochrome bitmap
 * @param[in] width Width of bitmap in pixels
 * @param[in] height Height of bitmap in pixels
 * @param[in] color 1-bit Color to draw pixels with
 */
void GFX::drawXBitmap(int_fast16_t xPos, int_fast16_t yPos, const uint8_t bitmap[], int_fast16_t width, int_fast16_t height, uint_fast8_t color)
{
    int_fast16_t byteWidth  {(width + 7) / 8};
    uint_fast8_t b          {0};

    startWrite();
    for (int_fast16_t j = 0; j < height; j++, yPos++)
    {
        for (int_fast16_t i = 0; i < width; i++)
        {
            if (i & 7)
            {
                b >>= 1;
            } else
            {
                b = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
            }
            if (b & 0x01)
            {
                writePixel(xPos + i, yPos, color);
            }
        }
    }
    endWrite();
}

// TEXT- AND CHARACTER-HANDLING FUNCTIONS ----------------------------------

// Draw a character
/**
 * @brief Draw a single character
 *
 * @param[in] xPos Bottom left corner x coordinate
 * @param[in] yPos Bottom left corner y coordinate
 * @param[in] character The 8-bit font-indexed character (likely ascii)
 * @param[in] color 1-bit Color to draw chraracter with
 * @param[in] bgColor 1-bit Color to fill background with (if same as color, no background)
 * @param[in] size Font magnification level, 1 is 'original' size
 */
void GFX::drawChar(int_fast16_t xPos, int_fast16_t yPos, unsigned char character, uint_fast8_t color, uint_fast8_t bgColor, uint_fast8_t size)
{
    drawChar(xPos, yPos, character, color, bgColor, size, size);
}

// Draw a character
/**
 * @brief Draw a single character
 *
 * @param[in] xPos Bottom left corner x coordinate
 * @param[in] yPos Bottom left corner y coordinate
 * @param[in] character The 8-bit font-indexed character (likely ascii)
 * @param[in] color 1-bit Color to draw chraracter with
 * @param[in] bgColor 1-bit Color to fill background with (if same as color, no background)
 * @param[in] sizeX Font magnification level in X-axis, 1 is 'original' size
 * @param[in] sizeY Font magnification level in Y-axis, 1 is 'original' size
 */
void GFX::drawChar(int_fast16_t xPos, int_fast16_t yPos, unsigned char character, uint_fast8_t color, uint_fast8_t bgColor, uint_fast8_t sizeX, uint_fast8_t sizeY)
{
    if (!_gfxFont)
    {
        if (!((xPos >= _width)              ||
              (yPos >= _height)             ||
             ((xPos + 6 * sizeX - 1) < 0)   ||
             ((yPos + 8 * sizeY - 1) < 0)))
        {
            startWrite();
            for (uint_fast8_t i = 0; i < 5; i++)
            {
                uint_fast8_t line {pgm_read_byte(&font[character * 5 + i])};

                for (uint_fast8_t j = 0; j < 8; j++, line >>= 1)
                {
                    if (line & 1)
                    {
                        if ((1 == sizeX) && (1 == sizeY))
                        {
                            writePixel((xPos + i), (yPos + j), color);
                        } else
                        {
                            writeFillRect((xPos + i * sizeX), (yPos + j * sizeY), sizeX, sizeY, color);
                        }
                    } else if (bgColor != color)
                    {
                        if ((1 == sizeX) && (1 == sizeY))
                        {
                            writePixel((xPos + i), (yPos + j), bgColor);
                        } else
                        {
                            writeFillRect((xPos + i * sizeX), (yPos + j * sizeY), sizeX, sizeY, bgColor);
                        }
                    }
                }
            }
            if (bgColor != color)
            {
                if ((1 == sizeX) && (1 == sizeY))
                {
                    writeFastVLine((xPos + 5), yPos, 8, bgColor);
                } else
                {
                    writeFillRect((xPos + 5 * sizeX), yPos, sizeX, (8 * sizeY), bgColor);
                }
            }
            endWrite();
        }

    } else
    {
        character -= (uint_fast8_t)pgm_read_byte(&_gfxFont->first);

        GFXglyph       *glyph   {pgm_read_glyph_ptr(_gfxFont, character)};
        uint8_t        *bitmap  {pgm_read_bitmap_ptr(_gfxFont)};

        uint_fast16_t   bo      {pgm_read_word(&glyph->bitmapOffset)};
        uint_fast8_t    width   {pgm_read_byte(&glyph->width)};
        uint_fast8_t    height  {pgm_read_byte(&glyph->height)};
        int_fast8_t     xo      {pgm_read_byte(&glyph->xOffset)};
        int_fast8_t     yo      {pgm_read_byte(&glyph->yOffset)};
        uint_fast8_t    xx      {0};
        uint_fast8_t    yy      {0};
        uint_fast8_t    bits    {0};
        uint_fast8_t    bit     {0};
        int_fast16_t    xo16    {0};
        int_fast16_t    yo16    {0};

        if ((sizeX > 1) || (sizeY > 1))
        {
            xo16 = xo;
            yo16 = yo;
        }

        // TODO: Add character clipping here

        startWrite();
        for (yy = 0; yy < height; yy++)
        {
            for (xx = 0; xx < width; xx++)
            {
                if (!(bit++ & 7))
                {
                    bits = pgm_read_byte(&bitmap[bo++]);
                }
                if (bits & 0x80)
                {
                    if ((1 == sizeX) && (1 == sizeY))
                    {
                        writePixel((xPos + xo + xx), (yPos + yo + yy), color);
                    } else
                    {
                        writeFillRect((xPos + (xo16 + xx) * sizeX), (yPos + (yo16 + yy) * sizeY), sizeX, sizeY, color);
                    }
                }
                bits <<= 1;
            }
        }
        endWrite();

    }
}

/**
 * @brief Print one byte/character of data, used to support print()
 *
 * @param[in] c The 8-bit ascii character to write
 */
size_t GFX::write(uint8_t c)
{
    if (!_gfxFont)
    {
        if ('\n' == c)
        {
            _cursorX = 0;
            _cursorY += _textSizeY * 8;
        } else if (c != '\r')
        {
            if (_wrap && ((_cursorX + (int_fast16_t)_textSizeX * 6) > _width))
            {
                _cursorX = 0;
                _cursorY += _textSizeY * 8;
            }
            drawChar(_cursorX, _cursorY, c, _textColor, _textBgColor, _textSizeX, _textSizeY);
            _cursorX += _textSizeX * 6;
        }
    } else
    {
        if ('\n' == c)
        {
            _cursorX = 0;
            _cursorY += (int_fast16_t)_textSizeY * (int_fast8_t)pgm_read_byte(&_gfxFont->yAdvance);
        } else if (c != '\r')
        {
            uint_fast8_t first {pgm_read_byte(&_gfxFont->first)};

            if ((c >= first) && (c <= (uint_fast8_t)pgm_read_byte(&_gfxFont->last)))
            {
                GFXglyph       *glyph   {pgm_read_glyph_ptr(_gfxFont, c - first)};
                uint_fast8_t    width   {pgm_read_byte(&glyph->width)};
                uint_fast8_t    height  {pgm_read_byte(&glyph->height)};

                if ((width > 0) && (height > 0))
                {
                    int_fast16_t xo {(int_fast8_t)pgm_read_byte(&glyph->xOffset)};

                    if (_wrap && ((_cursorX + (int_fast16_t)_textSizeX * (xo + (int_fast16_t)width)) > _width))
                    {
                        _cursorX = 0;
                        _cursorY += (int_fast16_t)_textSizeY * (uint_fast8_t)pgm_read_byte(&_gfxFont->yAdvance);
                    }
                    drawChar(_cursorX, _cursorY, c, _textColor, _textBgColor, _textSizeX, _textSizeY);
                }
                _cursorX += (uint_fast8_t)pgm_read_byte(&glyph->xAdvance) * (int_fast16_t)_textSizeX;
            }
        }
    }

    return 1;
}

/**************************************************************************/
/**
 * @brief Set text 'magnification' size. Each increase in size makes 1 pixel that much bigger.
 *
 * @param[in] size Desired text size. 1 is default 6x8, 2 is 12x16, 3 is 18x24, etc.
 */
void GFX::setTextSize(uint_fast8_t size)
{
    setTextSize(size, size);
}

/**
 * @brief Set text 'magnification' size. Each increase in size makes 1 pixel that much bigger.
 *
 * @param[in] sizeX Desired text width magnification level in X-axis. 1 is default
 * @param[in] sizeY Desired text height magnification level in Y-axis. 1 is default
 */
void GFX::setTextSize(uint_fast8_t sizeX, uint_fast8_t sizeY)
{
    _textSizeX = (sizeX > 0) ? sizeX : 1;
    _textSizeY = (sizeY > 0) ? sizeY : 1;
}

/**
 * @brief Set rotation setting for display
 *
 * @param[in] rotation 0 thru 3 corresponding to 4 cardinal rotations
 */
void GFX::setRotation(uint_fast8_t rotation)
{
    _rotation = (rotation & 3);
    switch (_rotation)
    {
        case 0:
        case 2:
            _width = _baseWidth;
            _height = _baseHeight;
            break;
        case 1:
        case 3:
            _width = _baseHeight;
            _height = _baseWidth;
            break;
    }
}

/**
 * @brief Set the font to display when print()ing, either custom or default
 *
 * @param[in] fontObj The GFXfont object, if NULL use built in 6x8 font
 */
void GFX::setFont(const GFXfont *fontObj)
{
    if (nullptr != fontObj)                                         // Font struct pointer passed in?
    {
        if (!_gfxFont)                                              // And no current font struct?
        {                                                           // Switching from classic to new font behavior.
                                                                    // Move cursor pos down 6 pixels so it's on baseline.
            _cursorY += 6;
        }
    } else if (nullptr != _gfxFont)                                 // nullptr passed. Current font struct defined?
    {                                                               // Switching from new to classic font behavior.
                                                                    // Move cursor pos up 6 pixels so it's at top-left of char.
        _cursorY -= 6;
    }
    _gfxFont = (GFXfont *)fontObj;
}

/**
 * @brief Helper to determine size of a character with current font/size.
 *        Broke this out as it's used by both the PROGMEM- and RAM-resident getTextBounds() functions.
 *
 * @param[in] character The ASCII character in question
 * @param[in,out] xPos Pointer to x location of character. Value is modified by this function to advance to next character.
 * @param[in,out] yPos Pointer to y location of character. Value is modified by this function to advance to next character.
 * @param[in,out] minX Pointer to minimum X coordinate, passed in to AND returned by this function.
 *                     This is used to incrementally build a bounding rectangle for a string.
 * @param[in,out] minY Pointer to minimum Y coord, passed in AND returned.
 * @param[in,out] maxX Pointer to maximum X coord, passed in AND returned.
 * @param[in,out] maxY Pointer to maximum Y coord, passed in AND returned.
 */
void GFX::charBounds(unsigned char character, int_fast16_t *xPos, int_fast16_t *yPos, int_fast16_t *minX, int_fast16_t *minY, int_fast16_t *maxX, int_fast16_t *maxY)
{
    if (_gfxFont)
    {
        if ('\n' == character)                                      // Newline?
        {
            *xPos = 0;                                              // Reset x to zero, advance y by one line
            *yPos += _textSizeY * (uint_fast8_t)pgm_read_byte(&_gfxFont->yAdvance);
        } else if (character != '\r')                               // Not a carriage return; is normal char
        {
            uint_fast8_t first  {pgm_read_byte(&_gfxFont->first)};
            uint_fast8_t last   {pgm_read_byte(&_gfxFont->last)};

            if ((character >= first) && (character <= last))        // Char present in this font?
            {
                GFXglyph       *glyph   {pgm_read_glyph_ptr(_gfxFont, character - first)};
                uint_fast8_t    gw      {pgm_read_byte(&glyph->width)};
                uint_fast8_t    gh      {pgm_read_byte(&glyph->height)};
                uint_fast8_t    xa      {pgm_read_byte(&glyph->xAdvance)};
                int_fast8_t     xo      {pgm_read_byte(&glyph->xOffset)};
                int_fast8_t     yo      {pgm_read_byte(&glyph->yOffset)};

                if (_wrap && ((*xPos + (((int_fast16_t)xo + (int_fast16_t)gw) * (int_fast16_t)_textSizeX)) > _width))
                {
                    *xPos = 0;                                      // Reset x to zero, advance y by one line
                    *yPos += _textSizeY * (uint_fast8_t)pgm_read_byte(&_gfxFont->yAdvance);
                }
                int_fast16_t    tsx     {(int_fast16_t)_textSizeX};
                int_fast16_t    tsy     {(int_fast16_t)_textSizeY};
                int_fast16_t    x1      {*xPos + xo * tsx};
                int_fast16_t    y1      {*yPos + yo * tsy};
                int_fast16_t    x2      {x1 + (int_fast16_t)gw * tsx - 1};
                int_fast16_t    y2      {y1 + (int_fast16_t)gh * tsy - 1};

                if (x1 < *minX)
                {
                    *minX = x1;
                }
                if (y1 < *minY)
                {
                    *minY = y1;
                }
                if (x2 > *maxX)
                {
                    *maxX = x2;
                }
                if (y2 > *maxY)
                {
                    *maxY = y2;
                }
                *xPos += xa * tsx;
            }
        }

    } else                                                          // Default font
    {
        if ('\n' == character)                                      // Newline?
        {
            *xPos = 0;                                              // Reset x to zero,
            *yPos += _textSizeY * 8;                                // advance y one line
                                                                    // min/max x/y unchaged - that waits for next 'normal' character
        } else if (character != '\r')                               // Normal char; ignore carriage returns
        {
            if (_wrap && ((*xPos + (int_fast16_t)_textSizeX * 6) > _width))
            {                                                       // Off right?
                *xPos = 0;                                          // Reset x to zero,
                *yPos += _textSizeY * 8;                            // advance y one line
            }
            // Lower-right pixel of char
            int_fast16_t x2 {*xPos + (int_fast16_t)_textSizeX * 6 - 1};
            int_fast16_t y2 {*yPos + (int_fast16_t)_textSizeY * 8 - 1};

            if (x2 > *maxX)
            {
                *maxX = x2;                                         // Track max x, y
            }
            if (y2 > *maxY)
            {
                *maxY = y2;
            }
            if (*xPos < *minX)
            {
                *minX = *xPos;                                      // Track min x, y
            }
            if (*yPos < *minY)
            {
                *minY = *yPos;
            }
            *xPos += _textSizeX * 6;                                // Advance x one char
        }
    }
}

/**
 * @brief Helper to determine size of a string with current font/size.
 *        Pass string and a cursor position, returns UL corner and W,H.
 *
 * @param[in] str The ASCII string to measure
 * @param[in] xPos The current cursor X
 * @param[in] yPos The current cursor Y
 * @param[out] x1 The boundary X coordinate, returned by function
 * @param[out] y1 The boundary Y coordinate, returned by function
 * @param[out] width The boundary width, returned by function
 * @param[out] height The boundary height, returned by function
 */
void GFX::getTextBounds(const char *str, int_fast16_t xPos, int_fast16_t yPos, int_fast16_t *x1, int_fast16_t *y1, uint_fast16_t *width, uint_fast16_t *height)
{
    uint_fast8_t c      {0};
    int_fast16_t minX   {0x7FFF};
    int_fast16_t minY   {0x7FFF};
    int_fast16_t maxX   {-1};
    int_fast16_t maxY   {-1};

    *x1 = xPos;
    *y1 = yPos;
    *width = *height = 0;

    while ((c = *str++))
    {
        charBounds(c, &xPos, &yPos, &minX, &minY, &maxX, &maxY);
    }

    if (maxX >= minX)
    {
        *x1 = minX;
        *width = maxX - minX + 1;
    }
    if (maxY >= minY)
    {
        *y1 = minY;
        *height = maxY - minY + 1;
    }
}

/**
 * @brief Helper to determine size of a string with current font/size.
 *        Pass string and a cursor position, returns UL corner and W,H.
 *
 * @param[in] str The ascii string to measure (as an arduino String() class)
 * @param[in] xPos The current cursor X
 * @param[in] yPos The current cursor Y
 * @param[out] x1 The boundary X coordinate, set by function
 * @param[out] y1 The boundary Y coordinate, set by function
 * @param[out] width The boundary width, set by function
 * @param[out] height The boundary height, set by function
 */
void GFX::getTextBounds(const String &str, int_fast16_t xPos, int_fast16_t yPos, int_fast16_t *x1, int_fast16_t *y1, uint_fast16_t *width, uint_fast16_t *height)
{
    if (str.length() != 0)
    {
        getTextBounds(const_cast<char *>(str.c_str()), xPos, yPos, x1, y1, width, height);
    }
}

/**
 * @brief Helper to determine size of a PROGMEM string with current font/size.
 *        Pass string and a cursor position, returns UL corner and W,H.
 *
 * @param[in] str The flash-memory ascii string to measure
 * @param[in] xPos The current cursor X
 * @param[in] yPos The current cursor Y
 * @param[out] x1 The boundary X coordinate, set by function
 * @param[out] y1 The boundary Y coordinate, set by function
 * @param[out] width The boundary width, set by function
 * @param[out] height The boundary height, set by function
 */
void GFX::getTextBounds(const __FlashStringHelper *str, int_fast16_t xPos, int_fast16_t yPos, int_fast16_t *x1, int_fast16_t *y1, uint_fast16_t *width, uint_fast16_t *height)
{
    uint8_t        *s       {(uint8_t *)str};
    uint8_t         c       {0};
    int_fast16_t    minX    {_width};
    int_fast16_t    minY    {_height};
    int_fast16_t    maxX    {-1};
    int_fast16_t    maxY    {-1};

    *x1 = xPos;
    *y1 = yPos;
    *width = *height = 0;

    while ((c = pgm_read_byte(s++)))
    {
        charBounds(c, &xPos, &yPos, &minX, &minY, &maxX, &maxY);
    }

    if (maxX >= minX)
    {
        *x1 = minX;
        *width = maxX - minX + 1;
    }
    if (maxY >= minY)
    {
        *y1 = minY;
        *height = maxY - minY + 1;
    }
}

/**
 * @brief Invert the display (ideally using built-in hardware command)
 *
 * @param[in] invert True if you want to invert, false to make 'normal'
 */
void GFX::invertDisplay(bool invert)
{                                                                   // Do nothing, must be subclassed if supported by hardware
    (void)invert;                                                   // disable -Wunused-parameter warning
}

/***************************************************************************/

/**
 * @brief Create a simple drawn button UI element
 */
GFXbutton::GFXbutton(void)
{
    _gfx = nullptr;
}

/**
 * @brief Initialize button with our desired color/size/settings, with upper-left coordinates
 *
 * @param[in] gfx Pointer to our display so we can draw to it!
 * @param[in] x1 The X coordinate of the Upper-Left corner of the button
 * @param[in] y1 The Y coordinate of the Upper-Left corner of the button
 * @param[in] w Width of the buttton
 * @param[in] h Height of the buttton
 * @param[in] outline Color of the outline (1-bit standard)
 * @param[in] fill Color of the button fill (1-bit standard)
 * @param[in] textcolor Color of the button label (1-bit standard)
 * @param[in] label Ascii string of the text inside the button
 * @param[in] textsize The font magnification of the label text
 */
void GFXbutton::initButton(GFX *gfx, int_fast16_t x1, int_fast16_t y1, uint_fast16_t w, uint_fast16_t h, uint_fast16_t outline, uint_fast16_t fill, uint_fast16_t textcolor, char *label, uint_fast8_t textsize)
{
    initButton(gfx, x1, y1, w, h, outline, fill, textcolor, label, textsize, textsize);
}

/**
 * @brief Initialize button with our desired color/size/settings, with upper-left coordinates
 *
 * @param[in] gfx Pointer to our display so we can draw to it!
 * @param[in] topLeftX The X coordinate of the Upper-Left corner of the button
 * @param[in] topLeftY The Y coordinate of the Upper-Left corner of the button
 * @param[in] width Width of the buttton
 * @param[in] height Height of the buttton
 * @param[in] outline Color of the outline (1-bit standard)
 * @param[in] fill Color of the button fill (1-bit standard)
 * @param[in] textcolor Color of the button label (1-bit standard)
 * @param[in] label Ascii string of the text inside the button
 * @param[in] textsize_x The font magnification in X-axis of the label text
 * @param[in] textsize_y The font magnification in Y-axis of the label text
 */
void GFXbutton::initButton(GFX *gfx, int_fast16_t topLeftX, int_fast16_t topLeftY, uint_fast16_t width, uint_fast16_t height, uint_fast16_t outline, uint_fast16_t fill, uint_fast16_t textcolor, char *label, uint_fast8_t textsize_x, uint_fast8_t textsize_y)
{
    _topLeftX = topLeftX;
    _topLeftY = topLeftY;
    _width = width;
    _height = height;
    _outlineColor = outline;
    _fillColor = fill;
    _textColor = textcolor;
    _textSizeX = textsize_x;
    _textSizeY = textsize_y;
    _gfx = gfx;
    strncpy(_label, label, LABEL_MAX_LEN - 1);
    _label[LABEL_MAX_LEN - 1] = '\0';
}

/**
 * @brief Draw the button on the screen
 *
 * @param[in] inverted Whether to draw with fill/text swapped to indicate 'pressed'
 */
void GFXbutton::drawButton(bool inverted)
{
    uint_fast16_t fill    {0};
    uint_fast16_t outline {0};
    uint_fast16_t text    {0};

    if (false == inverted)
    {
        fill = _fillColor;
        outline = _outlineColor;
        text = _textColor;
    } else
    {
        fill = _textColor;
        outline = _outlineColor;
        text = _fillColor;
    }

    uint_fast8_t r {(uint_fast8_t)(min(_width, _height) / 4)};

    _gfx->fillRoundRect(_topLeftX, _topLeftY, _width, _height, r, fill);
    _gfx->drawRoundRect(_topLeftX, _topLeftY, _width, _height, r, outline);

    _gfx->setCursor((_topLeftX + (_width / 2) - (strlen(_label) * 3 * _textSizeX)), (_topLeftY + (_height / 2) - (4 * _textSizeY)));
    _gfx->setTextColor(text);
    _gfx->setTextSize(_textSizeX, _textSizeY);
    _gfx->print(_label);
}

/**
 * @brief Helper to let us know if a coordinate is within the bounds of the button
 *
 * @param[in] xPos The X coordinate to check
 * @param[in] yPos The Y coordinate to check
 * @return true if within button graphics outline
 * @return false otherwise
 */
bool GFXbutton::contains(int_fast16_t xPos, int_fast16_t yPos)
{
    return ((xPos >= _topLeftX) && (xPos < (int_fast16_t)(_topLeftX + _width)) && (yPos >= _topLeftY) && (yPos < (int_fast16_t)(_topLeftY + _height)));
}

/**
 * @brief Query whether the button was pressed since we last checked state
 *
 * @return true if was not-pressed before, now is.
 * @return false otherwise
 */
bool GFXbutton::justPressed(void)
{
    bool result {(_currState && !_lastState)};
    return result;
}

/**
 * @brief Query whether the button was released since we last checked state
 *
 * @return true if was pressed before, now is not.
 * @return false otherwise
 */
bool GFXbutton::justReleased(void)
{
    bool result {(!_currState && _lastState)};
    return result;
}

// -------------------------------------------------------------------------

// GFXcanvas provide 1-bit offscreen canvases, the address of which can be passed to drawBitmap() or pushColors()
// This is here mostly to help with the proportionally-spaced fonts; adds a way to refresh a section of the
// screen without a massive flickering clear-and-redraw... but maybe you'll find other uses too.
// VERY RAM-intensive, since the buffer is in MCU memory and not the display driver... GXFcanvas might be minimally useful
// on an Uno-class board, but this and the others are much more likely to require at least a Mega or various recent ARM-type boards
// (recommended, as the text+bitmap draw can be pokey). GFXcanvas requires 1 bit per pixel (rounded up to nearest byte per scanline).

#ifdef __AVR__
// Bitmask tables of 0x80>>X and ~(0x80>>X), because X>>Y is slow on AVR
const uint8_t PROGMEM GFXcanvas::GFXsetBit[] {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
const uint8_t PROGMEM GFXcanvas::GFXclrBit[] {0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE};
#endif

/**
 * @brief Instatiate a GFX 1-bit canvas context for graphics
 *
 * @param[in] width Display width, in pixels
 * @param[in] height Display height, in pixels
 */
GFXcanvas::GFXcanvas(uint_fast16_t width, uint_fast16_t height) : GFX(width, height)
{
    uint32_t bytes {((width + 7) / 8) * height};

    if (nullptr != (_buffer = (uint8_t *)malloc(bytes)))
    {
        memset(_buffer, 0, bytes);
    }
}

/**
 * @brief Delete the canvas, free memory
 */
GFXcanvas::~GFXcanvas(void)
{
    if (_buffer)
    {
        free(_buffer);
    }
}

/**
 * @brief Draw a pixel to the canvas framebuffer
 *
 * @param[in] xPos x coordinate
 * @param[in] yPos y coordinate
 * @param[in] color 1-bit Color to fill with
 */
void GFXcanvas::drawPixel(int_fast16_t xPos, int_fast16_t yPos, uint_fast8_t color)
{
    if (_buffer)
    {
        if ((xPos >= 0) && (yPos >= 0) && (xPos < _width) && (yPos < _height))
        {
            int_fast16_t t;
            switch (_rotation)
            {
                case 1:
                    t = xPos;
                    xPos = _baseWidth - 1 - yPos;
                    yPos = t;
                    break;
                case 2:
                    xPos = _baseWidth - 1 - xPos;
                    yPos = _baseHeight - 1 - yPos;
                    break;
                case 3:
                    t = xPos;
                    xPos = yPos;
                    yPos = _baseHeight - 1 - t;
                    break;
            }

            uint8_t *ptr {&_buffer[(xPos / 8) + yPos * ((_baseWidth + 7) / 8)]};
        #ifdef __AVR__
            if (color)
            {
                *ptr |= pgm_read_byte(&GFXsetBit[xPos & 7]);
            } else
            {
                *ptr &= pgm_read_byte(&GFXclrBit[xPos & 7]);
            }
        #else
            if (color)
            {
                *ptr |= 0x80 >> (xPos & 7);
            } else
            {
                *ptr &= ~(0x80 >> (xPos & 7));
            }
        #endif
        }
    }
}

/**
 * @brief Get the pixel color value at a given coordinate
 *
 * @param[in] xPos x coordinate
 * @param[in] yPos y coordinate
 * @return true if pixel is on (0x1)
 * @return false if pixel is off (0x0)
 */
bool GFXcanvas::getPixel(int_fast16_t xPos, int_fast16_t yPos) const
{
    int_fast16_t t {0};

    switch (_rotation)
    {
        case 1:
            t = xPos;
            xPos = _baseWidth - 1 - yPos;
            yPos = t;
            break;
        case 2:
            xPos = _baseWidth - 1 - xPos;
            yPos = _baseHeight - 1 - yPos;
            break;
        case 3:
            t = xPos;
            xPos = yPos;
            yPos = _baseHeight - 1 - t;
            break;
    }
    return getRawPixel(xPos, yPos);
}

/**
 * @brief Get the pixel color value at a given, unrotated coordinate.
 *        This method is intended for hardware drivers to get pixel value in physical coordinates.
 *
 * @param[in] xPos x coordinate
 * @param[in] yPos y coordinate
 * @return true if pixel is on (0x1)
 * @return false if pixel is off (0x0)
 */
bool GFXcanvas::getRawPixel(int_fast16_t xPos, int_fast16_t yPos) const
{
    bool result {false};

    if ((0 <= xPos) && (0 <= yPos) && (_baseWidth > xPos) && (_baseHeight > yPos))
    {
        if (nullptr != _buffer)
        {
            uint8_t *ptr {&_buffer[(xPos / 8) + yPos * ((_baseWidth + 7) / 8)]};

        #ifdef __AVR__
            result = (((*ptr) & pgm_read_byte(&GFXsetBit[xPos & 7])) != 0);
        #else
            result = (((*ptr) & (0x80 >> (xPos & 7))) != 0);
        #endif
        }
    }

    return result;
}

/**
 * @brief Fill the framebuffer completely with one color
 *
 * @param[in] color 1-bit Color to fill with
 */
void GFXcanvas::fillScreen(uint_fast8_t color)
{
    if (nullptr != _buffer)
    {
        uint32_t bytes {(uint32_t)(((_baseWidth + 7) / 8) * _baseHeight)};
        memset(_buffer, (0 != color) ? 0xFF : 0x00, bytes);
    }
}

/**
 * @brief Speed optimized vertical line drawing
 *
 * @param[in] xPos Line horizontal start point
 * @param[in] yPos Line vertical start point
 * @param[in] heightArg Length of vertical line to be drawn, including first point
 * @param[in] color 1-bit Color to fill with
 */
void GFXcanvas::drawFastVLine(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t heightArg, uint_fast8_t color)
{
    if (0 > heightArg)                                              // Convert negative heights to positive equivalent
    {
        heightArg *= -1;
        yPos -= heightArg - 1;
        if (0 > yPos)
        {
            heightArg += yPos;
            yPos = 0;
        }
    }

    // Edge rejection (no-draw if totally off canvas)
    if ((0 <= xPos) && (width() > xPos) && (height() > yPos) && (0 <= (yPos + heightArg - 1)))
    {
        if (0 > yPos)                                               // Clip top
        {
            heightArg += yPos;
            yPos = 0;
        }
        if (height() < (yPos + heightArg))                          // Clip bottom
        {
            heightArg = height() - yPos;
        }

        if (0 == getRotation())
        {
            drawFastRawVLine(xPos, yPos, heightArg, color);
        } else if (1 == getRotation())
        {
            int_fast16_t t {xPos};

            xPos = _baseWidth - 1 - yPos;
            yPos = t;
            xPos -= heightArg - 1;
            drawFastRawHLine(xPos, yPos, heightArg, color);
        } else if (2 == getRotation())
        {
            xPos = _baseWidth - 1 - xPos;
            yPos = _baseHeight - 1 - yPos;

            yPos -= heightArg - 1;
            drawFastRawVLine(xPos, yPos, heightArg, color);
        } else if (3 == getRotation())
        {
            int_fast16_t t {xPos};
            xPos = yPos;
            yPos = _baseHeight - 1 - t;
            drawFastRawHLine(xPos, yPos, heightArg, color);
        }
    }
}

/**
 * @brief Speed optimized horizontal line drawing
 *
 * @param[in] xPos Line horizontal start point
 * @param[in] yPos Line vertical start point
 * @param[in] widthArg Length of horizontal line to be drawn, including first point
 * @param[in] color 1-bit Color to fill with
 */
void GFXcanvas::drawFastHLine(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t widthArg, uint_fast8_t color)
{
    if (0 > widthArg)                                               // Convert negative widths to positive equivalent
    {
        widthArg *= -1;
        xPos -= widthArg - 1;
        if (0 > xPos)
        {
            widthArg += xPos;
            xPos = 0;
        }
    }

    // Edge rejection (no-draw if totally off canvas)
    if ((0 <= yPos) && (height() > yPos) && (width() > xPos) && (0 <= (xPos + widthArg - 1)))
    {
        if (0 > xPos)                                               // Clip left
        {
            widthArg += xPos;
            xPos = 0;
        }
        if (width() <= (xPos + widthArg))                           // Clip right
        {
            widthArg = width() - xPos;
        }

        if (0 == getRotation())
        {
            drawFastRawHLine(xPos, yPos, widthArg, color);
        } else if (1 == getRotation())
        {
            int_fast16_t t {xPos};
            xPos = _baseWidth - 1 - yPos;
            yPos = t;
            drawFastRawVLine(xPos, yPos, widthArg, color);
        } else if (2 == getRotation())
        {
            xPos = _baseWidth - 1 - xPos;
            yPos = _baseHeight - 1 - yPos;

            xPos -= widthArg - 1;
            drawFastRawHLine(xPos, yPos, widthArg, color);
        } else if (3 == getRotation())
        {
            int_fast16_t t {xPos};
            xPos = yPos;
            yPos = _baseHeight - 1 - t;
            xPos -= widthArg - 1;
            drawFastRawVLine(xPos, yPos, widthArg, color);
        }
    }
}

/**
 * @brief Speed optimized vertical line drawing into the raw canvas buffer
 *
 * @param[in] xPos Line horizontal start point
 * @param[in] yPos Line vertical start point
 * @param[in] heightArg length of vertical line to be drawn, including first point
 * @param[in] color 1-bit Color to fill with
 */
void GFXcanvas::drawFastRawVLine(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t heightArg, uint_fast8_t color)
{
    // xPos & yPos already in raw (rotation 0) coordinates, no need to transform.
    int_fast16_t    row_bytes   {((_baseWidth + 7) / 8)};
    uint8_t        *ptr         {&_buffer[(xPos / 8) + yPos * row_bytes]};

    if (0 < color)
    {
    #ifdef __AVR__
        uint8_t bit_mask    {pgm_read_byte(&GFXsetBit[xPos & 7])};
    #else
        uint8_t bit_mask    {(uint8_t)(0x80 >> (xPos & 7))};
    #endif
        for (int_fast16_t i = 0; i < heightArg; i++)
        {
            *ptr |= bit_mask;
            ptr += row_bytes;
        }
    } else
    {
    #ifdef __AVR__
        uint8_t         bit_mask    {pgm_read_byte(&GFXclrBit[xPos & 7])};
    #else
        uint_fast8_t    bit_mask    {(uint8_t)(~(0x80 >> (xPos & 7)))};
    #endif
        for (int_fast16_t i = 0; i < heightArg; i++)
        {
            *ptr &= bit_mask;
            ptr += row_bytes;
        }
    }
}

/**
 * @brief Speed optimized horizontal line drawing into the raw canvas buffer
 *
 * @param[in] xPos Line horizontal start point
 * @param[in] yPos Line vertical start point
 * @param[in] widthArg length of horizontal line to be drawn, including first point
 * @param[in] color 1-bit Color to fill with
 */
void GFXcanvas::drawFastRawHLine(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t widthArg, uint_fast8_t color)
{
    // xPos & yPos already in raw (rotation 0) coordinates, no need to transform.
    int_fast16_t    rowBytes            {((_baseWidth + 7) / 8)};
    uint8_t        *ptr                 {&_buffer[(xPos / 8) + yPos * rowBytes]};
    size_t          remainingWidthBits  {(size_t)widthArg};

    if (0 < (xPos & 7))                                             // check to see if first byte needs to be partially filled
    {
        uint_fast8_t startByteBitMask {0x00};                       // create bit mask for first byte

        for (int_fast8_t i = (xPos & 7); ((i < 8) && (remainingWidthBits > 0)); i++)
        {
        #ifdef __AVR__
            startByteBitMask |= pgm_read_byte(&GFXsetBit[i]);
        #else
            startByteBitMask |= (0x80 >> i);
        #endif
            remainingWidthBits--;
        }
        if (0 < color)
        {
            *ptr |= startByteBitMask;
        } else
        {
            *ptr &= ~startByteBitMask;
        }

        ptr++;
    }

    if (0 < remainingWidthBits)                                     // do the next remainingWidthBits bits
    {
        size_t          remainingWholeBytes {remainingWidthBits / 8};
        size_t          lastByteBits        {remainingWidthBits % 8};
        uint_fast8_t    wholeByteColor      {(uint_fast8_t)(0 < color ? 0xFF : 0x00)};

        memset(ptr, wholeByteColor, remainingWholeBytes);

        if (0 < lastByteBits)
        {
            uint_fast8_t lastByteBitMask {0x00};

            for (size_t i = 0; i < lastByteBits; i++)
            {
            #ifdef __AVR__
                lastByteBitMask |= pgm_read_byte(&GFXsetBit[i]);
            #else
                lastByteBitMask |= (0x80 >> i);
            #endif
            }
            ptr += remainingWholeBytes;

            if (0 < color)
            {
                *ptr |= lastByteBitMask;
            } else
            {
                *ptr &= ~lastByteBitMask;
            }
        }
    }
}
