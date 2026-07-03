#include "TaskNotification.hpp"

bool TaskNotification::notify(const IThread &target_thread)
{
    TaskHandle_t handle = target_thread.Handle();
    if (handle == nullptr)
        return false;
    return xTaskNotifyGive(handle) == pdPASS;
}

bool TaskNotification::notify_from_isr(const IThread &target_thread, BaseType_t *pxHigherPriorityTaskWoken)
{
    TaskHandle_t handle = target_thread.Handle();
    if (handle == nullptr)
        return false;

    BaseType_t xHigherPriorityTaskWokenDummy = pdFALSE;
    BaseType_t *pWoken = (pxHigherPriorityTaskWoken != nullptr) ? pxHigherPriorityTaskWoken : &xHigherPriorityTaskWokenDummy;

    vTaskNotifyGiveFromISR(handle, pWoken);

    if (pxHigherPriorityTaskWoken == nullptr && xHigherPriorityTaskWokenDummy == pdTRUE)
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWokenDummy);
    }
    return true;
}

uint32_t TaskNotification::wait(bool clear_on_exit, uint32_t timeout_ms)
{
    TickType_t ticks = (timeout_ms == 0xFFFFFFFF) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return ulTaskNotifyTake(clear_on_exit ? pdTRUE : pdFALSE, ticks);
}
