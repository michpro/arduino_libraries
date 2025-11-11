# **Library: AstroTimes**

- [**Library: AstroTimes**](#library-astrotimes)
  - [**Summary**](#summary)
  - [**Example of Usage**](#example-of-usage)
    - [**Explanation of the Example**](#explanation-of-the-example)
  - [**API Reference**](#api-reference)
    - [**calcSunEvent()**](#calcsunevent)
    - [**moonPhase()**](#moonphase)
    - [**Event\_t Enum**](#event_t-enum)

## **Summary**

The AstroTimes library is a C++ utility namespace providing stateless functions to calculate solar events (sunrise, sunset, twilight) and the phase of the moon.
It is designed to be a lightweight, dependency-free (besides \<time.h\>) tool for Arduino or any C++ project.

* **Solar Event Calculation**: The calcSunEvent function calculates precise times for sunrise, sunset, and three levels of twilight (civil, nautical, astronomical).
* **Moon Phase Calculation**: The moonPhase function provides an approximate day of the lunar cycle (0-29).
* **Stateless Design**: All functions are pure and stateless, organized within the AstroTimes namespace. They require all necessary data (like date and location) as parameters.

* **Accurate**: The calcSunEvent function is based on algorithms by Paul Schlyter and uses a two-pass calculation to improve accuracy.
* **Return Value**: Note that calcSunEvent returns the time as **seconds from midnight UTC** on the given date, not as a full time_t epoch value.

## **Example of Usage**

Below is a complete example demonstrating how to use the AstroTimes namespace on an Arduino-compatible platform.
```cpp
#include "AstroTimes.h"
#include <time.h>                        // For time_t, tm, mktime

/**
 * @brief Helper function to format seconds-from-midnight into HH:MM:SS
 * @param secondsUTC The time_t value returned by calcSunEvent
 * @return A String formatted as "HH:MM:SS"
 */
String formatTime(time_t secondsUTC)
{
    if (0 == secondsUTC)                // Return "N/A" if the event doesn't occur (calcSunEvent returns 0)
    {
        return "N/A";
    }

    // Calculate hours, minutes, and seconds
    long h  {(long)secondsUTC / 3600};
    long m  {(long)(secondsUTC % 3600) / 60};
    long s  {(long)secondsUTC % 60};

    // Format with leading zeros
    char buf[10];
    sprintf(buf, "%02ld:%02ld:%02ld", h, m, s);

    return String(buf);
}

void setup()
{
    Serial.begin(115200);
    while (!Serial) { ; }               // Wait for serial port to connect

    // --- 1. Define Location ---
    // Example: Berlin, Germany
    const float LATITUDE = 52.5200;
    const float LONGITUDE = 13.4050;

    // --- 2. Define Date ---
    // We need a time_t for a specific date.
    // The time-of-day doesn't matter for calcSunEvent.
    tm timeinfo;
    timeinfo.tm_year = 2025 - 1900;     // Year - 1900
    timeinfo.tm_mon = 9;                // Month (0-11, so 9 is October)
    timeinfo.tm_mday = 26;              // Day
    timeinfo.tm_hour = 12;              // Noon (to avoid DST issues)
    timeinfo.tm_min = 0;
    timeinfo.tm_sec = 0;

    time_t date {mktime(&timeinfo)};    // Convert tm struct to time_t

    Serial.println("Calculating astronomical times for 2025-10-26");
    Serial.print("Location: ");
    Serial.print(LATITUDE, 4);
    Serial.print("\xB0""N, ");
    Serial.print(LONGITUDE, 4);
    Serial.println("\xB0""E");
    Serial.println("---");

    // --- 3. Calculate Sun Events ---
    // Note: Results are in seconds from midnight UTC.
    time_t sunrise      {AstroTimes::calcSunEvent(AstroTimes::SUNRISE_STANDARD, date, LATITUDE, LONGITUDE)};
    time_t sunset       {AstroTimes::calcSunEvent(AstroTimes::SUNSET_STANDARD, date, LATITUDE, LONGITUDE)};
    time_t civilDawn    {AstroTimes::calcSunEvent(AstroTimes::SUNRISE_CIVIL, date, LATITUDE, LONGITUDE)};
    time_t astroDusk    {AstroTimes::calcSunEvent(AstroTimes::SUNSET_ASTRONOMICAL, date, LATITUDE, LONGITUDE)};

    Serial.print("Standard Sunrise (UTC): ");
    Serial.println(formatTime(sunrise));

    Serial.print("Standard Sunset (UTC):  ");
    Serial.println(formatTime(sunset));

    Serial.print("Civil Dawn (UTC):       ");
    Serial.println(formatTime(civilDawn));

    Serial.print("Astro Dusk (UTC):       ");
    Serial.println(formatTime(astroDusk));

    // --- 4. Calculate Moon Phase ---
    // This function needs the full epoch time.
    int moon = AstroTimes::moonPhase(date);
    Serial.print("Moon Phase (0-29):      ");
    Serial.println(moon);
    Serial.println("(0=New Moon, 14=Full Moon)");
}

void loop()
{  
    // Nothing to do
}
```
### **Explanation of the Example**

1. **Helper Function**: A function formatTime is defined to convert the time_t seconds-from-midnight value (returned by calcSunEvent) into a human-readable HH:MM:SS string.
2. **Initialization**: A specific location (Berlin) and a specific date are defined.
3. **Date Setup**: A tm struct is populated for October 26, 2025, and then converted to a time_t using mktime(). This date variable is used for all calculations.
4. **Sun Calculation**: AstroTimes::calcSunEvent is called multiple times using different Event enums (e.g. SUNRISE_STANDARD, SUNSET_ASTRONOMICAL) to get the times for various solar events.
5. **Output**: The results are formatted using our helper function and printed to the Serial Monitor. Note that all times are in UTC.
6. **Moon Calculation**: AstroTimes::moonPhase is called with the same time_t to get the current lunar day (0-29).

## **API Reference**

### **calcSunEvent()**

Calculates the time of a specific solar event for a given date and location.
```cpp
time_t calcSunEvent(Event event, time_t time, float latitude, float longitude);
```

* **event**: The Event type to calculate (see enum below).
* **time**: A time_t representing any moment on the **date** for which to perform the calculation. The specific time-of-day component is ignored.
* **latitude**: The observer's latitude in decimal degrees (positive for North, negative for South).
* **longitude**: The observer's longitude in decimal degrees (positive for East, negative for West).
* **Returns**: time_t The time of the event in **seconds from midnight UTC** on the given date. Returns 0 if the event does not occur on that day (e.g. polar day or night).

### **moonPhase()**

Calculates the approximate moon phase for a given date.
```cpp
int moonPhase(time_t epochTime);
```

* **epochTime**: A time_t timestamp (seconds since Unix epoch) for the moment of calculation.
* **Returns**: int An integer from 0 to 29:
  * 0: New Moon
  * 14: Full Moon
  * 29: Waning crescent (approaching New Moon)

### **Event_t Enum**

Defines the specific solar event to calculate in calcSunEvent.
```cpp
typedef enum : uint8_t
{
    SUNRISE_STANDARD    = 0,    // Standard sunrise (top limb at horizon, 90.833°)
    SUNRISE_CIVIL,              // Civil twilight sunrise (center of sun 6° below horizon, 96°)
    SUNRISE_NAUTICAL,           // Nautical twilight sunrise (center of sun 12° below horizon, 102°)
    SUNRISE_ASTRONOMICAL,       // Astronomical twilight sunrise (center of sun 18° below horizon, 108°)
    SUNSET_STANDARD,            // Standard sunset (top limb at horizon, 90.833°)
    SUNSET_CIVIL,               // Civil twilight sunset (center of sun 6° below horizon, 96°)
    SUNSET_NAUTICAL,            // Nautical twilight sunset (center of sun 12° below horizon, 102°)
    SUNSET_ASTRONOMICAL,        // Astronomical twilight sunset (center of sun 18° below horizon, 108°)
} Event_t;
```
