/**
 * @file        GD32CAN.h
 * @brief       GD32F30x and GD32E50x CAN bus library.
 * Provides a C++ interface for managing CAN peripherals on GD32 microcontrollers.
 *
 * @copyright   SPDX-FileCopyrightText: Copyright 2024-2026 Michal Protasowicki
 * @license     SPDX-License-Identifier: MIT
 */

#pragma once

#include <Arduino.h>

#ifdef __cplusplus
extern "C"
{
#endif
#if defined(GD32F30x)
    #include <gd32f30x_can.h>
    #define HAS_CAN0 1
    #if defined(GD32F30X_CL)
        #define HAS_CAN1 1
    #elif defined(CAN1)
        #undef CAN1
    #else
    #endif
#elif defined(GD32E50X)
    #include <gd32e50x_can.h>
    #define HAS_CAN0 1
    #define HAS_CAN1 1
    #if defined(GD32E50X_CL) || defined(GD32E508)
        #define HAS_CAN2 1
    #else
        #if defined(CAN2)
            #undef CAN2
        #endif
    #endif
#else
    #error "Unsupported mcu!!!"
#endif
#ifdef __cplusplus
}
#endif

/**
 * @namespace CAN
 * @brief Namespace containing all definitions, types, and constants for the CAN library.
 */
namespace CAN
{
    /**
     * @namespace Baudrate
     * @brief Defines supported CAN bus baud rates.
     */
    namespace Baudrate
    {
        /**
         * @enum Baudrate
         * @brief Enumeration of available baud rates in bits per second.
         */
        typedef enum Baudrate : uint32_t
        {
            BD_1M   = 1000000,                                      ///< 1 Mbit/s
            BD_800k = 800000,                                       ///< 800 kbit/s
            BD_750k = 750000,                                       ///< 750 kbit/s
            BD_500k = 500000,                                       ///< 500 kbit/s
            BD_250k = 250000,                                       ///< 250 kbit/s
            BD_200k = 200000,                                       ///< 200 kbit/s
            BD_150k = 150000,                                       ///< 150 kbit/s
            BD_125k = 125000,                                       ///< 125 kbit/s
            BD_100k = 100000,                                       ///< 100 kbit/s
            BD_83k3 = 83333,                                        ///< 83.333 kbit/s
            BD_75k  = 75000,                                        ///< 75 kbit/s
            BD_62k5 = 62500,                                        ///< 62.5 kbit/s
            BD_50k  = 50000,                                        ///< 50 kbit/s
            BD_40k  = 40000,                                        ///< 40 kbit/s
            BD_33k3 = 33333,                                        ///< 33.333 kbit/s
            BD_25k  = 25000,                                        ///< 25 kbit/s
            BD_20k  = 20000,                                        ///< 20 kbit/s
            BD_15k  = 15000,                                        ///< 15 kbit/s
            BD_10k  = 10000,                                        ///< 10 kbit/s
            BD_5k   = 5000,                                         ///< 5 kbit/s
        }baudrate_t;
    }

    /**
     * @namespace WorkingMode
     * @brief Defines the operating modes of the CAN controller.
     */
    namespace WorkingMode
    {
        /**
         * @enum WorkingMode
         * @brief CAN controller working modes.
         */
        typedef enum WorkingMode : uint8_t
        {
            INITIALIZE  = CAN_MODE_INITIALIZE,                      ///< Initialization mode
            NORMAL      = CAN_MODE_NORMAL,                          ///< Normal communication mode
            SLEEP       = CAN_MODE_SLEEP,                           ///< Sleep mode (low power)
        }mode_t;
    }

    /**
     * @namespace Error
     * @brief Defines CAN bus error states and types.
     */
    namespace Error
    {
        static const uint_fast8_t BUS_ERROR_MASK    {0b00011000};
        static const uint_fast8_t DATA_ERROR_MASK   {0b00000111};

        /**
         * @enum Error
         * @brief Enumeration of CAN errors.
         */
        typedef enum Error
        {
            NONE            = CAN_ERROR_NONE,                       ///< No error
            FILL            = CAN_ERROR_FILL,                       ///< Stuff error
            FORMAT          = CAN_ERROR_FORMATE,                    ///< Format error
            ACK_ERR         = CAN_ERROR_ACK,                        ///< Acknowledge error
            BITRECESSIVE    = CAN_ERROR_BITRECESSIVE,               ///< Bit recessive error
            BITDOMINANT     = CAN_ERROR_BITDOMINANTER,              ///< Bit dominant error
            CRC_ERR         = CAN_ERROR_CRC,                        ///< CRC error
            SOFTWARECFG     = CAN_ERROR_SOFTWARECFG,                ///< Software configuration error
            WARNING         = (1 << 3),                             ///< Error warning state
            PASSIVE         = (2 << 3),                             ///< Error passive state
            BUS_OFF         = (3 << 3),                             ///< Bus-off state
        }error_t;
    }

