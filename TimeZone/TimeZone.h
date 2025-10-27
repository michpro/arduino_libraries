/**
 * @file TimeZone.h
 * @brief Public API for the TimeZone class, managing time conversions between
 * UTC and local time, considering Daylight Saving Time (DST) rules.
 * @copyright SPDX-FileCopyrightText: Copyright 2025 by Michal Protasowicki
 * @license SPDX-License-Identifier: MIT license
 */

#pragma once

#include <Arduino.h>
#include <time.h>

/**
 * @class TimeZone
 * @brief Manages time zone conversions, including Daylight Saving Time (DST).
 * @details This class calculates DST transition points based on rules 
 * (TimeChangeRule_t) and converts `time_t` values between UTC 
 * and the specified local time zone.
 */
class TimeZone
{
public:
    //----------------------------------------------------------------------
    // Type Definitions
    //----------------------------------------------------------------------

    /**
     * @brief Enumeration for days of the week, compatible with `tm_wday`.
     */
    typedef enum : uint8_t
    {
        Sun = 0, ///< Sunday
        Mon,     ///< Monday
        Tue,     ///< Tuesday
        Wed,     ///< Wednesday
        Thu,     ///< Thursday
        Fri,     ///< Friday
        Sat      ///< Saturday
    } DayOfWeek;

    /**
     * @brief Enumeration for the week of the month (used in time change rules).
     */
    typedef enum : uint8_t
    {
        First = 1, ///< First week
        Second,    ///< Second week
        Third,     ///< Third week
        Fourth,    ///< Fourth week
        Last       ///< Last week
    } Week;

    /**
     * @brief Enumeration for months of the year, compatible with `tm_mon` + 1.
     */
    typedef enum : uint8_t
    {
        Jan = 1, ///< January
        Feb,     ///< February
        Mar,     ///< March
        Apr,     ///< April
        May,     ///< May
        Jun,     ///< June
        Jul,     ///< July
        Aug,     ///< August
        Sep,     ///< September
        Oct,     ///< October
        Nov,     ///< November
        Dec      ///< December
    } Month;

    /**
     * @brief Structure to describe rules for when Daylight Saving Time (DST)
     * or Standard Time begins.
     */
    typedef struct
    {
        uint8_t week;   ///< @brief Week of the month (First, Second, Third, Fourth, Last).
        uint8_t dow;    ///< @brief Day of the week (Sun, Mon, ..., Sat).
        uint8_t month;  ///< @brief Month of the year (Jan, Feb, ..., Dec).
        uint8_t hour;   ///< @brief Hour of the day (0-23) for the transition.
        int     offset; ///< @brief Time zone offset from UTC in minutes for this rule.
    } TimeChangeRule_t;

    //----------------------------------------------------------------------
    // Public Methods
    //----------------------------------------------------------------------

    /**
     * @brief Deleted default constructor.
     * @details A TimeZone object must be initialized with at least one rule.
     */
    TimeZone(void) = delete;

    /**
     * @brief Constructs a TimeZone object with separate DST and Standard time rules.
     * @param dstStart Rule for the start of Daylight Saving Time.
     * @param stdStart Rule for the start of Standard Time.
     */
    TimeZone(TimeChangeRule_t dstStart, TimeChangeRule_t stdStart);

    /**
     * @brief Constructs a TimeZone object for a zone that does not observe DST.
     * @param stdTime A single rule defining the standard time offset (used for both DST and STD).
     */
    TimeZone(TimeChangeRule_t stdTime);

    /**
     * @brief Converts a UTC `time_t` to the corresponding local time.
     * @param time The UTC `time_t` to convert.
     * @return The corresponding local `time_t` (i.e., `time + offset`),
     * adjusted for the time zone and DST.
     */
    time_t toLocalTime(const time_t time);

    /**
     * @brief Converts a local `time_t` to the corresponding UTC time.
     * @details WARNING: This function has ambiguities during DST transitions.
     * See the implementation file (TimeZone.cpp) for detailed warnings.
     * @param localTime The local `time_t` value (i.e., `UTC_time + offset`) to convert.
     * @return The corresponding UTC `time_t`.
     */
    time_t toUTCTime(const time_t localTime);

    /**
     * @brief Checks if a given UTC time falls within the DST period.
     * @param time The UTC `time_t` to check.
     * @return true if the time is in the DST period, false otherwise.
     */
    bool timeIsDst(const time_t time);

