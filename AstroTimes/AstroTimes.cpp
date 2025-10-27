/**
 * @file AstroTimes.cpp
 * @brief Implementation of astronomical calculations for sunrise, sunset, and moon phase.
 *
 * @copyright SPDX-FileCopyrightText: Copyright 2025 by Michal Protasowicki
 * @license SPDX-License-Identifier: MIT license
 */

#include "AstroTimes.h"
#include <math.h> // Required for isnan, fmod, cos, sin, tan, asin, acos, floor, round

#ifndef PI
    /** \def PI A local definition of PI if not already defined. */
#   define PI 3.1415926535897932384626433832795
#endif

namespace AstroTimes
{
    /**
     * \namespace
     * \brief Anonymous namespace for internal helper functions and constants.
     *
     * These mathematical functions are based on algorithms by Paul Schlyter
     * (https://www.stjarnhimlen.se/comp/sunriset.c) and are used internally
     * to calculate the solar position and event times. They are not exposed
     * outside of this translation unit.
     */
    namespace
    {
        // --- Solar Zenith Angles ---
        // These represent the angle of the center of the sun relative to the zenith (straight up).
        // 90° is the horizon.

        //! Standard sun angle for sunset/sunrise (top limb at horizon)
        constexpr float SUN_ANGLE_STANDARD      {90.833F};
        //! Civil twilight sun angle (sun 6° below horizon)
        constexpr float SUN_ANGLE_CIVIL         {96.0F};
        //! Nautical twilight sun angle (sun 12° below horizon)
        constexpr float SUN_ANGLE_NAUTICAL      {102.0F};
        //! Astronomical twilight sun angle (sun 18° below horizon)
        constexpr float SUN_ANGLE_ASTONOMICAL   {108.0F};
        
        // --- Time Constants ---
        
        //! Minutes in half a day (from midnight to noon)
        constexpr float HALF_DAY_MIN            {720};
        //! Seconds in one minute
        constexpr int   SECS_PER_MIN            {60};
        //! Seconds in one full day
        constexpr int   SECS_PER_DAY            {24 * 60 * 60};

        /**
         * \brief Converts an angle from degrees to radians.
         * \param angleDeg Angle in degrees.
         * \return float Angle in radians.
         */
        float degToRad(float angleDeg)
        {
            return (PI * angleDeg / 180.0F);
        }

        /**
         * \brief Converts an angle from radians to degrees.
         * \param angleRad Angle in radians.
         * \return float Angle in degrees.
         */
        float radToDeg(float angleRad)
        {
            return (180.0F * angleRad / PI);
        }

        /**
         * \brief Calculates the mean obliquity of the ecliptic.
         * \param julianCentury Time in Julian centuries since J2000.
         * \return float Mean obliquity in degrees.
         */
        float calcMeanObliquityOfEcliptic(float julianCentury)
        {
            float seconds {21.448F - julianCentury * (46.8150F + julianCentury * (0.00059F - julianCentury * (0.001813F)))};

            return (23.0F + (26.0F + (seconds / 60.0F)) / 60.0F);
        }

        /**
         * \brief Calculates the geometric mean longitude of the Sun.
         * \param julianCentury Time in Julian centuries since J2000.
         * \return float Geometric mean longitude in degrees (0-360).
         */
        float calcGeomMeanLongSun(float julianCentury)
        {
            float result {NAN};

            if (not isnan(julianCentury))
            {
                float L {280.46646F + julianCentury * (36000.76983F + 0.0003032F * julianCentury)};

                result = fmod(L, 360.0F);
            }

            return result;
        }

        /**
         * \brief Calculates the obliquity correction.
         * \param julianCentury Time in Julian centuries since J2000.
         * \return float Corrected obliquity in degrees.
         */
        float calcObliquityCorrection(float julianCentury)
        {
            float e0    {calcMeanObliquityOfEcliptic(julianCentury)};
            float omega {125.04F - 1934.136F * julianCentury};

            return (e0 + 0.00256F * cos(degToRad(omega)));
        }

        /**
         * \brief Calculates the eccentricity of Earth's orbit.
         * \param julianCentury Time in Julian centuries since J2000.
         * \return float Orbital eccentricity.
         */
        float calcEccentricityEarthOrbit(float julianCentury)
        {
            return (0.016708634F - julianCentury * (0.000042037F + 0.0000001267F * julianCentury));
        }

        /**
         * \brief Calculates the geometric mean anomaly of the Sun.
         * \param julianCentury Time in Julian centuries since J2000.
         * \return float Mean anomaly in degrees.
         */
        float calcGeomMeanAnomalySun(float julianCentury)
        {
            return (357.52911F + julianCentury * (35999.05029F - 0.0001537F * julianCentury));
        }

