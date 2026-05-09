# OLED_SSD1306 Tool Scripts Guide

This directory contains Python utility scripts to help convert resources (fonts and images) into a format compatible with the `OLED_SSD1306` library architecture and the `GFX` module.

## Prerequisites

Before running the tools for the first time, make sure you have Python installed (version 3.6 or newer).
We strongly recommend using a **virtual environment (.venv)** to avoid cluttering global system packages.

To install the required packages (`Pillow` for raster graphics, `freetype-py` for vector font rendering), open a terminal in this directory and follow these instructions:

```bash
# 1. Create a virtual environment (optional but highly recommended)
python -m venv .venv

# 2. Activate the virtual environment
# Windows:
.venv\Scripts\activate
# Linux/macOS:
source .venv/bin/activate

# 3. Install packages
pip install -r requirements.txt
```

---

## 1. Image to Bitmap Converter (`img_to_bmp.py`)

This script converts any standard image (e.g., `.png`, `.jpg`) into a black-and-white bitmap formatted for the display as a Flash memory array (`PROGMEM`). The code arranges bits using horizontal addressing (each consecutive byte represents 8 pixels on the X-axis), which is natively supported by the SSD1306 display and optimized for minimal RAM usage.

### Key arguments:
* `-W` / `--width` - Set the target width (the script will automatically maintain the height aspect ratio).
* `-H` / `--height` - Set the target height (the script will automatically maintain the width aspect ratio).
* `-t` / `--threshold` - Brightness cutoff threshold on a 0-255 scale for B&W conversion (default: 128).
* `-b` / `--binary` - Formats the array in binary form (e.g., `0b01111110`), making it easier to "read" small icon code. Without this parameter, it defaults to a concise hexadecimal format (e.g., `0x7E`).
* `-n` / `--name` - Name of the generated C/C++ array.
* `-o` / `--output` - Output file name with a `.h` extension.
* `-i` / `--invert` - Inverts colors during conversion (e.g., white background becomes unlit, and black elements become lit on the OLED).

### Usage examples:

**Default hexadecimal conversion:**
```bash
python img_to_bmp.py my_image.png -n my_image_array -o my_image.h
```

**Small icon conversion (binary format):**
```bash
python img_to_bmp.py icon.png -W 32 -b -n my_icon_array -o my_icon.h
```

---

## 2. Adafruit GFX Font Converter (`ttf_to_gfx.py`)

This script transforms system fonts (`.ttf`, `.otf`) into `.h` header files containing the `GFXfont` structure required by the `GFX` layer. The tool reads vector proportions using the FreeType engine, creating 100% compliant glyph bounding boxes, margins, and offsets.

### Key arguments:
* `input` - Path to the font file (required positional argument 1).
* `size` - Target point/pixel size of the font (required positional argument 2).
* `--range` - Defines a continuous range of Unicode codes to include in the output file (e.g., `--range 32-127`). If omitted, this range is used by default.
* `--map` - Mapping functionality that allows inserting a specific Unicode glyph at a specific, virtual 8-bit cell address within the GFX font. This enables adding special characters outside the standard table and assigning them to lower bytes.

### Usage examples:

**Standard ASCII font generation for 12 pixels:**
```bash
python ttf_to_gfx.py Arial.ttf 12 -n Arial12pt -o Arial12.h
```

**Advanced Usage with Character Mapping and Remapping:**
Often, we want access to non-standard characters (e.g., Celsius degrees, symbols, or diacritics). Standard ASCII tables do not include them up to byte `127`. Instead of generating a massive, full font up to index `300`, you can use the `--map` function.

For example, suppose we want to keep the full English layout of codes `32-127` in our GFX font, but we also want to display the Unicode character `223` (the degree symbol °, from the `.ttf` file) by simply typing the character with index `128` on the microcontroller:

```bash
python ttf_to_gfx.py Arial.ttf 10 --range 32-127 --map 128:223 --map 129:224 --map 130:333 -o CustomFont.h -n CustomFont10pt
```
In the example above, the script will:
- Fetch standard glyphs 32-127 and keep them under their original codes.
- Locate codes `223`, `224`, and `333` in the source `.ttf` file, then build only their minimized bitmaps at the final array codes `128`, `129`, and `130`.
- Unnecessary gaps in characters between `128` and `333` won't take up any Flash memory space at all. Instead, you get a compact array from `32` to `130`.

Now, in your microcontroller program, you simply pass the index to the print function: `display.print((char)128);` to render character `223` from the original TTF file.
