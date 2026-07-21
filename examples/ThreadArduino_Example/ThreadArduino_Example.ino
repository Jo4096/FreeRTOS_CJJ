#include <FreeRTOS_CJJ.h>
#include <ThreadArduino.hpp>

using namespace fcjj;

// The first template parameter defines the stack size in Bytes. 
// Standard FreeRTOS uses Words or Bytes depending on the architecture (e.g., Words on Pico, Bytes on ESP32).
// FreeRTOS_CJJ abstracts this to Bytes across all platforms to prevent confusion.
constexpr uint32_t stack_size_in_bytes = Kernel::min_stack_bytes();

// The second template parameter defines the task priority (standard FreeRTOS numbering).
constexpr UBaseType_t priority = 1; 

class BlinkTask : public ThreadArduino<stack_size_in_bytes, priority>
{
  private:
    uint8_t pin = 0;
  public:
    BlinkTask(uint8_t pin)
    {
      this->pin = pin;
    }

  protected:
    // Optional: Overriding setup() allows task-specific initialization. 
    // If omitted, it defaults to an empty implementation.
    void setup() override
    {
      pinMode(pin, OUTPUT);
    }

    // This is your task's continuous execution loop.
    void loop() override
    {
      digitalWrite(pin, !digitalRead(pin));

      // Delays for 1000ms. Non-blocking; automatically maps to vTaskDelay(pdMS_TO_TICKS(ms)).
      Kernel::delay(1000);
    }
};

class HelloWorld : public ThreadArduino<stack_size_in_bytes, priority>
{
private:
  int i = 0;

protected:
  void loop() override
  {
    Serial.print("Hello World! ");
    Serial.println(i);
    i++;
    Kernel::delay(5000);
  }  
};



// Fallback for ESP32 or boards that do not natively define LED_BUILTIN
#ifndef LED_BUILTIN
  #define LED_BUILTIN 2
#endif

// Instantiate the tasks
BlinkTask blink1(LED_BUILTIN);
// BlinkTask blink2(ANOTHER_PIN); // To scale, simply declare another instance of the class.

HelloWorld hello;

void setup()
{
  Serial.begin(115200);
  while(!Serial) { delay(1); } // Wait for the serial port to connect (required for native USB boards)

  // The operator() overload starts the task. 
  // Passing a string sets the FreeRTOS task name, which is highly useful for debugging (Defaults to "Task").
  blink1("blink1"); 
  // blink2("blink2"); // Starts the second independent blinking task.
  
  hello(); // Starts the HelloWorld task using its default name.

  // Starts the FreeRTOS scheduler. 
  // Safe to call even if the scheduler is already running, as it performs an internal state check first.
  Kernel::start();  
}

void loop(){ /* Not used */}