        /**
         * \brief Calculates the Equation of Time.
         *
         * The Equation of Time is the difference between apparent solar time
         * (what a sundial shows) and mean solar time (what a clock shows).
         *
         * \param julianCentury Time in Julian centuries since J2000.
         * \return float Equation of Time in **minutes**.
         */
        float calcEquationOfTime(float julianCentury)
        {
            float epsilon   {calcObliquityCorrection(julianCentury)};
            float l0        {calcGeomMeanLongSun(julianCentury)};
            float e         {calcEccentricityEarthOrbit(julianCentury)};
            float m         {calcGeomMeanAnomalySun(julianCentury)};
            float y         {tan(degToRad(epsilon) / 2.0F)};
            y *= y;

            float sin2l0    {sin(2.0F * degToRad(l0))};
            float sinm      {sin(degToRad(m))};
            float cos2l0    {cos(2.0F * degToRad(l0))};
            float sin4l0    {sin(4.0F * degToRad(l0))};
            float sin2m     {sin(2.0F * degToRad(m))};
            float Etime     {y * sin2l0 - 2.0F * e * sinm + 4.0F * e * y * sinm * cos2l0 - 0.5F * y * y * sin4l0 - 1.25F * e * e * sin2m};

            return radToDeg(Etime) * 4.0F;
        }

        /**
         * \brief Calculates the time in Julian centuries from a Julian Day number.
         *
         * The epoch J2000.0 is Julian Day 2451545.0.
         *
         * \param julianDay The Julian Day number.
         * \return float Time in Julian centuries since J2000.
         */
        float calcTimeJulianCentury(float julianDay)
        {
            return (julianDay - 2451545.0F) / 36525.0F;
        }

        /**
         * \brief Calculates the Sun's Equation of the Center.
         * \param julianCentury Time in Julian centuries since J2000.
         * \return float Equation of the Center in degrees.
         */
        float calcSunEqOfCenter(float julianCentury)
        {
            float m     {calcGeomMeanAnomalySun(julianCentury)};
            float mrad  {degToRad(m)};
            float sinm  {sin(mrad)};
            float sin2m {sin(mrad + mrad)};
            float sin3m {sin(mrad + mrad + mrad)};

            return (sinm * (1.914602F - julianCentury * (0.004817F + 0.000014F * julianCentury)) + sin2m * (0.019993F - 0.000101F * julianCentury) + sin3m * 0.000289F);
        }

        /**
         * \brief Calculates the Sun's true longitude.
         * \param julianCentury Time in Julian centuries since J2000.
         * \return float True longitude in degrees.
         */
        float calcSunTrueLongitude(float julianCentury)
        {
            float l0    {calcGeomMeanLongSun(julianCentury)};
            float c     {calcSunEqOfCenter(julianCentury)};

            return (l0 + c);
        }

        /**
         * \brief Calculates the Sun's apparent longitude (corrected for aberration and nutation).
         * \param julianCentury Time in Julian centuries since J2000.
         * \return float Apparent longitude in degrees.
         */
        float calcSunApparentLongitude(float julianCentury)
        {
            float o         {calcSunTrueLongitude(julianCentury)};
            float omega     {125.04F - 1934.136F * julianCentury};

            return (o - 0.00569F - 0.00478F * sin(degToRad(omega)));
        }

        /**
         * \brief Calculates the Sun's declination.
         *
         * Declination is the Sun's angle north or south of the celestial equator.
         *
         * \param julianCentury Time in Julian centuries since J2000.
         * \return float Declination in degrees.
         */
        float calcSunDeclination(float julianCentury)
        {
            float e         {calcObliquityCorrection(julianCentury)};
            float lambda    {calcSunApparentLongitude(julianCentury)};
            float sint      {sin(degToRad(e)) * sin(degToRad(lambda))};

            return radToDeg(asin(sint));
        }

        /**
         * \brief Calculates the solar hour angle.
         * \param latitude Observer's latitude in degrees.
         * \param solarDec Sun's declination in degrees.
         * \param offset The zenith angle (e.g., 90.833 for standard sunset).
         * \return float Hour angle in **radians**. Returns `NaN` if the event does not occur (polar day/night).
         */
        float calcHourAngle(float latitude, float solarDec, float offset)
        {
            float latRad    {degToRad(latitude)};
            float sdRad     {degToRad(solarDec)};
            float cosH      {cos(degToRad(offset)) / (cos(latRad) * cos(sdRad)) - tan(latRad) * tan(sdRad)};

            // If cosH is > 1 or < -1, the sun never reaches the specified zenith.
            // This happens in polar regions (polar day or polar night). -> Return Not a Number

            return ((cosH > 1.0F || cosH < -1.0F) ? NAN : acos(cosH));
        }

