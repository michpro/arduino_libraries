/**
 * @file        GD32CAN.cpp
 * @brief       GD32F30x and GD32E50x CAN bus library implementation.
 *
 * @copyright   SPDX-FileCopyrightText: Copyright 2024-2026 Michal Protasowicki
 * @license     SPDX-License-Identifier: MIT
 */

#include "GD32CAN.h"

//*******************************************************************
//*                                                                 *
//*                         Free Functions                          *
//*                                                                 *
//*******************************************************************

//*************************** Ring Buffer ***************************

/**
 * @brief Initializes a ring buffer structure.
 * @param ring Reference to the ringBuffer_t to initialize.
 * @param buffer Pointer to the allocated memory array for messages.
 * @param size Size of the allocated buffer (number of messages).
 */
void initRingBuffer(volatile CAN::ringBuffer_t &ring, volatile CAN::message_t *buffer, uint32_t size);

/**
 * @brief Checks if the ring buffer is empty.
 * @param ring The ring buffer to check.
 * @return true if empty, false otherwise.
 */
bool isRingBufferEmpty(volatile CAN::ringBuffer_t &ring);

/**
 * @brief Checks if the ring buffer is full.
 * @param ring The ring buffer to check.
 * @return true if full, false otherwise.
 */
bool isRingBufferFull(volatile CAN::ringBuffer_t &ring);

/**
 * @brief Returns the number of elements currently in the ring buffer.
 * @param ring The ring buffer to count.
 * @return uint32_t Number of elements.
 */
uint32_t ringBufferCount(volatile CAN::ringBuffer_t &ring);

/**
 * @brief Adds a message to the ring buffer.
 * @param ring The ring buffer.
 * @param message The message to add.
 * @return true if added successfully, false if buffer is full.
 */
bool addToRingBuffer(volatile CAN::ringBuffer_t &ring, const CAN::message_t &message);

/**
 * @brief Reads a message from the ring buffer (peeks at tail).
 * Note: Does not increment the tail pointer (remove element).
 * @param ring The ring buffer.
 * @param message Reference to store the read message.
 * @return true if message read, false if buffer empty.
 */
bool readFromRingBuffer(volatile CAN::ringBuffer_t &ring, CAN::message_t &message);

/**
 * @brief Removes the oldest message from the ring buffer (increments tail).
 * @param ring The ring buffer.
 */
void removeFromRingBuffer(volatile CAN::ringBuffer_t &ring);

//*************************** CAN message ***************************

/**
 * @brief Transmits a CAN message directly using hardware mailboxes.
 * @param canPeriph The base address of the CAN peripheral.
 * @param message The message object to transmit.
 * @return true if placed in a mailbox, false if all mailboxes are full.
 */
bool canMessageTransmit(uint32_t canPeriph, const CAN::message_t &message);

/**
 * @brief Reads a CAN message from a specific hardware FIFO.
 * @param canPeriph The base address of the CAN peripheral.
 * @param fifo The FIFO number (0 or 1) to read from.
 * @param rx_message Pointer to the destination message object.
 */
void canMessageReceive(uint32_t canPeriph, uint8_t fifo, CAN::message_t *rx_message);

//*******************************************************************

namespace CAN
{
    #ifdef HAS_CAN0
    volatile CAN::ringBuffer_t *can0TxRing          {nullptr};
    volatile CAN::ringBuffer_t *can0RxRing          {nullptr};
    volatile bool               can0rxIrqEnabled    {false};
    #endif
    #ifdef HAS_CAN1
    volatile CAN::ringBuffer_t *can1TxRing          {nullptr};
    volatile CAN::ringBuffer_t *can1RxRing          {nullptr};
    volatile bool               can1rxIrqEnabled    {false};
    #endif
    #ifdef HAS_CAN2
    volatile CAN::ringBuffer_t *can2TxRing          {nullptr};
    volatile CAN::ringBuffer_t *can2RxRing          {nullptr};
    volatile bool               can2rxIrqEnabled    {false};
    #endif

    namespace Filter
    {
        static const uint16_t   SFID_SHIFT          {5};
        static const uint32_t   EFID_SHIFT          {3};
        static const uint32_t   HALFWORD_SHIFT      {sizeof(uint16_t) * __CHAR_BIT__};
        static const uint16_t   SHORT_FILTER_SHIFT  {3};
        static const uint32_t   TEST_RTR_FIELD      {Frame::Type::RTR};
    }
}

uint_fast8_t    GD32CAN::instances          {0x00};
int_fast8_t     GD32CAN::can1StartFilterId  {-1};

GD32CAN::GD32CAN(CAN::Device::device_t device)
{
    init(device, CAN::QueueSize::Q_64, static_cast<CAN::QueueSize::size_t>(0));
}

GD32CAN::GD32CAN(CAN::Device::device_t device, CAN::QueueSize::size_t rxQueueSize)
{
    init(device, rxQueueSize, static_cast<CAN::QueueSize::size_t>(0));
}

GD32CAN::GD32CAN(CAN::Device::device_t device, CAN::QueueSize::size_t rxQueueSize, CAN::QueueSize::size_t txQueueSize)
{
    init(device, rxQueueSize, txQueueSize);
}

GD32CAN::~GD32CAN(void)
{
    if (true == _isInstanceAllowed)
    {
        #if defined(HAS_CAN1)
        if ((CAN::Device::CAN_1_DEFAULT == _device) || (CAN::Device::CAN_1_ALT1 == _device))
        {
            can1StartFilterId = -1;
        }
        #endif
        
        delete[] _rxBuffer;
        delete[] _txBuffer;
        
        detachTransceiverSleepPin();
        canDeinit();
        nvicConfig(CAN::State::DISABLE);
        gpioConfig(CAN::State::DISABLE);
        instances &= ~_instance_mask;
    }
}

/**
 * @brief Internal initialization method called by constructors.
 * allocates memory for buffers and sets up instance pointers.
 */