    /**
     * @namespace Device
     * @brief Defines the CAN peripheral instances and pin mappings.
     */
    namespace Device
    {
        /**
         * @enum Device
         * @brief Enumeration of supported CAN device configurations (Instance + Pin Remap).
         */
        typedef enum Device : uint32_t
        {
        #ifdef HAS_CAN0
            CAN_0_DEFAULT   = (uint32_t)CAN0,                       ///< CAN0 on PA11/PA12 (Default)
            CAN_0_ALT1,                                             ///< CAN0 on PB8/PB9 (Remap 1)
            #if defined(PD0)
            CAN_0_ALT2,                                             ///< CAN0 on PD0/PD1 (Remap 2)
            #endif
        #endif
        #ifdef HAS_CAN1
            CAN_1_DEFAULT   = (uint32_t)CAN1,                       ///< CAN1 on PB12/PB13 (Default)
            CAN_1_ALT1,                                             ///< CAN1 on PB5/PB6 (Remap 1)
        #endif
        #ifdef HAS_CAN2
            CAN_2_DEFAULT   = (uint32_t)CAN2,                       ///< CAN2 on PB10/PB11 (Default)
            CAN_2_ALT1,                                             ///< CAN2 on PA9/PA10 (Remap 1)
            #if defined(PE0)
            CAN_2_ALT2,                                             ///< CAN2 on PE0/PE1 (Remap 2)
            #endif
        #endif
        }device_t;
    }

    /**
     * @namespace Frame
     * @brief Defines frame-related constants and types.
     */
    namespace Frame
    {
        static const uint32_t SFID_MASK {0x000007FFU};              ///< Mask for Standard Identifier (11 bits)
        static const uint32_t EFID_MASK {0x1FFFFFFFU};              ///< Mask for Extended Identifier (29 bits)

        /**
         * @namespace Id
         * @brief Identifier types (Standard vs Extended).
         */
        namespace Id
        {
            typedef enum IDType
            {
                STANDARD    = CAN_FF_STANDARD,                      ///< Standard Identifier
                EXTENDED    = CAN_FF_EXTENDED,                      ///< Extended Identifier
            }idType_t;
        }

        /**
         * @namespace Type
         * @brief Frame types (Data vs Remote).
         */
        namespace Type
        {
            typedef enum FrameType
            {
                DATA        = CAN_FT_DATA,                          ///< Data Frame
                RTR         = CAN_FT_REMOTE,                        ///< Remote Transmission Request Frame
            }frameType_t;
        }
    }

    /**
     * @namespace Filter
     * @brief Defines constants and types for CAN hardware filters.
     */
    namespace Filter
    {
        static const uint_fast8_t   MIN_BANK        {0};
        #if defined (GD32F30X_CL) || defined(GD32E50X_CL) || defined(GD32E508)
        static const uint_fast8_t   MAX_BANK        {27};
        static const uint_fast8_t   MAX_BANK_CAN2   {14};
        #else
        static const uint_fast8_t   MAX_BANK        {13};
        static const uint_fast8_t   MAX_BANK_CAN2   {0};
        #endif

        /**
         * @namespace Bank
         * @brief Filter bank identifiers.
         */
        namespace Bank
        {
            typedef enum Filter : uint_fast8_t
            {
                FLT00 = 0,
                FLT01 = 1,
                FLT02 = 2,
                FLT03 = 3,
                FLT04 = 4,
                FLT05 = 5,
                FLT06 = 6,
                FLT07 = 7,
                FLT08 = 8,
                FLT09 = 9,
                FLT10 = 10,
                FLT11 = 11,
                FLT12 = 12,
                FLT13 = 13,
            #if defined (GD32F30X_CL) || defined(GD32E50X_CL) || defined(GD32E508)
                FLT14 = 14,
                FLT15 = 15,
                FLT16 = 16,
                FLT17 = 17,
                FLT18 = 18,
                FLT19 = 19,
                FLT20 = 20,
                FLT21 = 21,
                FLT22 = 22,
                FLT23 = 23,
                FLT24 = 24,
                FLT25 = 25,
                FLT26 = 26,
                FLT27 = 27,
            #endif
            }filter_t;
        }

