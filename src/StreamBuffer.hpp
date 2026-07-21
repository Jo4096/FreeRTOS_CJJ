#ifndef STREAM_BUFFER_HPP
#define STREAM_BUFFER_HPP

#include "FreeRTOS_Base.h"
#include RTOS_INC("freertos/stream_buffer.h", <stream_buffer.h>, <stream_buffer.h>)

#if defined(__AVR_ATmega328P__) || defined(ARDUINO_AVR_UNO)
#ifdef xStreamBufferCreate
#ifndef xStreamBufferCreateStatic
#define xStreamBufferCreateStatic(xBufferSizeBytes, xTriggerLevelBytes, pucStreamBufferStorageArea, pxStaticStreamBuffer) \
    xStreamBufferCreate((xBufferSizeBytes), (xTriggerLevelBytes))
#endif
#ifndef xMessageBufferCreateStatic
#define xMessageBufferCreateStatic(xBufferSizeBytes, pucMessageBufferStorageArea, pxStaticMessageBuffer) \
    xMessageBufferCreate((xBufferSizeBytes))
#endif
#endif
#endif

namespace fcjj
{
    template <size_t BufferSize, size_t TriggerLevel = 1>
    class StreamBuffer
    {
        static_assert(BufferSize > 0, "BufferSize should be > 0.");
        static_assert(TriggerLevel > 0, "TriggerLevel should be >= 1.");
        static_assert(TriggerLevel <= BufferSize, "TriggerLevel can't be larger than BufferSize.");

    private:
        uint8_t ucStorage[BufferSize + 1];
        StaticStreamBuffer_t xBuffer;
        StreamBufferHandle_t xHandle = nullptr;

    public:
        StreamBuffer()
        {
            xHandle = xStreamBufferCreateStatic(BufferSize, TriggerLevel, ucStorage, &xBuffer);
        }

        StreamBuffer(const StreamBuffer &) = delete;
        StreamBuffer &operator=(const StreamBuffer &) = delete;
        StreamBuffer(StreamBuffer &&) = delete;
        StreamBuffer &operator=(StreamBuffer &&) = delete;

        ~StreamBuffer()
        {
            if (xHandle != nullptr)
            {
                vStreamBufferDelete(xHandle);
                xHandle = nullptr;
            }
        }

        [[nodiscard]] size_t send(const void *data, size_t len, uint32_t timeout_ms = 0xFFFFFFFF)
        {
            if (xHandle == nullptr || data == nullptr || len == 0)
                return 0;
            TickType_t ticks = (timeout_ms == 0xFFFFFFFF) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
            return xStreamBufferSend(xHandle, data, len, ticks);
        }

        [[nodiscard]] size_t send_from_isr(const void *data, size_t len, BaseType_t *pxHigherPriorityTaskWoken = nullptr)
        {
            if (xHandle == nullptr || data == nullptr || len == 0)
                return 0;

            BaseType_t dummy = pdFALSE;
            BaseType_t *pWoken = (pxHigherPriorityTaskWoken != nullptr) ? pxHigherPriorityTaskWoken : &dummy;

            size_t sent = xStreamBufferSendFromISR(xHandle, data, len, pWoken);

            if (pxHigherPriorityTaskWoken == nullptr && dummy == pdTRUE)
            {
                portYIELD_FROM_ISR(dummy);
            }
            return sent;
        }

        [[nodiscard]] size_t receive(void *buffer, size_t len, uint32_t timeout_ms = 0xFFFFFFFF)
        {
            if (xHandle == nullptr || buffer == nullptr || len == 0)
                return 0;
            TickType_t ticks = (timeout_ms == 0xFFFFFFFF) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
            return xStreamBufferReceive(xHandle, buffer, len, ticks);
        }

        [[nodiscard]] size_t receive_from_isr(void *buffer, size_t len, BaseType_t *pxHigherPriorityTaskWoken = nullptr)
        {
            if (xHandle == nullptr || buffer == nullptr || len == 0)
                return 0;

            BaseType_t dummy = pdFALSE;
            BaseType_t *pWoken = (pxHigherPriorityTaskWoken != nullptr) ? pxHigherPriorityTaskWoken : &dummy;

            size_t recvd = xStreamBufferReceiveFromISR(xHandle, buffer, len, pWoken);

            if (pxHigherPriorityTaskWoken == nullptr && dummy == pdTRUE)
            {
                portYIELD_FROM_ISR(dummy);
            }
            return recvd;
        }

