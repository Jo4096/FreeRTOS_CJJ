#ifndef FREE_RTOS_CJJ_H
#define FREE_RTOS_CJJ_H

#include "FreeRTOS_Base.h"
#include "Kernel.hpp"

// NOTE: This file is only for optional convenience that groups .hpp into modules.
// To minimize overhead during compile time, if you need to include just one .hpp you can simply #include <SafeQueues.hpp> for example. There is no need to include every module
// But if you want to include everything just do #define FREERTOS_CJJ_... before the #include <FreeRTOS_CJJ.h>

#ifdef FREERTOS_CJJ_USE_THREADS
  #include "Threads.hpp"
  #include "ThreadRTOS.hpp"
  #include "ThreadArduino.hpp"
#endif

#ifdef FREERTOS_CJJ_USE_MUTEX
  #include "Mutex.hpp"
  #include "LGuards.hpp"
#endif

#ifdef FREERTOS_CJJ_USE_QUEUES
  #include "SafeQueues.hpp"
#endif

#ifdef FREERTOS_CJJ_USE_TIMERS
  #include "SoftwareTimers.hpp"
#endif

#ifdef FREERTOS_CJJ_USE_STREAMBUFFER
  #include "StreamBuffer.hpp"
#endif

#ifdef FREERTOS_CJJ_USE_PERIPHERALS
  #include "SafeSerial.hpp"
  #include "SafeWire.hpp"
  #include "SafeSPI.hpp"
#endif

#ifdef FREERTOS_CJJ_USE_ADVANCED_SYNC
  #include "Semaphores.hpp"
  #include "EventGroups.hpp"
  #include "TaskNotification.hpp"
#endif

#endif