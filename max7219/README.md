# Library: MAX7219

- [Library: MAX7219](#library-max7219)
  - [Summary](#summary)
  - [Usage Example](#usage-example)
    - [Explanation](#explanation)

---

## Summary

The `Max7219` library is designed to control MAX7219 chips, which are serially interfaced drivers for 8-digit 7-segment LED displays or LED matrices. It provides a `Max7219` class within the `Max7219NS` namespace, allowing users to:

- Initialize and configure a chain of MAX7219 chips.
- Adjust display intensity.
- Clear the display or specific positions.
- Write data to individual digit positions, supporting both raw segment data and BCD decoding.
- Manage chip states (shutdown, activate, test mode).

The library uses a `context_t` structure to store configuration details such as pin assignments, number of devices, and display settings. It’s ideal for Arduino projects requiring LED display control.

---

## Usage Example

Here’s an example demonstrating how to use the library to display the digits "12345678" on a single MAX7219 chip connected to an Arduino.

```cpp
#include "max7219.h"

Max7219NS::context_t ctx;

void setup()
{
    // Configure the context for a single MAX7219 chip
    ctx.csbPin      = 2;        // Chip Select pin
    ctx.clkPin      = 3;        // Clock pin
    ctx.dataPin     = 4;        // Data pin
    ctx.scanDigits  = 8;        // Display all 8 digits
    ctx.intensity   = 0x0F;     // Maximum brightness
    ctx.numDevices  = 1;        // One chip in the chain
    ctx.decodeBcd   = true;     // Enable BCD decoding for digits 0-9

    // Create Max7219 instance
    Max7219 max7219(ctx);

    // Initialize the chip
    if (max7219.init())
    {
        max7219.clear();        // Clear the display

        // Write digits 1 to 8
        for (uint8_t pos = 0; pos < 8; pos++)
        {
            max7219.write(pos, pos + 1);            // Display 1 to 8
        }
    }
}

void loop()
{
    // No action needed in loop
}
```

### Explanation
- **Context Setup**: The `context_t` struct is configured with pin numbers (CSB=2, CLK=3, DATA=4), 8 digits, maximum intensity, and BCD decoding enabled.
- **Initialization**: The `Max7219` instance is created and initialized with `init()`.
- **Display**: The display is cleared, and digits 1 through 8 are written to positions 0 to 7.
- **Multiple Chips**: For a chain of multiple chips, increase `ctx.numDevices` and repeat `write` calls to address additional digits.

This example assumes the MAX7219 is wired to the specified Arduino pins and connected to a 7-segment display. Adjust pin numbers as needed for your hardware setup.

---