void GD32CAN::init(const CAN::Device::device_t device, CAN::QueueSize::size_t rxQueueSize, CAN::QueueSize::size_t txQueueSize)
{
    _device = device;
    _deviceBase = ((uint32_t)_device) & CAN_DEVICE_MASK;

    // Reset pointers before allocation to avoid double delete issues if init is called oddly
    _rxBuffer = nullptr;
    _txBuffer = nullptr;

    _rxBuffer = ((uint32_t)rxQueueSize > 0) ? (new CAN::message_t[(uint32_t)rxQueueSize]) : nullptr;
    initRingBuffer(_rxRing, _rxBuffer, (uint32_t)rxQueueSize);

    _txBuffer = ((uint32_t)txQueueSize > 0) ? (new CAN::message_t[(uint32_t)txQueueSize]) : nullptr;
    initRingBuffer(_txRing, _txBuffer, (uint32_t)txQueueSize);

    switch (_device)
    {
    #if defined(HAS_CAN1)
        case CAN::Device::CAN_1_DEFAULT:
        case CAN::Device::CAN_1_ALT1:
            CAN::can1TxRing = &_txRing;
            CAN::can1RxRing = &_rxRing;
            _rxIrqEnabled = &(CAN::can1rxIrqEnabled);
            _instance_mask = INSTANCES_MASK_CAN1;
            can1StartFilterId = DEFAULT_CAN1_START_FILTER_ID;
            _firstFilterId = DEFAULT_CAN1_START_FILTER_ID;
            break;
    #endif
    #if defined(HAS_CAN2)
        case CAN::Device::CAN_2_DEFAULT:
        case CAN::Device::CAN_2_ALT1:
        #if defined(PE0)
        case CAN::Device::CAN_2_ALT2:
        #endif
            CAN::can2TxRing = &_txRing;
            CAN::can2RxRing = &_rxRing;
            _rxIrqEnabled = &(CAN::can2rxIrqEnabled);
            _instance_mask = INSTANCES_MASK_CAN2;
            break;
    #endif
        default:
            CAN::can0TxRing = &_txRing;
            CAN::can0RxRing = &_rxRing;
            _rxIrqEnabled = &(CAN::can0rxIrqEnabled);
            _instance_mask = INSTANCES_MASK_CAN0;
            break;
    }

    _isInstanceAllowed = (0x00 == (instances & _instance_mask));
    instances |= _instance_mask;
}

bool GD32CAN::begin(const CAN::Baudrate::baudrate_t baudrate)
{
    bool result {_isInstanceAllowed};

    if (true == result)
    {
        gpioConfig(CAN::State::ENABLE);
        nvicConfig(CAN::State::ENABLE);
        result = canConfig(baudrate);

        _isInitialized = result;
    }

    return result;
}

bool GD32CAN::isInitialized(void)
{
    return _isInitialized;
}

bool GD32CAN::attachTransceiverSleepPin(const pin_size_t pin)
{
    bool result {_isInstanceAllowed};

    if (true == result)
    {
        _transceiverSleepPin = pin;
        digitalWrite(_transceiverSleepPin, HIGH);
        pinMode(_transceiverSleepPin, OUTPUT);
    }

    return result;
}

bool GD32CAN::setTransceiverMode(const CAN::State::state_t state)
{
    bool result {_isInitialized && (NO_PIN != _transceiverSleepPin)};

    if (true == result)
    {
        digitalWrite(_transceiverSleepPin, ((CAN::State::ENABLE == state) ? LOW : HIGH));
    }

    return result;
}

bool GD32CAN::setWorkingMode(const CAN::WorkingMode::mode_t mode)
{
    return (_isInitialized ? (SUCCESS == can_working_mode_set(_deviceBase, (uint8_t)mode)) : false);
}

bool GD32CAN::setBaudrate(const CAN::Baudrate::baudrate_t baudrate)
{
    bool result {_isInitialized};

    if ((true == result) && ((uint32_t)baudrate <= HW_MAX_BAUDRATE))          // The maximum baud rate support to 1M
    {
        if (true == setWorkingMode(CAN::WorkingMode::INITIALIZE))
        {
            uint32_t regTemp    {CAN_BT(_deviceBase) & (CAN_BT_LCMOD | CAN_BT_SCMOD)};
            uint32_t canBtVal   {calculateCANBTRegValue(baudrate)};

            if (0 != canBtVal)
            {
                CAN_BT(_deviceBase) = (regTemp | canBtVal);
                result = true;
            } 
        }
        result = (setWorkingMode(CAN::WorkingMode::NORMAL)) ? result : false;
    }

    return result;
}

bool GD32CAN::clearAllFilters(void)
{
    bool result {_isInstanceAllowed};

    if ((true == result) && (_firstFilterId >= 0))
    {
        int_fast8_t maxFilterId {getMaxFilterId()};

        if ((_firstFilterId >= 0) && (maxFilterId >= 0))
        {
            for (int_fast8_t idx = _firstFilterId; idx <= maxFilterId; idx++)
            {
                setFilter(  (CAN::Filter::Bank::filter_t)idx,
                            (uint32_t)0x00000000U,
                            (uint32_t)0x00000000U,
                            CAN::Filter::Mode::MASK,
                            FILTERBITS_32,
                            CAN::State::DISABLE);
            }
        }
    }

    return result;
}

bool GD32CAN::disableFilter(const CAN::Filter::Bank::filter_t filterId)
{
    return setFilterState(filterId, CAN::State::DISABLE);
}

bool GD32CAN::enableFilter(const CAN::Filter::Bank::filter_t filterId)
{
    uint32_t    mask    {(uint32_t)(1 << (uint32_t)filterId)};
    bool        result  {0 != (_filtersStates & mask)};

    return (result ? setFilterState(filterId, CAN::State::ENABLE) : result);
}

bool GD32CAN::allowReceiveAllMessages(CAN::Filter::Id::idType_t idType)
{
    bool result {_isInstanceAllowed};

    if ((true == result) && (_firstFilterId >= 0))
    {
        const uint32_t  ID_TYPE_MASK    {CAN::Filter::Id::EXTENDED};
        uint32_t        id              {(CAN::Filter::Id::EXTENDED == idType) ? (uint32_t)CAN::Filter::Id::EXTENDED : 0x00000000U};
        uint32_t        mask            {(CAN::Filter::Id::ALL == idType) ? 0x00000000U : ID_TYPE_MASK};

        setFilter(  (CAN::Filter::Bank::filter_t)_firstFilterId, 
                    id,
                    mask,
                    CAN::Filter::Mode::MASK,
                    FILTERBITS_32,
                    CAN::State::ENABLE
                 );
    }

    return result;
}

bool GD32CAN::setFilter(const CAN::Filter::Bank::filter_t filterId, const uint32_t frameId, const CAN::Filter::FrameType::filteredFrames_t filteredFrames)
{
    return setFilter(filterId, frameId, CAN::Frame::EFID_MASK, filteredFrames);
}

