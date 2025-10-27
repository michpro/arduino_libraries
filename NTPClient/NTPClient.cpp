/**
 * @file NTPClient.cpp
 * @brief Implementation of the NTPClient class.
 * @copyright SPDX-FileCopyrightText: Copyright 2025 by Michal Protasowicki
 * @license SPDX-License-Identifier: MIT license
 */

#include "NTPClient.h"

/**
 * @brief Constructs an NTPClient with a default update interval and server.
 * @param udp A reference to the UDP class instance (e.g., WiFiUDP).
 */
NTPClient::NTPClient(UDP &udp) :
                    _udp(&udp)
{}

/**
 * @brief Constructs an NTPClient with a custom update interval.
 * @param udp A reference to the UDP class instance.
 * @param updateInterval Time in milliseconds between NTP updates.
 */
NTPClient::NTPClient(UDP &udp, uint32_t updateInterval) :
                    _udp(&udp), _updateInterval(updateInterval)
{}

/**
 * @brief Constructs an NTPClient with a custom NTP server pool name.
 * @param udp A reference to the UDP class instance.
 * @param poolServerName The hostname of the NTP server (e.g., "pool.ntp.org").
 */
NTPClient::NTPClient(UDP &udp, const char *poolServerName) :
                    _udp(&udp), _poolServerName(poolServerName)
{}

/**
 * @brief Constructs an NTPClient with a static IP address for the NTP server.
 * @param udp A reference to the UDP class instance.
 * @param poolServerIP The IP address of the NTP server.
 */
NTPClient::NTPClient(UDP &udp, IPAddress poolServerIP) :
                    _udp(&udp), _poolServerName(NULL), _poolServerIP(poolServerIP)
{}

/**
 * @brief Constructs an NTPClient with a custom server name and update interval.
 * @param udp A reference to the UDP class instance.
 * @param poolServerName The hostname of the NTP server.
 * @param updateInterval Time in milliseconds between NTP updates.
 */
NTPClient::NTPClient(UDP &udp, const char *poolServerName, uint32_t updateInterval) :
                    _udp(&udp), _poolServerName(poolServerName), _updateInterval(updateInterval)
{}

/**
 * @brief Constructs an NTPClient with a static IP and custom update interval.
 * @param udp A reference to the UDP class instance.
 * @param poolServerIP The IP address of the NTP server.
 * @param updateInterval Time in milliseconds between NTP updates.
 */
NTPClient::NTPClient(UDP &udp, IPAddress poolServerIP, uint32_t updateInterval) :
                    _udp(&udp), _poolServerName(NULL), _poolServerIP(poolServerIP), _updateInterval(updateInterval)
{}

/**
 * @brief Destructor. Calls `end()` to stop the UDP client.
 */
NTPClient::~NTPClient(void)
{
    end();
}

/**
 * @brief Starts the underlying UDP client with the default local port.
 */
void NTPClient::begin(void)
{
    begin(DEFAULT_LOCAL_NTP_PORT);
}

/**
 * @brief Starts the underlying UDP client with a specified local port.
 * @param port The local port number to listen on.
 */
void NTPClient::begin(uint16_t port)
{
    _port = port;
    _udp->begin(_port);
    _udpSetup = true;
}

/**
 * @brief Updates the time from the NTP server if the update interval has passed.
 * @return true on a successful update, false if no update was attempted or it failed.
 */
bool NTPClient::update(void)
{
    // Update if the interval has passed OR if it has never been updated yet.
    if ((millis() - _lastUpdate >= _updateInterval) || 0 == _lastUpdate)
    {
        // Ensure UDP is started
        if (!_udpSetup || _port != DEFAULT_LOCAL_NTP_PORT)
        {
             begin(_port);
        }
        return forceUpdate();
    }
    return false;   // return false if update does not occur
}

/**
 * @brief Forces an immediate update from the NTP server.
 * @return true on success, false on failure (e.g., timeout).
 */
