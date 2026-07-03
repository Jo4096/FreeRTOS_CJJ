#include "Semaphores.hpp"

BinarySemaphore::BinarySemaphore()
{
    _xSemaphoreHandle = xSemaphoreCreateBinaryStatic(&_xSemaphoreBuffer);
}

BinarySemaphore::~BinarySemaphore()
{
    if (_xSemaphoreHandle != nullptr)
    {
        vSemaphoreDelete(_xSemaphoreHandle);
    }
}

bool BinarySemaphore::take(uint32_t timeout_ms)
{
    if (_xSemaphoreHandle == nullptr)
        return false;
    TickType_t ticks = (timeout_ms == 0xFFFFFFFF) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTake(_xSemaphoreHandle, ticks) == pdTRUE;
}

bool BinarySemaphore::give()
{
    if (_xSemaphoreHandle == nullptr)
        return false;
    return xSemaphoreGive(_xSemaphoreHandle) == pdTRUE;
}

bool BinarySemaphore::give_from_isr(BaseType_t *pxHigherPriorityTaskWoken)
{
    if (_xSemaphoreHandle == nullptr)
        return false;

    BaseType_t xHigherPriorityTaskWokenDummy = pdFALSE;
    BaseType_t *pWoken = (pxHigherPriorityTaskWoken != nullptr) ? pxHigherPriorityTaskWoken : &xHigherPriorityTaskWokenDummy;

    bool success = (xSemaphoreGiveFromISR(_xSemaphoreHandle, pWoken) == pdTRUE);

    if (pxHigherPriorityTaskWoken == nullptr && xHigherPriorityTaskWokenDummy == pdTRUE)
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWokenDummy);
    }

    return success;
}

CountingSemaphore::CountingSemaphore(UBaseType_t max_count, UBaseType_t initial_count)
{
    _xSemaphoreHandle = xSemaphoreCreateCountingStatic(max_count, initial_count, &_xSemaphoreBuffer);
}

CountingSemaphore::~CountingSemaphore()
{
    if (_xSemaphoreHandle != nullptr)
    {
        vSemaphoreDelete(_xSemaphoreHandle);
    }
}

bool CountingSemaphore::take(uint32_t timeout_ms)
{
    if (_xSemaphoreHandle == nullptr)
        return false;
    TickType_t ticks = (timeout_ms == 0xFFFFFFFF) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTake(_xSemaphoreHandle, ticks) == pdTRUE;
}

bool CountingSemaphore::give()
{
    if (_xSemaphoreHandle == nullptr)
        return false;
    return xSemaphoreGive(_xSemaphoreHandle) == pdTRUE;
}

bool CountingSemaphore::give_from_isr(BaseType_t *pxHigherPriorityTaskWoken)
{
    if (_xSemaphoreHandle == nullptr)
        return false;

    BaseType_t xHigherPriorityTaskWokenDummy = pdFALSE;
    BaseType_t *pWoken = (pxHigherPriorityTaskWoken != nullptr) ? pxHigherPriorityTaskWoken : &xHigherPriorityTaskWokenDummy;

    bool success = (xSemaphoreGiveFromISR(_xSemaphoreHandle, pWoken) == pdTRUE);

    if (pxHigherPriorityTaskWoken == nullptr && xHigherPriorityTaskWokenDummy == pdTRUE)
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWokenDummy);
    }
    return success;
}

UBaseType_t CountingSemaphore::get_count() const noexcept
{
    return uxSemaphoreGetCount(_xSemaphoreHandle);
}