bool GD32CAN::setFilter(const CAN::Filter::Bank::filter_t filterId, const uint32_t frameId_1, const CAN::Frame::Type::frameType_t frameType_1,
                                                                    const uint32_t frameId_2, const CAN::Frame::Type::frameType_t frameType_2)
{
    uint32_t    id_1        {(frameId_1 << CAN::Filter::EFID_SHIFT) | (uint32_t)CAN::Frame::Id::EXTENDED | (uint32_t)frameType_1};
    uint32_t    id_2        {(frameId_2 << CAN::Filter::EFID_SHIFT) | (uint32_t)CAN::Frame::Id::EXTENDED | (uint32_t)frameType_2};
    bool        canApply    {(frameId_1 <= CAN::Frame::EFID_MASK) && (frameId_2 <= CAN::Frame::EFID_MASK)};

    return (canApply ? setFilter(filterId, id_1, id_2, CAN::Filter::Mode::LIST, FILTERBITS_32, CAN::State::ENABLE) : false);
}

bool GD32CAN::setFilter(const CAN::Filter::Bank::filter_t filterId, const uint32_t frameId, const uint32_t frameIdMask, const CAN::Filter::FrameType::filteredFrames_t filteredFrames)
{
    uint32_t id     {(frameId << CAN::Filter::EFID_SHIFT) | (uint32_t)CAN::Frame::Id::EXTENDED};
    uint32_t mask   {(frameIdMask << CAN::Filter::EFID_SHIFT) | (uint32_t)CAN::Frame::Id::EXTENDED};

    if (CAN::Filter::FrameType::ANY != filteredFrames)
    {
        id |= (uint32_t)filteredFrames;
        mask |= CAN::Filter::TEST_RTR_FIELD;
    }

    return (frameId <= CAN::Frame::EFID_MASK) ? setFilter(filterId, id, mask, CAN::Filter::Mode::MASK, FILTERBITS_32, CAN::State::ENABLE) : false;
}

bool GD32CAN::setFilter(const CAN::Filter::Bank::filter_t filterId, const uint16_t frameId, const CAN::Filter::FrameType::filteredFrames_t filteredFrames)
{
    return setFilter(filterId, frameId, CAN::Frame::SFID_MASK, filteredFrames, frameId, CAN::Frame::SFID_MASK, filteredFrames);
}

bool GD32CAN::setFilter(const CAN::Filter::Bank::filter_t filterId, const uint16_t frameId_1, const CAN::Filter::FrameType::filteredFrames_t filteredFrames_1,
                                                                    const uint16_t frameId_2, const CAN::Filter::FrameType::filteredFrames_t filteredFrames_2)
{
    return setFilter(filterId, frameId_1, CAN::Frame::SFID_MASK, filteredFrames_1, frameId_2, CAN::Frame::SFID_MASK, filteredFrames_2);
}

bool GD32CAN::setFilter(const CAN::Filter::Bank::filter_t filterId, const uint16_t frameId_1, const CAN::Frame::Type::frameType_t frameType_1,
                                                                    const uint16_t frameId_2, const CAN::Frame::Type::frameType_t frameType_2,
                                                                    const uint16_t frameId_3, const CAN::Frame::Type::frameType_t frameType_3,
                                                                    const uint16_t frameId_4, const CAN::Frame::Type::frameType_t frameType_4)
{
    uint16_t    id_1        {(uint16_t)((frameId_1 << CAN::Filter::SFID_SHIFT) | ((uint16_t)frameType_1 << CAN::Filter::SHORT_FILTER_SHIFT))};
    uint16_t    id_2        {(uint16_t)((frameId_2 << CAN::Filter::SFID_SHIFT) | ((uint16_t)frameType_2 << CAN::Filter::SHORT_FILTER_SHIFT))};
    uint16_t    id_3        {(uint16_t)((frameId_3 << CAN::Filter::SFID_SHIFT) | ((uint16_t)frameType_3 << CAN::Filter::SHORT_FILTER_SHIFT))};
    uint16_t    id_4        {(uint16_t)((frameId_4 << CAN::Filter::SFID_SHIFT) | ((uint16_t)frameType_4 << CAN::Filter::SHORT_FILTER_SHIFT))};
    uint32_t    id_1_2      {((uint32_t)id_1 << CAN::Filter::HALFWORD_SHIFT) | (uint32_t)id_2};
    uint32_t    id_3_4      {((uint32_t)id_3 << CAN::Filter::HALFWORD_SHIFT) | (uint32_t)id_4};
    bool        canApply    {   (frameId_1 <= CAN::Frame::SFID_MASK) && (frameId_2 <= CAN::Frame::SFID_MASK)
                             && (frameId_3 <= CAN::Frame::SFID_MASK) && (frameId_4 <= CAN::Frame::SFID_MASK)};

    return (canApply ? setFilter(filterId, id_1_2, id_3_4, CAN::Filter::Mode::LIST, FILTERBITS_16, CAN::State::ENABLE) : false);
}

bool GD32CAN::setFilter(const CAN::Filter::Bank::filter_t filterId, const uint16_t frameId, const uint16_t frameIdMask, const CAN::Filter::FrameType::filteredFrames_t filteredFrames)
{
    return setFilter(filterId, frameId, frameIdMask, filteredFrames, frameId, frameIdMask, filteredFrames);
}

bool GD32CAN::setFilter(const CAN::Filter::Bank::filter_t filterId, const uint16_t frameId_1, const uint16_t frameIdMask_1, const CAN::Filter::FrameType::filteredFrames_t filteredFrames_1,
                                                                    const uint16_t frameId_2, const uint16_t frameIdMask_2, const CAN::Filter::FrameType::filteredFrames_t filteredFrames_2)
{
    uint16_t    id_1        {(uint16_t)(frameId_1 << CAN::Filter::SFID_SHIFT)};
    uint16_t    mask_1      {(uint16_t)(frameIdMask_1 << CAN::Filter::SFID_SHIFT)};
    uint16_t    id_2        {(uint16_t)(frameId_2 << CAN::Filter::SFID_SHIFT)};
    uint16_t    mask_2      {(uint16_t)(frameIdMask_2 << CAN::Filter::SFID_SHIFT)};
    bool        canApply    {(frameId_1 <= CAN::Frame::SFID_MASK) && (frameId_2 <= CAN::Frame::SFID_MASK)};

    if (CAN::Filter::FrameType::ANY != filteredFrames_1)
    {
        id_1 |= ((uint32_t)filteredFrames_1 << CAN::Filter::SHORT_FILTER_SHIFT);
        mask_1 |= (CAN::Filter::TEST_RTR_FIELD << CAN::Filter::SHORT_FILTER_SHIFT);
    }
    if (CAN::Filter::FrameType::ANY != filteredFrames_2)
    {
        id_2 |= ((uint32_t)filteredFrames_2 << CAN::Filter::SHORT_FILTER_SHIFT);
        mask_2 |= (CAN::Filter::TEST_RTR_FIELD << CAN::Filter::SHORT_FILTER_SHIFT);
    }

    uint32_t    id          {((uint32_t)id_1 << CAN::Filter::HALFWORD_SHIFT) | (uint32_t)id_2};
    uint32_t    mask        {((uint32_t)mask_1 << CAN::Filter::HALFWORD_SHIFT) | (uint32_t)mask_2};

    return canApply ? setFilter(filterId, id, mask, CAN::Filter::Mode::MASK, FILTERBITS_16, CAN::State::ENABLE) : false;
}

