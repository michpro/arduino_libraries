/**
 * @file TimeZone.cpp
 * @brief Implementation of the TimeZone class for managing DST and time conversions.
 * @copyright SPDX-FileCopyrightText: Copyright 2025 by Michal Protasowicki
 * @license SPDX-License-Identifier: MIT license
 */

#include "TimeZone.h"

static const int    DAYS_IN_WEEK    {7};
static const time_t SECS_PER_MIN    {(time_t)(60)};

/**
 * @brief Constructs a TimeZone object from the given time change rules.
 * @param dstStart Rule for the start of Daylight Saving Time.
 * @param stdStart Rule for the start of Standard Time.
 */
TimeZone::TimeZone(TimeChangeRule_t dstStart, TimeChangeRule_t stdStart) : _dst(dstStart), _std(stdStart)
{
    initTimeChanges(); // Clear cache
}

/**
 * @brief Constructs a TimeZone object for a zone that does not observe
 * daylight time.
 * @param stdTime A single rule defining the standard time offset.
 */
TimeZone::TimeZone(TimeChangeRule_t stdTime) : _dst(stdTime), _std(stdTime)
{
    initTimeChanges(); // Clear cache
}

/**
 * @brief Convert the given UTC `time_t` to local time.
 * @param time A `time_t` value (UTC).
 * @return A `time_t` value representing the corresponding local time
 * (i.e., `time + offset`).
 */
time_t TimeZone::toLocalTime(const time_t time)
{
    // Get the year of the UTC time
    int year {getYear(time)};

    // Recalculate transition points if the year has changed
    // We check against _dstUTC, the cached UTC transition time.
    if (year != getYear(_dstUTC))
    {
        calcTimeChanges(year);
    }

    // Determine the correct offset (DST or Standard) based on UTC time
    int32_t offset {timeIsDst(time) ? _dst.offset : _std.offset};
    offset *= SECS_PER_MIN;

    // Return local time = UTC time + offset
    return time + (time_t)offset;
}

/**
 * @brief Convert the given local time to UTC time.
 *
 * @warning This function is provided for completeness, but should seldom be
 * needed and should be used sparingly and carefully.
 *
 * Ambiguous situations occur after the Standard-to-DST and the
 * DST-to-Standard time transitions. When changing to DST, there is
 * one hour of local time that does not exist, since the clock moves
 * forward one hour. Similarly, when changing to standard time, there
 * is one hour of local times that occur twice since the clock moves
 * back one hour.
 *
 * This function does not test whether it is passed an erroneous time
 * value during the Local -> DST transition that does not exist.
 * If passed such a time, an incorrect UTC time value will be returned.
 *
 * If passed a local time value during the DST -> Local transition
 * that occurs twice, it will be treated as the earlier time, i.e.
 * the time that occurs before the transistion.
 *
 * Calling this function with local times during a transition interval
 * should be avoided!
 *
 * @param localTime A `time_t` value representing a local time (i.e., `UTC_time + offset`).
 * @return The corresponding `time_t` value in UTC.
 */
time_t TimeZone::toUTCTime(const time_t localTime)
{
    // Get the year of the *local* time
    int year {getYear(localTime)};

    // Recalculate transition points if the year has changed
    // We check against _dstLocal, the cached local transition time.
    if (year != getYear(_dstLocal))
    {
        calcTimeChanges(year);
    }

    // Determine the offset based on the *local* time
    int32_t offset {localTimeIsDst(localTime) ? _dst.offset : _std.offset};
    offset *= SECS_PER_MIN;

    // Return UTC time = local time - offset
    return localTime - (time_t)offset;
}

/**
 * @brief Determine whether the given UTC `time_t` is within the DST interval
 * or the Standard time interval.
 * @param time The `time_t` (UTC) to check.
 * @return true if the time is DST, false otherwise.
 */
bool TimeZone::timeIsDst(const time_t time)
{
    // Get the year of the UTC time
    int year {getYear(time)};

    // Recalculate transition points if the year has changed
    if (year != getYear(_dstUTC))
    {
        calcTimeChanges(year);
    }

    // Check if DST is observed at all (transitions are different)
    bool result {_stdUTC != _dstUTC};
    
    if (true == result)
    {
        // Check if time is between DST start and Standard start.
        // The logic handles both northern and southern hemispheres.
        
        if (_stdUTC > _dstUTC) // Northern hemisphere
        {
            result = (time >= _dstUTC) && (time < _stdUTC);
        }
        else // Southern hemisphere
        {
            result = !((time >= _stdUTC) && (time < _dstUTC));
        }
    }

    return result;
}