    /**
     * @brief Checks if a given local time falls within the DST period.
     * @param localTime The local `time_t` value (i.e., `UTC_time + offset`) to check.
     * @return true if the time is in the DST period, false otherwise.
     */
    bool localTimeIsDst(const time_t localTime);

    /**
     * @brief Updates the DST and Standard Time rules for this TimeZone object.
     * @details This forces a recalculation of transition times on the next conversion.
     * @param dstStart New rule for the start of Daylight Saving Time.
     * @param stdStart New rule for the start of Standard Time.
     */
    void setRules(const TimeChangeRule_t &dstStart, const TimeChangeRule_t &stdStart);

    /**
     * @brief Gets the currently configured DST and Standard Time rules.
     * @param[out] dst Reference to a `TimeChangeRule_t` to store the DST rule.
     * @param[out] std Reference to a `TimeChangeRule_t` to store the Standard Time rule.
     * @return true if the DST rule is different from the Standard rule (i.e., DST is observed).
     * @return false if the rules are the same (i.e., no DST).
     */
    bool getRules(TimeChangeRule_t &dst, TimeChangeRule_t &std);

    /**
     * @brief Gets the currently configured Standard Time rule.
     * @param[out] rule Reference to a `TimeChangeRule_t` to store the Standard Time rule.
     */
    void getStdRule(TimeChangeRule_t &rule);

    /**
     * @brief Gets the currently configured Daylight Saving Time rule.
     * @param[out] rule Reference to a `TimeChangeRule_t` to store the DST rule.
     * @return true if the DST rule is different from the Standard rule (i.e., DST is observed).
     * @return false if the rules are the same (i.e., no DST).
     */
    bool getDstRule(TimeChangeRule_t &rule);

private:
    //----------------------------------------------------------------------
    // Private Members
    //----------------------------------------------------------------------

    static const int    BASE_YEAR   {1900}; ///< @brief Base year for `tm` structures (tm_year = year - 1900).

    TimeChangeRule_t    _dst;       ///< @brief Rule for the start of DST (or summer time).
    TimeChangeRule_t    _std;       ///< @brief Rule for the start of Standard Time.
    time_t              _dstUTC     {0}; ///< @brief Cached DST start time in UTC for the calculated year.
    time_t              _stdUTC     {0}; ///< @brief Cached Standard Time start time in UTC for the calculated year.
    time_t              _dstLocal   {0}; ///< @brief Cached DST start time in local `time_t` for the calculated year.
    time_t              _stdLocal   {0}; ///< @brief Cached Standard Time start time in local `time_t` for the calculated year.

    //----------------------------------------------------------------------
    // Private Methods
    //----------------------------------------------------------------------

    /**
     * @brief Helper to check if a given year is a leap year.
     * @param year The year to check.
     * @return true if it's a leap year, false otherwise.
     */
    bool isLeap(const int year);

    /**
     * @brief Extracts the calendar year (e.g., 2025) from a `time_t` value.
     * @param time The `time_t` (UTC) from which to extract the year.
     * @return The calendar year.
     */
    int getYear(const time_t time);

    /**
     * @brief Resets the cached time change points to 0.
     * @details This forces a recalculation by `calcTimeChanges()` on the next conversion.
     */
    void initTimeChanges(void);

    /**
     * @brief Calculates and caches the DST and Standard Time transition points for a given year.
     * @param year The year for which to calculate transitions.
     */
    void calcTimeChanges(const int year);

    /**
     * @brief Checks if the currently stored DST and Standard rules are identical.
     * @return true if all fields of _dst and _std are the same.
     * @return false otherwise.
     */
    bool isRulesEqual(void);
    
    /**
     * @brief Calculates the "local `time_t`" value for a given rule in a specific year.
     * @details A "local `time_t`" is a `time_t` value that represents the local time,
     * not UTC. It's equivalent to `(UTC_time_t + offset_in_seconds)`.
     * This function calculates the UTC `time_t` of the transition wall clock time
     * and adds the rule's offset.
     * @param rule The TimeChangeRule_t to apply.
     * @param year The year for which to calculate the time.
     * @return The `time_t` value representing the *local* time of the transition.
     */
    time_t ruleToTime(const TimeChangeRule_t &rule, const int year);

};
