#ifndef SOFTWARE_TIMER_HPP
#define SOFTWARE_TIMER_HPP

#include "FreeRTOS_Base.h"
#include RTOS_INC("freertos/timers.h", <timers.h>, <timers.h>)

#if defined(__AVR_ATmega328P__) || defined(ARDUINO_AVR_UNO)
#ifndef xTimerCreateStatic
#define xTimerCreateStatic(pcTimerName, xTimerPeriod, uxAutoReload, pvTimerID, pxCallbackFunction, pxTimerBuffer) \
    xTimerCreate((pcTimerName), (xTimerPeriod), (uxAutoReload), (pvTimerID), (pxCallbackFunction))
#endif
#endif

namespace fcjj
{
    template <bool AutoReload>
    class SoftwareTimer
    {
    private:
        TimerHandle_t xHandle = nullptr;
        StaticTimer_t xBuffer;

        static void callback_hook(TimerHandle_t xTimer)
        {
            auto *self = static_cast<SoftwareTimer *>(pvTimerGetTimerID(xTimer));
            if (self != nullptr)
            {
                self->on_timer();
            }
        }

    protected:
        virtual void on_timer() = 0;

    public:
        SoftwareTimer() = default;
        virtual ~SoftwareTimer() { stop_and_delete(); }

        SoftwareTimer(const SoftwareTimer &) = delete;
        SoftwareTimer &operator=(const SoftwareTimer &) = delete;
        SoftwareTimer(SoftwareTimer &&) = delete;
        SoftwareTimer &operator=(SoftwareTimer &&) = delete;

        bool start(uint32_t period_ms, const char *name = "Timer", uint32_t timeout_ms = 0xFFFFFFFF)
        {
            TickType_t period = pdMS_TO_TICKS(period_ms);
            TickType_t wait = (timeout_ms == 0xFFFFFFFF) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);

            if (xHandle == nullptr)
            {
                xHandle = xTimerCreateStatic(
                    name,
                    period,
                    AutoReload ? pdTRUE : pdFALSE,
                    static_cast<void *>(this),
                    callback_hook,
                    &xBuffer);

                if (xHandle == nullptr)
                    return false;
            }
            else
            {
                if (xTimerChangePeriod(xHandle, period, wait) != pdPASS)
                    return false;
            }

            return xTimerStart(xHandle, wait) == pdPASS;
        }

        bool stop(uint32_t timeout_ms = 0xFFFFFFFF)
        {
            if (xHandle == nullptr)
                return false;
            TickType_t wait = (timeout_ms == 0xFFFFFFFF) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
            return xTimerStop(xHandle, wait) == pdPASS;
        }

        bool reset(uint32_t timeout_ms = 0xFFFFFFFF)
        {
            if (xHandle == nullptr)
                return false;
            TickType_t wait = (timeout_ms == 0xFFFFFFFF) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
            return xTimerReset(xHandle, wait) == pdPASS;
        }

        bool reset_from_isr(BaseType_t *pxHigherPriorityTaskWoken = nullptr)
        {
            if (xHandle == nullptr)
                return false;

            BaseType_t dummy = pdFALSE;
            BaseType_t *pWoken = (pxHigherPriorityTaskWoken != nullptr) ? pxHigherPriorityTaskWoken : &dummy;

            bool ok = (xTimerResetFromISR(xHandle, pWoken) == pdPASS);

            if (pxHigherPriorityTaskWoken == nullptr && dummy == pdTRUE)
            {
                portYIELD_FROM_ISR(dummy);
            }
            return ok;
        }

        bool set_period(uint32_t period_ms, uint32_t timeout_ms = 0xFFFFFFFF)
        {
            if (xHandle == nullptr)
                return false;
            TickType_t wait = (timeout_ms == 0xFFFFFFFF) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
            return xTimerChangePeriod(xHandle, pdMS_TO_TICKS(period_ms), wait) == pdPASS;
        }

        uint32_t get_period_ms() const
        {
            if (xHandle == nullptr)
                return 0;
            return (xTimerGetPeriod(xHandle) * 1000) / configTICK_RATE_HZ;
        }

        bool is_active() const
        {
            if (xHandle == nullptr)
                return false;
            return xTimerIsTimerActive(xHandle) == pdTRUE;
        }

        const char *get_name() const
        {
            if (xHandle == nullptr)
                return "";
            return pcTimerGetName(xHandle);
        }

        TimerHandle_t get_handle() const noexcept { return xHandle; }

    private:
        void stop_and_delete()
        {
            if (xHandle != nullptr)
            {
                xTimerStop(xHandle, 0);
                xTimerDelete(xHandle, portMAX_DELAY);
                xHandle = nullptr;
            }
        }
    };

    using PeriodicTimer = SoftwareTimer<true>;
    using OneShotTimer = SoftwareTimer<false>;
}
#endif