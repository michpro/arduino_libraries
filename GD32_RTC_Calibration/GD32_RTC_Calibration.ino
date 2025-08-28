/**
 * @file        GD32_RTC_Calibration.ino
 * @brief       Example Arduino sketch demonstrating the usage of the RTCCalibration library.
 *              This sketch initializes the RTC, attaches a PPS signal handler, and periodically
 *              calibrates the RTC using the library. It prints calibration status, frequency,
 *              and progress to the serial console.
 *
 * @note        This example assumes a GD32F50x or GD32F30x microcontroller. LED2 toggles on PPS events.
 *              PB4 is used as the PPS input pin.
 *
 * @copyright   SPDX-FileCopyrightText: Copyright 2025 Michal Protasowicki
 *
 * @license     SPDX-License-Identifier: MIT
 */

#include "src/RTCCalibration.h"

#if !defined(GD32F50x) && !defined(GD32F30x)
    #error "Unsupported hardware !!!"
#endif

const uint32_t  TICK_TIME       {10000};                // Status presentation interval (ms)
uint32_t        currentMillis   {0};                    // Current time (ms)
uint32_t        previousMillis  {0};                    // Previous event time (ms)
uint32_t        tick            {0};                    // Tick counter for periodic updates
uint32_t        state           {0};                    // Current calibration state
bool            calibrated      {false};                // Flag indicating if calibration is complete

/**
 * @brief   Callback function triggered on each PPS interrupt.
 *          Toggles LED2 to visually indicate PPS events.
 */
void ppsCallback(void)
{
    digitalToggle(LED2);
}

/**
 * @brief   Arduino setup function.
 *          Initializes RTC, pins, serial communication, and attaches PPS callback.
 */
void setup(void)
{
    rtc_Init();                                         // Initialize the RTC module

    pinMode(LED2, OUTPUT);                              // Configure LED2 as output for toggling
    pinMode(PB4, INPUT_PULLDOWN);                       // Configure PB4 as input for PPS signal
    Serial.begin(115200);                               // Start serial communication at 115200 baud

    RTCCalibration::attachPpsIrqCallback(ppsCallback);  // Attach user callback to PPS interrupt
    RTCCalibration::begin(PB4);                         // Start calibration with PPS on PB4
}

/**
 * @brief   Arduino loop function.
 *          Runs calibration periodically and prints status every 10 seconds.
 */
void loop(void)
{
    currentMillis = millis();                           // Update current time
    state = (uint32_t)RTCCalibration::calibrate();      // Perform calibration step and get state

    if (currentMillis - previousMillis >= TICK_TIME)    // Check if 10 seconds have elapsed
    {
        ++tick;                                         // Increment tick counter
        previousMillis = currentMillis;                 // Update previous time

        // Print current calibration status
        Serial.print("State: ");
        Serial.print(state);
        Serial.print(" tick: ");
        Serial.print(tick);
        Serial.print(" real Freq: ");
        Serial.print(RTCCalibration::getRealFrequency());
        Serial.print(" set Freq: ");
        Serial.print(RTCCalibration::getCalibratedFrequency());
        Serial.print(" corr: ");
        Serial.print(RTCCalibration::getCalibrationValue());
        Serial.print(RTCCalibration::isCalibrationValueValid() ? " VALID" : " INVALID");
        Serial.print(" | ");
        Serial.print(RTCCalibration::progress());
        Serial.print("% | RTC: ");
        Serial.println(rtc_counter_get());

        // Check and print if calibration is newly completed
        if ((false == calibrated) && (true == RTCCalibration::isRtcCalibrated()))
        {
            calibrated = true;
            Serial.print("\nRTC CALIBRATION DONE. Freq = ");
            Serial.print(RTCCalibration::getCalibratedFrequency());
            Serial.print(" [Hz]; calib. value = ");
            Serial.println(RTCCalibration::getCalibrationValue());
            Serial.println("");
        }
    }
}
