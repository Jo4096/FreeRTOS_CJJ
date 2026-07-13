#include <FreeRTOS_CJJ.h>
#include <SafeSerial.hpp>
#include <SoftwareTimers.hpp>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

// For more info on SafeSerial check the SafeSerial_Example
SafeSerial sSerial(Serial);

// --- Periodic timer: blinks the built-in LED every 500ms, auto-reloads forever ---
class BlinkTimer : public PeriodicTimer
{
private:
  bool led_state = false;

protected:
  void on_timer() override
  {
    led_state = !led_state;
    digitalWrite(LED_BUILTIN, led_state ? HIGH : LOW);

    if (auto s = sSerial.lockedStream())
    {
      s << "[BlinkTimer] LED -> " << (led_state ? "ON" : "OFF") << "\n";
    }
  }
};

// --- One-shot timer: fires once, 3 seconds after being started ---
class WarmupDoneTimer : public OneShotTimer
{
protected:
  void on_timer() override
  {
    if (auto s = sSerial.lockedStream())
    {
      s << "[WarmupDoneTimer] Warmup finished, system ready.\n";
    }
  }
};

BlinkTimer blinkTimer;
WarmupDoneTimer warmupTimer;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    delay(1);
  }

  pinMode(LED_BUILTIN, OUTPUT);

  (void)sSerial.println("=== SoftwareTimers Example ===");

  // Starts the periodic blink, ticking every 500ms from now on
  if (blinkTimer.start(500, "BlinkTimer"))
  {
    (void)sSerial.println("BlinkTimer started (period: 500ms, auto-reload).");
  }
  else
  {
    (void)sSerial.println("Failed to start BlinkTimer!");
  }

  // Starts the one-shot warmup timer, fires only once after 3000ms
  if (warmupTimer.start(3000, "WarmupTimer"))
  {
    (void)sSerial.println("WarmupDoneTimer started (fires once after 3000ms).");
  }
  else
  {
    (void)sSerial.println("Failed to start WarmupDoneTimer!");
  }

  Kernel::start();
}

void loop()
{
  /* Not Used */
}