        /**
         * @namespace Mode
         * @brief Filter modes (Mask vs List).
         */
        namespace Mode
        {
            typedef enum Mode
            {
                MASK            = CAN_FILTERMODE_MASK,              ///< Identifier Mask mode
                LIST            = CAN_FILTERMODE_LIST               ///< Identifier List mode
            }mode_t;
        }

        /**
         * @namespace FrameType
         * @brief Filter configuration for frame types.
         */
        namespace FrameType
        {
            typedef enum FilteredFramesType
            {
                DATA        = CAN_FT_DATA,                          ///< Match Data frames only
                RTR         = CAN_FT_REMOTE,                        ///< Match RTR frames only
                ANY,                                                ///< Match both Data and RTR frames
            }filteredFrames_t;
        }

        /**
         * @namespace Id
         * @brief Filter configuration for ID types.
         */
        namespace Id
        {
            typedef enum IDType
            {
                STANDARD    = CAN_FF_STANDARD,                      ///< Match Standard IDs
                EXTENDED    = CAN_FF_EXTENDED,                      ///< Match Extended IDs
                ALL,                                                ///< Match both types
            }idType_t;
        }
    }

    /**
     * @namespace State
     * @brief General Enable/Disable state.
     */
    namespace State
    {
        typedef enum State : uint_fast8_t
        {
            DISABLE = 0,
            ENABLE  = 1,
        }state_t;
    }

    /**
     * @namespace QueueSize
     * @brief Buffer size definitions for RX/TX rings.
     */
    namespace QueueSize
    {
        typedef enum Size : uint_fast16_t
        {
            Q_4     = 4,
            Q_8     = 8,
            Q_16    = 16,
            Q_32    = 32,
            Q_64    = 64,
            Q_128   = 128,
            Q_256   = 256,
            Q_512   = 512,
        }size_t;
    }

    static const uint_fast8_t       MAX_DLC     {8};                ///< Maximum Data Length Code (8 bytes for Classic CAN)

    /**
     * @struct Message
     * @brief Structure representing a CAN message.
     */
    typedef struct Message
    {
        uint32_t    id              {0};                            ///< Message Identifier
        uint8_t     idType          {0};                            ///< ID Type (Standard/Extended)
        uint8_t     frameType       {0};                            ///< Frame Type (Data/RTR)
        uint8_t     dataLen         {0};                            ///< Data Length (0-8)
        uint8_t     data[MAX_DLC]   {0};                            ///< Data payload
    }__attribute__((packed)) message_t;

    /**
     * @struct RingBuffer
     * @brief Circular buffer structure for buffering messages.
     */
    typedef struct RingBuffer
    {
        volatile uint_fast16_t      head;                           ///< Write index
        volatile uint_fast16_t      tail;                           ///< Read index
        uint_fast16_t               size;                           ///< Buffer capacity
        volatile CAN::message_t    *buffer;                         ///< Pointer to message array
    }__attribute__((packed)) ringBuffer_t;
}

/**
 * @class GD32CAN
 * @brief Main class to control GD32 CAN peripheral.
 *
 * This class handles initialization, configuration (baudrate, filters),
 * and transmission/reception of CAN messages using ring buffers and interrupts.
 */
class GD32CAN
{
    public:
        GD32CAN(void) = delete;

        /**
         * @brief Construct a new GD32CAN object.
         * Defaults to 64-message RX queue and 0 TX queue (direct send only).
         * @param device The CAN peripheral device instance to use.
         */
        GD32CAN(CAN::Device::device_t device);

        /**
         * @brief Construct a new GD32CAN object with specified RX queue size.
         * @param device The CAN peripheral device instance.
         * @param rxQueueSize Size of the receive ring buffer.
         */
        GD32CAN(CAN::Device::device_t device, CAN::QueueSize::size_t rxQueueSize);

        /**
         * @brief Construct a new GD32CAN object with specified RX and TX queue sizes.
         * @param device The CAN peripheral device instance.
         * @param rxQueueSize Size of the receive ring buffer.
         * @param txQueueSize Size of the transmit ring buffer.
         */
        GD32CAN(CAN::Device::device_t device, CAN::QueueSize::size_t rxQueueSize, CAN::QueueSize::size_t txQueueSize);
        
        /**
         * @brief Destroy the GD32CAN object, freeing resources and disabling interrupts.
         */
        ~GD32CAN(void);

