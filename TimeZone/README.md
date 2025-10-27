# **Library: TimeZone**

- [**Library: TimeZone**](#library-timezone)
  - [**Summary**](#summary)
  - [**Example of Usage**](#example-of-usage)
    - [**Explanation of the Example**](#explanation-of-the-example)

## **Summary**

The TimeZone library is a C++ class for Arduino-compatible platforms, designed to manage time zone conversions between UTC and local time, with full support for Daylight Saving Time (DST).
It allows you to define complex DST rules and easily convert time_t timestamps to the correct local time for that zone.

* **Time Zone Rules**: Define time zones using TimeChangeRule_t structs. These rules specify when DST and Standard Time transitions occur (e.g. "Second Sunday in March at 2:00 AM") and the corresponding UTC offset in minutes.
* **UTC to Local Conversion**: The toLocalTime() function converts a UTC time_t value to the correct local time, automatically applying the correct DST or Standard Time offset based on the rules.
* **Local to UTC Conversion**: The toUTCTime() function converts a local time_t value back to UTC. (Note: This function has ambiguities during the one-hour transition periods).
* **DST Status**: Helper functions like timeIsDst() and localTimeIsDst() determine if a given UTC or local time falls within the DST period.
* **Dynamic Updates**: The setRules() function allows for changing time zone rules at runtime.

The class caches the calculated transition times for a given year. When a conversion is requested, it checks the provided time against these cached transition points to determine the correct offset. If the year of the timestamp changes, it automatically recalculates the transition points.

## **Example of Usage**

Below is an example demonstrating how to use the TimeZone library on an Arduino-compatible platform. This example uses the rules for the US Eastern Time zone and a fixed-rules zone like Arizona.
```cpp
#include "TimeZone.h"
#include <time.h> // For ctime() to print time_t

// --- Define rules for US Eastern Time Zone (UTC-5/UTC-4) ---

// Daylight Saving Time Rule: Second Sunday in March at 2:00 AM
TimeZone::TimeChangeRule_t dstRule =
{                                                                   // EDT: UTC-4 hours (-240 minutes)
    TimeZone::Second, TimeZone::Sun, TimeZone::Mar, 2 + 5, -4 * 60  // change at 2 a.m. local time
};
// Standard Time Rule: First Sunday in November at 2:00 AM
TimeZone::TimeChangeRule_t stdRule =
{                                                                   // EST: UTC-5 hours (-300 minutes)
    TimeZone::First, TimeZone::Sun, TimeZone::Nov, 2 + 4, -5 * 60   // change at 2 a.m. local time
};

// Create a TimeZone object for US Eastern Time
TimeZone usET(dstRule, stdRule);

// --- Define rules for a No-DST zone (e.g., Arizona, UTC-7) ---
TimeZone::TimeChangeRule_t arizonaRule =
{
    TimeZone::First, TimeZone::Sun, TimeZone::Jan, 0, -7 * 60   // MST: UTC-7 hours.
                                                                // Rule details don't matter as it's the only one.
};
// Create a TimeZone object for Arizona time using the single-rule constructor
TimeZone usAZ(arizonaRule);

// Helper function to print time_t as a readable string
void printTime(const char* label, time_t t)
{
    Serial.print(label);
    Serial.println(ctime(&t));
}

void setup()
{
    Serial.begin(115200);
    while (!Serial); // Wait for Serial Monitor to open

    Serial.println("TimeZone Library Example");
    Serial.println("---");

    // --- Test 1: A time during Standard Time (February) ---
    // February 1, 2025 12:00:00 PM UTC
    time_t utcTime_std {1738411200};
#if defined(ARDUINO_ARCH_AVR)
    // The time.h library on AVR uses a different epoch (Jan 1, 2000)
    // than the standard Unix epoch (Jan 1, 1970) returned by NTPClient.
    // We must subtract the 30-year difference in seconds.
    // (946684800 = seconds from 1970-01-01 to 2000-01-01)
    utcTime_std -= 946684800;
#endif
    printTime("UTC (Standard): ", utcTime_std);

    // Convert to local time
    time_t localTime_std {usET.toLocalTime(utcTime_std)};
    printTime("Local (EST):    ", localTime_std); // Should be 7:00:00 AM EST (UTC-5)

    // Check DST status
    Serial.print("Is DST? ");
    Serial.println(usET.timeIsDst(utcTime_std) ? "Yes" : "No"); // Should be No
    Serial.println("---");

    // --- Test 2: A time during Daylight Saving Time (June) ---
    // June 1, 2025 12:00:00 PM UTC
    time_t utcTime_dst {1748779200};
#if defined(ARDUINO_ARCH_AVR)
    // The time.h library on AVR uses a different epoch (Jan 1, 2000)
    // than the standard Unix epoch (Jan 1, 1970) returned by NTPClient.
    // We must subtract the 30-year difference in seconds.
    // (946684800 = seconds from 1970-01-01 to 2000-01-01)
    utcTime_dst -= 946684800;
#endif
    printTime("UTC (Daylight): ", utcTime_dst);

    // Convert to local time
    time_t localTime_dst {usET.toLocalTime(utcTime_dst)};
    printTime("Local (EDT):    ", localTime_dst); // Should be 8:00:00 AM EDT (UTC-4)

    // Check DST status
    Serial.print("Is DST? ");
    Serial.println(usET.timeIsDst(utcTime_dst) ? "Yes" : "No"); // Should be Yes
    Serial.println("---");

    // --- Test 3: No-DST zone (Arizona) using the same June time ---
    Serial.println("Testing No-DST Zone (Arizona, UTC-7)");
    printTime("UTC (Daylight): ", utcTime_dst);

    time_t localTime_az {usAZ.toLocalTime(utcTime_dst)};
    printTime("Local (MST):    ", localTime_az); // Should be 5:00:00 AM MST (UTC-7)

    Serial.print("Is DST? ");
    Serial.println(usAZ.timeIsDst(utcTime_dst) ? "Yes" : "No"); // Should be No
    Serial.println("---");
}

void loop()
{
    // Nothing to do
}
```

### **Explanation of the Example**

1. **Rule Definition**: Two TimeChangeRule_t structs are defined: dstRule for US Daylight Saving Time (EDT, UTC-4) and stdRule for US Standard Time (EST, UTC-5). A separate rule, arizonaRule, is made for a fixed-offset zone (MST, UTC-7).
2. **Object Creation**: A TimeZone object, usET, is created using the two-rule constructor. A second object, usAZ, is created using the single-rule (no-DST) constructor.
3. **Standard Time Test**: A UTC time_t for February 2025 is provided. usET.toLocalTime() correctly subtracts 5 hours (300 minutes) to get EST, and usET.timeIsDst() returns 'No'.
4. **Daylight Time Test**: A UTC time_t for June 2025 is provided. usET.toLocalTime() correctly subtracts 4 hours (240 minutes) to get EDT, and usET.timeIsDst() returns 'Yes'.
5. **No-DST Zone Test**: The usAZ object is given the same June time_t. It correctly applies its single fixed offset of -7 hours (420 minutes) to get MST, and usAZ.timeIsDst() correctly returns 'No'.
