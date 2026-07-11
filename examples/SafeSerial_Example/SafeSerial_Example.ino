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
      int packet_id = 0;
      float voltage = 3.7f;
      float current = 0.42f;

      for(;;)
      {
        packet_id++;

        // Use Case 1: Standard usage using the default timeout.
        // Unambiguous: print()/println() without timeout always use default_time.
        // print()/println() are marked as [[nodiscard]] meaning that they return a value that can't be ignored
        // that value is a bool that when false it means that the printing failed (sSerial couldn't hold the mutex in time)
        // to suppress that compiler warning we can simply add a (void) before calling the function
        (void)sSerial.println("[Telemetry Task]: Processing system tasks...");

        // Use Case 2: Locked stream — multiple operations, ONE single lock/unlock.
        // Ideal for sequential printing without paying for N locks or using printf.
        //
        // NOTE: Evaluating lockedStream() inside the 'if' condition safely checks if the 
        // lock was acquired. Inside the block, you can freely call s.print()/println()/<< 
        // without worrying about individual [[nodiscard]] return warnings.
        if (auto s = sSerial.lockedStream())
        {
          s << "[Telemetry Task]: packet=" << packet_id << " voltage=" << voltage << "V current=" << current << "A\n";
          
          //or if you don't like the << operator
          /*
            s.print("[Telemetry Task]: packet=");
            s.print(packet_id);
            s.print(" voltage=");
            s.print(voltage);
            s.print("V current=");
            s.print(current);
            s.println("A");
          */
        }
        else
        {
          // Failed to acquire the mutex within the default timeout — ignore this log.
        }

        // Use Case 3: Locked stream with custom timeout + println() at the end of the block.
        if (auto s = sSerial.lockedStream(30))
        {
          s << "[Telemetry Task]: quick status = OK";
          s.println(); // extra line without the cost of a new lock
        }

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

        // Use Case 4: Explicit timeout WITHOUT casting
        // different names eliminate the overload ambiguity that previously existed with print((uint32_t)25, ...).
        if (sSerial.printTimeout(25, "[Debug Task]: Executing heartbeat loop iteration: "))
        {
          (void)sSerial.printlnTimeout(25, loop_count);
        }
        else
        {
          // Failed to log because the serial mutex couldn't be acquired within 25 ms
        }

        // Use Case 5: Group heartbeat + counter under the SAME 25ms lock
        // instead of two separate calls (printTimeout + printlnTimeout above).
        if (auto s = sSerial.lockedStream(25))
        {
          s << "[Debug Task]: heartbeat #" << loop_count << " (grouped, 1 lock)\n";
        }

        // Use Case 6: From the 10th iteration onwards, we relax the default timeout
        // of this specific task persistently (affects print()/println()
        // without explicit timeout from this point forward, across the entire program).
        if (loop_count == 10)
        {
          sSerial.setDefaultTimeout(200);
          (void)sSerial.println("[Debug Task]: default timeout relaxed to 200ms");
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

  (void)sSerial.println("[Setup]: current default timeout = ");
  // getDefaultTimeout() useful for configuration logging/diagnostics.
  (void)sSerial.println(sSerial.getDefaultTimeout());

  telemetry();
  debug_log();

  Kernel::start();
}

void loop() {  /* Not Used */}