bool GD32CAN::write(CAN::message_t &message)
{
    bool result {_isInitialized};

    if (true == result)
    {
        bool sentDirectly {false};

        // Optimisation: If ring buffer is empty, try to write directly to hardware
        if (isRingBufferEmpty(_txRing)) 
        {
            if (canMessageTransmit(_deviceBase, message)) 
            {
                sentDirectly = true;
            }
        }

        if (!sentDirectly) 
        {
            // If hardware was full or buffer had data, append to buffer
            if (nullptr != _txRing.buffer)
            {
                result = addToRingBuffer(_txRing, message);

                // Only enable "Mailbox Empty" interrupt if we have data pending in the buffer.
                // Enabling it when buffer is empty causes an infinite ISR loop (ISR Storm).
                // We enable it AFTER adding to buffer.
                if (true == result) 
                {
                    setIrqState(IRQ_TX_MAILBOX_E, CAN::State::ENABLE);
                }
            } else 
            {
                result = false;
            }
        }
        // else: Sent directly and buffer is empty. Do NOT enable TX IRQ.
        // We leave the ISR disabled if we successfully sent directly and didn't add to buffer.
    }

    return result;
}

uint32_t GD32CAN::availableForWrite(void)
{
    // Start with available space in the ring buffer (if initialized)
    uint32_t result {(0 != _txRing.size) ? (_txRing.size - ringBufferCount(_txRing)) : 0};

    // Read the Transmit Status Register
    uint32_t tstat {CAN_TSTAT(_deviceBase)};

    // Check each mailbox empty flag manually using bitwise AND.
    if (tstat & CAN_TSTAT_TME0) 
    {
        result++;
    }
    if (tstat & CAN_TSTAT_TME1) 
    {
        result++;
    }
    if (tstat & CAN_TSTAT_TME2) 
    {
        result++;
    }

    return result;
}

uint32_t GD32CAN::available(void)
{
    return (ringBufferCount(_rxRing) + (uint32_t)can_receive_message_length_get(_deviceBase, CAN_FIFO0));
}

bool GD32CAN::peek(CAN::message_t &message)
{
    bool result {_isInitialized};

    if (true == result)
    {
        if (true == *_rxIrqEnabled)
        {
            setIrqState(IRQ_RX_FIFO_NE, CAN::State::DISABLE);
        }
        result = readFromRingBuffer(_rxRing, message);
        if (true == *_rxIrqEnabled)
        {
            setIrqState(IRQ_RX_FIFO_NE, CAN::State::ENABLE);
        }
    }

    return result;
}

bool GD32CAN::read(CAN::message_t &message)
{
    bool result {_isInitialized};

    if (true == result)
    {
        if (true == *_rxIrqEnabled)
        {
            setIrqState(IRQ_RX_FIFO_NE, CAN::State::DISABLE);
        } else
        {
            *_rxIrqEnabled = true;
        }

        result = readFromRingBuffer(_rxRing, message);

        if (true == result)
        {
            removeFromRingBuffer(_rxRing);
        }

        setIrqState(IRQ_RX_FIFO_NE, CAN::State::ENABLE);
    }

    return result;
}

uint_fast8_t GD32CAN::getError(void)
{
    const uint_fast8_t  DATA_ERR_SHIFT  {4};
    const uint_fast8_t  PASSIVE_MASK    {1 << 1};
    const uint_fast8_t  BUS_OFF_MASK    {1 << 2};
    uint_fast8_t        result          {(uint_fast8_t)CAN::Error::NONE};

    if (true == _isInitialized)
    {
        uint_fast8_t tempVal {(uint_fast8_t)GET_BITS((uint32_t)(CAN_ERR(_deviceBase)), 0U, 6U)};

        if (0 != tempVal)
        {
            result |= (tempVal >> DATA_ERR_SHIFT);
            if (0 != (tempVal & BUS_OFF_MASK))
            {
                result |= (uint_fast8_t)CAN::Error::BUS_OFF;
            } else
            {
                if (0 != (tempVal & PASSIVE_MASK))
                {
                    result |= (uint_fast8_t)CAN::Error::PASSIVE;
                } else
                {
                    result |= (uint_fast8_t)CAN::Error::WARNING;
                }
            }
        }
    }

    return result;
}

#if defined(HAS_CAN1)
bool GD32CAN::setCan1StartFilterId(CAN::Filter::Bank::filter_t filterId)
{
    bool result {_isInstanceAllowed && (INSTANCES_MASK_CAN1 == _instance_mask)};

    if (true == result)
    {
        can1_filter_start_bank((uint8_t)filterId);
        _firstFilterId = (int_fast8_t)filterId;
        can1StartFilterId = (int_fast8_t)filterId;
    }

    return result;
}
#endif


//*******************************************************************
//*                                                                 *
//*                       protected functions                       *
//*                                                                 *
//*******************************************************************

inline void GD32CAN::detachTransceiverSleepPin(void)
{
    if (NO_PIN != _transceiverSleepPin)
    {
        digitalWrite(_transceiverSleepPin, HIGH);
        pinMode(_transceiverSleepPin, INPUT);
    }
}

void GD32CAN::canDeinit(void)
{
#if defined(HAS_CAN0)
    rcu_periph_reset_enum canDev {RCU_CAN0RST};

    switch (_deviceBase)
    {
    #if defined(HAS_CAN1)
    case (uint32_t)CAN1:
        canDev = RCU_CAN1RST;
        break;
    #endif
    #if defined(HAS_CAN2)
    case (uint32_t)CAN2:
        canDev = RCU_CAN2RST;
        break;
    #endif
    default:
        break;
    }

    rcu_periph_reset_enable(canDev);
    rcu_periph_reset_disable(canDev);
#endif
}