bool NTPClient::forceUpdate(void)
{
    static const int        NO_PACKET   {0};
    static const uint32_t   DELAY_10MS  {10};
    static const uint8_t    TIMEOUT_1S  {MS_IN_1S / DELAY_10MS}; // 100 iterations
    uint8_t                 timeout     {0};
    int                     response    {0};
    bool                    result      {true};

    // Flush any existing packets
    while(NO_PACKET != _udp->parsePacket())
    {
        _udp->flush();
    }

    // Send the NTP request packet
    sendNTPPacket();

    // Wait for a response or timeout
    do
    {
        // Use FreeRTOS delay if available, otherwise standard Arduino delay
#   if defined __has_include
#       if __has_include (<FreeRTOS.h>)
        vTaskDelay(DELAY_10MS / portTICK_PERIOD_MS);
#       else
        delay(DELAY_10MS);
#       endif
#   else
        delay(DELAY_10MS);
#   endif
        
        response = _udp->parsePacket();
        
        if (timeout > TIMEOUT_1S) // Check for timeout
        {
            result = false;
        }
        timeout++;
    } while ((NO_PACKET == response) && (true == result));

    // Process the response if received
    if (true == result)
    {
        // Compensate for the delay in reading the time
        _lastUpdate = millis() - (DELAY_10MS * (timeout + 1));
        
        // Read the packet into the buffer
        _udp->read(_packetBuffer, NTP_PACKET_SIZE);

        // --- Handle NTP Y2036 rollover ---
        // The NTP epoch is 1900, but $2^{32}$ seconds wraps in 2036.
        // The Unix epoch is 1970.
        // `UNIX_TIME_DELTA` is the 70-year difference.
        // `SECONDS_PER_ERA` is the $2^{32}$ wrap-around point.
        // `UNIX_END_OF_NTP_ERA` is the Unix time equivalent of 2036.
        static const uint32_t   UNIX_END_OF_NTP_ERA {SECONDS_PER_ERA - UNIX_TIME_DELTA};
        static const uint8_t    TRANSMIT_TIMESTAMP  {40}; // Offset for Transmit Timestamp

        // Extract the 32-bit seconds from the packet (bytes 40-43)
        uint32_t highWord = word(_packetBuffer[TRANSMIT_TIMESTAMP + 0], _packetBuffer[TRANSMIT_TIMESTAMP + 1]);
        uint32_t lowWord  = word(_packetBuffer[TRANSMIT_TIMESTAMP + 2], _packetBuffer[TRANSMIT_TIMESTAMP + 3]);
        
        // Combine into the full NTP timestamp (seconds since 1900)
        time_t ntpSeconds = (time_t)(highWord << 16) | lowWord;
        
        // Check if we are in the first NTP era (1900-2036) relative to Unix time
        bool isFirstNtpEra = (ntpSeconds >= UNIX_TIME_DELTA);
        
        // Convert to Unix epoch
        _currentEpoch = (true == isFirstNtpEra) ? (ntpSeconds - UNIX_TIME_DELTA)      // If 1970-2036
                                                : (ntpSeconds + UNIX_END_OF_NTP_ERA); // If 2036+
    }

    return result;
}

/**
 * @brief Checks if the time has been successfully set at least once.
 * @return true if time has been set, false otherwise.
 */
bool NTPClient::isTimeSet(void) const
{
    // `_lastUpdate` is non-zero only after a successful `forceUpdate`.
    return (0 != _lastUpdate);
}

/**
 * @brief Sets the NTP server pool by its hostname.
 * @param poolServerName The hostname of the NTP server.
 */
void NTPClient::setPoolServerName(const char *poolServerName)
{
    _poolServerName = poolServerName;
    _poolServerIP = (uint32_t)0; // Clear IP
}

/**
 * @brief Sets the NTP server by its static IP address.
 * @param poolServerIP The IP address of the NTP server.
 */
void NTPClient::setPoolServerIP(const IPAddress poolServerIP)
{
    _poolServerIP = poolServerIP;
    _poolServerName = nullptr; // Clear name
}

/**
 * @brief Sets the update interval.
 * @param updateInterval Time in milliseconds between NTP updates.
 */
void NTPClient::setUpdateInterval(uint32_t updateInterval)
{
    _updateInterval = updateInterval;
}

/**
 * @brief Gets the current calculated epoch time.
 * @return Time in seconds since Jan. 1, 1970 (Unix epoch).
 */
uint32_t NTPClient::getEpochTime(void) const
{
    // Return the last synced time plus the milliseconds elapsed since, converted to seconds.
    return _currentEpoch + ((millis() - _lastUpdate) / MS_IN_1S);
}

/**
 * @brief Stops the underlying UDP client.
 */
void NTPClient::end(void)
{
    _udp->stop();
    _udpSetup = false;
}

/**
 * @brief Sends an NTP request packet to the configured server.
 */
void NTPClient::sendNTPPacket(void)
{
    // Set all bytes in the buffer to 0
    memset(_packetBuffer, 0, NTP_PACKET_SIZE);

    // Initialize values needed to form NTP request
    // (LI = 3, Version = 4, Mode = 3) (Client)
    _packetBuffer[0] = 0b11100011;
    // Stratum (0=unspecified)
    _packetBuffer[1] = 0x00;
    // Polling Interval (6 = $2^6$s = 64s)
    _packetBuffer[2] = 0x06;
    // Peer Clock Precision
    _packetBuffer[3] = 0xEC;
    
    // 8 bytes of zero for Root Delay & Root Dispersion
    
    // Reference ID (set to "LOCL" for local/unsynchronized)
    _packetBuffer[12]  = 0x4C;
    _packetBuffer[13]  = 0x4F;
    _packetBuffer[14]  = 0x43;
    _packetBuffer[15]  = 0x4C;

    // Send the packet to the server
    if  (_poolServerName)
    {
        // Use DNS name
        _udp->beginPacket(_poolServerName, DEFAULT_NTP_PORT);
    } 
    else
    {
        // Use static IP
        _udp->beginPacket(_poolServerIP, DEFAULT_NTP_PORT);
    }
    
    _udp->write(_packetBuffer, NTP_PACKET_SIZE);
    _udp->endPacket();
}
