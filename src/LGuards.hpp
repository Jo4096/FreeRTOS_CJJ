#ifndef LGUARDS_HPP
#define LGUARDS_HPP

#include "FreeRTOS_Base.h"
#include RTOS_INC("freertos/semphr.h", <semphr.h>, <semphr.h>)
#include "Mutex.hpp"

namespace fcjj
{
    struct defer_lock_t
    {
        explicit defer_lock_t() = default;
    };

    constexpr defer_lock_t defer_lock{};

    class LockGuard
    {
    private:
        SemaphoreHandle_t &mutex;
        bool is_locked = false;

    public:
        LockGuard() = delete;
        LockGuard(const LockGuard &) = delete;
        LockGuard &operator=(const LockGuard &) = delete;

        LockGuard(SemaphoreHandle_t &mutex, uint32_t ms_timeout = 0xFFFFFFFF);

        LockGuard(Mutex &mutex, uint32_t ms_timeout = 0xFFFFFFFF);

        [[nodiscard]] bool owns_lock() const noexcept;

        ~LockGuard();
    };

    class UniqueLock
    {
    private:
        SemaphoreHandle_t &mutex;
        bool is_locked = false;

    public:
        UniqueLock() = delete;
        UniqueLock(const UniqueLock &) = delete;
        UniqueLock &operator=(const UniqueLock &) = delete;

        UniqueLock(SemaphoreHandle_t &mutex, uint32_t ms_timeout = 0xFFFFFFFF);

        UniqueLock(Mutex &mutex, uint32_t ms_timeout = 0xFFFFFFFF);

        UniqueLock(SemaphoreHandle_t &mutex, defer_lock_t dl) noexcept;

        UniqueLock(Mutex &mutex, defer_lock_t dl) noexcept;

        [[nodiscard]] bool lock(uint32_t ms_timeout = 0xFFFFFFFF);

        void unlock();

        [[nodiscard]] bool owns_lock() const noexcept;

        ~UniqueLock();
    };

    class RecursiveLockGuard
    {
    private:
        SemaphoreHandle_t &mutex;
        bool is_locked = false;

    public:
        RecursiveLockGuard() = delete;
        RecursiveLockGuard(const RecursiveLockGuard &) = delete;
        RecursiveLockGuard &operator=(const RecursiveLockGuard &) = delete;

        RecursiveLockGuard(SemaphoreHandle_t &mutex, uint32_t ms_timeout = 0xFFFFFFFF);

        RecursiveLockGuard(RecursiveMutex &mutex, uint32_t ms_timeout = 0xFFFFFFFF);
        [[nodiscard]] bool owns_lock() const noexcept;

        ~RecursiveLockGuard();
    };

    class RecursiveUniqueLock
    {
    private:
        SemaphoreHandle_t &mutex;
        bool is_locked = false;

    public:
        RecursiveUniqueLock() = delete;
        RecursiveUniqueLock(const RecursiveUniqueLock &) = delete;
        RecursiveUniqueLock &operator=(const RecursiveUniqueLock &) = delete;

        RecursiveUniqueLock(SemaphoreHandle_t &mutex, uint32_t ms_timeout = 0xFFFFFFFF);

        RecursiveUniqueLock(RecursiveMutex &mutex, uint32_t ms_timeout = 0xFFFFFFFF);

        RecursiveUniqueLock(SemaphoreHandle_t &mutex, defer_lock_t) noexcept;

        RecursiveUniqueLock(RecursiveMutex &mutex, defer_lock_t dl) noexcept;

        [[nodiscard]] bool lock(uint32_t ms_timeout = 0xFFFFFFFF);

        void unlock();

        [[nodiscard]] bool owns_lock() const noexcept;

        ~RecursiveUniqueLock();
    };
}
#endif