bool GD32CAN::canConfig(const CAN::Baudrate::baudrate_t baudrate)
{
    bool result {initCanHw(baudrate, CAN::State::ENABLE)};
    
    if (true == result)
    {
        #if defined(HAS_CAN1)
        setCan1StartFilterId((CAN::Filter::Bank::filter_t)DEFAULT_CAN1_START_FILTER_ID);
        #endif
        result = clearAllFilters();
    }

    return result;
}

inline uint32_t GD32CAN::calculateCANBTRegValue(const CAN::Baudrate::baudrate_t baudrate)
{
    uint32_t    result  {0};
    uint16_t    baudpsc {0};
    uint8_t     bs1     {0};
    uint8_t     bs2     {0};
    uint8_t     sjw     {0};

    if (true == calculateCANBTValues(baudrate, baudpsc, bs1, bs2, sjw))
    {
        result =  (BT_BS1((uint32_t)bs1))
                | (BT_BS2((uint32_t)bs2))
                | (((uint32_t)sjw << 24) & CAN_SJW_MASK)
                | (((uint32_t)baudpsc)   & CAN_BAUDPSC_MASK);
    }

    return result;
}

/**
 * @brief Calculates timing parameters (prescaler, BS1, BS2) for a given baudrate.
 * Iteratively searches for optimal Time Quanta to minimize error.
 */
bool GD32CAN::calculateCANBTValues(const CAN::Baudrate::baudrate_t baudrate, uint16_t &baudpsc, uint8_t &bs1, uint8_t &bs2, uint8_t &sjw)
{
    bool result {false};

    if ((uint32_t)baudrate <= HW_MAX_BAUDRATE)                      // The maximum baud rate support to 1M
    {
        const uint32_t  MULTIPLIER      {1000000};
        const uint32_t  PERCENT_POINT   {(uint32_t)(87.5 * MULTIPLIER) / 100};
        const uint32_t  FREQ_MAX_DELTA  {1000};

        uint32_t        clockFreq       {rcu_clock_freq_get(CK_APB1)};
        uint32_t        baseQuanta      {16};
        uint32_t        timeQuanta      {0};
        uint32_t        freqDelta       {0};

        while (freqDelta <= FREQ_MAX_DELTA)                         // allow for a small frequency error - so you can calculate
        {                                                           // parameters for bitrates like 83333, 33333
            uint32_t offset {0};

            while (8 >= offset)                                     // Looking for a timeQuanta of between 8 and 18,
            {                                                       // start at 16 and work outwards
                bool quantaDown {0 == (clockFreq % (baudrate * (baseQuanta - offset)))};
                bool quantaUp   {(2 >= offset) && (0 == (clockFreq % (baudrate * (baseQuanta + offset))))};
                if ((true == quantaDown) || (true == quantaUp))
                {
                    timeQuanta = (true == quantaDown) ? (baseQuanta - offset) : (baseQuanta + offset);
                    result = true;
                    break;
                }

                offset += 1;
            }
            if (true == result)
            {
                break;
            }
            clockFreq -= 1;
            freqDelta += 1;
        }

        uint32_t tempBs1 {uint32_t(PERCENT_POINT * timeQuanta) - (1 * MULTIPLIER)};

        baudpsc = clockFreq / (baudrate * timeQuanta);
        sjw = CAN_BT_SJW_1TQ;

        if (0x0F >= (tempBs1 / MULTIPLIER))
        {
            uint32_t samplePointA {((1 * MULTIPLIER) + tempBs1) / timeQuanta};
            uint32_t samplePointB {((2 * MULTIPLIER) + tempBs1) / timeQuanta};

            if (abs((int32_t)(samplePointB - PERCENT_POINT)) < abs((int32_t)(samplePointA - PERCENT_POINT)))
            {
                tempBs1 += MULTIPLIER;
            }
        }

        bs1 = (tempBs1 / MULTIPLIER) - 1;
        bs2 = timeQuanta - bs1 - 3;
    }

    return result;
}

