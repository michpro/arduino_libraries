/**
 * @file        RTCCalibration.cpp
 * @brief       Implementation of the RTCCalibration library for calibrating the RTC on GD32 microcontrollers.
 *              This file contains the logic for frequency trimming and calibration using a PPS signal.
 *              It handles state transitions, calculations, and hardware interactions.
 *
 * @note        Supports GD32F50x, GD32F30x, and GD32F10x variants with conditional compilation for differences.
 *
 * @copyright   SPDX-FileCopyrightText: Copyright 2025 Michal Protasowicki
 *
 * @license     SPDX-License-Identifier: MIT
 */

#include "RTCCalibration.h"

namespace RTCCalibration
{
    namespace
    {
        const uint32_t      RTC_CALIBRATION_PERIOD  {1048576};                  // 2^20 - Calibration period constant
        const uint32_t      RTC_CALIBRATION_TIMEOUT {1500};                     // Timeout in ms for calibration steps

        fnPpsIrqCallback_t  fnPpsIrqCallback        {nullptr};                  // Pointer to user PPS callback function
        volatile uint32_t   pulse                   {0};                        // PPS pulse counter
        volatile uint32_t   accPulse                {0};                        // Accumulated PPS pulses for progress
        volatile bool       tick                    {false};                    // Flag for RTC divider update
        volatile uint32_t   rtcDividerVal           {0};                        // Current RTC divider value
        volatile uint32_t   rtcVal                  {0};                        // Current RTC counter value
        int32_t             corr                    {0};                        // Calibration correction value
        int32_t             diff                    {0};                        // Difference in divider values
        uint32_t            currentMillis           {0};                        // Current time (ms)
        uint32_t            previousMillis          {0};                        // Previous event time (ms)
        uint32_t            firstRtcDivVal          {0};                        // First captured RTC divider value
        uint32_t            lastRtcVal              {0};                        // Last captured RTC counter value
        uint32_t            baseFreq                {BASE_FREQUENCY};           // Base RTC frequency (Hz)
        float               realFreq                {(float)BASE_FREQUENCY};    // Measured real frequency
        float               err                     {0.0};                      // Frequency error
        bool                calibrationDone         {false};                    // Flag for completed calibration
        bool                calibrationValueValid   {false};                    // Flag for valid calibration value
        states_t            state                   {ST_IDLE};                  // Current calibration state machine state
        PinName             pinname                 {NC};                       // PPS pin name (GD core internal type)

        /**
         * @brief   Interrupt handler for PPS signal.
         *          Updates counters, captures RTC values, and calls user callback if set.
         */
        void ppsIrqHandler(void)
        {
            if (0 == pulse % 10)                                        // Every 10th pulse, capture divider
            {
                rtcDividerVal = rtc_divider_get();
                tick = true;
            }
            rtcVal = rtc_counter_get();                                 // Capture current RTC counter
            ++pulse;                                                    // Increment pulse counter
            ++accPulse;                                                 // Increment accumulated pulses

            if (nullptr != fnPpsIrqCallback)                            // Call user callback if registered
            {
                fnPpsIrqCallback();
            }
        }

        /**
         * @brief   Performs frequency error calculations based on captured data.
         *          Updates error, correction, and real frequency values.
         */
        void doCalculations(void)
        {
            if (true == tick)
            {
                uint32_t halfBaseFreq {baseFreq >> 1};                  // Half base frequency for wrap-around handling

                diff = rtcDividerVal - firstRtcDivVal;                  // Calculate divider difference
                // Handle potential wrap-around in divider values
                diff = ((uint32_t)abs(diff) < halfBaseFreq) ? diff 
                                                            : ((uint32_t)abs(diff - (int32_t)baseFreq) < halfBaseFreq)  ? (diff - (int32_t)baseFreq)
                                                                                                                        : (diff + (int32_t)baseFreq);
                if (1 < pulse)                                          // Calculate error and correction if sufficient pulses
                {
                    err = (float)diff / (pulse - 1);
                    if (false == calibrationValueValid)
                    {
                        corr = (int32_t)round((float)((int64_t)diff * RTC_CALIBRATION_PERIOD) / (baseFreq * (pulse - 1)));
                    }
                } else
                {
                    err = (float)0.0;                                   // Reset error and correction for initial state
                    corr = 0;
                }
                realFreq = (float)baseFreq - err;                       // Update measured real frequency
            }
            tick = false;                                               // Reset tick flag
        }

