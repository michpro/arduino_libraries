/**
 * @file AstroTimes.h
 * @brief Provides functions to calculate solar events (sunrise, sunset) and moon phase.
 *
 * @copyright SPDX-FileCopyrightText: Copyright 2025 by Michal Protasowicki
 * @license SPDX-License-Identifier: MIT license
 */

#pragma once

#include <Arduino.h>
#include <time.h>

/**
 * \namespace AstroTimes
 * \brief Provides stateless utility functions for astronomical calculations.
 *
 * This namespace contains functions to calculate solar events (sunrise, sunset, twilight)
 * and the phase of the moon for a given date and location. All functions are pure
 * and stateless; they take all required data as parameters (time, location) and
 * return a calculated value.
 *
 * The main function, `calcSunEvent`, returns the time of an event in seconds from
 * midnight UTC on the specified day. This is not a full `time_t` epoch value,
 * as the calculations are valid for dates far in the past or future.
 */
namespace AstroTimes
{
    /**
     * \enum Event_t
     * \brief Defines the specific solar event to calculate.
     *
     * The `STANDARD` event corresponds to the "standard" sunrise/sunset,
     * where the top limb of the sun is at the horizon (90.833° zenith).
     * The other events correspond to the different stages of twilight.
     */
    typedef enum : uint8_t
    {
        SUNRISE_STANDARD    = 0,    //!< Standard sunrise (top limb at horizon, 90.833°)
        SUNRISE_CIVIL,              //!< Civil twilight sunrise (center of sun 6° below horizon, 96°)
        SUNRISE_NAUTICAL,           //!< Nautical twilight sunrise (center of sun 12° below horizon, 102°)
        SUNRISE_ASTRONOMICAL,       //!< Astronomical twilight sunrise (center of sun 18° below horizon, 108°)
        SUNSET_STANDARD,            //!< Standard sunset (top limb at horizon, 90.833°)
        SUNSET_CIVIL,               //!< Civil twilight sunset (center of sun 6° below horizon, 96°)
        SUNSET_NAUTICAL,            //!< Nautical twilight sunset (center of sun 12° below horizon, 102°)
        SUNSET_ASTRONOMICAL,        //!< Astronomical twilight sunset (center of sun 18° below horizon, 108°)
    } Event_t;
    
    /**
     * \brief Calculates the time of a specific solar event (sunrise/sunset) for a given date and location.
     *
     * This function performs a two-pass calculation to improve accuracy. The result
     * is the time of the event in seconds from midnight UTC on the specified date.
     *
     * \param event The type of solar event to calculate (e.g., SUNRISE_STANDARD, SUNSET_CIVIL).
     * \param time A time_t timestamp representing any moment on the **date** for which to perform the calculation. The specific time of day is ignored; only the date components (year, month, day) are used.
     * \param latitude The observer's latitude in decimal degrees (positive for North, negative for South).
     * \param longitude The observer's longitude in decimal degrees (positive for East, negative for West).
     * \return time_t The time of the event in **seconds from midnight UTC** on the given date. Returns 0 if the event does not occur on that day (e.g., polar day or night).
     */
    time_t calcSunEvent(Event_t event, time_t time, float latitude, float longitude);

    /**
     * \brief Calculates the approximate moon phase for a given date.
     *
     * This is a simple calculation to determine the approximate phase of the moon
     * based on a known new moon epoch.
     *
     * \param epochTime A time_t timestamp (seconds since Unix epoch) for the moment of calculation.
     * \return int An integer from 0 to 29:
     * - 0: New Moon
     * - 14: Full Moon
     * - 29: Waning crescent (approaching New Moon)
     */
    int moonPhase(time_t epochTime);
}
