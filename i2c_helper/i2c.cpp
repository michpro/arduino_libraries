/**
 * \file i2c.cpp
 * \brief Free functions to facilitate operation of devices connected to I2C bus.
 *
 * \copyright SPDX-FileCopyrightText: Copyright 2022-2023 Michal Protasowicki
 *
 * \license SPDX-License-Identifier: MIT
 *
 */

#include "i2c.h"

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
            } else
            {
                while(ctx->wire->available())
                {
                    ctx->wire->read();
                }
                resultCode = I2C::WRONG_DATA_AMOUNT;
            }
        } else
        {
            resultCode = (0 == ctx->readLen) ? I2C::WRONG_DATA_AMOUNT : I2C::DATA_TOO_LONG;
        }
    }

    return resultCode;
}

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
        } else
        {
            resultCode = (0 == ctx->writeLen) ? I2C::WRONG_DATA_AMOUNT : I2C::DATA_TOO_LONG;
        }
    }

    return resultCode;
}

uint8_t I2C::writeThenReadBytes(context_t *ctx)
{
    uint8_t resultCode {writeBytes(ctx)};

    if (I2C::SUCCESS == resultCode)
    {
        resultCode = readBytes(ctx);
    }

    return resultCode;
}