/**
 * @brief Determine whether the given Local `time_t` is within the DST interval
 * or the Standard time interval.
 * @param localTime The `time_t` value representing a local time.
 * @return true if the time is DST, false otherwise.
 */
bool TimeZone::localTimeIsDst(const time_t localTime)
{
    // Get the year of the *local* time
    int year {getYear(localTime)};

    // Recalculate transition points if the year has changed
    if (year != getYear(_dstLocal))
    {
        calcTimeChanges(year);
    }

    // Check if DST is observed at all
    bool result {_stdUTC != _dstUTC};

    if (true == result)
    {
        // Check if *local* time is between *local* DST start and *local* Standard start.
        // This logic also handles northern and southern hemispheres.
        
        if (_stdLocal > _dstLocal) // Northern hemisphere
        {
            result = (localTime >= _dstLocal) && (localTime < _stdLocal);
        }
        else // Southern hemisphere
        {
            result = !((localTime >= _stdLocal) && (localTime < _dstLocal));
        }
    }

    return result;
}

/**
 * @brief Update the daylight and standard time rules from RAM.
 * @param dstStart New rule for the start of Daylight Saving Time.
 * @param stdStart New rule for the start of Standard Time.
 */
void TimeZone::setRules(const TimeChangeRule_t &dstStart, const TimeChangeRule_t &stdStart)
{
    // Set the new rules
    _dst = dstStart;
    _std = stdStart;
    
    // Invalidate cache to force recalculation on next conversion
    initTimeChanges();
}

/**
 * @brief Gets the currently configured DST and Standard Time rules.
 * @param[out] dst Reference to a `TimeChangeRule_t` to store the DST rule.
 * @param[out] std Reference to a `TimeChangeRule_t` to store the Standard Time rule.
 * @return true if the DST rule is different from the Standard rule (i.e., DST is observed).
 * @return false if the rules are the same (i.e., no DST).
 */
bool TimeZone::getRules(TimeChangeRule_t &dst, TimeChangeRule_t &std)
{
    dst = _dst;
    std = _std;

    return !isRulesEqual();
}

/**
 * @brief Gets the currently configured Standard Time rule.
 * @param[out] rule Reference to a `TimeChangeRule_t` to store the Standard Time rule.
 */
void TimeZone::getStdRule(TimeChangeRule_t &rule)
{
    rule = _std;
}

/**
* @brief Gets the currently configured Daylight Saving Time rule.
* @param[out] rule Reference to a `TimeChangeRule_t` to store the DST rule.
* @return true if the DST rule is different from the Standard rule (i.e., DST is observed).
* @return false if the rules are the same (i.e., no DST).
*/
bool TimeZone::getDstRule(TimeChangeRule_t &rule)
{
    rule = _dst;

    // Return true if the rules are *not* equal (meaning DST is observed)
    return !isRulesEqual();
}

/**
 * @brief Helper to check for leap years.
 * @details A year is a leap year if it's divisible by 4, except for century years
 * that are not divisible by 400.
 * @param year The calendar year to check.
 * @return true if the year is a leap year, false otherwise.
 */
bool TimeZone::isLeap(const int year)
{
    return ((0 == (year % 4)) && (0 != (year % 100))) || (0 == (year % 400));
}

/**
 * @brief Extracts the calendar year (e.g., 2025) from a `time_t` value.
 * @details Converts `time_t` to `tm` struct in UTC (`gmtime`) and extracts the year.
 * `tm_year` is years since `BASE_YEAR` (1900), so we add `BASE_YEAR`.
 * @param time The `time_t` (UTC) from which to extract the year.
 * @return The calendar year.
 */
int TimeZone::getYear(time_t time)
{
    // Get the tm struct (in UTC) for the given time_t
    tm *timeObjPtr {gmtime(&time)};
    // tm_year is years since 1900
    return timeObjPtr->tm_year + BASE_YEAR;
}

/**
 * @brief Initialize (clear) the DST and standard time change points.
 * @details Sets cached transition times to 0, forcing a recalculation
 * by `calcTimeChanges()` on the next conversion.
 */
