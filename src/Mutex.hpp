#ifndef MUTEX_HPP
#define MUTEX_HPP

#include "FreeRTOS_Base.h"
#include RTOS_INC("freertos/semphr.h", <semphr.h>, <semphr.h>)

#if defined(__AVR_ATmega328P__) || defined(ARDUINO_AVR_UNO)
#define xSemaphoreCreateMutexStatic(pxMutexBuffer) \
    xSemaphoreCreateMutex()

#define xSemaphoreCreateRecursiveMutexStatic(pxMutexBuffer) \
    xSemaphoreCreateRecursiveMutex()
#endif

class Mutex
{
private:
    SemaphoreHandle_t xMutexHandle = nullptr;
    StaticSemaphore_t xMutexBuffer;

public:
    Mutex();

    Mutex(const Mutex &) = delete;
    Mutex &operator=(const Mutex &) = delete;
    Mutex(Mutex &&) = delete;
    Mutex &operator=(Mutex &&) = delete;

    ~Mutex();

    [[nodiscard]] SemaphoreHandle_t &get_handle() noexcept;
};

class RecursiveMutex
{
private:
    SemaphoreHandle_t xMutexHandle = nullptr;
    StaticSemaphore_t xMutexBuffer;

public:
    RecursiveMutex();

    RecursiveMutex(const RecursiveMutex &) = delete;
    RecursiveMutex &operator=(const RecursiveMutex &) = delete;
    RecursiveMutex(RecursiveMutex &&) = delete;
    RecursiveMutex &operator=(RecursiveMutex &&) = delete;

    ~RecursiveMutex();

    [[nodiscard]] SemaphoreHandle_t &get_handle() noexcept;
};

#endif