        /**
         * \brief Calculates the Julian Day number from a time_t.
         *
         * This calculation is for noon UTC on the given date.
         *
         * \param time time_t for the date.
         * \return float The Julian Day number.
         */
        float calcJulianDate(time_t time)
        {
            constexpr int   BASE_YEAR   {1900};
            tm             *timeObjPtr  {gmtime(&time)};
            int             year        {timeObjPtr->tm_year + BASE_YEAR};
            int             month       {timeObjPtr->tm_mon + 1};
            int             day         {timeObjPtr->tm_mday};

            if (month <= 2)
            {
                year -= 1;
                month += 12;
            }
            float A     {floor(year / 100.0F)};
            float B     {2.0F - A + floor(A / 4.0F)};
            float JD    {floor(365.25F * (year + 4716)) + floor(30.6001F * ( month + 1)) + day + B - 1524.5F};

            return JD;
        }

        /**
         * \brief Converts a time in Julian centuries back to a Julian Day number.
         * \param julianCentury Time in Julian centuries since J2000.
         * \return float The Julian Day number.
         */
        float calcJulianDateFromJulianCentury(float julianCentury)
        {
            return (julianCentury * 36525.0F + 2451545.0F);
        }
    } // end anonymous namespace

    /**
     * \brief Implementation of calcSunEvent.
     */
    time_t calcSunEvent(Event_t event, time_t time, float latitude, float longitude)
    {
        // Array of zenith angles corresponding to the Event enum
        static const float  angles[] =      {SUN_ANGLE_STANDARD, SUN_ANGLE_CIVIL, SUN_ANGLE_NAUTICAL, SUN_ANGLE_ASTONOMICAL};
        // Determine the zenith angle offset.
        float               offset          {angles[(uint8_t)event % (sizeof(angles) / sizeof(angles[0]))]};
        // Determine if this is a sunrise (1.0) or sunset (-1.0) event
        float               eventType       {(event >= (sizeof(angles) / sizeof(angles[0]))) ? -1.0F : 1.0F};
        // Calculate Julian century for noon UTC on the given date
        float               julianCentury   {calcTimeJulianCentury(calcJulianDate(time))};
        // *** First pass to approximate sunrise/sunset ***
        float               eqTime          {calcEquationOfTime(julianCentury)};
        float               solarDec        {calcSunDeclination(julianCentury)};
        float               hourAngle       {eventType * calcHourAngle(latitude, solarDec, offset)};
        time_t              result          {0};

        if (not isnan(hourAngle))                                   // Check for polar day/night
        {
            float delta     {longitude + radToDeg(hourAngle)};
            float timeDiff  {4 * delta};	                        // in minutes of time
            float fTimeUTC  {HALF_DAY_MIN - timeDiff - eqTime};     // in minutes

            // *** Second pass to refine the calculation ***
            // Use the approximate time (fTimeUTC) to recalculate the parameters
            // for a more accurate Julian century value.
            float newt          {calcTimeJulianCentury(calcJulianDateFromJulianCentury(julianCentury) + fTimeUTC / 1440.0F)};

            eqTime = calcEquationOfTime(newt);
            solarDec = calcSunDeclination(newt);
            hourAngle = eventType * calcHourAngle(latitude, solarDec, offset);

            if (not isnan(hourAngle))                               // Check again, as refinement might push it over the edge
            {
                delta = longitude + radToDeg(hourAngle);
                timeDiff = 4 * delta;
                fTimeUTC = HALF_DAY_MIN - timeDiff - eqTime;        // final refined time in minutes
                result = (time_t)(round(fTimeUTC) * SECS_PER_MIN);
            }
        }

        return result;	                                            // return time in seconds from midnight or 0 if error
    }

    /**
     * \brief Calculates the approximate moon phase.
     * \param epochTime time_t seconds from epoch to calculate the moonphase for
     *
     * This is a simple calculation to tell us roughly what the moon phase is.
     * It does not give position.
     *
     * \return int An integer from 0 to 29:
     * - 0: New Moon
     * - 14: Full Moon
     * - 29: Waning crescent
     */
    int moonPhase(time_t epochTime)
    {
        // Reference epoch: 1970-01-08 04:35:00 UTC (an approximate New Moon)
        constexpr time_t    MOON_EPOCH      {614100};
        // Synodic period of the Moon (New Moon to New Moon) is ~29.53059 days
        // 29.53059 * 24 * 60 * 60 = 2551443 seconds
        constexpr time_t    SYNODIC_PERIOD  {2551443};
        // Calculate seconds into the current cycle
        time_t              phase           {(epochTime - MOON_EPOCH) % SYNODIC_PERIOD};

        if (phase < 0)                                              // Ensure phase is positive if epochTime is before MOON_EPOCH
        {
            phase += SYNODIC_PERIOD;
        }

        // Convert seconds into the cycle to a day number (0-29)
        int result {static_cast<int>(floor((float)phase / SECS_PER_DAY)) + 1};

        // Handle wrap-around
        return (30 == result) ? 0 : result;
    }
}
