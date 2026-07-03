#include "Kernel.hpp"

Kernel::SchedulerState Kernel::scheduler_state()
{
    switch (xTaskGetSchedulerState())
    {
    case taskSCHEDULER_NOT_STARTED:
        return SchedulerState::NotStarted;
    case taskSCHEDULER_RUNNING:
        return SchedulerState::Running;
    case taskSCHEDULER_SUSPENDED:
        return SchedulerState::Suspended;
    default:
        return SchedulerState::NotStarted;
    }
}

bool Kernel::is_running()
{
    return xTaskGetSchedulerState() == taskSCHEDULER_RUNNING;
}

void Kernel::start()
{
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED)
    {
        vTaskStartScheduler();
    }
}

void Kernel::delay(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void Kernel::delay_until(TickType_t &lastTime, uint32_t ms)
{
    xTaskDelayUntil(&lastTime, pdMS_TO_TICKS(ms));
}

uint32_t Kernel::uptime_ms()
{
    return (xTaskGetTickCount() * 1000) / configTICK_RATE_HZ;
}

uint32_t Kernel::uptime_ms_from_isr()
{
    return (xTaskGetTickCountFromISR() * 1000) / configTICK_RATE_HZ;
}

void Kernel::yield()
{
    taskYIELD();
}

size_t Kernel::free_heap()
{
    return xPortGetFreeHeapSize();
}

void Kernel::suspend_scheduler()
{
    vTaskSuspendAll();
}

bool Kernel::resume_scheduler()
{
    return xTaskResumeAll() == pdTRUE;
}

void Kernel::enter_critical()
{
    taskENTER_CRITICAL();
}

void Kernel::exit_critical()
{
    taskEXIT_CRITICAL();
}

UBaseType_t Kernel::enter_critical_from_isr()
{
    return taskENTER_CRITICAL_FROM_ISR();
}

void Kernel::exit_critical_from_isr(UBaseType_t saved_status)
{
    taskEXIT_CRITICAL_FROM_ISR(saved_status);
}

constexpr uint32_t Kernel::WORDS_TO_BYTES(uint32_t words)
{
    return words * sizeof(StackType_t);
}

constexpr uint32_t Kernel::BYTES_TO_WORDS(uint32_t bytes)
{
    return (bytes + sizeof(StackType_t) - 1) / sizeof(StackType_t);
}

Kernel::CriticalGuard::CriticalGuard()
{
    enter_critical();
}

Kernel::CriticalGuard::~CriticalGuard()
{
    exit_critical();
}

Kernel::CriticalGuardFromISR::CriticalGuardFromISR() : saved_status(enter_critical_from_isr()) {}

Kernel::CriticalGuardFromISR::~CriticalGuardFromISR()
{
    exit_critical_from_isr(saved_status);
}
