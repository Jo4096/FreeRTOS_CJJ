#include <FreeRTOS_CJJ.h>
#include <ThreadRTOS.hpp>
#include <Mutex.hpp>
#include <LGuards.hpp>

using namespace fcjj;

// A shared resource accessed by multiple tasks. 
// Modifying this without protection would cause a data race.
int shared_variable = 0;

// Instantiates a static Mutex container wrapper. 
// Note: On standard platforms (like ESP32/Pico), this utilizes static memory allocation.
// For Arduino Uno, it safely falls back to dynamic allocation under the hood.
Mutex shared_var_mux; 

class Task_1 : public ThreadRTOS<Kernel::min_stack_bytes(), 1>
{
  protected:
    void Run() override
    {
      // Tracks the last execution time to enforce precise periodic timing
      TickType_t lastTime = xTaskGetTickCount();
      
      for(;;)
      {
        // Enclosing scope explicitly defines the lifetime of the LockGuard (RAII pattern)
        {
          // RAII: Automatically takes the mutex here (blocking indefinitely until free)
          LockGuard lock(shared_var_mux); 
          
          shared_variable = 1;
          Serial.print("[Task_1]: shared_variable = ");
          Serial.println(shared_variable);
          
          // RAII: The 'lock' goes out of scope here, automatically releasing the mutex
        }
        
        // Accurate periodic delay; ensures the task triggers exactly every 1000ms 
        // from the last execution point, compensating for execution jitter.
        Kernel::delay_until(lastTime, 1000);
      }
    }
};

class Task_2 : public ThreadRTOS<Kernel::min_stack_bytes(), 1>
{
  protected:
    void Run() override
    {
      TickType_t lastTime = xTaskGetTickCount();
      
      for(;;)
      {
        {
          // Timeout variation: Attempts to take the mutex but will give up after 10ms max.
          LockGuard lock(shared_var_mux, 10); 
          
          // Since a timeout can fail, always check ownership before modifying the shared data
          if(lock.owns_lock())
          { 
            shared_variable = 2;
            Serial.print("[Task_2]: shared_variable = ");
            Serial.println(shared_variable);
          }
          else
          {
            // Optional: Handle the lock acquisition failure case here
          }
        }
        
        // Periodic execution every 333ms
        Kernel::delay_until(lastTime, 333);
      }
    }
};

// Instantiate the tasks
Task_1 t1_;
Task_2 t2_;

void setup()
{
  Serial.begin(115200);
  while(!Serial){ delay(1); }

  // Launch both tasks
  t1_();
  t2_();

  // Hand over control to the FreeRTOS scheduler
  Kernel::start();
}

void loop(){/* Not Used */}
