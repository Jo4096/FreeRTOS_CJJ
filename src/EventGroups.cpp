#include "EventGroups.hpp"

EventGroup::EventGroup()
{
    xHandle = xEventGroupCreateStatic(&xBuffer);
}

EventGroup::~EventGroup()
{
    if (xHandle != nullptr)
    {
        vEventGroupDelete(xHandle);
        xHandle = nullptr;
    }
}

EventBits_t EventGroup::set(EventBits_t bits)
{
    if (xHandle == nullptr)
        return 0;
    return xEventGroupSetBits(xHandle, bits);
}

bool EventGroup::set_from_isr(EventBits_t bits, BaseType_t *pxHigherPriorityTaskWoken)
{
    if (xHandle == nullptr)
        return false;

    BaseType_t dummy = pdFALSE;
    BaseType_t *pWoken = (pxHigherPriorityTaskWoken != nullptr) ? pxHigherPriorityTaskWoken : &dummy;
    BaseType_t result = xEventGroupSetBitsFromISR(xHandle, bits, pWoken);

    if (pxHigherPriorityTaskWoken == nullptr && dummy == pdTRUE)
    {
        portYIELD_FROM_ISR(dummy);
    }
    return result == pdPASS;
}

EventBits_t EventGroup::clear(EventBits_t bits)
{
    if (xHandle == nullptr)
        return 0;
    return xEventGroupClearBits(xHandle, bits);
}

EventBits_t EventGroup::clear_from_isr(EventBits_t bits)
{
    if (xHandle == nullptr)
        return 0;
    return xEventGroupClearBitsFromISR(xHandle, bits);
}

EventBits_t EventGroup::get() const
{
    if (xHandle == nullptr)
        return 0;
    return xEventGroupGetBits(xHandle);
}

EventBits_t EventGroup::get_from_isr() const
{
    if (xHandle == nullptr)
        return 0;
    return xEventGroupGetBitsFromISR(xHandle);
}

EventBits_t EventGroup::wait_any(EventBits_t bits_to_wait, bool clear_on_exit, uint32_t timeout_ms)
{
    if (xHandle == nullptr)
        return 0;
    TickType_t ticks = (timeout_ms == 0xFFFFFFFF) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xEventGroupWaitBits(xHandle, bits_to_wait, clear_on_exit ? pdTRUE : pdFALSE, pdFALSE, ticks);
}

EventBits_t EventGroup::wait_all(EventBits_t bits_to_wait, bool clear_on_exit, uint32_t timeout_ms)
{
    if (xHandle == nullptr)
        return 0;
    TickType_t ticks = (timeout_ms == 0xFFFFFFFF) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xEventGroupWaitBits(xHandle, bits_to_wait, clear_on_exit ? pdTRUE : pdFALSE, pdTRUE, ticks);
}

EventBits_t EventGroup::sync(EventBits_t bits_to_set, EventBits_t bits_to_wait, uint32_t timeout_ms)
{
    if (xHandle == nullptr)
        return 0;
    TickType_t ticks = (timeout_ms == 0xFFFFFFFF) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xEventGroupSync(xHandle, bits_to_set, bits_to_wait, ticks);
}

bool EventGroup::all_set(EventBits_t bits) const
{
    return (get() & bits) == bits;
}

bool EventGroup::any_set(EventBits_t bits) const
{
    return (get() & bits) != 0;
}

EventGroupHandle_t EventGroup::get_handle() const noexcept
{
    return xHandle;
}