void GD32CAN::gpioConfig(const CAN::State::state_t state)
{
    const uint32_t  CAN0_DEFAULT_REMAP  {(uint32_t)0x001D0000U | PCF0_CAN_REMAP(0)};
    #if defined(HAS_CAN1) || defined(HAS_CAN2)
    const uint32_t  CAN1_DEFAULT_REMAP  {(uint32_t)0x00200000U | PCF0_CAN_REMAP(0)};

    bool            canBeSet        {true};
    #endif
    rcu_periph_enum canPeriph       {RCU_CAN0};                     // settings for CAN0 attached to default IO PA11/PA12
    rcu_periph_enum gpioPeriph      {RCU_GPIOA};
    uint32_t        gpioPort        {GPIOA};
    uint32_t        pinRx           {GPIO_PIN_11};
    uint32_t        pinTx           {GPIO_PIN_12};
    uint32_t        remap           {CAN0_DEFAULT_REMAP};

    switch (_device)
    {
        case CAN::Device::CAN_0_ALT1:
            gpioPeriph = RCU_GPIOB;
            gpioPort = GPIOB;
            pinRx = GPIO_PIN_8;
            pinTx = GPIO_PIN_9;
        #if defined(GD32F30X_CL) || defined(GD32E50X_CL) || defined(GD32E508)
            remap = GPIO_CAN0_PARTIAL_REMAP;
        #else
            remap = GPIO_CAN_PARTIAL_REMAP;
        #endif
            break;
    #if defined(PD0)
        case CAN::Device::CAN_0_ALT2:
            gpioPeriph = RCU_GPIOD;
            gpioPort = GPIOD;
            pinRx = GPIO_PIN_0;
            pinTx = GPIO_PIN_1;
        #if defined(GD32F30X_CL) || defined(GD32E50X_CL) || defined(GD32E508)
            remap = GPIO_CAN0_FULL_REMAP;
        #else
            remap = GPIO_CAN_FULL_REMAP;
        #endif
            break;
    #endif
    #if defined(HAS_CAN1)
        case CAN::Device::CAN_1_DEFAULT:
            pinRx = GPIO_PIN_12;
            pinTx = GPIO_PIN_13;
            remap = CAN1_DEFAULT_REMAP;
            canBeSet = false;
        case CAN::Device::CAN_1_ALT1:
            canPeriph = RCU_CAN1;
            gpioPeriph = RCU_GPIOB;
            gpioPort = GPIOB;
            if (true == canBeSet)
            {
                pinRx = GPIO_PIN_5;
                pinTx = GPIO_PIN_6;
                remap = GPIO_CAN1_REMAP;
            }
            break;
    #endif
    #if defined(HAS_CAN2)
        case CAN::Device::CAN_2_DEFAULT:
            gpioPeriph = RCU_GPIOB;
            gpioPort = GPIOB;
            pinRx = GPIO_PIN_10;
            pinTx = GPIO_PIN_11;
            remap = CAN2_DEFAULT_REMAP;
            canBeSet = false;
        case CAN::Device::CAN_2_ALT1:
            if (true == canBeSet)
            {
                pinRx = GPIO_PIN_9;
                pinTx = GPIO_PIN_10;
                canBeSet = false;
                remap = GPIO_CAN2_PARTIAL_REMAP; 
            }
        #if defined(PE0)
        case CAN::Device::CAN_2_ALT2:
            if (true == canBeSet)
            {
                gpioPeriph = RCU_GPIOE;
                gpioPort = GPIOE;
                pinRx = GPIO_PIN_0;
                pinTx = GPIO_PIN_1;
                remap = GPIO_CAN2_FULL_REMAP; 
            }
        #endif
            canPeriph = RCU_CAN2;
            break;
    #endif
        default:
            break;
    }

    if (CAN::State::ENABLE == state)
    {
        /* enable CANx clock */
        rcu_periph_clock_enable(canPeriph);
    #if defined(HAS_CAN1)
        if ((RCU_CAN1 == canPeriph) && (0 == (instances & INSTANCES_MASK_CAN0)))
        {
            rcu_periph_clock_enable(RCU_CAN0);          // For CAN1 You need to enable CAN0 RCU clock even if it was not used.
        }
    #endif
        rcu_periph_clock_enable(gpioPeriph);
        rcu_periph_clock_enable(RCU_AF);
        
        /* configure CANx GPIO */
        gpio_init(gpioPort, GPIO_MODE_IPU,   GPIO_OSPEED_50MHZ, pinRx);
        gpio_init(gpioPort, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, pinTx);
        if (0 != remap)
        {
            gpio_pin_remap_config(remap, (ControlStatus)CAN::State::ENABLE);
        }
    } else
    {
        const uint8_t GPIO_SPEED_INPUT {(uint8_t)0x00U};

        gpio_pin_remap_config(remap, (ControlStatus)CAN::State::DISABLE);
        gpio_init(gpioPort, GPIO_MODE_IN_FLOATING, GPIO_SPEED_INPUT, pinRx);
        gpio_init(gpioPort, GPIO_MODE_IN_FLOATING, GPIO_SPEED_INPUT, pinTx);
        rcu_periph_clock_disable(canPeriph);
    #if defined(HAS_CAN1)
        if ((RCU_CAN1 == canPeriph) && (0 == (instances & INSTANCES_MASK_CAN0)))
        {
            rcu_periph_clock_disable(RCU_CAN0);
        }
    #endif
    }
}

void GD32CAN::nvicConfig(const CAN::State::state_t state)
{
    #if defined(GD32F30X_CL) || defined(GD32E50X_CL) || defined(GD32E508)
    uint8_t nvicTxIrq             {CAN0_TX_IRQn};
    uint8_t nvicRxIrq             {CAN0_RX0_IRQn};
    #else
    uint8_t nvicTxIrq             {USBD_HP_CAN0_TX_IRQn};
    uint8_t nvicRxIrq             {USBD_LP_CAN0_RX0_IRQn};
    #endif
    uint8_t nvicIrqPrePriority  {2}; 
    uint8_t nvicIrqSubPriority  {0};

    #if defined(HAS_CAN1)
    switch (_device)
    {
        case CAN::Device::CAN_1_DEFAULT:
        case CAN::Device::CAN_1_ALT1:
            nvicTxIrq = CAN1_TX_IRQn;
            nvicRxIrq = CAN1_RX0_IRQn;
            nvicIrqSubPriority = 1;
            break;
    #if defined(HAS_CAN2)
        case CAN::Device::CAN_2_DEFAULT:
        case CAN::Device::CAN_2_ALT1:
        #if defined(PE0)
        case CAN::Device::CAN_2_ALT2:
        #endif
            nvicTxIrq = CAN2_TX_IRQn;
            nvicRxIrq = CAN2_RX0_IRQn;
            nvicIrqSubPriority = 2;
            break;
    #endif
        default:
            break;
    }
    #endif

    if (CAN::State::ENABLE == state)
    {
        nvic_irq_enable(nvicRxIrq, nvicIrqPrePriority, nvicIrqSubPriority);
        nvic_irq_enable(nvicTxIrq, nvicIrqPrePriority, nvicIrqSubPriority);
    } else
    {
        nvic_irq_disable(nvicRxIrq);
        nvic_irq_disable(nvicTxIrq);
    }
}

bool GD32CAN::initCanHw(const CAN::Baudrate::baudrate_t baudrate, const CAN::State::state_t autoRetrans)
{
    bool                    result      {false};
    can_parameter_struct    parameters  {};

    if (true == calculateCANBTValues(baudrate, parameters.prescaler, parameters.time_segment_1, parameters.time_segment_2, parameters.resync_jump_width))
    {
        parameters.working_mode = (uint8_t)CAN_NORMAL_MODE;
        parameters.time_triggered = (ControlStatus)DISABLE;
        parameters.rec_fifo_overwrite = (ControlStatus)DISABLE;
        parameters.auto_retrans = (ControlStatus)autoRetrans;
        parameters.trans_fifo_order = (ControlStatus)ENABLE;        // Order with first-in and first-out
        parameters.auto_bus_off_recovery = (ControlStatus)ENABLE;
        parameters.auto_wake_up = (ControlStatus)ENABLE;
        canDeinit();
        if (SUCCESS == can_init(_deviceBase, &parameters))
        {
            setIrqState((IRQ_RX_FIFO_NE | IRQ_TX_MAILBOX_E), CAN::State::ENABLE);
            result = true;
        }
    }

    return result;
}

void GD32CAN::setIrqState(const uint32_t irq, const CAN::State::state_t state)
{
    if (CAN::State::ENABLE == state)
    {
        can_interrupt_enable(_deviceBase, irq);
        if (IRQ_RX_FIFO_NE == (irq & IRQ_RX_FIFO_NE))
        {
            *_rxIrqEnabled = true;
        }
    } else
    {
        can_interrupt_disable(_deviceBase, irq);
        if (IRQ_RX_FIFO_NE == (irq & IRQ_RX_FIFO_NE))
        {
            *_rxIrqEnabled = false;
        }
    }
}

