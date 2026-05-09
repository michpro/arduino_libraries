/**
 * @file        gfx.h
 * @brief       Graphics superclass handles drawing primitives
 *
 * @copyright   SPDX-FileCopyrightText: Copyright 2026 Michal Protasowicki
 *              Based on Adafruit GFX Library, Copyright (c) 2013 Adafruit Industries
 * @license     SPDX-License-Identifier: MIT
 */

#pragma once

#if ARDUINO >= 100
#   include "Arduino.h"
#   include "Print.h"
#else
#   include "WProgram.h"
#endif

#include "gfxfont.h"

//*******************************************************************
//*                                                                 *
//*                        Class Definition                         *
//*                                                                 *
//*******************************************************************

/**
 * @brief A generic graphics superclass that can handle all sorts of drawing.
 *        At a minimum you can subclass and provide drawPixel().
 *        At a maximum you can do a ton of overriding to optimize.
 */
class GFX : public Print
{

//*******************************************************************
//*                                                                 *
//*                         Public Methods                          *
//*                                                                 *
//*******************************************************************
public:
    GFX(int_fast16_t width, int_fast16_t height); // Constructor
    virtual ~GFX(void) = default;                 // Virtual destructor

    /**
     * @brief Draw to the screen/framebuffer/etc. Must be overridden in subclass.
     *
     * @param[in] xPos X coordinate in pixels
     * @param[in] yPos Y coordinate in pixels
     * @param[in] color 1-bit pixel color.
     */
    virtual void drawPixel(int_fast16_t xPos, int_fast16_t yPos, uint_fast8_t color) = 0;

    // TRANSACTION API / CORE DRAW API
    // These MAY be overridden by the subclass to provide device-specific optimized code. Otherwise 'generic' versions are used.
    virtual void startWrite(void);
    virtual void writePixel(int_fast16_t xPos, int_fast16_t yPos, uint_fast8_t color);
    virtual void writeFillRect(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t width, int_fast16_t height, uint_fast8_t color);
    virtual void writeFastVLine(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t height, uint_fast8_t color);
    virtual void writeFastHLine(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t width, uint_fast8_t color);
    virtual void writeLine(int_fast16_t x0, int_fast16_t y0, int_fast16_t x1, int_fast16_t y1, uint_fast8_t color);
    virtual void endWrite(void);

    // CONTROL API
    // These MAY be overridden by the subclass to provide device-specific optimized code. Otherwise 'generic' versions are used.
    virtual void setRotation(uint_fast8_t rotation);
    virtual void invertDisplay(bool invert);

    // BASIC DRAW API
    // These MAY be overridden by the subclass to provide device-specific optimized code. Otherwise 'generic' versions are used.

    // It's good to implement those, even if using transaction API
    virtual void drawFastVLine(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t height, uint_fast8_t color);
    virtual void drawFastHLine(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t width, uint_fast8_t color);
    virtual void fillRect(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t width, int_fast16_t height, uint_fast8_t color);
    virtual void fillScreen(uint_fast8_t color);
    // Optional and probably not necessary to change
    virtual void drawLine(int_fast16_t x0, int_fast16_t y0, int_fast16_t x1, int_fast16_t y1, uint_fast8_t color);
    virtual void drawRect(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t width, int_fast16_t height, uint_fast8_t color);

    // These exist only with GFX (no subclass overrides)

    /**
     * @brief Draw a circle outline.
     *
     * @param[in] x0     Center X coordinate.
     * @param[in] y0     Center Y coordinate.
     * @param[in] radius Radius in pixels.
     * @param[in] color  Color of the circle.
     */
    void drawCircle(int_fast16_t x0, int_fast16_t y0, int_fast16_t radius, uint_fast8_t color);

    /**
     * @brief Helper to draw quarters of a circle.
     *
     * @param[in] x0         Center X coordinate.
     * @param[in] y0         Center Y coordinate.
     * @param[in] radius     Radius in pixels.
     * @param[in] cornername Mask indicating which quarters to draw.
     * @param[in] color      Color of the circle.
     */
    void drawCircleHelper(int_fast16_t x0, int_fast16_t y0, int_fast16_t radius, uint_fast8_t cornername, uint_fast8_t color);