        /**
         * @brief Initializes the CAN peripheral and starts the clock.
         * @param baudrate The desired baud rate.
         * @return true If initialization was successful.
         * @return false If initialization failed.
         */
        bool begin(const CAN::Baudrate::baudrate_t baudrate);

        /**
         * @brief Checks if the library is successfully initialized.
         * @return true if initialized, false otherwise.
         */
        bool isInitialized(void);

        /**
         * @brief Configures a pin to control an external transceiver's sleep mode.
         * @param pin The GPIO pin number connected to the transceiver's silent/sleep pin.
         * @return true on success, false on failure.
         */
        bool attachTransceiverSleepPin(const pin_size_t pin);

        /**
         * @brief Sets the external transceiver mode.
         * @param state ENABLE to wake up (active), DISABLE to sleep.
         * @return true on success, false on failure.
         */
        bool setTransceiverMode(const CAN::State::state_t state);

        /**
         * @brief Sets the internal working mode of the CAN controller.
         * @param mode NORMAL, LOOPBACK, SILENT, or SILENT_LOOPBACK (mapped in enum).
         * @return true on success, false on failure.
         */
        bool setWorkingMode(const CAN::WorkingMode::mode_t mode);

        /**
         * @brief Changes the baud rate of the CAN bus.
         * Calculates time quanta and segments dynamically.
         * @param baudrate The new baud rate.
         * @return true on success, false on failure.
         */
        bool setBaudrate(const CAN::Baudrate::baudrate_t baudrate);

        /**
         * @brief Clears and disables all hardware filters.
         * @return true on success, false on failure.
         */
        bool clearAllFilters(void);

        /**
         * @brief Disables a specific filter bank.
         * @param filterId The filter bank index.
         * @return true on success, false on failure.
         */
        bool disableFilter(const CAN::Filter::Bank::filter_t filterId);

        /**
         * @brief Enables a specific filter bank.
         * @param filterId The filter bank index.
         * @return true on success, false on failure.
         */
        bool enableFilter(const CAN::Filter::Bank::filter_t filterId);

        /**
         * @brief Configures filters to accept all messages of a given ID type.
         * @param idType STANDARD, EXTENDED, or ALL.
         * @return true on success, false on failure.
         */
        bool allowReceiveAllMessages(CAN::Filter::Id::idType_t idType);

        // --- Extended Frame Filters ---

        /**
         * @brief Sets a 32-bit Mask mode filter for Extended IDs.
         * @param filterId Filter bank index.
         * @param frameId The ID to match.
         * @param filteredFrames Frame type to match (DATA, RTR, ANY).
         * @return true on success.
         */
        bool setFilter(const CAN::Filter::Bank::filter_t filterId, const uint32_t frameId,
                       const CAN::Filter::FrameType::filteredFrames_t filteredFrames);
        
        /**
         * @brief Sets a 32-bit List mode filter for two specific Extended IDs.
         * @param filterId Filter bank index.
         * @param frameId_1 First ID to match.
         * @param frameType_1 Frame type for first ID.
         * @param frameId_2 Second ID to match.
         * @param frameType_2 Frame type for second ID.
         * @return true on success.
         */
        bool setFilter(const CAN::Filter::Bank::filter_t filterId, const uint32_t frameId_1, const CAN::Frame::Type::frameType_t frameType_1,
                                                                   const uint32_t frameId_2, const CAN::Frame::Type::frameType_t frameType_2);
        
        /**
         * @brief Sets a 32-bit Mask mode filter with a custom mask for Extended IDs.
         * @param filterId Filter bank index.
         * @param frameId The ID pattern.
         * @param frameIdMask The mask (1 = bit must match, 0 = don't care).
         * @param filteredFrames Frame type to match.
         * @return true on success.
         */
        bool setFilter(const CAN::Filter::Bank::filter_t filterId, const uint32_t frameId,
                       const uint32_t frameIdMask,
                       const CAN::Filter::FrameType::filteredFrames_t filteredFrames);
        
        // --- Standard Frame Filters ---

        /**
         * @brief Sets a 16-bit Mask mode filter for Standard IDs (sets 2 filters essentially).
         * @param filterId Filter bank index.
         * @param frameId The ID to match.
         * @param filteredFrames Frame type to match.
         * @return true on success.
         */
        bool setFilter(const CAN::Filter::Bank::filter_t filterId, const uint16_t frameId,
                       const CAN::Filter::FrameType::filteredFrames_t filteredFrames);
        
