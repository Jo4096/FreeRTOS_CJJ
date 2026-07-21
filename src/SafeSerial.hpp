#ifndef SAFESERIAL_HPP
#define SAFESERIAL_HPP

#include <Arduino.h>
#include "FreeRTOS_Base.h"
#include "LGuards.hpp"

namespace fcjj
{
    class SafeSerial
    {
    private:
        SemaphoreHandle_t xMutexHandle = nullptr;
        StaticSemaphore_t xMutexBuffer;
        Stream &xSerial;
        uint32_t default_time;

    public:
        class Locked
        {
        private:
            UniqueLock lock;
            Stream &xSerial;

        public:
            Locked(SemaphoreHandle_t &mtx, Stream &s, uint32_t timeout_ms) : lock(mtx, timeout_ms), xSerial(s) {}

            Locked(const Locked &) = delete;
            Locked &operator=(const Locked &) = delete;

            [[nodiscard]] explicit operator bool() const noexcept { return lock.owns_lock(); }
            [[nodiscard]] bool owns_lock() const noexcept { return lock.owns_lock(); }

            template <typename T>
            Locked &operator<<(T &&v)
            {
                if (lock.owns_lock())
                    xSerial.print(custom_std::forward<T>(v));
                return *this;
            }

            template <typename... Args>
            Locked &println(Args &&...args)
            {
                if (lock.owns_lock())
                    xSerial.println(custom_std::forward<Args>(args)...);
                return *this;
            }

            template <typename... Args>
            Locked &print(Args &&...args)
            {
                if (lock.owns_lock())
                    xSerial.print(custom_std::forward<Args>(args)...);
                return *this;
            }
        };

        SafeSerial() = delete;
        SafeSerial(const SafeSerial &) = delete;
        SafeSerial &operator=(const SafeSerial &) = delete;
        SafeSerial(SafeSerial &&) = delete;
        SafeSerial &operator=(SafeSerial &&) = delete;

        SafeSerial(Stream &serial_ref, uint32_t default_time = 100) : xSerial(serial_ref), default_time(default_time)
        {
            xMutexHandle = xSemaphoreCreateMutexStatic(&xMutexBuffer);
        }

        Locked lockedStream(uint32_t timeout_ms)
        {
            return Locked(xMutexHandle, xSerial, timeout_ms);
        }

        Locked lockedStream()
        {
            return Locked(xMutexHandle, xSerial, default_time);
        }

        void setDefaultTimeout(uint32_t ms) noexcept { default_time = ms; }
        [[nodiscard]] uint32_t getDefaultTimeout() const noexcept { return default_time; }

        template <typename... Args>
        [[nodiscard]] bool printTimeout(uint32_t timeout_ms, Args &&...args)
        {
            if (xMutexHandle == nullptr)
                return false;

            UniqueLock lock(xMutexHandle, timeout_ms);
            if (lock.owns_lock())
            {
                xSerial.print(custom_std::forward<Args>(args)...);
                return true;
            }
            return false;
        }

        template <typename... Args>
        [[nodiscard]] bool printlnTimeout(uint32_t timeout_ms, Args &&...args)
        {
            if (xMutexHandle == nullptr)
                return false;

            UniqueLock lock(xMutexHandle, timeout_ms);
            if (lock.owns_lock())
            {
                xSerial.println(custom_std::forward<Args>(args)...);
                return true;
            }
            return false;
        }

        template <typename... Args>
        [[nodiscard]] bool print(Args &&...args)
        {
            return printTimeout(default_time, custom_std::forward<Args>(args)...);
        }

        template <typename... Args>
        [[nodiscard]] bool println(Args &&...args)
        {
            return printlnTimeout(default_time, custom_std::forward<Args>(args)...);
        }
    };
}
#endif