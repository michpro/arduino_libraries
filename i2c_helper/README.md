# Library: I²C Utility

- [Library: I²C Utility](#library-ic-utility)
  - [Summary](#summary)
  - [Usage Example](#usage-example)
    - [Explanation of Example](#explanation-of-example)
    - [Notes](#notes)

---

## Summary

The I²C utility library provides a set of functions to simplify communication with devices over the I²C bus using the Arduino Wire library. The library is encapsulated in the `I²C` namespace and includes the following key components:

- **Enumerations**:
  - `results_t`: Defines status codes for I²C operations (e.g., `SUCCESS`, `DATA_TOO_LONG`, `NACK_AFTER_ADDRESS`).
  - `stopBit_t`: Specifies whether to send a stop bit (`SEND_STOP`) or not (`NO_STOP`) after a transaction.

- **Structure**:
  - `context_t`: Holds transaction parameters, including the TwoWire object, device address, data buffers, lengths, and stop bit settings.

- **Functions**:
  - `isDevicePresent`: Checks if a slave device is available on the I²C bus.
  - `readBytes`: Reads a specified number of bytes from a slave device into a buffer.
  - `writeBytes`: Writes a specified number of bytes from a buffer to a slave device.
  - `writeThenReadBytes`: Combines writing and reading in a single transaction.

The library ensures robust error handling, including validation of buffer sizes, retry mechanisms for read operations, and detailed status codes. It is designed for use with Arduino-compatible platforms and relies on the Wire library for low-level I²C communication.

---

## Usage Example

The following example demonstrates how to use the I²C library to communicate with an I2C device, such as an EEPROM or sensor, by checking its presence, writing a register address, and reading data.

```cpp
#include <Wire.h>
#include "i2c.h"

void setup()
{
    // Initialize the Wire library
    Wire.begin();

    // Define the I2C context
    I2C::context_t ctx;
    ctx.wire = &Wire;
    ctx.devAddress = 0x50;                              // Example device address (e.g., EEPROM)
    ctx.stopAfterWrite = I2C::SEND_STOP;
    ctx.stopAfterRead = I2C::SEND_STOP;

    // Check if the device is present
    if (I2C::isDevicePresent(&ctx))
    {
        Serial.begin(115200);
        Serial.println("Device detected.");

        // Prepare to write a register address and read data
        uint8_t writeData[] = {0x00, 0x10};             // Example: register address 0x0010
        uint8_t readData[4];                            // Buffer for reading 4 bytes
        ctx.writeBuffer = writeData;
        ctx.writeLen = 2;
        ctx.readBuffer = readData;
        ctx.readLen = 4;

        // Perform a write-then-read transaction
        uint8_t result = I2C::writeThenReadBytes(&ctx);
        if (result == I2C::SUCCESS) {
            Serial.print("Read data: ");
            for (uint8_t i = 0; i < ctx.readLen; i++)
            {
                Serial.print(readData[i], HEX);
                Serial.print(" ");
            }
            Serial.println();
        } else
        {
            Serial.print("Error: ");
            Serial.println(result, HEX);
        }
    } else
    {
        Serial.begin(9600);
        Serial.println("Device not found.");
    }
}

void loop()
{
    // No operation in loop
}
```

### Explanation of Example

1. **Initialization**: The `Wire` library is initialized to enable I²C communication.
2. **Context Setup**: A `context_t` structure is configured with the TwoWire object, device address (e.g., 0x50 for an EEPROM), and stop bit settings.
3. **Device Detection**: `isDevicePresent` checks if the device is available on the I²C bus.
4. **Data Transaction**: A write-then-read operation is performed to write a 2-byte register address and read 4 bytes of data. The result is checked for success, and the read data is printed to the Serial monitor.
5. **Error Handling**: The status code is checked to handle potential errors, such as `NACK_AFTER_ADDRESS` or `WRONG_DATA_AMOUNT`.

### Notes

- Ensure the `Wire` library is properly initialized before using the I²C functions.
- The `BUFFER_SIZE` constant limits the maximum data transfer size per transaction, as defined by the Wire library.
- The `RETRIES` constant controls the number of read attempts before reporting an error.
- Always validate buffer pointers and lengths to avoid `OTHER_ERROR` or `DATA_TOO_LONG` errors.

This library provides a robust and flexible interface for I²C communication, suitable for a wide range of Arduino-based projects involving I²C devices.

---