        /**
         * @brief Sets a 16-bit Mask mode filter for two different Standard IDs.
         * @param filterId Filter bank index.
         * @param frameId_1 First ID.
         * @param filteredFrames_1 Frame type for first ID.
         * @param frameId_2 Second ID.
         * @param filteredFrames_2 Frame type for second ID.
         * @return true on success.
         */
        bool setFilter(const CAN::Filter::Bank::filter_t filterId, const uint16_t frameId_1, const CAN::Filter::FrameType::filteredFrames_t filteredFrames_1,
                                                                   const uint16_t frameId_2, const CAN::Filter::FrameType::filteredFrames_t filteredFrames_2);
        
        /**
         * @brief Sets a 16-bit List mode filter for four specific Standard IDs.
         * @param filterId Filter bank index.
         * @param frameId_1 ID 1.
         * @param frameType_1 Type 1.
         * @param frameId_2 ID 2.
         * @param frameType_2 Type 2.
         * @param frameId_3 ID 3.
         * @param frameType_3 Type 3.
         * @param frameId_4 ID 4.
         * @param frameType_4 Type 4.
         * @return true on success.
         */
        bool setFilter(const CAN::Filter::Bank::filter_t filterId, const uint16_t frameId_1, const CAN::Frame::Type::frameType_t frameType_1,
                                                                   const uint16_t frameId_2, const CAN::Frame::Type::frameType_t frameType_2,
                                                                   const uint16_t frameId_3, const CAN::Frame::Type::frameType_t frameType_3,
                                                                   const uint16_t frameId_4, const CAN::Frame::Type::frameType_t frameType_4);
        
        /**
         * @brief Sets a 16-bit Mask mode filter with custom mask for Standard IDs.
         * @param filterId Filter bank index.
         * @param frameId ID pattern.
         * @param frameIdMask ID mask.
         * @param filteredFrames Frame type.
         * @return true on success.
         */
        bool setFilter(const CAN::Filter::Bank::filter_t filterId, const uint16_t frameId,
                       const uint16_t frameIdMask,
                       const CAN::Filter::FrameType::filteredFrames_t filteredFrames);
        
        /**
         * @brief Sets a 16-bit Mask mode filter with two custom masks for Standard IDs.
         * @param filterId Filter bank index.
         * @param frameId_1 ID 1.
         * @param frameIdMask_1 Mask 1.
         * @param filteredFrames_1 Frame type 1.
         * @param frameId_2 ID 2.
         * @param frameIdMask_2 Mask 2.
         * @param filteredFrames_2 Frame type 2.
         * @return true on success.
         */
        bool setFilter(const CAN::Filter::Bank::filter_t filterId, const uint16_t frameId_1, const uint16_t frameIdMask_1, const CAN::Filter::FrameType::filteredFrames_t filteredFrames_1,
                                                                   const uint16_t frameId_2, const uint16_t frameIdMask_2, const CAN::Filter::FrameType::filteredFrames_t filteredFrames_2);
        
        /**
         * @brief Queues a message for transmission.
         * If TX hardware is busy, adds to ring buffer if available.
         * @param message The message to send.
         * @return true if sent or queued, false if buffer full or error.
         */
        bool write(CAN::message_t &message);

        /**
         * @brief Returns the number of slots available in the TX buffer.
         * @return Number of messages that can be written without blocking/failing.
         */
        uint32_t availableForWrite(void);

        /**
         * @brief Returns the number of messages available in the RX buffer.
         * @return Number of received messages waiting to be read.
         */
        uint32_t available(void);

        /**
         * @brief Peeks at the next message in the RX buffer without removing it.
         * @param message Reference to store the peeked message.
         * @return true if a message was available, false otherwise.
         */
        bool peek(CAN::message_t &message);

        /**
         * @brief Reads and removes the next message from the RX buffer.
         * @param message Reference to store the read message.
         * @return true if a message was read, false if buffer was empty.
         */
        bool read(CAN::message_t &message);

        /**
         * @brief Gets the current error state of the CAN controller.
         * @return Error bitmask (see CAN::Error::error_t).
         */
        uint_fast8_t getError(void);

        #if defined(HAS_CAN1)
        /**
         * @brief Configures the split point between CAN0 and CAN1 filters.
         * @param filterId The first filter bank assigned to CAN1.
         * @return true on success.
         */
        bool setCan1StartFilterId(CAN::Filter::Bank::filter_t filterId);
        #endif