    /**
     * @brief Draw a filled circle.
     *
     * @param[in] x0     Center X coordinate.
     * @param[in] y0     Center Y coordinate.
     * @param[in] radius Radius in pixels.
     * @param[in] color  Color of the circle.
     */
    void fillCircle(int_fast16_t x0, int_fast16_t y0, int_fast16_t radius, uint_fast8_t color);

    /**
     * @brief Helper to draw filled quarters of a circle.
     *
     * @param[in] x0         Center X coordinate.
     * @param[in] y0         Center Y coordinate.
     * @param[in] radius     Radius in pixels.
     * @param[in] cornername Mask indicating which quarters to draw.
     * @param[in] delta      Offset from center-line, used to draw roundrects.
     * @param[in] color      Color of the circle.
     */
    void fillCircleHelper(int_fast16_t x0, int_fast16_t y0, int_fast16_t radius, uint_fast8_t cornername, int_fast16_t delta, uint_fast8_t color);

    /**
     * @brief Draw an ellipse outline.
     *
     * @param[in] x0      Center X coordinate.
     * @param[in] y0      Center Y coordinate.
     * @param[in] radiusW Horizontal radius.
     * @param[in] radiusH Vertical radius.
     * @param[in] color   Color of the ellipse.
     */
    void drawEllipse(int_fast16_t x0, int_fast16_t y0, int_fast16_t radiusW, int_fast16_t radiusH, uint_fast8_t color);

    /**
     * @brief Draw a filled ellipse.
     *
     * @param[in] x0      Center X coordinate.
     * @param[in] y0      Center Y coordinate.
     * @param[in] radiusW Horizontal radius.
     * @param[in] radiusH Vertical radius.
     * @param[in] color   Color of the ellipse.
     */
    void fillEllipse(int_fast16_t x0, int_fast16_t y0, int_fast16_t radiusW, int_fast16_t radiusH, uint_fast8_t color);

    /**
     * @brief Draw a triangle outline.
     *
     * @param[in] x0    First point X coordinate.
     * @param[in] y0    First point Y coordinate.
     * @param[in] x1    Second point X coordinate.
     * @param[in] y1    Second point Y coordinate.
     * @param[in] x2    Third point X coordinate.
     * @param[in] y2    Third point Y coordinate.
     * @param[in] color Color of the triangle.
     */
    void drawTriangle(int_fast16_t x0, int_fast16_t y0, int_fast16_t x1, int_fast16_t y1, int_fast16_t x2, int_fast16_t y2, uint_fast8_t color);

    /**
     * @brief Draw a filled triangle.
     *
     * @param[in] x0    First point X coordinate.
     * @param[in] y0    First point Y coordinate.
     * @param[in] x1    Second point X coordinate.
     * @param[in] y1    Second point Y coordinate.
     * @param[in] x2    Third point X coordinate.
     * @param[in] y2    Third point Y coordinate.
     * @param[in] color Color of the triangle.
     */
    void fillTriangle(int_fast16_t x0, int_fast16_t y0, int_fast16_t x1, int_fast16_t y1, int_fast16_t x2, int_fast16_t y2, uint_fast8_t color);

    /**
     * @brief Draw a rectangle with rounded corners.
     *
     * @param[in] x0     Top left X coordinate.
     * @param[in] y0     Top left Y coordinate.
     * @param[in] width  Width in pixels.
     * @param[in] height Height in pixels.
     * @param[in] radius Radius of the corners.
     * @param[in] color  Color of the rectangle.
     */
    void drawRoundRect(int_fast16_t x0, int_fast16_t y0, int_fast16_t width, int_fast16_t height, int_fast16_t radius, uint_fast8_t color);

    /**
     * @brief Draw a filled rectangle with rounded corners.
     *
     * @param[in] x0     Top left X coordinate.
     * @param[in] y0     Top left Y coordinate.
     * @param[in] width  Width in pixels.
     * @param[in] height Height in pixels.
     * @param[in] radius Radius of the corners.
     * @param[in] color  Color of the rectangle.
     */
    void fillRoundRect(int_fast16_t x0, int_fast16_t y0, int_fast16_t width, int_fast16_t height, int_fast16_t radius, uint_fast8_t color);

