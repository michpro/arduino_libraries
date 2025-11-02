# Ethernet Library

This library is designed to work with the Arduino Ethernet Shield, Arduino Ethernet Shield 2, Leonardo Ethernet, and any other W5100/W5200/W5500-based devices. The library allows an Arduino board to connect to the Internet. The board can serve as either a server accepting incoming connections or a client making outgoing ones. The library supports up to eight (W5100 and boards with <= 2 kB SRAM are limited to four) concurrent connections (incoming, outgoing, or a combination).

For a complete API reference, see the [API documentation](api.md).

## SPI Interface

The Arduino board communicates with the Ethernet controller chip using the SPI bus.
* **On classic AVR-based boards** like the Uno, this is on digital pins 11 (MOSI), 12 (MISO), and 13 (SCK). Pin 10 is used by default as the chip select (SS) pin.
* **On the Mega**, the SPI pins are 50 (MISO), 51 (MOSI), 52 (SCK), and 53 (SS). Pin 53 is the hardware SS pin, but the Ethernet shield typically uses pin 10 for SS. Therefore, pin 53 must be kept as an output, even if it is not used, to keep the SPI interface in master mode.

![Arduino UNO Pin map.](https://raw.githubusercontent.com/arduino-libraries/Ethernet/master/docs/arduino_uno_ethernet_pins.png)

![Arduino MEGA Pin map.](https://raw.githubusercontent.com/arduino-libraries/Ethernet/master/docs/arduino_mega_ethernet_pins.png)

## Basic Usage

To use this library, you must pass the SPI bus instance and the MAC address of your shield to `Ethernet.begin()`.

*Specifying the SPI instance as a function call parameter allows you to use an SPI interface other than the default one, which is particularly useful in microcontrollers that have more than one such interface or allow you to configure alternative GPIO pins for communication.*

```cpp
#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker.
byte mac[] =
{
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

void setup()
{
    // Start the Ethernet connection:
    if (0 == Ethernet.begin(SPI, mac))
    {
        Serial.println("Failed to configure Ethernet using DHCP");
        // no point in carrying on, so do nothing forevermore:
        for (;;)
        ;
    }
    
    // Print your local IP address:
    Serial.print("My IP address: ");
    Serial.println(Ethernet.localIP());
}

void loop()
{
  // Your code here
}

```