        [[nodiscard]] size_t bytes_available() const { return xStreamBufferBytesAvailable(xHandle); }
        [[nodiscard]] size_t spaces_available() const { return xStreamBufferSpacesAvailable(xHandle); }
        [[nodiscard]] bool is_empty() const { return xStreamBufferIsEmpty(xHandle) == pdTRUE; }
        [[nodiscard]] bool is_full() const { return xStreamBufferIsFull(xHandle) == pdTRUE; }

        bool reset() { return xStreamBufferReset(xHandle) == pdPASS; }

        [[nodiscard]] StreamBufferHandle_t get_handle() const noexcept { return xHandle; }
    };

    template <size_t BufferSize>
    class MessageBuffer
    {
        static_assert(BufferSize > sizeof(size_t), "BufferSize should be greater than sizeof(size_t).");

    private:
        uint8_t ucStorage[BufferSize + 1];
        StaticMessageBuffer_t xBuffer;
        MessageBufferHandle_t xHandle = nullptr;

    public:
        MessageBuffer()
        {
            xHandle = xMessageBufferCreateStatic(BufferSize, ucStorage, &xBuffer);
        }

        MessageBuffer(const MessageBuffer &) = delete;
        MessageBuffer &operator=(const MessageBuffer &) = delete;
        MessageBuffer(MessageBuffer &&) = delete;
        MessageBuffer &operator=(MessageBuffer &&) = delete;

        ~MessageBuffer()
        {
            if (xHandle != nullptr)
            {
                vMessageBufferDelete(xHandle);
                xHandle = nullptr;
            }
        }

        [[nodiscard]] size_t send(const void *data, size_t len, uint32_t timeout_ms = 0xFFFFFFFF)
        {
            if (xHandle == nullptr || data == nullptr || len == 0)
                return 0;
            TickType_t ticks = (timeout_ms == 0xFFFFFFFF) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
            return xMessageBufferSend(xHandle, data, len, ticks);
        }

        template <typename T>
        [[nodiscard]] size_t send_value(const T &value, uint32_t timeout_ms = 0xFFFFFFFF)
        {
            return send(&value, sizeof(T), timeout_ms);
        }

        [[nodiscard]] size_t send_from_isr(const void *data, size_t len, BaseType_t *pxHigherPriorityTaskWoken = nullptr)
        {
            if (xHandle == nullptr || data == nullptr || len == 0)
                return 0;

            BaseType_t dummy = pdFALSE;
            BaseType_t *pWoken = (pxHigherPriorityTaskWoken != nullptr) ? pxHigherPriorityTaskWoken : &dummy;

            size_t sent = xMessageBufferSendFromISR(xHandle, data, len, pWoken);

            if (pxHigherPriorityTaskWoken == nullptr && dummy == pdTRUE)
            {
                portYIELD_FROM_ISR(dummy);
            }
            return sent;
        }

        [[nodiscard]] size_t receive(void *buffer, size_t max_len, uint32_t timeout_ms = 0xFFFFFFFF)
        {
            if (xHandle == nullptr || buffer == nullptr || max_len == 0)
                return 0;
            TickType_t ticks = (timeout_ms == 0xFFFFFFFF) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
            return xMessageBufferReceive(xHandle, buffer, max_len, ticks);
        }

        template <typename T>
        [[nodiscard]] bool receive_value(T &value, uint32_t timeout_ms = 0xFFFFFFFF)
        {
            return receive(&value, sizeof(T), timeout_ms) == sizeof(T);
        }

        [[nodiscard]] size_t receive_from_isr(void *buffer, size_t max_len, BaseType_t *pxHigherPriorityTaskWoken = nullptr)
        {
            if (xHandle == nullptr || buffer == nullptr || max_len == 0)
                return 0;

            BaseType_t dummy = pdFALSE;
            BaseType_t *pWoken = (pxHigherPriorityTaskWoken != nullptr) ? pxHigherPriorityTaskWoken : &dummy;

            size_t recvd = xMessageBufferReceiveFromISR(xHandle, buffer, max_len, pWoken);

            if (pxHigherPriorityTaskWoken == nullptr && dummy == pdTRUE)
            {
                portYIELD_FROM_ISR(dummy);
            }
            return recvd;
        }

        [[nodiscard]] size_t spaces_available() const { return xMessageBufferSpacesAvailable(xHandle); }
        [[nodiscard]] bool is_empty() const { return xMessageBufferIsEmpty(xHandle) == pdTRUE; }
        [[nodiscard]] bool is_full() const { return xMessageBufferIsFull(xHandle) == pdTRUE; }

        bool reset() { return xMessageBufferReset(xHandle) == pdPASS; }

        [[nodiscard]] MessageBufferHandle_t get_handle() const noexcept { return xHandle; }
    };
}
#endif