    /**
     * @brief Draw a rotated rectangle.
     *
     * @param[in] cenX     Center X coordinate.
     * @param[in] cenY     Center Y coordinate.
     * @param[in] width    Width in pixels.
     * @param[in] height   Height in pixels.
     * @param[in] angleDeg Rotation angle in degrees.
     * @param[in] color    Color of the rectangle.
     */
    void drawRotatedRect(int_fast16_t cenX, int_fast16_t cenY, int_fast16_t width, int_fast16_t height, int_fast16_t angleDeg, uint_fast8_t color);

    /**
     * @brief Draw a filled rotated rectangle.
     *
     * @param[in] cenX     Center X coordinate.
     * @param[in] cenY     Center Y coordinate.
     * @param[in] width    Width in pixels.
     * @param[in] height   Height in pixels.
     * @param[in] angleDeg Rotation angle in degrees.
     * @param[in] color    Color of the rectangle.
     */
    void fillRotatedRect(int_fast16_t cenX, int_fast16_t cenY, int_fast16_t width, int_fast16_t height, int_fast16_t angleDeg, uint_fast8_t color);

    /**
     * @brief Rotate a point around origin (0,0).
     *
     * @param[in,out] x0       X coordinate to rotate.
     * @param[in,out] y0       Y coordinate to rotate.
     * @param[in]     angleDeg Rotation angle in degrees.
     */
    void rotatePoint(int_fast16_t &x0, int_fast16_t &y0, int_fast16_t angleDeg);

    /**
     * @brief Draw a PROGMEM-resident 1-bit image.
     *
     * @param[in] xPos   Top left X coordinate.
     * @param[in] yPos   Top left Y coordinate.
     * @param[in] bitmap Pointer to the bitmap array in PROGMEM.
     * @param[in] width  Width of the bitmap in pixels.
     * @param[in] height Height of the bitmap in pixels.
     * @param[in] color  Color to draw pixels.
     */
    void drawBitmap(int_fast16_t xPos, int_fast16_t yPos, const uint8_t bitmap[], int_fast16_t width, int_fast16_t height, uint_fast8_t color);

    /**
     * @brief Draw a PROGMEM-resident 1-bit image with a background color.
     *
     * @param[in] xPos    Top left X coordinate.
     * @param[in] yPos    Top left Y coordinate.
     * @param[in] bitmap  Pointer to the bitmap array in PROGMEM.
     * @param[in] width   Width of the bitmap in pixels.
     * @param[in] height  Height of the bitmap in pixels.
     * @param[in] color   Color to draw foreground pixels.
     * @param[in] bgColor Color to draw background pixels.
     */
    void drawBitmap(int_fast16_t xPos, int_fast16_t yPos, const uint8_t bitmap[], int_fast16_t width, int_fast16_t height, uint_fast8_t color, uint_fast8_t bgColor);

    /**
     * @brief Draw a RAM-resident 1-bit image.
     *
     * @param[in] xPos   Top left X coordinate.
     * @param[in] yPos   Top left Y coordinate.
     * @param[in] bitmap Pointer to the bitmap array in RAM.
     * @param[in] width  Width of the bitmap in pixels.
     * @param[in] height Height of the bitmap in pixels.
     * @param[in] color  Color to draw pixels.
     */
    void drawBitmap(int_fast16_t xPos, int_fast16_t yPos, uint8_t *bitmap, int_fast16_t width, int_fast16_t height, uint_fast8_t color);

    /**
     * @brief Draw a RAM-resident 1-bit image with a background color.
     *
     * @param[in] xPos    Top left X coordinate.
     * @param[in] yPos    Top left Y coordinate.
     * @param[in] bitmap  Pointer to the bitmap array in RAM.
     * @param[in] width   Width of the bitmap in pixels.
     * @param[in] height  Height of the bitmap in pixels.
     * @param[in] color   Color to draw foreground pixels.
     * @param[in] bgColor Color to draw background pixels.
     */
    void drawBitmap(int_fast16_t xPos, int_fast16_t yPos, uint8_t *bitmap, int_fast16_t width, int_fast16_t height, uint_fast8_t color, uint_fast8_t bgColor);