        /**
         * @brief   Stores initial RTC values and timestamps for calibration.
         */
        inline void storeVals(void)
        {
            firstRtcDivVal = rtcDividerVal;                             // Store first divider value
            lastRtcVal = rtcVal;                                        // Store last RTC counter
            previousMillis = currentMillis;                             // Update previous timestamp
        }

        /**
         * @brief   Checks for calibration timeout based on RTC activity.
         * @return  True if timeout occurred, false otherwise.
         */
        bool isTimeout(void)
        {
            bool result {false};

            if (rtcVal != lastRtcVal)                                       // Reset timeout if RTC is running
            {
                previousMillis = currentMillis;
                lastRtcVal = rtcVal;
            }

            if (currentMillis - previousMillis >= RTC_CALIBRATION_TIMEOUT)  // Check for timeout
            {
                pulse = 0;                                                  // Reset pulse counter on timeout
                result = true;
            }

            return result;
        }
    }

    /**
     * @brief   Initializes calibration with PPS pin.
     * @param   ppsPin Pin number for PPS input.
     */
    void begin(const pin_size_t ppsPin)
    {
        state = ST_IDLE;
        
        pinname = DIGITAL_TO_PINNAME(ppsPin);                           // Convert pin to PinName
        gpio_interrupt_enable(GD_PORT_GET(pinname), GD_PIN_GET(pinname), ppsIrqHandler, EXTI_TRIG_RISING);  // Enable interrupt
    }

    /**
     * @brief   Initializes calibration with PPS pin and attaches a callback.
     * @param   ppsPin      Pin number for PPS input.
     * @param   callbackFn  Function to call on each PPS interrupt.
     */
    void begin(const pin_size_t ppsPin, fnPpsIrqCallback_t callbackFn)
    {
        attachPpsIrqCallback(callbackFn);                               // Attach callback first
        begin(ppsPin);                                                  // Call base begin
    }

    /**
     * @brief   Advances the calibration state machine.
     * @return  Current state after processing.
     */
    states_t calibrate(void)
    {
        currentMillis = millis();                                       // Update current time (for timeout)

        switch (state)
        {
            case ST_DONE:
                doCalculations();                                       // Continue calculations in done state
                break;

            case ST_IDLE:
                pulse = 0;                                              // Reset counters
                accPulse = 0;
                baseFreq = BASE_FREQUENCY;                              // Reset to base frequency
                apply(baseFreq, 0);                                     // Apply initial settings
                state = ST_FREQ_TRIM_START;                             // Transition to frequency trim start
                break;

            case ST_FREQ_TRIM_START:
                if (true == tick)                                       // Wait for first tick
                {
                    storeVals();                                        // Store initial values
                    state = ST_FREQ_TRIM;                               // Transition to trimming
                }
                break;

            case ST_FREQ_TRIM:
                doCalculations();                                       // Perform calculations
                if (true == isTimeout())                                // Handle timeout
                {
                    state = ST_IDLE;
                } else
                {
                    if (pulse > FREQUENCY_ACQUISITION_TIME)             // Check if acquisition time reached
                    {
                        baseFreq = (uint32_t)round(realFreq);           // Round to nearest frequency
#if defined(GD32F10x)
                        if ((float)baseFreq > realFreq)
                        {
                            baseFreq -= 1;                              // Adjust for GD32F10x limitations (can only slow down)
                        }
#endif
                        apply(baseFreq);                                // Apply trimmed frequency
                        pulse = 0;                                      // Reset pulse for next phase
                        state = ST_CALIBRATION_START;                   // Transition to calibration start
                    }
                }
                break;

            case ST_CALIBRATION_START:
                if (true == tick)                                       // Wait for tick
                {
                    storeVals();                                        // Store values
                    state = ST_CALIBRATION;                             // Transition to calibration
                }
                break;

            case ST_CALIBRATION:
                doCalculations();                                       // Perform calculations
                if (true == isTimeout())                                // Handle timeout
                {
                    state = ST_IDLE;
                } else
                {
                    if (pulse > CALIBRATION_ACQUISITION_TIME)           // Check if calibration time reached
                    {
                        state = ST_CALIBRATION_DONE;                    // Transition to done
                    }
                }
                break;
            
            case ST_CALIBRATION_DONE:
                calibrationValueValid = isCalibrationValueValid();      // Validate correction
                if (true == calibrationValueValid)
                {
#if defined(GD32F10x)
                    apply((uint8_t)abs(corr));                          // Apply positive correction for GD32F10x
#else
                    apply((int8_t)corr);                                // Apply signed correction
#endif
                    calibrationDone = true;                             // Set done flag
                    state = ST_DONE;                                    // Final state
                } else
                {
                    state = ST_IDLE;                                    // Invalid, reset to idle
                }
                break;

            default:
                break;
        }

        return state;
    }

