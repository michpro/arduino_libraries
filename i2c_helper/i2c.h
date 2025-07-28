/**
 * @file    i2c.h
 * @brief   Free functions to facilitate operation of devices connected to I2C bus.
 *
 * This header provides a set of utilities for communicating with devices over the I2C bus using the Arduino Wire library.
 * It defines data structures, enumerations, and functions to handle I2C communication,
 * including device detection, reading, writing, and combined write-then-read operations.
 *
 * @copyright SPDX-FileCopyrightText: Copyright 2022-2025 Michal Protasowicki
 *
 * @license SPDX-License-Identifier: MIT
 */

#pragma once

#include "Wire.h"

namespace I2C
{
    /**
     * @brief Enumeration of status codes returned by I2C communication functions.
     *
     * This enumeration defines possible outcomes of I2C operations, including success and various error conditions.
     */
    typedef enum : uint8_t
    {
        SUCCESS             = 0x00,     // Operation completed successfully.
        DATA_TOO_LONG       = 0x01,     // Data exceeds the transmit buffer size.
        NACK_AFTER_ADDRESS  = 0x02,     // Received NACK after transmitting the device address.
        NACK_AFTER_DATA     = 0x03,     // Received NACK after transmitting data.
        OTHER_ERROR         = 0x04,     // Unspecified error occurred during transmission.
        TIMEOUT             = 0x05,     // Transmission timed out.
        WRONG_DATA_AMOUNT   = 0x80,     // Number of bytes received does not match the expected amount, or readLen is zero.
    } results_t;

    /**
     * @brief Enumeration to control whether a stop bit is sent after an I2C transaction.
     *
     * This enumeration specifies whether to send a stop condition at the end of a transaction, affecting bus release.
     */
    typedef enum : bool
    {
        NO_STOP             = false,    // Do not send a stop bit, keeping the bus active.
        SEND_STOP           = true,     // Send a stop bit, releasing the bus.
    } stopBit_t;

    /**
     * @brief Structure to hold I2C transaction context and settings.
     *
     * This structure encapsulates all necessary information for an I2C transaction,
     * including pointers to the I2C interface, data buffers, device address, and transaction parameters.
     *
     * @param[in]   wire            Pointer to an initialized TwoWire object for I2C communication.
     * @param[in]   writeBuffer     Pointer to a buffer containing data to write to the slave device.
     * @param[out]  readBuffer      Pointer to a buffer where data read from the slave device will be stored.
     * @param[in]   devAddress      7-bit address of the slave device on the I2C bus.
     * @param[in]   writeLen        Number of bytes to write to the slave device.
     * @param[in]   readLen         Number of bytes to read from the slave device.
     * @param[in]   stopAfterWrite  Indicates whether to send a stop bit after writing.
     * @param[in]   stopAfterRead   Indicates whether to send a stop bit after reading.
     */
    typedef struct
    {
        TwoWire           * wire;           // I2C interface object.
        uint8_t           * writeBuffer;    // Buffer for data to be written.
        uint8_t           * readBuffer;     // Buffer for data to be read.
        uint8_t             devAddress;     // Slave device address.
        uint8_t             writeLen;       // Number of bytes to write.
        uint8_t             readLen;        // Number of bytes to read.
        bool                stopAfterWrite; // Send stop bit after write operation.
        bool                stopAfterRead;  // Send stop bit after read operation.
    } context_t;

    /**
     * @brief Size of the data buffer for I2C transactions, derived from the Wire library.
     */
    constexpr uint8_t BUFFER_SIZE {BUFFER_LENGTH};

    /**
     * @brief Number of retry attempts for reading data from a slave device before reporting an error.
     */
    constexpr uint8_t RETRIES {20};

    /**
     * @brief Checks if a slave device is present on the I2C bus.
     *
     * This function initiates a transmission to the device specified in the context and checks for an acknowledgment.
     *
     * @param[in] ctx Pointer to the I2C context containing the device address and TwoWire object.
     *
     * @return true if the device acknowledges, false otherwise.
     */
    bool isDevicePresent(context_t *ctx);

    /**
     * @brief Reads a specified number of bytes from a slave device.
     *
     * This function requests a specified number of bytes from the slave device and stores them in the read buffer.
     * The number of bytes to read is specified in the context's readLen field.
     *
     * @param[in,out] ctx Pointer to the I2C context containing the device address, read buffer, and read length.
     *
     * @return Status code of type results_t indicating the outcome of the operation.
     *         If successful, the read data is stored in the buffer pointed to by readBuffer.
     */
    uint8_t readBytes(context_t *ctx);

    /**
     * @brief Writes a specified number of bytes to a slave device.
     *
     * This function writes a specified number of bytes from the write buffer to the slave device.
     * The number of bytes to write is specified in the context's writeLen field.
     *
     * @param[in] ctx Pointer to the I2C context containing the device address, write buffer, and write length.
     *
     * @return Status code of type results_t indicating the outcome of the operation.
     */
    uint8_t writeBytes(context_t *ctx);

    /**
     * @brief Writes and then reads bytes from a slave device in a single transaction.
     *
     * This function first writes a specified number of bytes to the slave device and then reads a specified number
     * of bytes, as defined in the context's writeLen and readLen fields, respectively.
     *
     * @param[in,out] ctx Pointer to the I2C context containing the device address, write and read buffers, and lengths.
     *
     * @return Status code of type results_t indicating the outcome of the operation.
     *         If successful, the read data is stored in the buffer pointed to by readBuffer.
     */
    uint8_t writeThenReadBytes(context_t *ctx);
}
