/**
 * \file i2c.h
 * \brief Free functions to facilitate operation of devices connected to I2C bus.
 *
 * \copyright SPDX-FileCopyrightText: Copyright 2022-2023 Michal Protasowicki
 *
 * \license SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "Wire.h"

namespace I2C
{
    /**
     * \brief Type, describing codes returned by functions that support communication via I2C bus.
    **/
    typedef enum : uint8_t
    {
        SUCCESS             = 0x00,     // success
        DATA_TOO_LONG       = 0x01,     // data too long to fit in transmit buffer
        NACK_AFTER_ADDRESS  = 0x02,     // received NACK on transmit of address
        NACK_AFTER_DATA     = 0x03,     // received NACK on transmit of data
        OTHER_ERROR         = 0x04,     // other error
        TIMEOUT             = 0x05,     // transmission timeout
        WRONG_DATA_AMOUNT   = 0x80,     // occurs when number of bytes returned from peripheral device
                                        // is different from what is expected in readLen parameter of context,
                                        // or readLen parameter is equal to 0.
    } results_t;

    /**
     * \brief Type that determines whether transmission is to end with sending a stop bit or not.
    **/
    typedef enum : bool
    {
        NO_STOP             = false,
        SEND_STOP           = true,
    } stopBit_t;

    /**
     * \brief Context in which all transmission settings and pointers to data buffers for a given chip are stored.
     * 
     * \param[in]   wire            A pointer to an initialized TwoWire class object that will be used
     *                              for data transmission over the I2C bus.
     * \param[in]   writeBuffer     A pointer to a buffer with data to write to.
     * \param[out]  readBuffer      A pointer to a buffer with data to read.
     * \param[in]   devAddress      Address of slave device on I2C bus with which we will communicate.
     * \param[in]   writeLen        Amount of data to write to slave device.
     * \param[in]   readLen         Amount of data to read from slave device.
     * \param[in]   stopAfterWrite  Indicates whether to send a stop bit after writing.
     * \param[in]   stopAfterRead   Indicates whether to send a stop bit after reading.
    **/
    typedef struct
    {
        TwoWire           * wire;
        uint8_t           * writeBuffer;
        uint8_t           * readBuffer;
        uint8_t             devAddress;
        uint8_t             writeLen;
        uint8_t             readLen;
        bool                stopAfterWrite;
        bool                stopAfterRead;
    } context_t;

    /**
     * \brief   Size of data buffer that can be sent via I2C bus in one operation.
     *          (Taken from Wire library)
    **/
    constexpr uint8_t BUFFER_SIZE   {BUFFER_LENGTH};

    /**
     * \brief Number of attempts to read data from slave I2C device, after which an error will be reported.
    **/
    constexpr uint8_t RETRIES       {20};

    /**
     * \brief A method that checks whether a slave device with address indicated in context is available on the I2C bus.
     * 
     * \param ctx[in] Current I2C device context.
     * 
     * \return true when device is available, otherwise false.
    **/
    bool isDevicePresent(context_t *ctx);

    /**
     * \brief   A method that reads n bytes from a slave device. In context,
     *          in readLen field, we indicate the number of bytes to be read
     *          from the device with address indicated in devAddress.
     *          Read data will be written to the buffer indicated by readBuffer pointer.
     *
     * \param ctx[in,out] Current I2C device context.
     *
     * \return  If the operation was successful, data is read in the buffer indicated by readBuffer pointer.
     *          Status of operations of results_t type (uint8_t) is also returned.
    **/
    uint8_t readBytes(context_t *ctx);

    /**
     * \brief   A method that writes n bytes to a slave device. In context,
     *          in writeLen field, we indicate the number of bytes to be written
     *          to the device with the address indicated in devAddress.
     *          Data to be written will be taken from the buffer indicated by writeBuffer pointer.
     *
     * \param ctx[in] Current I2C device context.
     *
     * \return Operation status of results_t (uint8_t) type is returned.
    **/
    uint8_t writeBytes(context_t *ctx);

    /**
     * \brief   A method that writes x bytes and then reads y bytes from a slave device.
     *          In context, in writeLen field, we indicate the number of bytes to write
     *          to the device with the address indicated in devAddress, and in readLen field,
     *          we indicate the number of bytes to read. Data for writing will be taken from
     *          the buffer pointed to by writeBuffer pointer, and read data will be placed in
     *          the buffer pointed to by readBuffer pointer.
     *
     * \param ctx[in,out] Current I2C device context.
     *
     * \return  If the operation was successful, data is read in the buffer indicated by readBuffer pointer.
     *          Status of operations of results_t type (uint8_t) is also returned.
    **/
    uint8_t writeThenReadBytes(context_t *ctx);
}
