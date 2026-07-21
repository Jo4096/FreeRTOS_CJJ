#ifndef SAFE_QUEUES_HPP
#define SAFE_QUEUES_HPP

#include "FreeRTOS_Base.h"
#include RTOS_INC("freertos/queue.h", <queue.h>, <queue.h>)

#if defined(__AVR_ATmega328P__) || defined(ARDUINO_AVR_UNO)
#define xQueueCreateStatic(uxQueueLength, uxItemSize, pucQueueStorageBuffer, pxQueueBuffer) \
    xQueueCreate((uxQueueLength), (uxItemSize))
#endif

namespace fcjj
{
    template <typename T, UBaseType_t QueueLength>
    class SafeQueue
    {
    private:
        uint8_t ucQueueStorageArea[QueueLength * sizeof(T)];
        QueueHandle_t xQueue = nullptr;
        StaticQueue_t xStaticQueue;

    public:
        SafeQueue()
        {
            xQueue = xQueueCreateStatic(QueueLength, sizeof(T), ucQueueStorageArea, &xStaticQueue);
        }

        SafeQueue(const SafeQueue &) = delete;
        SafeQueue &operator=(const SafeQueue &) = delete;
        SafeQueue(SafeQueue &&) = delete;
        SafeQueue &operator=(SafeQueue &&) = delete;

        ~SafeQueue() = default;

        [[nodiscard]] bool send(const T &item, uint32_t ms_timeout = 0xFFFFFFFF)
        {
            if (xQueue == nullptr)
                return false;
            TickType_t ticks = (ms_timeout == 0xFFFFFFFF) ? portMAX_DELAY : pdMS_TO_TICKS(ms_timeout);
            return xQueueSend(xQueue, &item, ticks) == pdPASS;
        }

        [[nodiscard]] bool receive(T &item, uint32_t ms_timeout = 0xFFFFFFFF)
        {
            if (xQueue == nullptr)
                return false;
            TickType_t ticks = (ms_timeout == 0xFFFFFFFF) ? portMAX_DELAY : pdMS_TO_TICKS(ms_timeout);
            return xQueueReceive(xQueue, &item, ticks) == pdPASS;
        }

        [[nodiscard]] bool send_from_isr(const T &item, BaseType_t *pxHigherPriorityTaskWoken = nullptr)
        {
            if (xQueue == nullptr)
                return false;

            BaseType_t xHigherPriorityTaskWokenDummy = pdFALSE;
            BaseType_t *pWoken = (pxHigherPriorityTaskWoken != nullptr) ? pxHigherPriorityTaskWoken : &xHigherPriorityTaskWokenDummy;

            bool success = (xQueueSendFromISR(xQueue, &item, pWoken) == pdPASS);

            if (pxHigherPriorityTaskWoken == nullptr && xHigherPriorityTaskWokenDummy == pdTRUE)
            {
                portYIELD_FROM_ISR(xHigherPriorityTaskWokenDummy);
            }

            return success;
        }

        [[nodiscard]] bool receive_from_isr(T &item, BaseType_t *pxHigherPriorityTaskWoken = nullptr)
        {
            if (xQueue == nullptr)
                return false;

            BaseType_t xHigherPriorityTaskWokenDummy = pdFALSE;
            BaseType_t *pWoken = (pxHigherPriorityTaskWoken != nullptr) ? pxHigherPriorityTaskWoken : &xHigherPriorityTaskWokenDummy;

            bool success = (xQueueReceiveFromISR(xQueue, &item, pWoken) == pdPASS);

            if (pxHigherPriorityTaskWoken == nullptr && xHigherPriorityTaskWokenDummy == pdTRUE)
            {
                portYIELD_FROM_ISR(xHigherPriorityTaskWokenDummy);
            }

            return success;
        }

        [[nodiscard]] UBaseType_t messages_waiting() const noexcept { return uxQueueMessagesWaiting(xQueue); }
        [[nodiscard]] UBaseType_t spaces_available() const noexcept { return uxQueueSpacesAvailable(xQueue); }
        void reset() { xQueueReset(xQueue); }
    };

}
#endif