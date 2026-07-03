#ifndef SAFESERIAL_HPP
#define SAFESERIAL_HPP

#include <Arduino.h>
#include "FreeRTOS_Base.h"
#include "LGuards.hpp"

class SafeSerial
{
private:
    SemaphoreHandle_t xMutexHandle = nullptr;
    StaticSemaphore_t xMutexBuffer;
    Stream &xSerial;
    uint32_t default_time;

public:
    SafeSerial() = delete;
    SafeSerial(const SafeSerial &) = delete;
    SafeSerial &operator=(const SafeSerial &) = delete;
    SafeSerial(SafeSerial &&) = delete;
    SafeSerial &operator=(SafeSerial &&) = delete;

    SafeSerial(Stream &serial_ref, uint32_t default_time = 100) : xSerial(serial_ref), default_time(default_time)
    {
        xMutexHandle = xSemaphoreCreateMutexStatic(&xMutexBuffer);
    }

    // Nota: Removido o xSerial.begin() porque nem toda classe herdada de 'Stream' possui .begin() com a mesma assinatura.
    // É melhor chamar Serial.begin() direto no setup().

    template <typename... Args>
    bool print(uint32_t timeout_ms, Args &&...args)
    {
        if (xMutexHandle == nullptr)
            return false;

        UniqueLock lock(xMutexHandle, timeout_ms);
        if (lock.owns_lock())
        {
            // custom_std::forward vai resolver o problema do AVR
            xSerial.print(custom_std::forward<Args>(args)...);
            return true;
        }
        return false;
    }

    template <typename... Args>
    bool println(uint32_t timeout_ms, Args &&...args)
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
    bool print(Args &&...args)
    {
        return print(default_time, custom_std::forward<Args>(args)...);
    }

    template <typename... Args>
    bool println(Args &&...args)
    {
        return println(default_time, custom_std::forward<Args>(args)...);
    }
};

#endif