// Font structures for GFX library.
// Example fonts are included in 'fonts' directory.
// To use a font in your Arduino sketch, #include the corresponding *.h
// file and pass address of GFXfont struct to setFont(). Pass NULL to
// revert to 'classic' fixed-space bitmap font.

#pragma once

/// Font data stored PER GLYPH
typedef struct
{
    uint16_t        bitmapOffset;   // Pointer into GFXfont->bitmap
    uint_fast8_t    width;          // Bitmap dimensions in pixels
    uint_fast8_t    height;         // Bitmap dimensions in pixels
    uint_fast8_t    xAdvance;       // Distance to advance cursor (x axis)
    int_fast8_t     xOffset;        // X dist from cursor pos to UL corner
    int_fast8_t     yOffset;        // Y dist from cursor pos to UL corner
} GFXglyph;

/// Data stored for FONT AS A WHOLE
typedef struct
{
    uint8_t        *bitmap;         // Glyph bitmaps, concatenated
    GFXglyph       *glyph;          // Glyph array
    uint_fast16_t   first;          // ASCII extents (first char)
    uint_fast16_t   last;           // ASCII extents (last char)
    uint_fast8_t    yAdvance;       // Newline distance (y axis)
} GFXfont;
