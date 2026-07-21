#ifndef SAFE_RING_BUFFER_HPP
#define SAFE_RING_BUFFER_HPP

#include "FreeRTOS_Base.h"
#include "Kernel.hpp"

#if defined(ARDUINO_ARCH_RP2040) || defined(TARGET_RP2040)
#if !defined(configNUMBER_OF_CORES)
#warning "Notice: configNUMBER_OF_CORES is not defined by your FreeRTOSConfig.h..."
#elif configNUMBER_OF_CORES < 2
#warning "Notice: configNUMBER_OF_CORES is defined but < 2 on an RP2040 target..."
#endif
#endif
namespace fcjj
{
    enum class Overwrite
    {
        None,
        Front,
        Back
    };

    // Thread-safe (and ISR-safe) circular buffer / deque.
    //
    // Backed by static storage (no heap, no new/placement-new, no
    // reinterpret_cast/const_cast) — JSF++ compliant.
    //
    // T must be default-constructible: plain T data[Capacity]{} storage,
    // elements are assigned/reset via operator=, never constructed or
    // destroyed individually. Supports trivial types, complex types with a
    // default constructor, and pointers.
    //
    // Locking uses Kernel critical sections (not a Mutex): operations never
    // block/wait — they either succeed immediately or fail immediately
    // (e.g. Overwrite::None on a full buffer). This is what makes ISR usage
    // possible for push/pop.
    template <typename T, size_t Capacity>
    class SafeRingBuffer
    {
        static_assert(Capacity > 0u, "Capacity must be > 0.");

    public:
        SafeRingBuffer() = default;
        SafeRingBuffer(const SafeRingBuffer &) = delete;
        SafeRingBuffer &operator=(const SafeRingBuffer &) = delete;
        SafeRingBuffer(SafeRingBuffer &&) = delete;
        SafeRingBuffer &operator=(SafeRingBuffer &&) = delete;
        ~SafeRingBuffer() = default;

        // Inserts value at the back. Fails if full and policy == Overwrite::None.
        [[nodiscard]] bool push_back(const T &value, Overwrite policy = Overwrite::None)
        {
            Kernel::CriticalGuard guard;
            if (!make_room_unlocked(policy))
            {
                return false;
            }
            data[phys(count)] = value;
            ++count;
            return true;
        }

        // Inserts value at the front. Fails if full and policy == Overwrite::None.
        [[nodiscard]] bool push_front(const T &value, Overwrite policy = Overwrite::None)
        {
            Kernel::CriticalGuard guard;
            if (!make_room_unlocked(policy))
            {
                return false;
            }
            head = (head + Capacity - 1u) % Capacity;
            data[head] = value;
            ++count;
            return true;
        }

        // Removes and returns (via out) the back element. Fails if empty
        [[nodiscard]] bool pop_back(T &out)
        {
            Kernel::CriticalGuard guard;
            if (count == 0u)
            {
                return false;
            }
            out = data[phys(count - 1u)];
            reset_back_unlocked();
            return true;
        }

        // Removes and returns (via out) the front element. Fails if empty
        [[nodiscard]] bool pop_front(T &out)
        {
            Kernel::CriticalGuard guard;
            if (count == 0u)
            {
                return false;
            }
            out = data[head];
            reset_front_unlocked();
            return true;
        }

        [[nodiscard]] bool push_back_from_isr(const T &value, Overwrite policy = Overwrite::None)
        {
            UBaseType_t saved = Kernel::enter_critical_from_isr();
            bool ok = make_room_unlocked(policy);
            if (ok)
            {
                data[phys(count)] = value;
                ++count;
            }
            Kernel::exit_critical_from_isr(saved);
            return ok;
        }

        [[nodiscard]] bool push_front_from_isr(const T &value, Overwrite policy = Overwrite::None)
        {
            UBaseType_t saved = Kernel::enter_critical_from_isr();
            bool ok = make_room_unlocked(policy);
            if (ok)
            {
                head = (head + Capacity - 1u) % Capacity;
                data[head] = value;
                ++count;
            }
            Kernel::exit_critical_from_isr(saved);
            return ok;
        }

        [[nodiscard]] bool pop_back_from_isr(T &out)
        {
            UBaseType_t saved = Kernel::enter_critical_from_isr();
            bool ok = (count != 0u);
            if (ok)
            {
                out = data[phys(count - 1u)];
                reset_back_unlocked();
            }
            Kernel::exit_critical_from_isr(saved);
            return ok;
        }

        [[nodiscard]] bool pop_front_from_isr(T &out)
        {
            UBaseType_t saved = Kernel::enter_critical_from_isr();
            bool ok = (count != 0u);
            if (ok)
            {
                out = data[head];
                reset_front_unlocked();
            }
            Kernel::exit_critical_from_isr(saved);
            return ok;
        }

        // ---- random access: insert / remove / peek ----
        // Task context only: shifting elements can take longer than a push/pop,
        // so these are not offered as _from_isr variants.

