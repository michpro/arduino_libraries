"""
This module converts TrueType fonts (TTF) into Adafruit GFX compatible
header files for use with microcontroller display libraries.
"""
import argparse
import os
import freetype


def _pack_glyph_bitmap(bitmap, width, height):
    """
    Packs a FreeType mono bitmap into Adafruit GFX byte format.
    """
    packed_bytes = []
    if width <= 0 or height <= 0:
        return packed_bytes

    current_byte = 0
    bit_pos = 0
    pitch = bitmap.pitch

    for y in range(height):
        for x in range(width):
            byte_idx = y * pitch + (x // 8)
            bit_val = (bitmap.buffer[byte_idx] >> (7 - (x % 8))) & 1

            current_byte |= (bit_val << (7 - bit_pos))
            bit_pos += 1
            if bit_pos == 8:
                packed_bytes.append(current_byte)
                current_byte = 0
                bit_pos = 0
    if bit_pos != 0:
        packed_bytes.append(current_byte)
    return packed_bytes


def _build_char_map(ranges, mappings):
    """Builds a mapping from GFX index to Unicode codepoint."""
    char_map = {}
    if not ranges and not mappings:
        for i in range(32, 127):
            char_map[i] = i
    else:
        for r in ranges:
            start_str, end_str = r.split("-")
            for i in range(int(start_str), int(end_str) + 1):
                char_map[i] = i
        for m in mappings:
            idx_str, uni_str = m.split(":")
            char_map[int(idx_str)] = int(uni_str)
    return char_map


def _write_gfx_header(output_file, ttf_path, size_pt, font_name, bitmaps_data, glyphs_info, first_char, last_char, y_advance): # pylint: disable=too-many-arguments, too-many-positional-arguments, line-too-long
    """Writes the generated data to a C header file."""
    with open(output_file, "w", encoding="utf-8") as f:
        f.write(f"// Generated from {os.path.basename(ttf_path)}, size {size_pt}\n")
        f.write("#include <stdint.h>\n")
        f.write("#ifdef __AVR__\n")
        f.write("#   include <avr/pgmspace.h>\n")
        f.write("#else\n")
        f.write("#   define PROGMEM\n")
        f.write("#endif\n\n")

        f.write(f"const uint8_t {font_name}Bitmaps[] PROGMEM = {{\n")
        for i in range(0, len(bitmaps_data), 12):
            chunk = bitmaps_data[i:i+12]
            row_str = ", ".join([f"0x{b:02X}" for b in chunk])
            f.write(f"    {row_str},\n")
        f.write("};\n\n")

        f.write(f"const GFXglyph {font_name}Glyphs[] PROGMEM = {{\n")
        for i, g in enumerate(glyphs_info):
            f.write(f"    {{ {g['offset']:5}, {g['width']:3}, {g['height']:3}, {g['x_advance']:3}, {g['x_offset']:4}, {g['y_offset']:4} }}, // 0x{(first_char + i):02X}\n") # pylint: disable=line-too-long
        f.write("};\n\n")

        f.write(f"const GFXfont {font_name} PROGMEM = {{\n")
        f.write(f"    (uint8_t  *){font_name}Bitmaps,\n")
        f.write(f"    (GFXglyph *){font_name}Glyphs,\n")
        f.write(f"    0x{first_char:02X}, 0x{last_char:02X}, {y_advance}\n")
        f.write("};\n")


def _process_glyphs(face, char_map, first_char, last_char):
    """Processes glyphs and returns bitmap data and glyph info."""
    bitmaps_data = []
    glyphs_info = []
    current_bitmap_offset = 0

    for gfx_idx in range(first_char, last_char + 1):
        if gfx_idx in char_map:
            uni_cp = char_map[gfx_idx]
            face.load_char(chr(uni_cp), freetype.FT_LOAD_RENDER | freetype.FT_LOAD_TARGET_MONO)

            bitmap = face.glyph.bitmap
            width = bitmap.width
            height = bitmap.rows

            packed_bytes = _pack_glyph_bitmap(bitmap, width, height)

            glyphs_info.append({
                "offset": current_bitmap_offset,
                "width": width,
                "height": height,
                "x_advance": face.glyph.advance.x >> 6,
                "x_offset": face.glyph.bitmap_left,
                "y_offset": -face.glyph.bitmap_top # Relative to baseline
            })

            bitmaps_data.extend(packed_bytes)
            current_bitmap_offset += len(packed_bytes)
        else:
            # Missing character, insert empty glyph
            glyphs_info.append({
                "offset": current_bitmap_offset,
                "width": 0,
                "height": 0,
                "x_advance": 0,
                "x_offset": 0,
                "y_offset": 0
            })
    return bitmaps_data, glyphs_info


def generate_gfx_font(ttf_path, size_pt, output_file, font_name, ranges, mappings): # pylint: disable=too-many-arguments, too-many-positional-arguments
    """
    Generate an Adafruit GFX compatible font header from a TTF file.

    Args:
        ttf_path (str): Path to the TrueType font file.
        size_pt (int): Font size in points/pixels.
        output_file (str): Path to the generated output header file.
        font_name (str): Name of the generated font structure.
        ranges (list of str): List of character ranges to include (e.g., '32-126').
        mappings (list of str): List of specific character mappings (e.g., '128:223').
    """
    char_map = _build_char_map(ranges, mappings)

    if not char_map:
        print("No characters specified to convert.")
        return

    first_char = min(char_map.keys())
    last_char = max(char_map.keys())

    try:
        face = freetype.Face(ttf_path)
    except Exception as e: # pylint: disable=broad-exception-caught
        print(f"Error loading font: {e}")
        return

    face.set_pixel_sizes(0, size_pt)

    y_advance = face.size.height >> 6
    if y_advance == 0:
        y_advance = size_pt

    bitmaps_data, glyphs_info = _process_glyphs(face, char_map, first_char, last_char)

    _write_gfx_header(output_file, ttf_path, size_pt, font_name, bitmaps_data, glyphs_info, first_char, last_char, y_advance) # pylint: disable=line-too-long

    print(f"Successfully generated {output_file} (Index 0x{first_char:02X} to 0x{last_char:02X})")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Convert TTF to Adafruit GFX Font format.")
    parser.add_argument("input", help="Path to input TTF file")
    parser.add_argument("size", type=int, help="Font size in pixels/points")
    parser.add_argument("-o", "--output", help="Path to output .h file", default="output_font.h")
    parser.add_argument("-n", "--name", help="Name of the font struct", default="custom_font")

    parser.add_argument("--range", action="append", default=[], help="Range of characters (e.g., 32-127). Can be used multiple times.") # pylint: disable=line-too-long
    parser.add_argument("--map", action="append", default=[], help="Map a unicode char to a specific GFX index (e.g., 128:223). Can be used multiple times.") # pylint: disable=line-too-long

    args = parser.parse_args()

    generate_gfx_font(args.input, args.size, args.output, args.name, args.range, args.map)