    /**
     * @brief   Checks if RTC calibration is complete.
     * @return  True if calibrated, false otherwise.
     */
    bool isRtcCalibrated(void)
    {
        return calibrationDone;
    }

    /**
     * @brief   Validates the calibration correction value based on hardware limits.
     * @return  True if valid, false otherwise.
     */
    bool isCalibrationValueValid(void)
    {
#if defined(GD32F10x)
        return (abs(corr) <= INT8_MAX);                             // GD32F10x: Positive up to 127
#else
        return (corr >= INT8_MIN) && (corr <= INT8_MAX);            // Others: Signed -128 to 127
#endif
    }

#if defined(GD32F10x)
    /**
     * @brief   Gets the absolute calibration value (GD32F10x specific).
     * @return  Calibration value as uint8_t.
     */
    uint8_t getCalibrationValue(void)
    {
        return (uint8_t)abs(corr);
    }
#else 
    /**
     * @brief   Gets the signed calibration value.
     * @return  Calibration value as int8_t.
     */
    int8_t getCalibrationValue(void)
    {
        return (int8_t)corr;
    }
#endif

    /**
     * @brief   Gets the calibrated base frequency.
     * @return  Frequency in Hz.
     */
    uint32_t getCalibratedFrequency(void)
    {
        return baseFreq;
    }

    /**
     * @brief   Gets the measured real frequency.
     * @return  Frequency as float in Hz.
     */
    float getRealFrequency(void)
    {
        return realFreq;
    }

    /**
     * @brief   Calculates calibration progress percentage.
     * @return  Progress as uint_fast8_t (0-100).
     */
    uint_fast8_t progress(void)
    {
        constexpr uint32_t  maxPulses   {FREQUENCY_ACQUISITION_TIME + CALIBRATION_ACQUISITION_TIME};    // Total expected pulses
        uint32_t            progressVal {(accPulse > 0) ? (((accPulse - 1) * 100) / maxPulses) : 0};    // Calculate percentage

        return ((100 >= progressVal) ? (uint_fast8_t)progressVal : 100);                                // Cap at 100
    }

    /**
     * @brief   Applies the prescaler for the given frequency.
     * @param   frequency Target frequency in Hz.
     */
    void apply(uint32_t frequency)
    {
        rtc_prescaler_set(frequency - 1);                           // Set RTC prescaler
    }

#if defined(GD32F10x)
    /**
     * @brief   Applies calibration value (GD32F10x specific).
     * @param   calibrationValue Positive correction value.
     */
    void apply(uint8_t calibrationValue)
    {
        bkp_rtc_calibration_value_set(calibrationValue & 0x7F);
    }
# else
    /**
     * @brief   Applies signed calibration value.
     * @param   calibrationValue Signed correction value.
     */
    void apply(int8_t calibrationValue)
    {
        uint16_t dir {(0 <= calibrationValue) ? RTC_CLOCK_SPEED_UP : RTC_CLOCK_SLOWED_DOWN};  // Determine direction

        bkp_rtc_clock_calibration_direction(dir);                       // Set direction
        bkp_rtc_calibration_value_set((uint8_t)abs(calibrationValue));  // Set absolute value
    }
#endif

#if defined(GD32F10x)
    /**
     * @brief   Applies both frequency and calibration value (GD32F10x specific).
     * @param   frequency           Target frequency in Hz.
     * @param   calibrationValue    Positive correction value.
     */
    void apply(uint32_t frequency, uint8_t calibrationValue)
#else
    /**
     * @brief   Applies both frequency and signed calibration value.
     * @param   frequency           Target frequency in Hz.
     * @param   calibrationValue    Signed correction value.
     */
    void apply(uint32_t frequency, int8_t calibrationValue)
#endif
    {
        apply(frequency);                                           // Apply frequency first
        apply(calibrationValue);                                    // Then calibration
    }

    /**
     * @brief   Attaches a callback function to PPS interrupts.
     * @param   callbackFn Function to attach.
     */
    void attachPpsIrqCallback(fnPpsIrqCallback_t callbackFn)
    {
        fnPpsIrqCallback = callbackFn;                              // Set callback pointer
    }
}
