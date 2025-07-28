# Library: MCP402X

- [Library: MCP402X](#library-mcp402x)
  - [Summary](#summary)
  - [Usage Example](#usage-example)
    - [How to Use](#how-to-use)

---

## Summary

The `Mcp402x` library provides an interface to control MCP402X digital potentiometers using an Arduino. These are non-volatile, 6-bit (64 wiper steps) devices with a simple up/down serial interface. The library allows you to:

- Initialize the chip with specific GPIO pins for chip select (CS) and up/down (U/D) control.
- Set the wiper to a specific value (0 to 63).
- Increment or decrement the wiper position by one step.
- Retrieve the current wiper value from the context.
- Save the wiper position to the chip’s non-volatile memory for persistence across power cycles.

Key features:
- **Context Management**: Uses a `context_t` structure to store pin assignments and wiper state.
- **Non-Volatile Support**: The `keepNonVolatile()` method saves the current position, but the initial value in non-volatile mode must be manually updated if needed (via `updateWiperValue()`).
- **Error Handling**: Methods return boolean success indicators and handle edge cases (e.g., clamping values to 0-63).

---

## Usage Example

Below is an example Arduino sketch demonstrating how to use the `Mcp402x` library to set the potentiometer to mid-scale (32) and save it to non-volatile memory:

```cpp
#include "mcp402x.h"

// Define the context structure with pin assignments
Mcp402xNS::context_t potContext =
{
    .csPin          = 2,        // Chip select pin
    .udPin          = 3,        // Up/Down pin
    .currentValue   = 0,        // Initial wiper value
    .isInitialized  = false
};

// Create Mcp402x instance
Mcp402x pot(potContext);

void setup()
{
    // Initialize the chip
    if (!pot.init())
    {
        // Handle initialization failure (e.g., log error)
        while (1);              // Halt
    }

    // Set wiper to mid-scale (32)
    pot.set(32);

    // Save the setting to non-volatile memory
    pot.keepNonVolatile();
}

void loop()
{
    // Example: Increment wiper every second (optional)
    pot.up();
    delay(1000);
}
```

### How to Use
1. **Connect the Hardware**: Wire the MCP402X chip’s CS pin to Arduino pin 2 and U/D pin to pin 3 (adjust as needed).
2. **Include the Library**: Add `mcp402x.h` and `mcp402x.cpp` to your project.
3. **Compile and Upload**: Use the Arduino IDE to compile and upload the sketch.
4. **Test**: The potentiometer wiper will be set to 32 (mid-scale) on startup and retained after power-off.

This example provides a basic starting point. You can extend it to adjust the wiper dynamically (e.g., via serial input) or integrate it into a larger project. Note that non-volatile writes have a limited number of cycles, so use `keepNonVolatile()` judiciously.

---
