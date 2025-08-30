/**
 * @file        RTCCalibration.h
 * @brief       Header file for the RTCCalibration library.
 *              Defines constants, types, and function prototypes for RTC calibration on GD32 microcontrollers.
 *              This library uses a PPS signal to measure and adjust RTC frequency for improved accuracy.
 *
 * @note        Compatible with GD32F50x, GD32F30x, and GD32F10x. GD32F10x has limitations (can only slow down RTC).
 *
 * @copyright   SPDX-FileCopyrightText: Copyright 2025 Michal Protasowicki
 *
 * @license     SPDX-License-Identifier: MIT
 */

#pragma once

#include <Arduino.h>

#if !defined(GD32F50x) && !defined(GD32F30x) && !defined(GD32F10x)
    #error "Unsupported hardware !!!"
#endif

namespace RTCCalibration
{
    const uint32_t BASE_FREQUENCY               {32768};                ///< Nominal RTC crystal frequency in Hz.
    const uint32_t FREQUENCY_ACQUISITION_TIME   {120};                  ///< Pulses required for frequency trimming.
    const uint32_t CALIBRATION_ACQUISITION_TIME {1280};                 ///< Pulses required for calibration.
    const uint32_t PPS_PER_TICK                 {10};                   ///< PPS pulses per RTC divider tick.

    /**
     * @brief       Type definition for PPS interrupt callback function.
     */
    typedef void (* fnPpsIrqCallback_t)(void);

    /**
     * @brief       Enumeration of calibration state machine states.
     */
    typedef enum
    {
        ST_IDLE,                ///< Idle state, no calibration in progress.
        ST_FREQ_TRIM_START,     ///< Start of frequency trimming phase.
        ST_FREQ_TRIM,           ///< Frequency trimming in progress.
        ST_CALIBRATION_START,   ///< Start of calibration phase.
        ST_CALIBRATION,         ///< Calibration in progress.
        ST_CALIBRATION_DONE,    ///< Calibration calculations complete.
        ST_DONE,                ///< Calibration fully applied and done.
    } states_t;

    /**
     * @brief   Initializes calibration with PPS pin.
     * @param   ppsPin Pin number for PPS input.
     */
    void begin(const pin_size_t ppsPin);

    /**
     * @brief   Initializes calibration with PPS pin and callback.
     * @param   ppsPin      Pin number for PPS input.
     * @param   callbackFn  Function to call on each PPS interrupt.
     */
    void begin(const pin_size_t ppsPin, const fnPpsIrqCallback_t callbackFn);

    /**
     * @brief   Advances the calibration process.
     * @return  Current state after update.
     */
    states_t calibrate(void);

    /**
     * @brief   Checks if RTC is calibrated.
     * @return  True if calibrated.
     */
    bool isRtcCalibrated(void);

    /**
     * @brief   Validates calibration value.
     * @return  True if valid.
     */
    bool isCalibrationValueValid(void);

#if defined(GD32F10x)
    /**
     * @brief   Gets calibration value (GD32F10x specific).
     * @return  Positive calibration value.
     */
    uint8_t getCalibrationValue(void);
#else
    /**
     * @brief   Gets signed calibration value.
     * @return  Signed calibration value.
     */
    int8_t getCalibrationValue(void);
#endif

    /**
     * @brief   Gets calibrated frequency.
     * @return  Frequency in Hz.
     */
    uint32_t getCalibratedFrequency(void);

    /**
     * @brief   Gets measured real frequency.
     * @return  Frequency as float in Hz.
     */
    float getRealFrequency(void);

    /**
     * @brief   Gets calibration progress.
     * @return  Percentage (0-100).
     */
    uint_fast8_t progress(void);

    /**
     * @brief   Applies frequency prescaler.
     * @param   frequency Target frequency in Hz.
     */
    void apply(const uint32_t frequency);

#if defined(GD32F10x)
    /**
     * @brief   Applies calibration value (GD32F10x).
     * @param   calibrationValue Positive value.
     */
    void apply(const uint8_t calibrationValue);

    /**
     * @brief   Applies frequency and calibration (GD32F10x).
     * @param   frequency           Target frequency.
     * @param   calibrationValue    Positive value.
     */
    void apply(const uint32_t frequency, const uint8_t calibrationValue);
# else
    /**
     * @brief   Applies signed calibration value.
     * @param   calibrationValue Signed value.
     */
    void apply(const int8_t calibrationValue);

    /**
     * @brief   Applies frequency and signed calibration.
     * @param   frequency           Target frequency.
     * @param   calibrationValue    Signed value.
     */
    void apply(const uint32_t frequency, const int8_t calibrationValue);
#endif

    /**
     * @brief   Attaches PPS callback.
     * @param   callbackFn Function to attach.
     */
    void attachPpsIrqCallback(const fnPpsIrqCallback_t callbackFn);
};
