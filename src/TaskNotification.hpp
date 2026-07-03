#ifndef TASK_NOTIFICATION_HPP
#define TASK_NOTIFICATION_HPP

#include "FreeRTOS_Base.h"
#include RTOS_INC("freertos/task.h", <task.h>, <task.h>)
#include "Threads.hpp"

class TaskNotification
{
public:
    static bool notify(const IThread &target_thread);

    static bool notify_from_isr(const IThread &target_thread, BaseType_t *pxHigherPriorityTaskWoken = nullptr);

    static uint32_t wait(bool clear_on_exit = true, uint32_t timeout_ms = 0xFFFFFFFF);
};

#endif