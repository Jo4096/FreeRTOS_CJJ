#ifndef THREADS_HPP
#define THREADS_HPP

#include "FreeRTOS_Base.h"
#include RTOS_INC("freertos/task.h", <task.h>, <task.h>)

#if defined(__AVR_ATmega328P__) || defined(ARDUINO_AVR_UNO)
    #define THREADS_NO_STATIC_ALLOC 1
    #warning "Notice: On Arduino UNO, tasks are DYNAMICALLY allocated (xTaskCreate) due to memory architecture constraints, unlike ESP32 or Raspberry Pi which use STATIC allocation."
#else
    #define THREADS_NO_STATIC_ALLOC 0
#endif

class IThread
{
public:
    virtual ~IThread() = default;

    virtual void operator()(const char *name = "Task") = 0;
    virtual bool IsRunning() const = 0;
    virtual TaskHandle_t Handle() const = 0;
    virtual size_t stack_high_water_mark() const = 0;

    IThread() = default;
    IThread(const IThread &) = delete;
    IThread &operator=(const IThread &) = delete;
    IThread(IThread &&) = delete;
    IThread &operator=(IThread &&) = delete;
};

template <uint32_t StackSizeBytes, UBaseType_t Priority>
class ThreadABS : public IThread
{
private:
    static constexpr uint32_t StackDepth = (StackSizeBytes + sizeof(StackType_t) - 1) / sizeof(StackType_t);

    static_assert(StackDepth >= configMINIMAL_STACK_SIZE, "Error: StackSizeBytes converted is smaller than configMINIMAL_STACK_SIZE.");
    static_assert(Priority < configMAX_PRIORITIES, "Error: Priority exceeds configMAX_PRIORITIES.");

    TaskHandle_t xHandle = nullptr;

#if !THREADS_NO_STATIC_ALLOC
    StaticTask_t xTaskBuffer;
    StackType_t xStack[StackDepth];
#endif

    static void TaskHook(void *pvParameters)
    {
        auto *instance = static_cast<ThreadABS *>(pvParameters);
        instance->Run();

        TaskHandle_t tempHandle = instance->xHandle;
        instance->xHandle = nullptr;
        vTaskDelete(tempHandle);
        for (;;)
        {
            vTaskDelay(portMAX_DELAY);
        }
    }

protected:
    virtual void Run() = 0;

public:

    virtual ~ThreadABS()
    {
        if (xHandle != nullptr)
        {
            vTaskDelete(xHandle);
            xHandle = nullptr;
        }
    }

    void operator()(const char *name = "Task") override
    {
        if (xHandle == nullptr)
        {
#if THREADS_NO_STATIC_ALLOC
            xTaskCreate(TaskHook, name, StackDepth, this, Priority, &xHandle);
#else
            xHandle = xTaskCreateStatic(TaskHook, name, StackDepth, this, Priority, xStack, &xTaskBuffer);
#endif
        }
    }

    TaskHandle_t Handle() const noexcept override { return xHandle; }
    bool IsRunning() const noexcept override { return xHandle != nullptr; }

    size_t stack_high_water_mark() const override
    {
        if (xHandle == nullptr)
            return StackSizeBytes;
        return uxTaskGetStackHighWaterMark(xHandle) * sizeof(StackType_t);
    }
};

#endif