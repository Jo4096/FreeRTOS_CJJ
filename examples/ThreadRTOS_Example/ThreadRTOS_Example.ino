#include <FreeRTOS_CJJ.h>
#include <ThreadRTOS.hpp>


// This class demonstrates a One-shot task.
// Since there is no infinite loop inside Run(), the task executes once and terminates naturally.
class Oneshot : public ThreadRTOS<Kernel::min_stack_bytes(), 1>
{
protected:
  void Run() override
  {
    Kernel::delay(10000); // Wait for 10 seconds before executing
    Serial.println("[Oneshot]: This will be the only time that I will print this. Good bye world");
    
    // Once this method returns, FreeRTOS_CJJ handles the clean-up and deletes the underlying FreeRTOS task.
  }
};


// This class demonstrates a traditional, continuous FreeRTOS task.
// Unlike ThreadArduino, ThreadRTOS uses a single Run() method where you handle initialization 
// and implement your own infinite loop.
class BlinkTask : public ThreadRTOS<Kernel::min_stack_bytes(), 1>
{

private:
  uint8_t pin = 0;
public:
  BlinkTask(uint8_t pin)
  {
    this->pin = pin;
  }
protected:
  void Run() override
  {
    // Task-specific initialization (Runs only once when the task starts)
    pinMode(pin, OUTPUT);

    // Infinite loop required to keep the task alive
    for(;;)
    {
      digitalWrite(pin, !digitalRead(pin));
      Kernel::delay(1000); // Yields control to the scheduler for 1000ms
    }
  }
};


// This class monitors the execution state of other tasks dynamically.
class CheckRunning : public ThreadRTOS<Kernel::min_stack_bytes(), 1>
{
private:
  Oneshot* t_1;
  BlinkTask* t_2;

public:
  // Constructor passes pointers to the monitored tasks
  CheckRunning(Oneshot* t_1, BlinkTask* t_2) : t_1(t_1), t_2(t_2) {}

protected:
  void Run() override
  {
    for(;;)
    {
      // IsRunning() returns true if the FreeRTOS task is active, 
      // and false if it has finished executing or hasn't started yet.
      if(t_1 && t_1->IsRunning()){ Serial.println("[CheckRunning]: Oneshot is running"); }
      else { Serial.println("[CheckRunning]: Oneshot is not running"); }

      if(t_2 && t_2->IsRunning()){ Serial.println("[CheckRunning]: BlinkTask is running"); }
      else { Serial.println("[CheckRunning]: BlinkTask is not running"); }

      Kernel::delay(6000); // Check the status every 6 seconds
    }
  }
};

// Fallback for ESP32 or boards that do not natively define LED_BUILTIN
#ifndef LED_BUILTIN
  #define LED_BUILTIN 2
#endif

// Instantiate the tasks globally
Oneshot one_shot;
BlinkTask blink1(LED_BUILTIN);
CheckRunning check(&one_shot, &blink1); // Pass addresses of the tasks to the monitor



void setup()
{
  Serial.begin(115200);
  while(!Serial){ delay(1); }

  // Start all tasks using the operator() overload
  one_shot();
  blink1();
  check();
  
  // Start the FreeRTOS scheduler
  Kernel::start();
}
void loop() {/* Not Used */}