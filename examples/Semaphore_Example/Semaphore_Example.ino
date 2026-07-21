#include <FreeRTOS_CJJ.h>
#include <Semaphores.hpp>
#include <SafeSerial.hpp>
#include <ThreadRTOS.hpp>

using namespace fcjj;

SafeSerial sSerial(Serial);

// BinarySemaphore: classic 1:1 synchronization, an ISR "signals" a task
// that an event has occurred (e.g., external pin, button, sensor).
BinarySemaphore button_event;

// CountingSemaphore: controls access to a "pool" of N resources, or counts
// events that accumulate until they are consumed.
// Here we simulate a circular buffer with 5 free slots at startup.
constexpr UBaseType_t POOL_SIZE = 5;
CountingSemaphore resource_pool(/*max_count=*/POOL_SIZE, /*initial_count=*/POOL_SIZE);

// Simulated ISR (e.g., called in a real attachInterrupt).
// Never blocks, always uses give_from_isr.
void onButtonPress()
{
  BaseType_t woken = pdFALSE;
  button_event.give_from_isr(&woken);
  // If you pass 'woken' itself (instead of nullptr), you must perform the yield
  // manually at the end of the ISR. By passing nullptr, the class handles it.
}

class ButtonTask : public ThreadRTOS<Kernel::min_stack_bytes(), 2>
{
  protected:
    void Run() override
    {
      for(;;)
      {
        // Use Case 1: block indefinitely waiting for the event from the ISR.
        if (button_event.take())
        {
          sSerial.println("[ButtonTask]: event received from the ISR!");
        }
      }
    }
};

class ResourceConsumerTask : public ThreadRTOS<Kernel::min_stack_bytes(), 1>
{
  protected:
    void Run() override
    {
      for(;;)
      {
        // Use Case 2: try to reserve a resource from the pool with a timeout —
        // if none are free within 200ms, give up and try again later.
        if (resource_pool.take(200))
        {
          if (auto s = sSerial.lockedStream())
          {
            s << "[Consumer]: resource reserved, available = " << resource_pool.get_count() << "\n";
          }

          // Simulates resource usage...
          Kernel::delay(300);

          // Return the resource to the pool.
          resource_pool.give();

          if (auto s = sSerial.lockedStream())
          {
            s << "[Consumer]: resource returned, available = " << resource_pool.get_count() << "\n";
          }
        }
        else
        {
          sSerial.println("[Consumer]: no resources available, trying again later...");
        }

        Kernel::delay(100);
      }
    }
};

class ResourceMonitorTask : public ThreadRTOS<Kernel::min_stack_bytes(), 1>
{
  protected:
    void Run() override
    {
      for(;;)
      {
        // Use Case 3: only read the pool state without reserving anything,
        // useful for telemetry/diagnostics.
        UBaseType_t free_now = resource_pool.get_count();

        if (auto s = sSerial.lockedStream())
        {
          s << "[Monitor]: free resources in the pool = " << free_now << " / " << POOL_SIZE << "\n";
        }

        Kernel::delay(1000);
      }
    }
};

ButtonTask button_task;
ResourceConsumerTask consumer_task;
ResourceMonitorTask monitor_task;

void setup()
{
  Serial.begin(115200);
  while (!Serial) { delay(1); }

  // On real hardware, you would attach the ISR to a physical pin, for example:
  // pinMode(2, INPUT_PULLUP);
  // attachInterrupt(digitalPinToInterrupt(2), onButtonPress, FALLING);

  sSerial.println("[Setup]: semaphores initialized.");

  button_task();
  consumer_task();
  monitor_task();

  Kernel::start();
}

void loop() { /* Not Used */ }