int_fast8_t GD32CAN::getMaxFilterId(void)
{
    int_fast8_t result {-1};

    switch (_device)
    {
        case CAN::Device::CAN_0_DEFAULT:
        case CAN::Device::CAN_0_ALT1:
    #if defined(PD0)
        case CAN::Device::CAN_0_ALT2:
    #endif
            result = (can1StartFilterId >= 0) ? (can1StartFilterId - 1) : (int_fast8_t)CAN::Filter::MAX_BANK;
            break;
    #if defined(HAS_CAN2)
        case CAN::Device::CAN_2_DEFAULT:
        case CAN::Device::CAN_2_ALT1:
        #if defined(PE0)
        case CAN::Device::CAN_2_ALT2:
        #endif
            result = (int_fast8_t)CAN::Filter::MAX_BANK_CAN2;
            break;
    #endif
        default:
            result = (int_fast8_t)CAN::Filter::MAX_BANK;
            break;
    }

    return result;
}

bool GD32CAN::setFilter(const CAN::Filter::Bank::filter_t filterId, const uint32_t frameId,    const uint32_t frameIdMask,
                        const CAN::Filter::Mode::mode_t filterMode, const scale_t filterScale, const CAN::State::state_t state)
{
    bool result {isFilterAvailable(filterId)};

    if (true == result)
    {
        can_filter_parameter_struct filter  {};
        uint32_t                    mask    {(uint32_t)(1 << (uint32_t)filterId)};

        _filtersStates |= mask;
        filter.filter_number = (uint16_t)filterId;
        filter.filter_mode = (uint16_t)filterMode;
        filter.filter_bits = (uint16_t)filterScale;
        filter.filter_fifo_number = CAN_FIFO0;
        filter.filter_enable = (ControlStatus)state;
        filter.filter_list_high = (uint16_t)(frameId >> CAN::Filter::HALFWORD_SHIFT);
        filter.filter_list_low = (uint16_t)(frameId & CAN_FILTER_MASK_16BITS);
        filter.filter_mask_high = (uint16_t)(frameIdMask >> CAN::Filter::HALFWORD_SHIFT);
        filter.filter_mask_low = (uint16_t)(frameIdMask & CAN_FILTER_MASK_16BITS);

        can_filter_init(&filter);
    }

    return result;
}

bool GD32CAN::setFilterState(const CAN::Filter::Bank::filter_t filterId, const CAN::State::state_t state)
{
    bool result {isFilterAvailable(filterId)};

    if (true == result)
    {
        uint32_t device {(uint32_t)CAN::Device::CAN_0_DEFAULT};
        uint32_t mask   {(uint32_t)(1 << (uint32_t)filterId)};

    #if defined(HAS_CAN2)
        switch (_device)
        {
            case CAN::Device::CAN_2_DEFAULT:
            case CAN::Device::CAN_2_ALT1:
        #if defined(PE0)
            case CAN::Device::CAN_2_ALT2:
        #endif
                device = (uint32_t)CAN::Device::CAN_2_DEFAULT;
                break;
            default:
                break;
        }
    #endif

        CAN_FCTL(device) |= CAN_FCTL_FLD;     // filter lock disable

        if (CAN::State::ENABLE == state)
        {
            if (0 != (_filtersStates & mask))
            {
                CAN_FW(device) |= mask;
            }
        } else
        {
            CAN_FW(device) &= ~mask;
        }

        CAN_FCTL(device) &= ~CAN_FCTL_FLD;    // filter lock enable
    }

    return result;
}

inline bool GD32CAN::isFilterAvailable(const CAN::Filter::Bank::filter_t filterId)
{
    return (_isInstanceAllowed && ((int_fast8_t)filterId >= _firstFilterId) && ((int_fast8_t)filterId <= getMaxFilterId()));
}

//*******************************************************************
//*                                                                 *
//*                         Free Functions                          *
//*                                                                 *
//*******************************************************************

//*************************** Ring Buffer ***************************

void initRingBuffer(volatile CAN::ringBuffer_t &ring, volatile CAN::message_t *buffer, uint32_t size)
{
    ring.buffer = buffer;
    ring.size = size;
    ring.head = 0;
    ring.tail = 0;
}

bool isRingBufferEmpty(volatile CAN::ringBuffer_t &ring)
{
    return ((ring.head == ring.tail) ? true : false);
}

bool isRingBufferFull(volatile CAN::ringBuffer_t &ring)
{
    return (((ring.head + 1) % ring.size) == ring.tail);
}

uint32_t ringBufferCount(volatile CAN::ringBuffer_t &ring)
{
    int32_t entries {(int32_t)ring.head - (int32_t)ring.tail};

    return ((entries < 0) ? ((uint32_t)(entries + (uint32_t)ring.size)) : (uint32_t)entries);
}

bool addToRingBuffer(volatile CAN::ringBuffer_t &ring, const CAN::message_t &message)
{
    uint_fast16_t   nextEntry   {(ring.head + 1) % ring.size};
    bool            result      {nextEntry != ring.tail};           // ring buffer is NOT full

    if (true == result)
    {
        memcpy((void *)&ring.buffer[ring.head], (void *)&message, sizeof(CAN::message_t));
        ring.head = nextEntry;
    }

    return result;
}

bool readFromRingBuffer(volatile CAN::ringBuffer_t &ring, CAN::message_t &message)
{
    bool result {true == (not isRingBufferEmpty(ring))};

    if (true == result)
    {
        memcpy((void *)&message, (void *)&ring.buffer[ring.tail], sizeof(CAN::message_t));
    }

    return result;
}

void removeFromRingBuffer(volatile CAN::ringBuffer_t &ring)
{
    if (true == (not isRingBufferEmpty(ring)))
    {
        ring.tail = (ring.tail + 1) % ring.size;
    }
}

//*************************** CAN message ***************************

