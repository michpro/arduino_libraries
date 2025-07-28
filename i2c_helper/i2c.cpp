/**
 * @file i2c.cpp
 * @brief Implementation of functions to facilitate operation of devices connected to I2C bus.
 *
 * This file provides the implementation of the I2C utility functions declared in i2c.h, enabling communication with
 * I2C slave devices using the Arduino Wire library. The functions handle device detection, reading, writing, and
 * combined write-then-read operations, with error checking and retry mechanisms.
 *
 * @copyright SPDX-FileCopyrightText: Copyright 2022-2025 Michal Protasowicki
 *
 * @license SPDX-License-Identifier: MIT
 */

#include "i2c.h"

/**
 * @brief Checks if a slave device is present on the I2C bus.
 *
 * This function initiates a transmission to the device specified in the context and checks for an acknowledgment
 * by ending the transmission and evaluating the result.
 *
 * @param[in] ctx Pointer to the I2C context containing the device address and TwoWire object.
 *
 * @return true if the device acknowledges (SUCCESS), false otherwise.
 */
bool I2C::isDevicePresent(context_t *ctx)
{
    bool result {false};

    if (nullptr != ctx->wire)
    {
        ctx->wire->beginTransmission(ctx->devAddress);
        result = (I2C::SUCCESS == ctx->wire->endTransmission());
    }

    return result;
}

/**
 * @brief Reads a specified number of bytes from a slave device.
 *
 * This function requests a specified number of bytes from the slave device, as defined in the context's readLen field,
 * and stores them in the read buffer. It includes retry logic to handle transient failures and validates buffer and length parameters.
 *
 * @param[in,out] ctx Pointer to the I2C context containing the device address, read buffer, and read length.
 *
 * @return Status code of type results_t:
 *         - SUCCESS:           Data was read successfully.
 *         - WRONG_DATA_AMOUNT: Incorrect number of bytes received or readLen is zero.
 *         - DATA_TOO_LONG:     readLen exceeds buffer size.
 *         - OTHER_ERROR:       Invalid context or wire object.
 */
uint8_t I2C::readBytes(context_t *ctx)
{
    uint8_t resultCode {I2C::OTHER_ERROR};

    if ((nullptr != ctx->wire) && (nullptr != ctx->readBuffer))
    {
        if ((ctx->readLen > 0) && (ctx->readLen <= BUFFER_SIZE))
        {
            uint8_t retries     {RETRIES};
            bool    retry       {true};
            bool    matchLen    {false};

            do
            {
                matchLen = (ctx->readLen == ctx->wire->requestFrom((uint8_t)(ctx->devAddress), (uint8_t)(ctx->readLen), (uint8_t)ctx->stopAfterRead));
                retry = !matchLen && retries--;
            } while (retry);

            if (matchLen)
            {
                for (uint8_t idx = 0; idx < ctx->readLen; idx++)
                {
                    ctx->readBuffer[idx] = ctx->wire->read();
                }
                resultCode = I2C::SUCCESS;
            }
            else
            {
                while(ctx->wire->available())
                {
                    ctx->wire->read();
                }
                resultCode = I2C::WRONG_DATA_AMOUNT;
            }
        }
        else
        {
            resultCode = (0 == ctx->readLen) ? I2C::WRONG_DATA_AMOUNT : I2C::DATA_TOO_LONG;
        }
    }

    return resultCode;
}

/**
 * @brief Writes a specified number of bytes to a slave device.
 *
 * This function writes a specified number of bytes from the write buffer to the slave device, as defined in the
 * context's writeLen field. It validates buffer and length parameters before initiating the transmission.
 *
 * @param[in] ctx Pointer to the I2C context containing the device address, write buffer, and write length.
 *
 * @return Status code of type results_t:
 *         - SUCCESS:           Data was written successfully.
 *         - WRONG_DATA_AMOUNT: writeLen is zero.
 *         - DATA_TOO_LONG:     writeLen exceeds buffer size.
 *         - OTHER_ERROR:       Invalid context or wire object.
 *         - NACK_AFTER_ADDRESS,
 *           NACK_AFTER_DATA,
 *           TIMEOUT:           As reported by the Wire library.
 */
uint8_t I2C::writeBytes(context_t *ctx)
{
    uint8_t resultCode {I2C::OTHER_ERROR};

    if ((nullptr != ctx->wire) && (nullptr != ctx->writeBuffer))
    {
        if ((ctx->writeLen > 0) && (ctx->writeLen <= BUFFER_SIZE))
        {
            ctx->wire->beginTransmission(ctx->devAddress);
            for (uint8_t idx = 0; idx < ctx->writeLen; idx++)
            {
                ctx->wire->write(ctx->writeBuffer[idx]);
            }
            resultCode = ctx->wire->endTransmission(ctx->stopAfterWrite);
        }
        else
        {
            resultCode = (0 == ctx->writeLen) ? I2C::WRONG_DATA_AMOUNT : I2C::DATA_TOO_LONG;
        }
    }

    return resultCode;
}

/**
 * @brief Writes and then reads bytes from a slave device in a single transaction.
 *
 * This function first calls writeBytes to send data to the slave device and, if successful, calls readBytes to
 * retrieve data. The transaction parameters are specified in the context.
 *
 * @param[in,out] ctx Pointer to the I2C context containing the device address, write and read buffers, and lengths.
 *
 * @return Status code of type results_t, reflecting the outcome of either the write or read operation.
 *         If successful, the read data is stored in the buffer pointed to by readBuffer.
 */
uint8_t I2C::writeThenReadBytes(context_t *ctx)
{
    uint8_t resultCode {writeBytes(ctx)};

    if (I2C::SUCCESS == resultCode)
    {
        resultCode = readBytes(ctx);
    }

    return resultCode;
}
