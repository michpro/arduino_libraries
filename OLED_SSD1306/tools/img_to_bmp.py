"""
This module provides a command-line tool to convert images into C arrays
for use with SSD1306 OLED displays.
"""
import argparse
import os
from PIL import Image


def _resize_image(img, width, height):
    """Resizes the image while maintaining aspect ratio if only one dimension is provided."""
    if width is not None and height is not None:
        return img.resize((width, height), Image.Resampling.LANCZOS)
    if width is not None:
        aspect_ratio = img.height / img.width
        return img.resize((width, int(width * aspect_ratio)), Image.Resampling.LANCZOS)
    if height is not None:
        aspect_ratio = img.width / img.height
        return img.resize((int(height * aspect_ratio), height), Image.Resampling.LANCZOS)
    return img


def _convert_to_bitmap_data(img, threshold, invert):
    """Converts image to black/white and packs pixels into byte array."""
    img = img.convert("L")
    if invert:
        img = img.point(lambda p: 0 if p > threshold else 1, mode="1")
    else:
        img = img.point(lambda p: 1 if p > threshold else 0, mode="1")

    img_w, img_h = img.size
    bytes_per_row = (img_w + 7) // 8
    data = []

    for y in range(img_h):
        for x_byte in range(bytes_per_row):
            byte_val = 0
            for bit in range(8):
                x = x_byte * 8 + bit
                if x < img_w:
                    pixel = img.getpixel((x, y))
                    if pixel != 0:
                        byte_val |= (1 << (7 - bit))
            data.append(byte_val)
    return data, img_w, img_h, bytes_per_row


def _write_c_array(out_path, img_path, array_name, img_w, img_h, bytes_per_row, data, binary_fmt):
    """Writes the packed bitmap data to a C header file."""
    with open(out_path, "w", encoding="utf-8") as f:
        f.write(f"// Generated from {os.path.basename(img_path)}\n")
        f.write(f"// Dimensions: {img_w}x{img_h} pixels\n")
        f.write("#include <stdint.h>\n")
        f.write("#ifdef __AVR__\n")
        f.write("#   include <avr/pgmspace.h>\n")
        f.write("#else\n")
        f.write("#   define PROGMEM\n")
        f.write("#endif\n\n")

        f.write(f"static const uint8_t PROGMEM {array_name}[]\n{{\n")

        for i in range(0, len(data), bytes_per_row):
            row_data = data[i:i + bytes_per_row]
            if binary_fmt:
                row_str = ", ".join([f"0b{b:08b}" for b in row_data])
            else:
                row_str = ", ".join([f"0x{b:02X}" for b in row_data])
            f.write(f"    {row_str},\n")

        f.write("};\n")


def image_to_c_array(img_path, array_name, out_path, width=None, height=None, threshold=128, binary_fmt=False, invert=False): # pylint: disable=too-many-arguments, too-many-positional-arguments, line-too-long
    """
    Convert an image file to a C-style array and save it to a header file.

    Args:
        img_path (str): Path to the input image.
        array_name (str): Name of the C array to generate.
        out_path (str): Path to the output header file.
        width (int, optional): Target width for resizing.
        height (int, optional): Target height for resizing.
        threshold (int, optional): Threshold for black/white conversion (0-255).
        binary_fmt (bool, optional): Output bytes in binary format if True.
        invert (bool, optional): Invert the colors if True.
    """
    try:
        img = Image.open(img_path)
    except Exception as e: # pylint: disable=broad-exception-caught
        print(f"Error opening image: {e}")
        return

    img = _resize_image(img, width, height)
    data, img_w, img_h, bytes_per_row = _convert_to_bitmap_data(img, threshold, invert)
    _write_c_array(out_path, img_path, array_name, img_w, img_h, bytes_per_row, data, binary_fmt)

    print(f"Successfully generated {out_path} ({img_w}x{img_h})")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Convert an image to a C PROGMEM array for SSD1306 (Horizontal format).") # pylint: disable=line-too-long
    parser.add_argument("input", help="Path to input image")
    parser.add_argument("-o", "--output", help="Path to output .h file", default="output.h")
    parser.add_argument("-n", "--name", help="Name of the C array", default="image_data")
    parser.add_argument("-W", "--width", type=int, help="Scale width (pixels)", default=None)
    parser.add_argument("-H", "--height", type=int, help="Scale height (pixels)", default=None)
    parser.add_argument("-t", "--threshold", type=int, help="B&W threshold (0-255)", default=128)
    parser.add_argument("-b", "--binary", action="store_true", help="Output in binary (0b) format instead of hex") # pylint: disable=line-too-long
    parser.add_argument("-i", "--invert", action="store_true", help="Invert colors (e.g., white background becomes unlit, black becomes lit)") # pylint: disable=line-too-long

    args = parser.parse_args()
    image_to_c_array(args.input, args.name, args.output, args.width, args.height, args.threshold, args.binary, args.invert) # pylint: disable=line-too-long
