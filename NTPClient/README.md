# **Library: NTPClient**

- [**Library: NTPClient**](#library-ntpclient)
  - [**Summary**](#summary)
  - [**Example of Usage**](#example-of-usage)
    - [**Explanation of the Example**](#explanation-of-the-example)

## **Summary**

The NTPClient library provides a simple way to get the current Coordinated Universal Time (UTC) from a Network Time Protocol (NTP) server. It is designed for Arduino platforms with network connectivity.
It supports:

* **UDP Dependency**: The library requires an instance of the UDP class (e.g., EthernetUDP for wired shields like W5500, or WiFiUDP for WiFi-based boards).
* **Flexible Configuration**: You can specify the NTP server by its hostname (e.g., "pool.ntp.org") or by a static IP address.
* **Automatic Updates**: The client manages periodic time synchronization with a configurable update interval.
* **Time Retrieval**: getEpochTime() returns the current time in seconds since the Unix epoch (Jan. 1, 1970).
* **Y2036 Rollover Handling**: The library correctly handles the 2036 NTP rollover, ensuring correct time calculation beyond that year.

The library works by sending an NTP request packet via UDP, parsing the server's response, and calculating the current epoch time. The update() function should be called in the main loop to manage periodic updates.

## **Example of Usage**

Below is a complete example demonstrating how to use the NTPClient library with an **Arduino and an Ethernet W5500 shield**.
```cpp
#include <SPI.h>
#include <Ethernet.h>       // Required for W5500 shield
#include "NTPClient.h"      // The NTPClient library
#include <time.h>           // For formatting the time string

// --- Ethernet Configuration ---
// Enter a MAC address for your controller.
// Newer Ethernet shields have a MAC address printed on a sticker.
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// --- NTPClient Configuration ---
EthernetUDP ntpUDP; // Create an EthernetUDP instance for the W5500

// Initialize the NTPClient
// - Pass the UDP instance
// - Set the server (default is "pool.ntp.org")
// - Set the update interval in milliseconds (e.g., 60000ms = 1 minute)
NTPClient ntpClient(ntpUDP, "pool.ntp.org", 60000);

void setup()
{
    Serial.begin(115200);
    while (!Serial) { ; } // Wait for serial port to connect

    Serial.println("Starting Ethernet...");
    if (0 == Ethernet.begin(mac))
    {
        Serial.println("Failed to configure Ethernet using DHCP");
        // If DHCP fails, you may need to set a static IP
        // Example: IPAddress ip(192, 168, 0, 177);
        // Ethernet.begin(mac, ip);
        while (true) { ; } // Stop here on failure
    }

    Serial.print("Ethernet IP address: ");
    Serial.println(Ethernet.localIP());

    // Start the NTP client. This begins the UDP listener.
    ntpClient.begin();
    Serial.println("NTP client started.");

    // It's good to force an update at the start
    Serial.println("Forcing first time update...");
    if (ntpClient.forceUpdate())
    {
        Serial.println("Initial time update success.");
    } else
    {
        Serial.println("Initial time update failed.");
    }
}

void loop()
{
    // This should be called regularly in the loop.
    // It checks if the update interval has passed and requests a new time if needed.
    bool updated {ntpClient.update()};

    if (updated)
    {
        Serial.println("Time was updated from NTP server.");
    }

    // Print the current time every 5 seconds
    static uint32_t lastPrint {0};
    if (millis() - lastPrint > 5000)
    {
        lastPrint = millis();

        // Check if the time has been set at least once
        if (ntpClient.isTimeSet())
        {
            // Get the epoch time (seconds since 1970)
            uint32_t epochTime {ntpClient.getEpochTime()};

            // Convert epoch to a time_t structure
            time_t now {epochTime};
#if defined(ARDUINO_ARCH_AVR)
            // The time.h library on AVR uses a different epoch (Jan 1, 2000)
            // than the standard Unix epoch (Jan 1, 1970) returned by NTPClient.
            // We must subtract the 30-year difference in seconds.
            // (946684800 = seconds from 1970-01-01 to 2000-01-01)
            now -= 946684800;
#endif
            // Format as a human-readable string (YYYY-MM-DD HH:MM:SS)
            tm *ti {gmtime(&now)};
            char buff[30];
            strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", ti);

            Serial.print("Current UTC time: ");
            Serial.print(buff);
            Serial.print(" (Epoch: ");
            Serial.print(epochTime);
            Serial.println(")");
        } else
        {
            Serial.println("Time has not been set yet. Waiting for update...");
        }
    }
}
```

### **Explanation of the Example**

1. **Includes**: We include SPI.h and Ethernet.h for the W5500 shield, NTPClient.h for this library, and <time.h> to help format the time.
2. **Configuration**: A mac address is defined for the Ethernet shield.
3. **Initialization**: An EthernetUDP instance is created. This is a requirement for NTPClient. The ntpClient object is then created, passing the ntpUDP instance, the server name, and a 60-second (60,000 ms) update interval.
4. **setup()**: The Serial monitor and Ethernet connection are started. ntpClient.begin() is called to prepare the UDP port. We then call ntpClient.forceUpdate() to get the time immediately on startup, rather than waiting for the first interval.
5. **loop()**: ntpClient.update() is called on every loop. This function is non-blocking and will only send a request if the 60-second interval has passed since the last successful update.
6. **Getting Time**: Every 5 seconds, the code checks if the time is valid using ntpClient.isTimeSet(). If it is, ntpClient.getEpochTime() retrieves the current time as a uint32_t integer.
7. **Formatting**: The raw epoch time is converted into a standard time_t struct, which is then formatted into a YYYY-MM-DD HH:MM:SS string using gmtime() and strftime(). This formatted string is then printed to the Serial Monitor.
