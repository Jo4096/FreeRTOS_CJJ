#ifndef SEMAPHORES_HPP
#define SEMAPHORES_HPP

#include "FreeRTOS_Base.h"
#include RTOS_INC("freertos/semphr.h", <semphr.h>, <semphr.h>)


#if defined(__AVR_ATmega328P__) || defined(ARDUINO_AVR_UNO)
#define xSemaphoreCreateBinaryStatic(pxSemaphoreBuffer) \
    xSemaphoreCreateBinary()
#define xSemaphoreCreateCountingStatic(uxMaxCount, uxInitialCount, pxSemaphoreBuffer) \
    xSemaphoreCreateCounting((uxMaxCount), (uxInitialCount))
#endif

class BinarySemaphore
{
private:
    SemaphoreHandle_t _xSemaphoreHandle = nullptr;
    StaticSemaphore_t _xSemaphoreBuffer;

public:
    BinarySemaphore();
    BinarySemaphore(const BinarySemaphore &) = delete;
    BinarySemaphore &operator=(const BinarySemaphore &) = delete;

    ~BinarySemaphore();

    bool take(uint32_t timeout_ms = 0xFFFFFFFF);

    bool give();

    bool give_from_isr(BaseType_t *pxHigherPriorityTaskWoken = nullptr);
};

class CountingSemaphore
{
private:
    SemaphoreHandle_t _xSemaphoreHandle = nullptr;
    StaticSemaphore_t _xSemaphoreBuffer;

public:
    CountingSemaphore() = delete;
    CountingSemaphore(const CountingSemaphore &) = delete;
    CountingSemaphore &operator=(const CountingSemaphore &) = delete;
    CountingSemaphore(UBaseType_t max_count, UBaseType_t initial_count);

    ~CountingSemaphore();

    bool take(uint32_t timeout_ms = 0xFFFFFFFF);

    bool give();

    bool give_from_isr(BaseType_t *pxHigherPriorityTaskWoken = nullptr);

    UBaseType_t get_count() const noexcept;
};

#endif