    /**
     * @brief Draw XBitMap Files (*.xbm) exported from GIMP.
     *
     * @param[in] xPos   Top left X coordinate.
     * @param[in] yPos   Top left Y coordinate.
     * @param[in] bitmap Pointer to the XBM array.
     * @param[in] width  Width of the bitmap in pixels.
     * @param[in] height Height of the bitmap in pixels.
     * @param[in] color  Color to draw pixels.
     */
    void drawXBitmap(int_fast16_t xPos, int_fast16_t yPos, const uint8_t bitmap[], int_fast16_t width, int_fast16_t height, uint_fast8_t color);

    /**
     * @brief Draw a single character.
     *
     * @param[in] xPos      Top left X coordinate.
     * @param[in] yPos      Top left Y coordinate.
     * @param[in] character The 8-bit font-indexed character (from CP437 or custom).
     * @param[in] color     Color to draw character.
     * @param[in] bgColor   Background color.
     * @param[in] size      Size multiplier.
     */
    void drawChar(int_fast16_t xPos, int_fast16_t yPos, unsigned char character, uint_fast8_t color, uint_fast8_t bgColor, uint_fast8_t size);

    /**
     * @brief Draw a single character with different X and Y sizes.
     *
     * @param[in] xPos      Top left X coordinate.
     * @param[in] yPos      Top left Y coordinate.
     * @param[in] character The 8-bit font-indexed character.
     * @param[in] color     Color to draw character.
     * @param[in] bgColor   Background color.
     * @param[in] sizeX     X axis size multiplier.
     * @param[in] sizeY     Y axis size multiplier.
     */
    void drawChar(int_fast16_t xPos, int_fast16_t yPos, unsigned char character, uint_fast8_t color, uint_fast8_t bgColor, uint_fast8_t sizeX, uint_fast8_t sizeY);

    /**
     * @brief Calculate bounding box for a string.
     *
     * @param[in]  string C-string to calculate bounds for.
     * @param[in]  xPos   Starting X coordinate.
     * @param[in]  yPos   Starting Y coordinate.
     * @param[out] x1     Pointer to return calculated top left X coordinate.
     * @param[out] y1     Pointer to return calculated top left Y coordinate.
     * @param[out] width  Pointer to return calculated width.
     * @param[out] height Pointer to return calculated height.
     */
    void getTextBounds(const char *string, int_fast16_t xPos, int_fast16_t yPos, int_fast16_t *x1, int_fast16_t *y1, uint_fast16_t *width, uint_fast16_t *height);

    /**
     * @brief Calculate bounding box for a PROGMEM string.
     *
     * @param[in]  s      PROGMEM C-string to calculate bounds for.
     * @param[in]  xPos   Starting X coordinate.
     * @param[in]  yPos   Starting Y coordinate.
     * @param[out] x1     Pointer to return calculated top left X coordinate.
     * @param[out] y1     Pointer to return calculated top left Y coordinate.
     * @param[out] width  Pointer to return calculated width.
     * @param[out] height Pointer to return calculated height.
     */
    void getTextBounds(const __FlashStringHelper *s, int_fast16_t xPos, int_fast16_t yPos, int_fast16_t *x1, int_fast16_t *y1, uint_fast16_t *width, uint_fast16_t *height);

    /**
     * @brief Calculate bounding box for an Arduino String.
     *
     * @param[in]  str    String object to calculate bounds for.
     * @param[in]  xPos   Starting X coordinate.
     * @param[in]  yPos   Starting Y coordinate.
     * @param[out] x1     Pointer to return calculated top left X coordinate.
     * @param[out] y1     Pointer to return calculated top left Y coordinate.
     * @param[out] width  Pointer to return calculated width.
     * @param[out] height Pointer to return calculated height.
     */
    void getTextBounds(const String &str, int_fast16_t xPos, int_fast16_t yPos, int_fast16_t *x1, int_fast16_t *y1, uint_fast16_t *width, uint_fast16_t *height);

    /**
     * @brief Set text 'magnification' size.
     *
     * @param[in] size Size multiplier.
     */
    void setTextSize(uint_fast8_t size);

    /**
     * @brief Set text 'magnification' size with independent X/Y values.
     *
     * @param[in] sizeX X axis size multiplier.
     * @param[in] sizeY Y axis size multiplier.
     */
    void setTextSize(uint_fast8_t sizeX, uint_fast8_t sizeY);

    /**
     * @brief Set current font.
     *
     * @param[in] font Pointer to the GFXfont structure. Pass nullptr to revert to default font.
     */
    void setFont(const GFXfont *font = nullptr);

