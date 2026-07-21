#define FREERTOS_CJJ_USE_THREADS
#include <FreeRTOS_CJJ.h>
#include <SafeRingBuffer.hpp>
#include <SafeSerial.hpp>

using namespace fcjj;

// For more info on SafeSerial check the SafeSerial_Example
SafeSerial sSerial(Serial);

struct Reading
{
  uint32_t timestamp_ms;
  float value;
};

// Shared buffer: last 5 sensor readings, visible to all tasks below.
// When full, new pushes evict the OLDEST reading (Overwrite::Front),
// so it always holds the 5 most recent samples.
SafeRingBuffer<Reading, 5> readingsBuffer;

// --- Producer: simulates a sensor, pushes a new reading every 300ms ---
class SensorProducer : public ThreadArduino<Kernel::min_stack_bytes(), 1>
{
protected:
  void loop() override
  {
    Reading r{Kernel::uptime_ms(), static_cast<float>(analogRead(A0))};

    // Buffer never blocks: if full, evict the oldest to make room.
    if (!readingsBuffer.push_back(r, Overwrite::Front))
    {
      if (auto s = sSerial.lockedStream())
        s << "[Producer] push failed (unexpected)\n";
    }

    Kernel::delay(300);
  }
};

// --- Printer: every 1s, dumps the whole buffer using for_each (enumerate-style) ---
class HistoryPrinter : public ThreadArduino<4096, 1>
{
protected:
  void loop() override
  {
    // Snapshot first (fast copy inside the critical section)...
    Reading snapshot[5];
    size_t n = 0;
    readingsBuffer.for_each([&](size_t /*index*/, const Reading &r)
    {
      if (n < 5)
      {
        snapshot[n++] = r;
      }
    });

    // then print AFTER the lock, safe to use SafeSerial here.
    if (auto s = sSerial.lockedStream())
    {
      s << "--- Last " << n << " readings ---\n";
      for (size_t i = 0; i < n; ++i)
      {
        s << i << ": t=" << snapshot[i].timestamp_ms << "ms value=" << snapshot[i].value << "\n";
      }
    }

    Kernel::delay(1000);
  }
};

// --- SerialConsumer: type "pop" in the Serial Monitor to consume the most recent reading ---
class SerialConsumer : public ThreadArduino<4096, 1>
{
protected:
  void loop() override
  {
    if (Serial.available())
    {
      String cmd = Serial.readStringUntil('\n');
      cmd.trim();

      if (cmd == "pop")
      {
        Reading r;
        if (readingsBuffer.pop_back(r))
        {
          if (auto s = sSerial.lockedStream())
            s << "[Consumer] consumed reading: value=" << r.value << "\n";
        }
        else
        {
          (void)sSerial.println("[Consumer] buffer empty, nothing to consume.");
        }
      }
      else if (cmd == "clear")
      {
        readingsBuffer.clear();
        (void)sSerial.println("[Consumer] buffer cleared.");
      }
    }

    Kernel::delay(50);
  }
};

SensorProducer producer;
HistoryPrinter printer;
SerialConsumer consumer;

void setup()
{
  Serial.begin(115200);
  while (!Serial) { delay(1); }

  (void)sSerial.println("=== SafeRingBuffer Example ===");
  (void)sSerial.println("Type 'pop' to consume the latest reading, 'clear' to empty the buffer.");

  producer("Producer");
  printer("Printer");
  consumer("Consumer");
}

void loop()
{
  Kernel::delay(1000);
}