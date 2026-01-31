- [GD32CAN Library](#gd32can-library)
  - [⚠️ Hardware Compatibility \& Testing Status](#️-hardware-compatibility--testing-status)
  - [Features](#features)
  - [Installation](#installation)
  - [Usage Example](#usage-example)
  - [API Reference](#api-reference)
    - [Initialization](#initialization)
    - [Data Transmission](#data-transmission)
    - [Data Reception](#data-reception)
    - [Filtering](#filtering)

---

# GD32CAN Library

A lightweight and efficient C++ CAN bus library for **GD32F30x** and **GD32F50x** microcontrollers in the Arduino environment. This library provides an easy-to-use interface for Controller Area Network (CAN) communication, featuring ring buffers, interrupt-driven I/O, and advanced filter management.

## ⚠️ Hardware Compatibility & Testing Status

**Supported Families:**
* **GD32F30x** (e.g., GD32F303)
* **GD32F50x** (e.g., GD32F503, GD32F507)

**Tested Hardware:**
* ✅ **GD32F303CC** (Generic "Blue Pill" style boards)

> **Note:** While the library is written to support the register maps of both the F30x and F50x families, it has currently **only been verified physically on the GD32F303CC**. Use on other chips within the supported families should work but requires verification.

## Features

* **Interrupt-Driven:** Uses hardware interrupts for both TX (Transmit) and RX (Receive) to ensure non-blocking operation.
* **Ring Buffers:** Configurable software ring buffers for RX and TX queues to handle bursts of data.
* **Filtering:** Comprehensive support for hardware filters:
    * Mask Mode and List Mode.
    * Standard (11-bit) and Extended (29-bit) ID support.
    * Easy setup for "Receive All" or specific IDs.
* **Baudrate:** Automatic calculation of time quanta and bit timing segments for standard baud rates (up to 1 Mbps).
* **Low Power:** Support for controlling external transceiver standby/sleep pins.

## Installation

1.  Download this repository.
2.  Place the `GD32CAN` folder into your Arduino `libraries` folder.
3.  Restart the Arduino IDE.

## Usage Example

Below is a complete example showing how to initialize the CAN bus, send a heartbeat message, and listen for incoming data.

```cpp
#include <Arduino.h>
#include <GD32CAN.h>

// Initialize CAN0 using the default pin mapping (PA11/PA12 on many F303 boards)
// You can also use CAN::Device::CAN_0_ALT1 (PB8/PB9) if needed.
GD32CAN can0(CAN::Device::CAN_0_DEFAULT);

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println("Initializing GD32CAN...");

    // Start CAN at 500 kbps
    if (can0.begin(CAN::Baudrate::BD_500k))
    {
        Serial.println("CAN initialized successfully!");
    } else
    {
        Serial.println("CAN initialization failed!");
        while (1); // Halt
    }

    // Optional: Configure a filter to accept only specific Standard IDs (e.g., 0x123)
    // If skipped, the library accepts all messages by default (depending on implementation).
    // can0.setFilter(CAN::Filter::Bank::FLT00, 0x123, CAN::Filter::FrameType::DATA);
}

void loop()
{
    static uint32_t lastSend = 0;

    // --- Transmit Task (Send every 1000ms) ---
    if (millis() - lastSend > 1000)
    {
        lastSend = millis();

        CAN::message_t msg;
        msg.id = 0x100;                 // Message ID
        msg.idType = CAN::Frame::Id::STANDARD;
        msg.frameType = CAN::Frame::Type::DATA;
        msg.dataLen = 4;
        msg.data[0] = 0xDE;
        msg.data[1] = 0xAD;
        msg.data[2] = 0xBE;
        msg.data[3] = 0xEF;

        if (can0.write(msg))
        {
            Serial.println("Message Sent");
        } else
        {
            Serial.println("TX Buffer Full / Send Failed");
        }
    }

    // --- Receive Task (Polling) ---
    // Although ISR handles the hardware, we poll the software ring buffer here.
    if (can0.available())
    {
        CAN::message_t rxMsg;
        if (can0.read(rxMsg))
        {
            Serial.print("Received ID: 0x");
            Serial.print(rxMsg.id, HEX);
            Serial.print(" Len: ");
            Serial.print(rxMsg.dataLen);
            Serial.print(" Data: ");
            for (uint8_t i = 0; i < rxMsg.dataLen; i++)
            {
                Serial.print(rxMsg.data[i], HEX);
                Serial.print(" ");
            }
            Serial.println();
        }
    }
}
```

## API Reference

### Initialization
* `GD32CAN(device, rxQueueSize, txQueueSize)`: Constructor.
    * `device`: `CAN::Device::CAN_0_DEFAULT`, `CAN_0_ALT1`, etc.
    * `rxQueueSize` / `txQueueSize`: Size of ring buffers (default is 64 for RX, 0 for TX direct mode).
* `begin(baudrate)`: Starts the CAN peripheral.

### Data Transmission
* `write(message)`: Queues a message for transmission. Returns `true` if successful.
* `availableForWrite()`: Returns free space in the TX buffer.

### Data Reception
* `available()`: Returns number of messages waiting in the RX buffer.
* `read(message)`: Reads the oldest message from the buffer.
* `peek(message)`: Reads without removing from buffer.

### Filtering
* `allowReceiveAllMessages(idType)`: Opens filters for all Standard or Extended IDs.
* `setFilter(...)`: Highly configurable method for setting Mask or List mode filters. See `GD32CAN.h` for all overloads.
