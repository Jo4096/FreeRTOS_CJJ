#ifndef THREAD_RTOS_HPP
#define THREAD_RTOS_HPP

#include "FreeRTOS_Base.h"
#include RTOS_INC("freertos/task.h", <task.h>, <task.h>)
#include "Threads.hpp"

template <uint32_t StackSizeBytes, UBaseType_t Priority>
class ThreadRTOS : public ThreadABS<StackSizeBytes, Priority>
{
protected:
    virtual void Run() = 0;
};

#endif