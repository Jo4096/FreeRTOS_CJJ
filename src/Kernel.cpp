#include "Kernel.hpp"

namespace fcjj
{
    namespace Kernel
    {
        SchedulerState scheduler_state()
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

        bool is_running()
        {
            return xTaskGetSchedulerState() == taskSCHEDULER_RUNNING;
        }

        void start()
        {
            if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED)
            {
                vTaskStartScheduler();
            }
        }

        void delay(uint32_t ms)
        {
            vTaskDelay(pdMS_TO_TICKS(ms));
        }

        void delay_until(TickType_t &lastTime, uint32_t ms)
        {
            xTaskDelayUntil(&lastTime, pdMS_TO_TICKS(ms));
        }

        uint32_t uptime_ms()
        {
            return (xTaskGetTickCount() * 1000) / configTICK_RATE_HZ;
        }

        uint32_t uptime_ms_from_isr()
        {
            return (xTaskGetTickCountFromISR() * 1000) / configTICK_RATE_HZ;
        }

        void yield()
        {
            taskYIELD();
        }

        size_t free_heap()
        {
            return xPortGetFreeHeapSize();
        }

        void suspend_scheduler()
        {
            vTaskSuspendAll();
        }

        bool resume_scheduler()
        {
            return xTaskResumeAll() == pdTRUE;
        }

        void enter_critical()
        {
            taskENTER_CRITICAL();
        }

        void exit_critical()
        {
            taskEXIT_CRITICAL();
        }

        UBaseType_t enter_critical_from_isr()
        {
            return taskENTER_CRITICAL_FROM_ISR();
        }

        void exit_critical_from_isr(UBaseType_t saved_status)
        {
            taskEXIT_CRITICAL_FROM_ISR(saved_status);
        }

        constexpr uint32_t WORDS_TO_BYTES(uint32_t words)
        {
            return words * sizeof(StackType_t);
        }

        constexpr uint32_t BYTES_TO_WORDS(uint32_t bytes)
        {
            return (bytes + sizeof(StackType_t) - 1) / sizeof(StackType_t);
        }

        CriticalGuard::CriticalGuard()
        {
            enter_critical();
        }

        CriticalGuard::~CriticalGuard()
        {
            exit_critical();
        }

        CriticalGuardFromISR::CriticalGuardFromISR() : saved_status(enter_critical_from_isr()) {}

        CriticalGuardFromISR::~CriticalGuardFromISR()
        {
            exit_critical_from_isr(saved_status);
        }
    }
}