    /**
     * @brief Set text cursor location
     *
     * @param[in] xPos X coordinate in pixels
     * @param[in] yPos Y coordinate in pixels
     */
    inline void setCursor(int_fast16_t xPos, int_fast16_t yPos)
    {
        _cursorX = xPos;
        _cursorY = yPos;
    }

    /**
     * @brief Set text font color with transparant background
     *
     * @param[in] color 1-bit Color to draw text with
     *
     * @note For 'transparent' background, background and foreground
     *       are set to same color rather than using a separate flag.
     */
    inline void setTextColor(uint_fast16_t color)
    {
        _textColor = color;
        _textBgColor = color;
    }

    /**
     * @brief Set text font color with custom background color
     *
     * @param[in] color 1-bit Color to draw text with
     * @param[in] bgColor 1-bit Color to draw background/fill with
     */
    inline void setTextColor(uint_fast16_t color, uint_fast8_t bgColor)
    {
        _textColor = color;
        _textBgColor = bgColor;
    }

    /**
     * @brief Set whether text that is too long for the screen width should
     *        automatically wrap around to the next line (else clip right).
     *
     * @param[in] wrap true for wrapping, false for clipping
     */
    inline void setTextWrap(bool wrap)
    {
        _wrap = wrap;
    }


    using Print::write;
#if ARDUINO >= 100
    virtual size_t write(uint8_t );
#else
    virtual void write(uint8_t );
#endif

    /**
     * @brief Get width of the display, accounting for current rotation
     *
     * @return int_fast16_t Width in pixels
     */
    inline int_fast16_t width(void) const
    {
        return _width;
    }

    /**
     * @brief Get height of the display, accounting for current rotation
     *
     * @return int_fast16_t Height in pixels
     */
    inline int_fast16_t height(void) const
    {
        return _height;
    }

    /**
     * @brief Get rotation setting for display
     *
     * @return uint_fast8_t 0 thru 3 corresponding to 4 cardinal rotations
     */
    inline uint_fast8_t getRotation(void) const
    {
        return _rotation;
    }

    /**
     * @brief Get text cursor X location
     *
     * @return int_fast16_t X coordinate in pixels
     */
    inline int_fast16_t getCursorX(void) const
    {
        return _cursorX;
    }

    /**
     * @brief Get text cursor Y location
     *
     * @return int_fast16_t Y coordinate in pixels
     */
    inline int_fast16_t getCursorY(void) const
    {
        return _cursorY;
    }

//*******************************************************************
//*                                                                 *
//*                        Protected Methods                        *
//*                                                                 *
//*******************************************************************
protected:
    /**
     * @brief Calculate bounding box for a single character.
     *
     * @param[in]  character Character to calculate bounds for.
     * @param[in]  xPos      Current X cursor position.
     * @param[in]  yPos      Current Y cursor position.
     * @param[out] minx      Pointer to return calculated minimum X coordinate.
     * @param[out] miny      Pointer to return calculated minimum Y coordinate.
     * @param[out] maxx      Pointer to return calculated maximum X coordinate.
     * @param[out] maxy      Pointer to return calculated maximum Y coordinate.
     */
    void charBounds(unsigned char character, int_fast16_t *xPos, int_fast16_t *yPos, int_fast16_t *minx, int_fast16_t *miny, int_fast16_t *maxx, int_fast16_t *maxy);

    int_fast16_t    _baseWidth;     ///< This is the 'raw' display width - never changes
    int_fast16_t    _baseHeight;    ///< This is the 'raw' display height - never changes
    int_fast16_t    _width;         ///< Display width as modified by current rotation
    int_fast16_t    _height;        ///< Display height as modified by current rotation
    int_fast16_t    _cursorX;       ///< x location to start print()ing text
    int_fast16_t    _cursorY;       ///< y location to start print()ing text
    uint_fast16_t   _textColor;     ///< 1-bit background color for print()
    uint_fast16_t   _textBgColor;   ///< 1-bit text color for print()
    uint_fast8_t    _textSizeX;     ///< Desired magnification in X-axis of text to print()
    uint_fast8_t    _textSizeY;     ///< Desired magnification in Y-axis of text to print()
    uint_fast8_t    _rotation;      ///< Display rotation (0 thru 3)
    bool            _wrap;          ///< If set, 'wrap' text at right edge of display
    GFXfont        *_gfxFont;       ///< Pointer to special font
};