bool canMessageTransmit(uint32_t canPeriph, const CAN::message_t &message)
{
    can_trasnmit_message_struct tempMessage {};

    ((uint8_t)CAN::Frame::Id::EXTENDED == message.idType)   ? tempMessage.tx_efid = message.id
                                                            : tempMessage.tx_sfid = message.id;
    tempMessage.tx_ff = message.idType;
    tempMessage.tx_ft = message.frameType;
    tempMessage.tx_dlen = (message.dataLen <= CAN::MAX_DLC) ? message.dataLen : CAN::MAX_DLC;
    for (uint_fast8_t idx = 0; idx < CAN::MAX_DLC; idx++)
    {
        tempMessage.tx_data[idx] = message.data[idx];
    }

    return (CAN_NOMAILBOX != can_message_transmit(canPeriph, &tempMessage));
}

void canMessageReceive(uint32_t canPeriph, uint8_t fifo, CAN::message_t *rx_message)
{
    // CAN_RFIFOMI_FF (Bit 2) is 1 for Extended Frame -> goes to idType
    // CAN_RFIFOMI_FT (Bit 1) is 1 for Remote Frame -> goes to frameType
    rx_message->frameType = (uint8_t)(CAN_RFIFOMI_FT & CAN_RFIFOMI(canPeriph, fifo));
    rx_message->idType = (uint8_t)(CAN_RFIFOMI_FF & CAN_RFIFOMI(canPeriph, fifo));
    rx_message->id = (CAN_FF_STANDARD == rx_message->idType) ? (uint32_t)(GET_RFIFOMI_SFID(CAN_RFIFOMI(canPeriph, fifo)))
                                                             : (uint32_t)(GET_RFIFOMI_EFID(CAN_RFIFOMI(canPeriph, fifo)));

    uint8_t dlen {(uint8_t)(GET_RFIFOMP_DLENC(CAN_RFIFOMP(canPeriph, fifo)))};
    rx_message->dataLen = (dlen <= CAN::MAX_DLC) ? dlen : CAN::MAX_DLC;

    union Buffer
    {
        struct U32Data
        {
            uint32_t data3_0;
            uint32_t data7_4;
        }u32data;

        uint8_t u8data[CAN::MAX_DLC];
    };
    #if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
        #error "Unsupported Endianness !!!"
    #endif
    Buffer buffer {};

    buffer.u32data.data3_0 = CAN_RFIFOMDATA0(canPeriph, fifo);
    buffer.u32data.data7_4 = CAN_RFIFOMDATA1(canPeriph, fifo);

    for (uint_fast8_t idx = 0; idx < CAN::MAX_DLC; idx++)
    {
        rx_message->data[idx] = buffer.u8data[idx];
    }

    // release FIFO
    (CAN_FIFO0 == fifo) ? (CAN_RFIFO0(canPeriph) |= CAN_RFIFO0_RFD0)
                        : (CAN_RFIFO1(canPeriph) |= CAN_RFIFO1_RFD1);
}

//*******************************************************************
//*                                                                 *
//*                        CAN IRQ Handlers                         *
//*                                                                 *
//*******************************************************************

inline __attribute__((always_inline)) void commonCanRxIrqHandler(const uint32_t canPeriph, volatile CAN::ringBuffer_t *ring, volatile bool &irqEnabledVar)
{
    if ((nullptr != ring) && (nullptr != ring->buffer))
    {
        if (false == isRingBufferFull(*ring))
        {
            CAN::message_t message;

            canMessageReceive(canPeriph, CAN_FIFO0, &message);
            (void)addToRingBuffer(*ring, message);
        } else
        {
            can_interrupt_disable(canPeriph, CAN_INT_RFNE0);
            irqEnabledVar = false;
        }
    }
}

inline __attribute__((always_inline)) void commonCanTxIrqHandler(const uint32_t canPeriph, volatile CAN::ringBuffer_t *ring)
{
    uint32_t mailboxes {CAN_TSTAT(canPeriph) & (CAN_TSTAT_MTF0 | CAN_TSTAT_MTF1 | CAN_TSTAT_MTF2)};

    if ((nullptr != ring) && (nullptr != ring->buffer))
    {
        CAN::message_t tempMessage;
        
        while (!isRingBufferEmpty(*ring)) 
        {
            // Peek at next message
            if (readFromRingBuffer(*ring, tempMessage))
            {
                // Try to transmit. If successful, remove from ring. 
                // If failed (HW full), break loop immediately to avoid blocking.
                if (true == canMessageTransmit(canPeriph, tempMessage))
                {
                    removeFromRingBuffer(*ring);
                } else 
                {
                    break;                                          // Mailboxes full, exit ISR immediately.
                }
            } else 
            {
                break;                                              // Should not happen given isRingBufferEmpty check, but safety first
            }
        }

        // If the ring buffer is empty after our work, we MUST disable the "Mailbox Empty" interrupt.
        // If we don't, this ISR will fire continuously (because the hardware mailbox is empty),
        // starving the main thread/RTOS and hanging the system.
        if (isRingBufferEmpty(*ring))
        {
            can_interrupt_disable(canPeriph, CAN_INT_TME);
        }
    }

    CAN_TSTAT(canPeriph) = mailboxes;
}

#if defined(HAS_CAN0)
    #if defined(GD32F30X_CL) || defined(GD32E50X_CL) || defined(GD32E508)
extern "C" void CAN0_RX0_IRQHandler(void)
{
    commonCanRxIrqHandler(CAN0, CAN::can0RxRing, CAN::can0rxIrqEnabled);
}

extern "C" void CAN0_TX_IRQHandler(void)
{
    commonCanTxIrqHandler(CAN0, CAN::can0TxRing);
}
    #else
extern "C" void USBD_LP_CAN0_RX0_IRQHandler(void)
{
    commonCanRxIrqHandler(CAN0, CAN::can0RxRing, CAN::can0rxIrqEnabled);
}

extern "C" void USBD_HP_CAN0_TX_IRQHandler(void)
{
    commonCanTxIrqHandler(CAN0, CAN::can0TxRing);
}
    #endif
#endif

#if defined(HAS_CAN1)
extern "C" void CAN1_RX0_IRQHandler(void)
{
    commonCanRxIrqHandler(CAN1, CAN::can1RxRing, CAN::can1rxIrqEnabled);
}

extern "C" void CAN1_TX_IRQHandler(void)
{
    commonCanTxIrqHandler(CAN1, CAN::can1TxRing);
}
#endif

#if defined(HAS_CAN2)
extern "C" void CAN2_RX0_IRQHandler(void)
{
    commonCanRxIrqHandler(CAN2, CAN::can2RxRing, CAN::can2rxIrqEnabled);
}

extern "C" void CAN2_TX_IRQHandler(void)
{
    commonCanTxIrqHandler(CAN2, CAN::can2TxRing);
}
#endif
