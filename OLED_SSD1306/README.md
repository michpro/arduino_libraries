# **Library: OLED_SSD1306**

- [**Library: OLED\_SSD1306**](#library-oled_ssd1306)
  - [**Summary**](#summary)
    - [**Key Features**](#key-features)
  - [**Example of Usage**](#example-of-usage)
    - [**Explanation of the Example**](#explanation-of-the-example)

---

## **Summary**

The `OLED_SSD1306` library is a robust, modernized C++ library for monochrome OLED displays based on popular display controllers. It is built on the foundation of the original Adafruit SSD1306 and Adafruit GFX libraries but has been highly optimized and adapted for modern C++ standards.

### **Key Features**

* **Broad Resolution Support**: Pre-configured definitions for a wide array of display resolutions and sizes, including: `128x64`, `128x32`, `96x16`, `72x40`, `64x128`, `64x48`, `64x32`, and `48x88`.
* **Supported Controllers**: Native support for SSD1306, SSD1309, SSD1312, SSD1315, SH1106, SH1107, and CH1115 display controllers.
* **Modern C++ Architecture**: Utilizes strong typing (`enum class`), `constexpr` for hardware constants (eliminating "magic numbers"), and uniform braced initialization for safer and more maintainable code.
* **Hardware-Specific Optimizations**: Includes tailored fixes and routines for specific hardware quirks, such as diagonal scrolling corrections for the SSD1315 and specific voltage pump configurations for the SH1106.
* **Hardware Interfaces**: Full support for I²C and SPI (both hardware and software SPI).
* **Integrated GFX Subsystem**: Incorporates an optimized `GFX` graphics engine providing a complete set of drawing primitives (pixels, lines, rectangles, circles, triangles, rounded rectangles), custom fonts, and text manipulation (including independent X/Y scaling and text wrapping).
* **Advanced Hardware Control**: Access to native display capabilities like hardware scrolling, display inversion, and contrast dimming.

---

## **Example of Usage**

Below is a simple example demonstrating how to initialize an I²C OLED display, draw text, and basic graphics.

```cpp
#include "OLED_SSD1306.h"

// Declaration for a 128x64 display connected to I²C (SDA, SCL pins)
OLED::SSD1306 display(OLED::display_t::SSD1306_128x64, &Wire, OLED::Constants::PIN_NONE);

void setup()
{
    Serial.begin(115200);

    // Initialize display with internal charge pump and default I2C address
    if(!display.begin(OLED::voltageSrc_t::V_INTERNAL, OLED::Constants::I2C_ADDR_3C, true, true))
    {
        Serial.println("SSD1306 allocation failed");
        for(;;); // Don't proceed, loop forever
    }

    // Clear the buffer
    display.clearDisplay();

    // Draw a single pixel
    display.drawPixel(10, 10, (uint8_t)OLED::pixel_t::ON);

    // Draw a rectangle
    display.drawRect(20, 20, 80, 18, (uint8_t)OLED::pixel_t::ON);

    // Setup text properties
    display.setTextSize(1);
    display.setTextColor((uint8_t)OLED::pixel_t::ON);
    display.setCursor(25, 25);

    // Print text
    display.println("Hello, OLED!");

    // Push the buffer to the screen
    display.display();
}

void loop()
{
    // Nothing to do in the loop
}
```

### **Explanation of the Example**

1. **Initialization**: The `display` object is created, passing the display resolution and controller type (`OLED::display_t::SSD1306_128x64`), a pointer to the I²C interface (`&Wire`), and `OLED::Constants::PIN_NONE` to indicate no separate reset pin is used.
2. **Setup**: In `setup()`, the `begin()` method is called to initialize the OLED with its I²C address (`OLED::Constants::I2C_ADDR_3C`) using the internal charge pump (`OLED::voltageSrc_t::V_INTERNAL`).
3. **Drawing Primitives**:
   - `clearDisplay()` clears the internal memory buffer.
   - `drawPixel()` sets a single pixel at the specified coordinates.
   - `drawRect()` outlines a rectangle.
4. **Text Output**: Properties like size, color, and cursor position are set before printing the string "Hello, OLED!".
5. **Update Screen**: `display()` transfers the internal buffer contents to the actual OLED display, making the drawing visible.

---
