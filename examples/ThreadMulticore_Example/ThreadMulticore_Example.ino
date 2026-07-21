#define FREERTOS_CJJ_USE_THREADS
#include <FreeRTOS_CJJ.h>
#include <SafeSerial.hpp>

using namespace fcjj;

// For more info on SafeSerial check the SafeSerial_Example
SafeSerial sSerial(Serial);


// Helper function just to check the current core
static inline int current_core_id()
{
#if defined(ESP32)
    return xPortGetCoreID();
#elif defined(ARDUINO_ARCH_RP2040) || defined(TARGET_RP2040)
    return get_core_num(); // from pico-sdk
#else
    return 0; // AVR: single-core
#endif
}

class PinnedWorker : public ThreadArduino<Kernel::min_stack_bytes(), 1>
{
private:
  const char *label;

public:
  PinnedWorker(const char *lbl) : label(lbl) {}

protected:
  void loop() override
  {
    if (auto s = sSerial.lockedStream())
    {
      s << label << " -> running on core " << current_core_id() << "\n";
    }
    Kernel::delay(1000);
  }
};

class BlinkTask : public ThreadArduino<Kernel::min_stack_bytes(), 1>
{
  private:
    uint8_t pin = 0;
  public:
    BlinkTask(uint8_t pin)
    {
      this->pin = pin;
    }

  protected:
    void setup() override
    {
      pinMode(pin, OUTPUT);
    }

    void loop() override
    {
      digitalWrite(pin, !digitalRead(pin));
      if (auto s = sSerial.lockedStream())
      {
        s <<"Blink -> running on core " << current_core_id() << "\n";
      }
      Kernel::delay(1000);
    }
};



PinnedWorker workerCore0("Worker A (requested: CORE_0)");
PinnedWorker workerCore1("Worker B (requested: CORE_1)");
PinnedWorker workerCoreAny("Worker C (requested: CORE_ANY)");

#ifndef LED_BUILTIN
  #define LED_BUILTIN 2
#endif
BlinkTask blink(LED_BUILTIN);

void setup()
{
  Serial.begin(115200);
  while(!Serial){delay(1);}

  // SafeSerial::println is marked [[nodiscard]]. 
  // Casting to (void) acknowledges the return state and avoids a compiler warning.
  (void)sSerial.println("\n\n=== Multicore Task Pinning Test ===");

  // Launch tasks using the function call operator override.
  // The architecture handles core affinity parameters seamlessly behind the scenes:
  // - ESP32 uses: xTaskCreateStaticPinnedToCore(..., affinity)
  // - RP2040 creates the static task first, then evaluates the bitmask via vTaskCoreAffinitySet()
  // - AVR discards the parameter safely and falls back to a standard dynamic xTaskCreate()
  blink("blink");
  workerCore0("WorkerA", CORE_0);
  workerCore1("WorkerB", CORE_1);
  
  // CORE_ANY leaves the task unrestricted:
  // the scheduler is free to run it on whichever core is available at any given moment, and may move it between cores across its lifetime.
  workerCoreAny("WorkerC", CORE_ANY);

  Kernel::start();
}

void loop() 
{
  // Unlike the other examples where loop() is left completely empty, 
  // we must include a delay here to prevent CPU starvation on multi-core platforms.
  // The background Arduino loop task often runs at a high priority; if left unblocked, 
  // it can fully monopolize its assigned core and completely block our pinned workers 
  // (such as WorkerA on the RP2040) from ever executing.
  Kernel::delay(1000);
}
