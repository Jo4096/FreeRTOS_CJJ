#include <FreeRTOS_CJJ.h>
#include <ThreadRTOS.hpp>
#include <LGuards.hpp>

// Vanilla FreeRTOS mutex protecting the shared counter below
SemaphoreHandle_t counter_mutex = nullptr;

volatile int shared_counter = 0;


// Task A: increments the shared counter every 500ms.
// Uses UniqueLock the "normal" way (locks immediately in the constructor).
class WriterTask : public ThreadRTOS<Kernel::min_stack_bytes(), 1>
{
  protected:
    void Run() override
    {
      for (;;)
      {
        {
          // Locks 'counter_mutex' right away, just like a normal LockGuard.
          UniqueLock lock(counter_mutex);

          shared_counter += 1;
          Serial.print("[Writer]: Incremented counter to ");
          Serial.println(shared_counter); 

        } // Destructor runs here -> mutex released automatically (RAII)

        Kernel::delay(500);
      }
    }
};


// Task B: tries to read the counter every 700ms, but shows off the
// extra features UniqueLock has over a plain LockGuard:
// 1) defer_lock -> create the lock object WITHOUT locking immediately
// 2) try to lock later, with a short timeout
// 3) check owns_lock() before touching shared data
// 4) manual unlock() before the scope ends, then re-lock if needed
class ReaderTask : public ThreadRTOS<Kernel::min_stack_bytes(), 1>
{
  protected:
    void Run() override
    {
      for (;;)
      {
        // Create the lock WITHOUT locking yet, using defer_lock.
        // This is useful when you want to do some setup/checks first,
        // and only lock the mutex right before you actually need it.
        UniqueLock lock(counter_mutex, defer_lock);

        Serial.println("[Reader]: About to try locking the mutex...");

        // Now actually attempt to lock it, with a short 100ms timeout.
        // lock() returns true/false depending on success, exactly like
        // calling xSemaphoreTake() yourself, but with RAII safety.
        if (lock.lock(100))
        {
          // owns_lock() confirms we really hold the mutex right now. 
          // Note: lock() return true if owns_lock(), so calling if(lock.lock(100)) is the same as doing lock.lock(100); if(lock.owns_lock())...
          Serial.print("[Reader]: Lock acquired. Counter value = ");
          Serial.println(shared_counter);

          // We can unlock manually whenever we want, before the
          // object goes out of scope. Here we release it early on
          // purpose, to show the mutex is free again immediately.
          lock.unlock();
          Serial.println("[Reader]: Released lock manually.");

          // Since owns_lock() is now false, we could safely re-lock it
          // again later using lock.lock() if we needed to, the same UniqueLock object can be reused within its own scope.
        }
        else
        {
          // If the Writer held the mutex for longer than our 100ms
          // timeout, lock() returns false and owns_lock() stays false.
          // No mutex was taken, so there's nothing to clean up - the
          // destructor below will simply do nothing.
          Serial.println("[Reader]: Could not acquire lock in time, skipping this round.");
        }

        // Whether we locked successfully or not, it's always safe to let
        // 'lock' go out of scope here: the destructor only calls
        // xSemaphoreGive() if owns_lock() is true.

        Kernel::delay(700);
      }
    }
};

WriterTask writer_task;
ReaderTask reader_task;

void setup()
{
  Serial.begin(115200);
  while (!Serial) { delay(1); }

  counter_mutex = xSemaphoreCreateMutex(); // Vanilla FreeRTOS mutex init

  if (counter_mutex != nullptr)
  {
    writer_task();
    reader_task();
    Kernel::start();
  }
  else
  {
    Serial.println("Failed to create mutex");
  }
}

void loop() {/* Not used */}