    protected:
        static const uint32_t       CAN_DEVICE_MASK                 {0xFFFFFF00U};
        static const uint32_t       HW_MAX_BAUDRATE                 {1000000U};
        #if defined(GD32E508)
        // BS1[6:0] + BS2[4:0]
        static const uint32_t       CAN_BT_SEG_MAX                  {158U};
        static const uint32_t       CAN_BT_SEG_MIN                  {3U};
        // BS1[3:0] + BS2[2:0]
        static const uint32_t       CAN_FD_BT_SEG_MAX               {22U};
        static const uint32_t       CAN_FD_BT_SEG_MIN               {3U};
        // CAN related register mask
        static const uint32_t       CAN_BS1_MASK                    {0x000F0000U};
        static const uint32_t       CAN_BS2_MASK                    {0x00700000U};
        static const uint32_t       CAN_FD_SJW_MASK                 {0x07000000U};
        #else
        // BS1[3:0] + BS2[2:0]
        static const uint32_t       CAN_BT_SEG_MAX                  {22U};
        static const uint32_t       CAN_BT_SEG_MIN                  {3U};
        #endif
        // CAN related register mask
        static const uint32_t       CAN_BAUDPSC_MASK                {0x000003FFU};
        static const uint32_t       CAN_SJW_MASK                    {0x1F000000U};

        static const int_fast8_t    DEFAULT_CAN1_START_FILTER_ID    {14};
        static const uint_fast8_t   INSTANCES_MASK_CAN0             {0x01};
        static const uint_fast8_t   INSTANCES_MASK_CAN1             {0x02};
        static const uint_fast8_t   INSTANCES_MASK_CAN2             {0x04};
        static const pin_size_t     NO_PIN                          {0xFF};

        typedef enum Scale : uint16_t
        {
            FILTERBITS_16   = CAN_FILTERBITS_16BIT,
            FILTERBITS_32   = CAN_FILTERBITS_32BIT,
        }scale_t;

        typedef enum IRQ : uint32_t
        {
            IRQ_RX_FIFO_NE      = CAN_INT_RFNE0,
            IRQ_TX_MAILBOX_E    = CAN_INT_TME,
        }irq_t;

        static uint_fast8_t         instances;
        static int_fast8_t          can1StartFilterId;

        CAN::Device::device_t       _device                         {static_cast<CAN::Device::device_t>(0)};
        uint32_t                    _deviceBase                     {0x00000000U};
        uint_fast8_t                _instance_mask                  {0x00};
        bool                        _isInstanceAllowed              {false};
        bool                        _isInitialized                  {false};
        uint32_t                    _filtersStates                  {0x00000000U};
        int_fast8_t                 _firstFilterId                  {CAN::Filter::MIN_BANK};
        pin_size_t                  _transceiverSleepPin            {NO_PIN};
        volatile CAN::message_t    *_rxBuffer                       {nullptr};
        volatile CAN::ringBuffer_t  _rxRing                         {};
        volatile bool              *_rxIrqEnabled                   {nullptr};
        volatile CAN::message_t    *_txBuffer                       {nullptr};
        volatile CAN::ringBuffer_t  _txRing                         {};

        void init(const CAN::Device::device_t device, CAN::QueueSize::size_t rxQueueSize, CAN::QueueSize::size_t txQueueSize);
        inline void detachTransceiverSleepPin(void);
        void canDeinit(void);
        bool canConfig(const CAN::Baudrate::baudrate_t baudrate);
        inline uint32_t calculateCANBTRegValue(CAN::Baudrate::baudrate_t baudrate);
        bool calculateCANBTValues(const CAN::Baudrate::baudrate_t baudrate, uint16_t &baudpsc, uint8_t &bs1, uint8_t &bs2, uint8_t &sjw);
        void gpioConfig(const CAN::State::state_t state);
        void nvicConfig(const CAN::State::state_t state);
        bool initCanHw(const CAN::Baudrate::baudrate_t baudrate, CAN::State::state_t autoRetrans);
        void setIrqState(const uint32_t irq, const CAN::State::state_t state);
        int_fast8_t getMaxFilterId(void);
        bool setFilter( const CAN::Filter::Bank::filter_t filterId, const uint32_t frameId,    const uint32_t frameIdMask,
                        const CAN::Filter::Mode::mode_t filterMode, const scale_t filterScale, const CAN::State::state_t state);
        inline bool isFilterAvailable(const CAN::Filter::Bank::filter_t filterId) __attribute__((always_inline));
        bool setFilterState(const CAN::Filter::Bank::filter_t filterId, const CAN::State::state_t state);
};