//*******************************************************************
//*                                                                 *
//*                        Class Definition                         *
//*                                                                 *
//*******************************************************************

/**
 * @brief A simple drawn button UI element
 */
class GFXbutton
{

//*******************************************************************
//*                                                                 *
//*                         Public Methods                          *
//*                                                                 *
//*******************************************************************
public:
    static constexpr uint8_t LABEL_MAX_LEN {10};    ///< Maximum characters allowed for button label (including null terminator)

    /**
     * @brief Construct a new GFXbutton object.
     */
    GFXbutton(void);

    /**
     * @brief Initialize button with a specific size and common text size.
     *
     * @param[in] gfx          Pointer to GFX instance.
     * @param[in] x1           Top-left X coordinate.
     * @param[in] y1           Top-left Y coordinate.
     * @param[in] width        Button width.
     * @param[in] height       Button height.
     * @param[in] outlineColor Outline color.
     * @param[in] fillColor    Fill color.
     * @param[in] textColor    Text color.
     * @param[in] label        Button label text.
     * @param[in] textSize     Text size multiplier.
     */
    void initButton(GFX *gfx, int_fast16_t x1, int_fast16_t y1, uint_fast16_t width, uint_fast16_t height, uint_fast16_t outlineColor, uint_fast16_t fillColor, uint_fast16_t textColor, char *label, uint_fast8_t textSize);

    /**
     * @brief Initialize button with independent X/Y text sizes.
     *
     * @param[in] gfx          Pointer to GFX instance.
     * @param[in] x1           Top-left X coordinate.
     * @param[in] y1           Top-left Y coordinate.
     * @param[in] width        Button width.
     * @param[in] height       Button height.
     * @param[in] outlineColor Outline color.
     * @param[in] fillColor    Fill color.
     * @param[in] textColor    Text color.
     * @param[in] label        Button label text.
     * @param[in] textSizeX    X axis text size multiplier.
     * @param[in] textSizeY    Y axis text size multiplier.
     */
    void initButton(GFX *gfx, int_fast16_t x1, int_fast16_t y1, uint_fast16_t width, uint_fast16_t height, uint_fast16_t outlineColor, uint_fast16_t fillColor, uint_fast16_t textColor, char *label, uint_fast8_t textSizeX, uint_fast8_t textSizeY);

    /**
     * @brief Draw the button on the screen.
     *
     * @param[in] inverted If true, draw inverted colors.
     */
    void drawButton(bool inverted = false);

    /**
     * @brief Check if point is inside the button bounding box.
     *
     * @param[in] xPos X coordinate.
     * @param[in] yPos Y coordinate.
     * @return true if point is inside button.
     * @return false otherwise.
     */
    bool contains(int_fast16_t xPos, int_fast16_t yPos);

    /**
     * @brief Sets button state, should be done by some touch function
     *
     * @param[in] pressed True for pressed, false for not.
     */
    inline void press(bool pressed)
    {
        _lastState = _currState;
        _currState = pressed;
    }

    /**
     * @brief Query whether the button was just pressed.
     *
     * @return true if button changed from not pressed to pressed.
     * @return false otherwise.
     */
    bool justPressed(void);

    /**
     * @brief Query whether the button was just released.
     *
     * @return true if button changed from pressed to not pressed.
     * @return false otherwise.
     */
    bool justReleased(void);

    /**
     * @brief Query whether the button is currently pressed
     *
     * @return true if pressed
     * @return false otherwise
     */
    inline bool isPressed(void)
    {
        bool result {_currState};
        return result;
    }

//*******************************************************************
//*                                                                 *
//*                         Private Methods                         *
//*                                                                 *
//*******************************************************************
private:
    GFX            *_gfx          {nullptr};              ///< Pointer to graphics context
    int_fast16_t    _topLeftX     {0};                    ///< Coordinates of top-left corner X
    int_fast16_t    _topLeftY     {0};                    ///< Coordinates of top-left corner Y
    uint_fast16_t   _width        {0};
    uint_fast16_t   _height       {0};
    uint_fast8_t    _textSizeX    {0};
    uint_fast8_t    _textSizeY    {0};
    uint_fast16_t   _outlineColor {0};
    uint_fast16_t   _fillColor    {0};
    uint_fast16_t   _textColor    {0};
    char            _label[LABEL_MAX_LEN] {0};

