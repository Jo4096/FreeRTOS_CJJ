#include <FreeRTOS_CJJ.h>
#include <SafeSerial.hpp>
#include <ThreadRTOS.hpp>

// Instantiates SafeSerial wrapping the standard hardware Serial interface.
// By default, it sets a 100 ms timeout window to acquire the internal mutex.
SafeSerial sSerial(Serial);

class TelemetryTask : public ThreadRTOS<Kernel::min_stack_bytes(), 1>
{
  protected:
    void Run() override
    {
      for(;;)
      {
        // Use Case 1: Standard usage using the default 100ms timeout.
        // This invokes the implicit match wrapper perfectly.
        sSerial.println("[Telemetry Task]: Processing system tasks...");

        Kernel::delay(1000);
      }
    }
};

class DebugTask : public ThreadRTOS<Kernel::min_stack_bytes(), 1>
{
  protected:
    void Run() override
    {
      int loop_count = 0;
      
      for(;;)
      {
        loop_count++;

        // Use Case 2: Explicitly overriding with a custom timeout (e.g., 25 ms).
        // We cast '25' to (uint32_t) so the compiler bypasses the general fallback 
        // template and uses the explicit timeout signature.
        if (sSerial.print((uint32_t)25, "[Debug Task]: Executing heartbeat loop iteration: "))
        {
          sSerial.println((uint32_t)25, loop_count);
        }
        else
        {
          // Failed to log because the serial mutex couldn't be acquired within 25 ms
        }

        Kernel::delay(500);
      }
    }
};

TelemetryTask telemetry;
DebugTask debug_log;

void setup()
{
  Serial.begin(115200);
  while(!Serial){ delay(1); }

  telemetry();
  debug_log();

  Kernel::start();
}

void loop() {  /* Not Used */}
