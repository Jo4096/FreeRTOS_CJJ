#ifndef THREAD_ARDUINO_HPP
#define THREAD_ARDUINO_HPP

#include "FreeRTOS_Base.h"
#include "Threads.hpp"

template <uint32_t StackSizeBytes, UBaseType_t Priority>
class ThreadArduino : public ThreadABS<StackSizeBytes, Priority>
{
protected:
    virtual void setup() {}
    virtual void loop() = 0;

private:
    void Run() final
    {
        setup();
        for (;;)
        {
            loop();
        }
    }
};

#endif