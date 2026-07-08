#include "LGuards.hpp"

//===========================
//        LOCKGUARD
//===========================

LockGuard::LockGuard(SemaphoreHandle_t &mutex, uint32_t ms_timeout) : mutex(mutex)
{
    if (this->mutex)
    {
        TickType_t timeout_ticks = (ms_timeout == 0xFFFFFFFF) ? portMAX_DELAY : pdMS_TO_TICKS(ms_timeout);
        is_locked = (xSemaphoreTake(this->mutex, timeout_ticks) == pdTRUE);
    }
}

LockGuard::LockGuard(Mutex &mutex, uint32_t ms_timeout) : LockGuard(mutex.get_handle(), ms_timeout) {}

bool LockGuard::owns_lock() const noexcept { return is_locked; }

LockGuard::~LockGuard()
{
    if (mutex && is_locked)
    {
        xSemaphoreGive(mutex);
    }
}

//===========================
//        UNIQUELOCK
//===========================

UniqueLock::UniqueLock(SemaphoreHandle_t &mutex, uint32_t ms_timeout) : mutex(mutex)
{
    (void)lock(ms_timeout);
}

UniqueLock::UniqueLock(Mutex &mutex, uint32_t ms_timeout) : UniqueLock(mutex.get_handle(), ms_timeout) {}

UniqueLock::UniqueLock(SemaphoreHandle_t &mutex, defer_lock_t dl) noexcept : mutex(mutex), is_locked(false) {}

UniqueLock::UniqueLock(Mutex &mutex, defer_lock_t dl) noexcept : UniqueLock(mutex.get_handle(), dl) {}

bool UniqueLock::lock(uint32_t ms_timeout)
{
    if (mutex && !is_locked)
    {
        TickType_t timeout_ticks = (ms_timeout == 0xFFFFFFFF) ? portMAX_DELAY : pdMS_TO_TICKS(ms_timeout);
        is_locked = (xSemaphoreTake(mutex, timeout_ticks) == pdTRUE);
    }
    return is_locked;
}

void UniqueLock::unlock()
{
    if (mutex && is_locked)
    {
        xSemaphoreGive(mutex);
        is_locked = false;
    }
}

bool UniqueLock::owns_lock() const noexcept { return is_locked; }

UniqueLock::~UniqueLock()
{
    unlock();
}

//===========================
//    RECURSIVELOCKGUARD
//===========================

RecursiveLockGuard::RecursiveLockGuard(SemaphoreHandle_t &mutex, uint32_t ms_timeout) : mutex(mutex)
{
    if (this->mutex)
    {
        TickType_t timeout_ticks = (ms_timeout == 0xFFFFFFFF) ? portMAX_DELAY : pdMS_TO_TICKS(ms_timeout);
        is_locked = (xSemaphoreTakeRecursive(this->mutex, timeout_ticks) == pdTRUE);
    }
}

RecursiveLockGuard::RecursiveLockGuard(RecursiveMutex &mutex, uint32_t ms_timeout) : RecursiveLockGuard(mutex.get_handle(), ms_timeout) {}

bool RecursiveLockGuard::owns_lock() const noexcept { return is_locked; }

RecursiveLockGuard::~RecursiveLockGuard()
{
    if (mutex && is_locked)
    {
        xSemaphoreGiveRecursive(mutex);
    }
}

//===========================
//   RECURSIVEUNIQUELOCK
//===========================

RecursiveUniqueLock::RecursiveUniqueLock(SemaphoreHandle_t &mutex, uint32_t ms_timeout) : mutex(mutex)
{
    (void)lock(ms_timeout);
}

RecursiveUniqueLock::RecursiveUniqueLock(RecursiveMutex &mutex, uint32_t ms_timeout) : RecursiveUniqueLock(mutex.get_handle(), ms_timeout) {}

RecursiveUniqueLock::RecursiveUniqueLock(SemaphoreHandle_t &mutex, defer_lock_t) noexcept : mutex(mutex), is_locked(false) {}

RecursiveUniqueLock::RecursiveUniqueLock(RecursiveMutex &mutex, defer_lock_t dl) noexcept : RecursiveUniqueLock(mutex.get_handle(), dl) {}

bool RecursiveUniqueLock::lock(uint32_t ms_timeout)
{
    if (mutex)
    {
        TickType_t timeout_ticks = (ms_timeout == 0xFFFFFFFF) ? portMAX_DELAY : pdMS_TO_TICKS(ms_timeout);
        is_locked = (xSemaphoreTakeRecursive(mutex, timeout_ticks) == pdTRUE);
    }
    return is_locked;
}

void RecursiveUniqueLock::unlock()
{
    if (mutex && is_locked)
    {
        xSemaphoreGiveRecursive(mutex);
        is_locked = false;
    }
}

bool RecursiveUniqueLock::owns_lock() const noexcept { return is_locked; }

RecursiveUniqueLock::~RecursiveUniqueLock()
{
    unlock();
}
