/**
 * @file NTPClient.h
 * @brief Public API for the NTPClient class, used for fetching time from NTP servers.
 * @copyright SPDX-FileCopyrightText: Copyright 2025 by Michal Protasowicki
 * @license SPDX-License-Identifier: MIT license
 */

#pragma once

#include <Arduino.h>
#include <Udp.h>
#include <time.h>

/**
 * @class NTPClient
 * @brief Manages communication with a Network Time Protocol (NTP) server to get
 * the current UTC epoch time.
 * @details This class handles sending NTP requests, parsing the responses, and
 * calculating the current time based on a periodic update interval.
 */
class NTPClient
{
public:
    /**
     * @brief Deleted default constructor.
     * @details An NTPClient must be initialized with a UDP instance.
     */
    NTPClient(void) = delete;

    /**
     * @brief Constructs an NTPClient with a default update interval and server.
     * @param udp A reference to the UDP class instance (e.g., WiFiUDP).
     */
    NTPClient(UDP &udp);

    /**
     * @brief Constructs an NTPClient with a custom update interval.
     * @param udp A reference to the UDP class instance.
     * @param updateInterval Time in milliseconds between NTP updates (default is 60000ms).
     */
    NTPClient(UDP &udp, uint32_t updateInterval);

    /**
     * @brief Constructs an NTPClient with a custom NTP server pool name.
     * @param udp A reference to the UDP class instance.
     * @param poolServerName The hostname of the NTP server (e.g., "pool.ntp.org").
     */
    NTPClient(UDP &udp, const char *poolServerName);

    /**
     * @brief Constructs an NTPClient with a custom server name and update interval.
     * @param udp A reference to the UDP class instance.
     * @param poolServerName The hostname of the NTP server.
     * @param updateInterval Time in milliseconds between NTP updates.
     */
    NTPClient(UDP &udp, const char *poolServerName, uint32_t updateInterval);

    /**
     * @brief Constructs an NTPClient with a static IP address for the NTP server.
     * @param udp A reference to the UDP class instance.
     * @param poolServerIP The IP address of the NTP server.
     */
    NTPClient(UDP &udp, IPAddress poolServerIP);

    /**
     * @brief Constructs an NTPClient with a static IP and custom update interval.
     * @param udp A reference to the UDP class instance.
     * @param poolServerIP The IP address of the NTP server.
     * @param updateInterval Time in milliseconds between NTP updates.
     */
    NTPClient(UDP &udp, IPAddress poolServerIP, uint32_t updateInterval);

    /**
     * @brief Destructor. Calls `end()` to stop the UDP client.
     */
    ~NTPClient(void);

    /**
     * @brief Starts the underlying UDP client with the default local port.
     */
    void begin(void);

    /**
     * @brief Starts the underlying UDP client with a specified local port.
     * @param port The local port number to listen on.
     */
    void begin(uint16_t port);

    /**
     * @brief Updates the time from the NTP server if the update interval has passed.
     * @details This should be called in the main loop of your application.
     * @return true on a successful update, false if no update was attempted or it failed.
     */
    bool update(void);

    /**
     * @brief Forces an immediate update from the NTP server, bypassing the update interval.
     * @return true on success, false on failure (e.g., timeout).
     */
    bool forceUpdate(void);

    /**
     * @brief Checks if the time has been successfully set at least once.
     * @return true if time has been set, false otherwise.
     */
    bool isTimeSet(void) const;

    /**
     * @brief Sets the NTP server pool by its hostname.
     * @details This also clears any previously set static IP address.
     * @param poolServerName The hostname of the NTP server.
     */
    void setPoolServerName(const char *poolServerName);

    /**
     * @brief Sets the NTP server by its static IP address.
     * @details This also clears any previously set server name.
     * @param poolServerIP The IP address of the NTP server.
     */
    void setPoolServerIP(const IPAddress poolServerIP);

    /**
     * @brief Sets the update interval.
     * @param updateInterval Time in milliseconds between NTP updates.
     */
    void setUpdateInterval(const uint32_t updateInterval);

    /**
     * @brief Gets the current calculated epoch time.
     * @details This returns the last fetched time from the server plus the
     * time elapsed (in seconds) since the last update.
     * @return Time in seconds since Jan. 1, 1970 (Unix epoch).
     */
    uint32_t getEpochTime(void) const;

    /**
     * @brief Stops the underlying UDP client.
     */
    void end(void);

private:
    //----------------------------------------------------------------------
    // Constants
    //----------------------------------------------------------------------
    static const int        MS_IN_1S                {1000};         ///< @brief Milliseconds in one second.
    static const uint64_t   SECONDS_PER_ERA         {1ULL << 32};   ///< @brief Seconds in an NTP era ($2^{32}$), used for Y2036 rollover.
    static const uint32_t   UNIX_TIME_DELTA         {2208988800UL}; ///< @brief Seconds between NTP epoch (1900) and Unix epoch (1970).
    static const uint8_t    NTP_PACKET_SIZE         {48};           ///< @brief NTP time stamp is in the first 48 bytes of the message.
    static const uint16_t   DEFAULT_LOCAL_NTP_PORT  {12300};        ///< @brief Default local port to listen for NTP packets.
    static const uint16_t   DEFAULT_NTP_PORT        {123};          ///< @brief Default NTP server port.

    //----------------------------------------------------------------------
    // Member Variables
    //----------------------------------------------------------------------
    UDP                    *_udp                    {nullptr};      ///< @brief Pointer to the UDP instance.
    bool                    _udpSetup               {false};        ///< @brief Flag to track if `_udp->begin()` has been called.
    const char             *_poolServerName         {"pool.ntp.org"}; ///< @brief Default NTP server hostname.
    IPAddress               _poolServerIP;                          ///< @brief IP address of the NTP server (if used).
    uint16_t                _port                   {DEFAULT_LOCAL_NTP_PORT}; ///< @brief Local port to use.
    uint32_t                _updateInterval         {60000};        ///< @brief Update interval in milliseconds.
    uint32_t                _currentEpoch           {0};            ///< @brief Last epoch time received from the server (in seconds).
    uint32_t                _lastUpdate             {0};            ///< @brief `millis()` timestamp of the last successful update.
    uint8_t                 _packetBuffer[NTP_PACKET_SIZE];         ///< @brief Buffer to hold NTP packets.

    //----------------------------------------------------------------------
    // Private Methods
    //----------------------------------------------------------------------
    
    /**
     * @brief Sends an NTP request packet to the configured server.
     */
    void sendNTPPacket(void);
};