    bool            _currState    {false};
    bool            _lastState    {false};
};

//*******************************************************************
//*                                                                 *
//*                        Class Definition                         *
//*                                                                 *
//*******************************************************************

/**
 * @brief A GFX 1-bit canvas context for graphics
 */
class GFXcanvas : public GFX
{
//*******************************************************************
//*                                                                 *
//*                         Public Methods                          *
//*                                                                 *
//*******************************************************************
public:
    /**
     * @brief Construct a new GFXcanvas object.
     *
     * @param[in] width  Canvas width in pixels.
     * @param[in] height Canvas height in pixels.
     */
    GFXcanvas(uint_fast16_t width, uint_fast16_t height);

    /**
     * @brief Destroy the GFXcanvas object.
     */
    ~GFXcanvas(void);

    /**
     * @brief Draw a pixel in the canvas.
     *
     * @param[in] xPos  X coordinate.
     * @param[in] yPos  Y coordinate.
     * @param[in] color Pixel color.
     */
    void drawPixel(int_fast16_t xPos, int_fast16_t yPos, uint_fast8_t color);

    /**
     * @brief Fill the entire canvas with a color.
     *
     * @param[in] color Fill color.
     */
    void fillScreen(uint_fast8_t color);

    /**
     * @brief Draw a fast vertical line in the canvas.
     *
     * @param[in] xPos   Starting X coordinate.
     * @param[in] yPos   Starting Y coordinate.
     * @param[in] height Line height.
     * @param[in] color  Line color.
     */
    void drawFastVLine(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t height, uint_fast8_t color);

    /**
     * @brief Draw a fast horizontal line in the canvas.
     *
     * @param[in] xPos  Starting X coordinate.
     * @param[in] yPos  Starting Y coordinate.
     * @param[in] width Line width.
     * @param[in] color Line color.
     */
    void drawFastHLine(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t width, uint_fast8_t color);

    /**
     * @brief Get a pixel color from the canvas.
     *
     * @param[in] xPos X coordinate.
     * @param[in] yPos Y coordinate.
     * @return true if pixel is set.
     * @return false if pixel is cleared.
     */
    bool getPixel(int_fast16_t xPos, int_fast16_t yPos) const;

    /**
     * @brief Get a pointer to the internal buffer memory
     *
     * @return uint8_t* A pointer to the allocated buffer
     */
    inline uint8_t *getBuffer(void) const
    {
        return _buffer;
    }

//*******************************************************************
//*                                                                 *
//*                        Protected Methods                        *
//*                                                                 *
//*******************************************************************
protected:
    /**
     * @brief Get a raw pixel color from the internal buffer.
     *
     * @param[in] xPos X coordinate.
     * @param[in] yPos Y coordinate.
     * @return true if pixel is set.
     * @return false if pixel is cleared.
     */
    bool getRawPixel(int_fast16_t xPos, int_fast16_t yPos) const;

    /**
     * @brief Draw a fast raw vertical line.
     *
     * @param[in] xPos   Starting X coordinate.
     * @param[in] yPos   Starting Y coordinate.
     * @param[in] height Line height.
     * @param[in] color  Line color.
     */
    void drawFastRawVLine(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t height, uint_fast8_t color);

    /**
     * @brief Draw a fast raw horizontal line.
     *
     * @param[in] xPos  Starting X coordinate.
     * @param[in] yPos  Starting Y coordinate.
     * @param[in] width Line width.
     * @param[in] color Line color.
     */
    void drawFastRawHLine(int_fast16_t xPos, int_fast16_t yPos, int_fast16_t width, uint_fast8_t color);

    uint8_t *_buffer;    ///< Raster data: allow subclass access

//*******************************************************************
//*                                                                 *
//*                         Private Methods                         *
//*                                                                 *
//*******************************************************************
private:
#ifdef __AVR__
  // Bitmask tables of 0x80>>X and ~(0x80>>X), because X>>Y is slow on AVR
    static const uint8_t PROGMEM GFXsetBit[], GFXclrBit[];
#endif
};