void TimeZone::initTimeChanges(void)
{
    _dstLocal = 0;
    _stdLocal = 0;
    _dstUTC = 0;
    _stdUTC = 0;
}

/**
 * @brief Calculate the DST and standard time change points for the given
 * year as local and UTC `time_t` values.
 * @param year The year to calculate changes for.
 */
void TimeZone::calcTimeChanges(const int year)
{
    // ruleToTime returns a "local time_t" (UTC + offset)
    _dstLocal = ruleToTime(_dst, year);
    _stdLocal = ruleToTime(_std, year);

    // The UTC time of the transition is the local time_t minus the rule's offset
    _dstUTC = _dstLocal - (_dst.offset * SECS_PER_MIN);
    _stdUTC = _stdLocal - (_std.offset * SECS_PER_MIN);
}

/**
 * @brief Checks if the currently stored DST and Standard rules are identical.
 * @return true if all fields of _dst and _std are the same.
 * @return false otherwise.
 */
bool TimeZone::isRulesEqual(void)
{
    return _dst.week   == _std.week
        && _dst.dow    == _std.dow
        && _dst.month  == _std.month
        && _dst.hour   == _std.hour
        && _dst.offset == _std.offset;
}

/**
 * @brief Calculates the "local `time_t`" value for a given rule in a specific year.
 * @details This function determines the UTC `time_t` of the transition's
 * wall-clock time (e.g., "2:00 AM") and then adds the rule's offset
 * to create a "local `time_t`" value used for comparisons.
 * @param rule The TimeChangeRule_t to apply (DST or Standard).
 * @param year The year for which to calculate the time.
 * @return The `time_t` value representing the *local* time of the transition.
 */
time_t TimeZone::ruleToTime(const TimeChangeRule_t &rule, const int year)
{
    // Array of days in each month (non-leap year)
    static const int    monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    tm                  timeObj;

    // Zero out the tm struct
    memset(&timeObj, 0, sizeof(timeObj));

    // Set year and month. Day is 1st to find the DOW of the 1st.
    timeObj.tm_year = year - BASE_YEAR; // tm_year is years since 1900
    timeObj.tm_mon = rule.month - 1;    // tm_mon is 0-11
    timeObj.tm_mday = 1;                // Start with the 1st of the month

    // Get the time_t for the 1st of the month.
    // We assume mktime() interprets the tm struct as local time.
    // For this library to be portable, the system time should be set to UTC.
    time_t  firstOfMonth_t  {mktime(&timeObj)};
    
    // Get the weekday (tm_wday) for the 1st of the month.
    tm     *firstOfMonth_tm {gmtime(&firstOfMonth_t)};
    int     firstDow        {firstOfMonth_tm->tm_wday}; // 0=Sun..6=Sat
    int     targetDow       {(int)rule.dow};            // The rule's DOW
    
    // Find the date (mday) of the first occurrence of the target weekday
    // (targetDow - firstDow + 7) % 7 calculates the days to add to the 1st.
    int     mday            {1 + (targetDow - firstDow + DAYS_IN_WEEK) % DAYS_IN_WEEK};

    // Adjust for the rule's "week" (First, Second, Third, Fourth, Last)
    if (Week::Last == rule.week)
    {
        // Get the number of days in the target month
        int daysInMonth {monthDays[timeObj.tm_mon]};
        
        // Check for leap year if it's February
        if ((Month::Feb == rule.month) && isLeap(year))
        {
            daysInMonth++;
        }
        
        // Start with the first occurrence and add 4 weeks (28 days).
        mday += 4 * DAYS_IN_WEEK;
        // If this date is past the end of the month, roll back one week.
        if (mday > daysInMonth)
        {
            mday -= DAYS_IN_WEEK;
        }
    } else
    {
        // For First, Second, Third, Fourth week:
        // Add (week - 1) * 7 days to the first occurrence.
        mday += (rule.week - 1) * DAYS_IN_WEEK;
    }

    // Create the final time structure for the transition
    timeObj.tm_mday = mday;
    timeObj.tm_hour = rule.hour;

    // Get the UTC time_t corresponding to the local wall-clock time
    time_t localTransition = mktime(&timeObj);
    
    // Return the "local time_t" value by adding the rule's offset.
    // This value is used for comparisons against other "local time_t" values.
    return localTransition + ((time_t)rule.offset * SECS_PER_MIN);
}