        // Inserts value at logical_index, shifting neighbours to make room.
        // If full, applies policy first (may evict front or back before shifting).
        [[nodiscard]] bool insert(size_t logical_index, const T &value, Overwrite policy = Overwrite::None)
        {
            Kernel::CriticalGuard guard;
            if (logical_index > count)
            {
                return false;
            }
            if (!make_room_unlocked(policy))
            {
                return false;
            }
            if (logical_index > count)
            {
                logical_index = count;
            }

            if (logical_index <= count / 2u)
            {
                head = (head + Capacity - 1u) % Capacity;
                for (size_t i = 0u; i < logical_index; ++i)
                {
                    data[phys(i)] = data[phys(i + 1u)];
                }
                data[phys(logical_index)] = value;
            }
            else
            {
                for (size_t i = count; i > logical_index; --i)
                {
                    data[phys(i)] = data[phys(i - 1u)];
                }
                data[phys(logical_index)] = value;
            }
            ++count;
            return true;
        }

        // Removes the element at logical_index, shifting neighbours to close the
        // gap. If out != nullptr, the removed value is copied into it first.
        [[nodiscard]] bool remove(size_t logical_index, T *out = nullptr)
        {
            Kernel::CriticalGuard guard;
            if (logical_index >= count)
            {
                return false;
            }

            if (out != nullptr)
            {
                *out = data[phys(logical_index)];
            }

            if (logical_index <= count / 2u)
            {
                for (size_t i = logical_index; i > 0u; --i)
                {
                    data[phys(i)] = data[phys(i - 1u)];
                }
                data[head] = T{};
                head = (head + 1u) % Capacity;
            }
            else
            {
                for (size_t i = logical_index; i < count - 1u; ++i)
                {
                    data[phys(i)] = data[phys(i + 1u)];
                }
                data[phys(count - 1u)] = T{};
            }
            --count;
            return true;
        }

        // Copies out the element at logical_index (0 = front) without removing it.
        [[nodiscard]] bool get(size_t logical_index, T &out) const
        {
            Kernel::CriticalGuard guard;
            if (logical_index >= count)
            {
                return false;
            }
            out = data[phys(logical_index)];
            return true;
        }

        [[nodiscard]] bool front(T &out) const
        {
            return get(0u, out);
        }

        // Reads count and data[] atomically under a single lock, avoiding the
        // TOCTOU gap you'd get from computing (count - 1) outside the guard
        // and only locking inside a subsequent get() call.
        [[nodiscard]] bool back(T &out) const
        {
            Kernel::CriticalGuard guard;
            if (count == 0u)
            {
                return false;
            }
            out = data[phys(count - 1u)];
            return true;
        }

        // ---- iteration ----
        // Runs func(index, value) for every element, front to back, under a
        // single critical section — like Python's enumerate(), but locked.
        //
        // NOTE: the whole traversal happens inside one critical section, so
        // `func` must be fast and must NOT call back into this buffer (push/pop/
        // insert/remove/etc. would deadlock/corrupt state) and must NOT block
        // (no vTaskDelay, no takes, no locking Serial, etc.) — treat it like
        // ISR-context code. For heavy per-element work, copy into a local array
        // first (e.g. via get() in a loop, or your own snapshot), then process
        // that outside the lock.
        template <typename Func>
        void for_each(Func &&func)
        {
            Kernel::CriticalGuard guard;
            for (size_t i = 0u; i < count; ++i)
            {
                func(i, data[phys(i)]);
            }
        }

        template <typename Func>
        void for_each(Func &&func) const
        {
            Kernel::CriticalGuard guard;
            for (size_t i = 0u; i < count; ++i)
            {
                const T &value = data[phys(i)];
                func(i, value);
            }
        }

        template <typename Func>
        void for_each_until(Func &&func)
        {
            Kernel::CriticalGuard guard;
            for (size_t i = 0u; i < count; ++i)
            {
                if (!func(i, data[phys(i)]))
                {
                    break;
                }
            }
        }

        // Same as for_each, but the callback can return false to stop early.
        template <typename Func>
        void for_each_until(Func &&func) const
        {
            Kernel::CriticalGuard guard;
            for (size_t i = 0u; i < count; ++i)
            {
                const T &value = data[phys(i)];
                if (!func(i, value))
                {
                    break;
                }
            }
        }

        [[nodiscard]] size_t size() const noexcept
        {
            Kernel::CriticalGuard guard;
            return count;
        }

        [[nodiscard]] bool is_empty() const noexcept
        {
            Kernel::CriticalGuard guard;
            return count == 0u;
        }

        [[nodiscard]] bool is_full() const noexcept
        {
            Kernel::CriticalGuard guard;
            return count == Capacity;
        }

        // Resets every element back to T{} and empties the buffer.
        void clear() noexcept
        {
            Kernel::CriticalGuard guard;
            while (count > 0u)
            {
                reset_front_unlocked();
            }
            head = 0u;
        }

        static constexpr size_t capacity() noexcept { return Capacity; }

    private:
        T data[Capacity]{};
        size_t head = 0u;
        size_t count = 0u;

        size_t phys(size_t logical_index) const noexcept
        {
            return (head + logical_index) % Capacity;
        }

        void reset_front_unlocked() noexcept
        {
            data[head] = T{};
            head = (head + 1u) % Capacity;
            --count;
        }

        void reset_back_unlocked() noexcept
        {
            data[phys(count - 1u)] = T{};
            --count;
        }

        bool make_room_unlocked(Overwrite policy) noexcept
        {
            if (count < Capacity)
            {
                return true;
            }

            switch (policy)
            {
            case Overwrite::Front:
                reset_front_unlocked();
                return true;
            case Overwrite::Back:
                reset_back_unlocked();
                return true;
            case Overwrite::None:
            default:
                return false;
            }
        }
    };
}

#endif