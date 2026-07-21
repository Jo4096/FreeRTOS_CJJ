#ifndef KERNEL_HPP
#define KERNEL_HPP

#include "FreeRTOS_Base.h"
#include RTOS_INC("freertos/task.h", <task.h>, <task.h>)

namespace fcjj
{
    namespace Kernel
    {
        enum class SchedulerState
        {
            NotStarted,
            Running,
            Suspended
        };

        [[nodiscard]] SchedulerState scheduler_state();

        [[nodiscard]] bool is_running();

        void start();

        void delay(uint32_t ms);

        void delay_until(TickType_t &lastTime, uint32_t ms);

        [[nodiscard]] uint32_t uptime_ms();

        [[nodiscard]] uint32_t uptime_ms_from_isr();

        void yield();

        [[nodiscard]] size_t free_heap();

        void suspend_scheduler();

        [[nodiscard]] bool resume_scheduler();

        void enter_critical();

        void exit_critical();

        [[nodiscard]] UBaseType_t enter_critical_from_isr();

        void exit_critical_from_isr(UBaseType_t saved_status);

        class CriticalGuard
        {
        public:
            CriticalGuard();
            ~CriticalGuard();

            CriticalGuard(const CriticalGuard &) = delete;
            CriticalGuard &operator=(const CriticalGuard &) = delete;
            CriticalGuard(CriticalGuard &&) = delete;
            CriticalGuard &operator=(CriticalGuard &&) = delete;
        };

        class CriticalGuardFromISR
        {
        private:
            UBaseType_t saved_status;

        public:
            CriticalGuardFromISR();
            ~CriticalGuardFromISR();

            CriticalGuardFromISR(const CriticalGuardFromISR &) = delete;
            CriticalGuardFromISR &operator=(const CriticalGuardFromISR &) = delete;
            CriticalGuardFromISR(CriticalGuardFromISR &&) = delete;
            CriticalGuardFromISR &operator=(CriticalGuardFromISR &&) = delete;
        };

        constexpr uint32_t WORDS_TO_BYTES(uint32_t words);
        constexpr uint32_t BYTES_TO_WORDS(uint32_t bytes);
        constexpr uint32_t min_stack_words()
        {
            return configMINIMAL_STACK_SIZE;
        }
        constexpr uint32_t min_stack_bytes()
        {
            return configMINIMAL_STACK_SIZE * sizeof(StackType_t);
        }
    }
}
#endif