#include "Mutex.hpp"
namespace fcjj
{
    //===========================
    //          MUTEX
    //===========================

    Mutex::Mutex()
    {
        xMutexHandle = xSemaphoreCreateMutexStatic(&xMutexBuffer);
    }

    Mutex::~Mutex()
    {
        if (xMutexHandle != nullptr)
        {
            vSemaphoreDelete(xMutexHandle);
        }
    }

    SemaphoreHandle_t &Mutex::get_handle() noexcept
    {
        return xMutexHandle;
    }

    //===========================
    //      RECURSIVEMUTEX
    //===========================

    RecursiveMutex::RecursiveMutex()
    {
        xMutexHandle = xSemaphoreCreateRecursiveMutexStatic(&xMutexBuffer);
    }

    RecursiveMutex::~RecursiveMutex()
    {
        if (xMutexHandle != nullptr)
        {
            vSemaphoreDelete(xMutexHandle);
        }
    }

    SemaphoreHandle_t &RecursiveMutex::get_handle() noexcept
    {
        return xMutexHandle;
    }
}